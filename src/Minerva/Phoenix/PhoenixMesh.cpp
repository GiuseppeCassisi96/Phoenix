#include "PhoenixMesh.h"
#include <iostream>
#include <random>
#include <chrono>
#include "glm/gtx/norm.hpp"

/*Merge and Group methods are based on: https://jglrxavpok.github.io/2024/01/19/recreating-nanite-lod-generation.html
post*/

namespace Phoenix
{
    void PhoenixMesh::BuildLodsHierarchy(std::vector<MINERVA_VERTEX>& vertices, std::vector<uint32_t> &indices)
    {
        //LOD 0 creation
        LOD firstLod;
        firstLod.lodVertexBuffer = vertices;
        firstLod.lodIndexBuffer = indices;
        Split(firstLod, firstLod.lodIndexBuffer);
        Group(firstLod);
        lods.emplace_back(firstLod);
        int lodIndex = 0;
        

        while (lodIndex < MAX_LOD_NUMBER)
        {
            //Next LODs creation
            LOD* prevLod = &lods[lods.size() - 1];
            LOD currentLod;
            currentLod.lod = lodIndex + 1;
            currentLod.lodVertexBuffer = prevLod->lodVertexBuffer;
            std::vector<uint32_t> groupIndexBuffer;
            float totalError = 0.0f;
            for(auto& group : prevLod->groups) 
            {
                float outError = 0.0f;
                Merge(group,*prevLod,group.localGroupIndexBuffer);
                groupIndexBuffer = group.localGroupIndexBuffer;
                assert(groupIndexBuffer.size() % 3 == 0);
                auto simplifiedCount = Simplify(groupIndexBuffer, currentLod, outError);
                if(simplifiedCount > 0)
                    group.nextMeshlets = Split(currentLod, groupIndexBuffer);
                totalError += outError;
            } 
            lodIndex++;
            currentLod.lod = lodIndex;
            Group(currentLod);
            currentLod.lodError = (totalError / currentLod.groups.size()) + prevLod->lodError;
            lods.emplace_back(currentLod);
        }
        for(auto& group : lods[lods.size() - 1].groups) 
        {
            Merge(group,lods[lods.size() - 1],group.localGroupIndexBuffer);
        }
        std::cout << "pippo\n";
    }

    std::vector<MeshletGroup> PhoenixMesh::Group(LOD& currentLod)
    {  

        auto groupWithAllMeshlets = [&]() 
        {
            MeshletGroup group;
            for (int i = 0; i < currentLod.lodVerticesMeshlets.size(); ++i) 
            {
                group.meshlets.push_back(i);
            }
            return std::vector { group };
        };

        if(currentLod.lodVerticesMeshlets.size() < 8) 
        {
            return groupWithAllMeshlets();
        }


        //I use set and unordered map to avoid duplicate edges
        std::unordered_map<MeshletEdge, std::unordered_set<size_t>, MeshletEdgeHasher> edges2Meshlets;
        std::unordered_map<size_t, std::unordered_set<MeshletEdge, MeshletEdgeHasher>> meshlets2Edges;
        for(size_t meshletIndex = 0; meshletIndex < currentLod.lodVerticesMeshlets.size(); meshletIndex++) 
        {
            const auto& meshlet = currentLod.lodVerticesMeshlets[meshletIndex];
            
            auto getVertexIndex = [&](size_t index) 
            {
                size_t indexVertex = currentLod.lodMeshletsClusterIndex[currentLod.lodMeshletsClusterTriangle
                [index + meshlet.triangle_offset] + meshlet.vertex_offset];
                return indexVertex;
            };

            const size_t triangleCount = meshlet.triangle_count;
            // for each triangle of the meshlet
            for(size_t triangleIndex = 0; triangleIndex < triangleCount; triangleIndex++) 
            {
                // for each edge of the triangle
                for(size_t i = 0; i < 3; i++) 
                {
                    MeshletEdge edge { getVertexIndex(i + triangleIndex * 3), 
                    getVertexIndex(((i+1) % 3) + triangleIndex * 3) };
                    if(edge.first != edge.second) 
                    {
                        edges2Meshlets[edge].insert(meshletIndex);
                        meshlets2Edges[meshletIndex].insert(edge);
                    }
                }
            }
        }

        std::erase_if(edges2Meshlets, [&](const auto& pair) 
        {
            return pair.second.size() <= 1;
        });

        if(edges2Meshlets.empty()) 
        {
            return groupWithAllMeshlets();
        }
        
        // vertex count, from the point of view of METIS, where Meshlet = graph vertex
        idx_t vertexCount = static_cast<idx_t>(currentLod.lodVerticesMeshlets.size());
        // only one constraint, minimum required by METIS
        idx_t ncon = 1; 
        // groups of MAX_GROUP_NUMBER 
        idx_t nparts = static_cast<idx_t>(currentLod.lodVerticesMeshlets.size() / MAX_GROUP_NUMBER); 
        assert(nparts > 1);
        idx_t options[METIS_NOPTIONS];
        METIS_SetDefaultOptions(options);
        options[METIS_OPTION_OBJTYPE] = METIS_OBJTYPE_CUT;
         // identify connected components first
        options[METIS_OPTION_CCORDER] = 1;
        options[METIS_OPTION_NUMBERING] = 0;
        std::vector<idx_t> partition;
        partition.resize(vertexCount);

        // xadj
        std::vector<idx_t> xadjacency;
        xadjacency.reserve(vertexCount + 1);

        // adjncy
        std::vector<idx_t> edgeAdjacency;
        // weight of each edge
        std::vector<idx_t> edgeWeights;

        for(size_t meshletIndex = 0; meshletIndex < currentLod.lodVerticesMeshlets.size(); meshletIndex++) 
        {
            size_t startIndexInEdgeAdjacency = edgeAdjacency.size();
            for(const auto& edge : meshlets2Edges[meshletIndex]) 
            {
                auto connectionsIter = edges2Meshlets.find(edge);
                if(connectionsIter == edges2Meshlets.end()) 
                {
                    continue;
                }
                const auto& connections = connectionsIter->second;
                for(const auto& connectedMeshlet : connections) 
                {
                    if(connectedMeshlet != meshletIndex) 
                    {
                        auto existingEdgeIter = std::find(edgeAdjacency.begin()+startIndexInEdgeAdjacency, 
                        edgeAdjacency.end(), connectedMeshlet);
                        if(existingEdgeIter == edgeAdjacency.end()) 
                        {
                            // first time we see this connection to the other meshlet
                            edgeAdjacency.emplace_back(connectedMeshlet);
                            edgeWeights.emplace_back(1);
                        } 
                        else 
                        {
                            // not the first time! increase number of times we encountered this meshlet
                            std::ptrdiff_t d = std::distance(edgeAdjacency.begin(), existingEdgeIter);
                            assert(d >= 0);
                            assert(d < edgeWeights.size());
                            edgeWeights[d]++;
                        }
                    }
                }
            }
            xadjacency.push_back(static_cast<idx_t>(startIndexInEdgeAdjacency));
        }
        xadjacency.push_back(static_cast<idx_t>(edgeAdjacency.size()));
        assert(xadjacency.size() == currentLod.lodVerticesMeshlets.size() + 1);
        assert(edgeAdjacency.size() == edgeWeights.size());
        idx_t edgeCut; // final cost of the cut found by METIS
        int result = METIS_PartGraphKway(&vertexCount,
                                            &ncon,
                                            xadjacency.data(),
                                            edgeAdjacency.data(),
                                            nullptr, 
                                            nullptr, 
                                            edgeWeights.data(),
                                            &nparts,
                                            nullptr,
                                            nullptr,
                                            options,
                                            &edgeCut,
                                            partition.data()
                            );
        assert(result == METIS_OK);
        currentLod.groups.resize(nparts);
        for(size_t i = 0; i < currentLod.lodVerticesMeshlets.size(); i++) 
        {
          idx_t partitionNumber = partition[i];
          currentLod.groups[partitionNumber].meshlets.push_back(i);
          currentLod.meshletToGroup.insert({i, partitionNumber});
        }
        return currentLod.groups;
    }

    void PhoenixMesh::Merge(const MeshletGroup& group, const LOD& currentLod, 
    std::vector<uint32_t>& localGroupIndexBuffer)
    {
      
       for(const auto& meshletIndex : group.meshlets) 
       {
            const auto& meshlet = currentLod.lodVerticesMeshlets[meshletIndex];
            size_t start = localGroupIndexBuffer.size();
            localGroupIndexBuffer.resize(start + meshlet.triangle_count * 3);
            for(size_t j = 0; j < meshlet.triangle_count * 3; j++) 
            {
                uint32_t index = currentLod.lodMeshletsClusterIndex[currentLod.lodMeshletsClusterTriangle
                [meshlet.triangle_offset + j] + meshlet.vertex_offset];
                localGroupIndexBuffer[j + start] = index;
            }
        }
    }

    size_t PhoenixMesh::Simplify(std::vector<uint32_t>& localGroupIndexBuffer, LOD& currentLod,
    float& outError)
    {
        float targetError = 0.05f; 
        // I force the simplifier to simplify more
        float threshold =  0.5f;
        size_t targetIndexCount = localGroupIndexBuffer.size() * threshold;

        auto simplifiedSize = meshopt_simplify(localGroupIndexBuffer.data(), localGroupIndexBuffer.data(), 
        localGroupIndexBuffer.size(), &currentLod.lodVertexBuffer[0].pos.x,currentLod.lodVertexBuffer.size(),
        sizeof(Minerva::Mesh::Vertex),targetIndexCount, targetError, meshopt_SimplifyLockBorder,&outError);
        
        localGroupIndexBuffer.resize(simplifiedSize);
        
        currentLod.lodIndexBuffer.insert(currentLod.lodIndexBuffer.end(), localGroupIndexBuffer.begin(), 
        localGroupIndexBuffer.end());
        //Fill the lodIndexBuffer
        return simplifiedSize;
    }



    std::vector<size_t> PhoenixMesh::Split(LOD& currentLod, std::vector<uint32_t> indexBuffer)
    {
        const float cone_weight = 0.0f;

        size_t max_meshlets = meshopt_buildMeshletsBound(indexBuffer.size(), MESHLET_VERTICES_NUMBER,
        MESHLET_TRIANGLE_NUMBER);
        
        std::vector<meshopt_Meshlet> localmeshlets;
        std::vector<uint32_t> localMeshletsClusterIndex;
        std::vector<unsigned char> localMeshletsClusterTriangle;
        localmeshlets.resize(max_meshlets);
        localMeshletsClusterIndex.resize(max_meshlets * MESHLET_VERTICES_NUMBER);
        localMeshletsClusterTriangle.resize(max_meshlets * MESHLET_TRIANGLE_NUMBER * 3);

        size_t vertexArrayOffset = currentLod.lodMeshletsClusterIndex.size();
        size_t triangleArrayOffset = currentLod.lodMeshletsClusterTriangle.size();
        size_t meshletArrayOffset  = currentLod.lodVerticesMeshlets.size();

        size_t meshletCount = meshopt_buildMeshlets(localmeshlets.data(),localMeshletsClusterIndex.data(), 
        localMeshletsClusterTriangle.data(), indexBuffer.data(), indexBuffer.size(),&currentLod.lodVertexBuffer[0].pos.x,
        currentLod.lodVertexBuffer.size(),sizeof(Minerva::Mesh::Vertex), MESHLET_VERTICES_NUMBER,
        MESHLET_TRIANGLE_NUMBER, cone_weight);
        
        const meshopt_Meshlet& last = localmeshlets[meshletCount - 1];
        localMeshletsClusterIndex.resize(last.vertex_offset  + last.vertex_count);
        localMeshletsClusterTriangle.resize(last.triangle_offset  + ((last.triangle_count * 3 + 3) & ~3));
        localmeshlets.resize(meshletCount);

        size_t value = currentLod.lodVerticesMeshlets.size();
        currentLod.lodMeshletsClusterIndex.resize(vertexArrayOffset + (last.vertex_offset  + last.vertex_count));
        currentLod.lodMeshletsClusterTriangle.resize(triangleArrayOffset + (last.triangle_offset  + 
        ((last.triangle_count * 3 + 3) & ~3)));
        currentLod.lodVerticesMeshlets.resize(meshletArrayOffset + meshletCount);
        std::vector<size_t> nextMeshlets;

        //Fill the phoenixMeshlets
        for(size_t i = meshletArrayOffset; i < localmeshlets.size() + meshletArrayOffset; i++)
        {
            meshopt_Meshlet currentMeshlet = localmeshlets[i - meshletArrayOffset];
            if(currentLod.lod != 0)
            {
                currentMeshlet.vertex_offset = static_cast<uint32_t>(vertexArrayOffset) + currentMeshlet.vertex_offset;
                currentMeshlet.triangle_offset = static_cast<uint32_t>(triangleArrayOffset) + currentMeshlet.triangle_offset;
            }
            localmeshlets[i - meshletArrayOffset] = currentMeshlet;
            currentLod.lodVerticesMeshlets[i] = localmeshlets[i - meshletArrayOffset];
            nextMeshlets.emplace_back(value);
            value++;
        }

        //Fill the lodMeshletsClusterIndex
        for(size_t i = vertexArrayOffset; i < localMeshletsClusterIndex.size() + vertexArrayOffset; i++)
        {
            currentLod.lodMeshletsClusterIndex[i] = localMeshletsClusterIndex[i - vertexArrayOffset];
        }

        //Fill the lodMeshletsClusterTriangle
        for(size_t i = triangleArrayOffset; i < localMeshletsClusterTriangle.size() +
        triangleArrayOffset; i++)
        {
            currentLod.lodMeshletsClusterTriangle[i] = localMeshletsClusterTriangle
            [i - triangleArrayOffset];
        }
        return nextMeshlets;
    }
   
    void PhoenixMesh::ColourMeshelets(MeshletGroup& group, std::vector<MINERVA_VERTEX> &vertices)
    {        
        std::random_device rd;
        std::mt19937 gen(rd()); 
 
        float min = 0.15f; 
        float max = 1.0f; 
        std::uniform_real_distribution<float> dist(min, max);
        float r = dist(gen);
        float g = dist(gen);
        float b = dist(gen);
        for(int i = 0; i < group.localGroupIndexBuffer.size(); i++)
        {
            vertices[group.localGroupIndexBuffer[i]].color = glm::vec3(r, g, b);
        }
    }

    

   
}

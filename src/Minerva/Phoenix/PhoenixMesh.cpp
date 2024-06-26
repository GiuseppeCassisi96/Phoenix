#include "PhoenixMesh.h"
#include <iostream>
#include <random>
#include <chrono>
#include "glm/gtx/norm.hpp"
#include <numeric>


/*Merge and Group methods are based on: https://jglrxavpok.github.io/2024/01/19/recreating-nanite-lod-generation.html
post*/

namespace Phoenix
{
    void PhoenixMesh::BuildLodsHierarchy(std::vector<MINERVA_VERTEX>& vertices, std::vector<uint32_t> &indices)
    {
        //LOD 0 creation
        LOD firstLod;
        firstLod.lodVertexBuffer = vertices;
        firstLod.lod = 0;
        Split(firstLod, indices);
        Group(firstLod);
        
        
        lods.emplace_back(firstLod);
        int lodIndex = 1;
        float maxChildrenError = 0.0f;

        while (lodIndex < MAX_LOD_NUMBER)
        {
            //Next LODs creation
            LOD* prevLod = &lods[lodIndex - 1];
            LOD currentLod;
            currentLod.lod = lodIndex;
            currentLod.lodVertexBuffer = prevLod->lodVertexBuffer;
            uniqueIndex.clear();
            for(int i = 0; i < prevLod->groups.size(); i++) 
            {
                std::vector<uint32_t> groupIndexBuffer;
                MeshletGroup* group = &prevLod->groups[i];
                float outError = 0.0f;

                /*In this case the merge operation fills the local Index Buffers of the
                previous lod groups*/
                Merge(*group,*prevLod,groupIndexBuffer, uniqueIndex); 
                auto simplifiedCount = Simplify(groupIndexBuffer,vertices, currentLod, outError);
                Split(currentLod, groupIndexBuffer,outError,prevLod, group, maxChildrenError); 
            }
            for(const auto& pMeshlet : prevLod->lodVerticesMeshlets)
            {
                totalMeshlets.emplace_back(pMeshlet);
            }
            if(currentLod.lodVerticesMeshlets.size() < 1)
            {
                break;
            }
            maxChildrenError = 0.0f;
            //I compute the max children error for the next LOD computation
            for(const auto& pMeshlet : currentLod.lodVerticesMeshlets)
            {
                maxChildrenError = glm::max(maxChildrenError, pMeshlet.error);
            }
            Group(currentLod, prevLod);
            lods.emplace_back(currentLod);
            lodIndex++;
        }

        LOD* lastLod =  &lods[lods.size() - 1];
        
        uniqueIndex.clear();
        for(auto& group : lastLod->groups) 
        {
            std::vector<uint32_t> groupIndexBuffer;            
            Merge(group,*lastLod,groupIndexBuffer, uniqueIndex);
        }

        for(const auto& pMeshlet : lastLod->lodVerticesMeshlets)
        {
            totalMeshlets.emplace_back(pMeshlet);
        }
        std::cout << "\n";
    }

 
    std::vector<MeshletGroup> PhoenixMesh::Group(LOD& currentLod, LOD* prevLod)
    {
        auto groupWithAllMeshlets = [&]() 
        {
            MeshletGroup group;
            for (int i = 0; i < currentLod.lodVerticesMeshlets.size(); ++i) 
            {
                group.meshlets.insert(i);
            }
            currentLod.groups.emplace_back(group);
            return currentLod.groups;
        };

        if(currentLod.lodVerticesMeshlets.size() < 8) 
        {
            return groupWithAllMeshlets();
        }


        //I use set to avoid duplicate edges
        std::unordered_map<Edge, std::unordered_set<size_t>, EdgeHasher> edges2Meshlets;
        std::unordered_map<size_t, std::unordered_set<Edge, EdgeHasher>> meshlets2Edges;
        for(size_t meshletIndex = 0; meshletIndex < currentLod.lodVerticesMeshlets.size(); meshletIndex++) 
        {
            const auto& meshlet = currentLod.lodVerticesMeshlets[meshletIndex].meshletData;
            
            auto getVertexIndex = [&](size_t index) 
            {
                size_t indexVertex = currentLod.lodMeshletsClusterIndex[currentLod.lodMeshletsClusterTriangle
                [index + meshlet.triangle_offset] + meshlet.vertex_offset];
                return indexVertex;
            };

            // for each triangle of the meshlet
            for(size_t triangleIndex = 0; triangleIndex < meshlet.triangle_count; triangleIndex++) 
            {
                // for each edge of the triangle
                for(size_t i = 0; i < 3; i++) 
                {
                    Edge edge { getVertexIndex(i + triangleIndex * 3), 
                    getVertexIndex(((i+1) % 3) + triangleIndex * 3) };
                    if(edge.firstVertex != edge.secondVertex) 
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
        idx_t nparts = static_cast<idx_t>(currentLod.lodVerticesMeshlets.size() / MAX_GROUP_NUMBER);
        
        idx_t options[METIS_NOPTIONS];
        METIS_SetDefaultOptions(options);
        options[METIS_OPTION_OBJTYPE] = METIS_OBJTYPE_CUT;
         // identify connected components first
        options[METIS_OPTION_CCORDER] = 1;
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
            PhoenixMeshlet currentMeshlet = currentLod.lodVerticesMeshlets[meshletIndex];
            size_t startIndexInEdgeAdjacency = edgeAdjacency.size();
            for(const Edge& edge : meshlets2Edges[meshletIndex]) 
            {
                std::unordered_set<size_t> connectedMeshlets = edges2Meshlets[edge];
                for(const size_t& connectedMeshlet : connectedMeshlets) 
                {
                    if(connectedMeshlet == meshletIndex)
                        continue;
                    auto existingEdgeIter = std::find(edgeAdjacency.begin() + startIndexInEdgeAdjacency, 
                    edgeAdjacency.end(), connectedMeshlet);
                    if(existingEdgeIter == edgeAdjacency.end()) //Not find
                    {
                        //first time we see this connection to the other meshlet
                        edgeAdjacency.emplace_back(connectedMeshlet);
                        edgeWeights.emplace_back(1);
                    } 
                    else 
                    {
                        // not the first time! increase number of times we encountered this meshlet
                        //std::distance returns the number of jumps from first to last.
                        ptrdiff_t d = std::distance(edgeAdjacency.begin(), existingEdgeIter);
                        assert(d >= 0);
                        assert(d < edgeWeights.size());
                        edgeWeights[d]++;
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
        int count = 0;
        for(size_t i = 0; i < currentLod.lodVerticesMeshlets.size(); i++) 
        {
            currentLod.groups[partition[i]].meshlets.insert(i);                
        }

        //Remove group with zero meshlets
        for (int i = 0; i < currentLod.groups.size(); i++)
        {
            MeshletGroup group = currentLod.groups[i];
            if(group.meshlets.size() == 0)
            {
                currentLod.groups.erase(currentLod.groups.begin() + i);
            }
        }

        return currentLod.groups;
    }

    void PhoenixMesh::Merge(const MeshletGroup& group, LOD& prevLod, std::vector<uint32_t>& groupIndexBuffer, 
    std::unordered_set<uint32_t>& uniqueIndex)
    {
        
        std::vector<glm::vec3> meshletCenters;
        for(const auto& meshletIndex : group.meshlets) 
        {
            glm::vec3 meshletCenter {0.0f};
            PhoenixMeshlet* meshlet = &prevLod.lodVerticesMeshlets[meshletIndex];
            SetColor(*meshlet);
            
            for(size_t j = 0; j < meshlet->meshletData.triangle_count * 3; ++j) 
            {
                uint32_t index = prevLod.lodMeshletsClusterIndex[prevLod.lodMeshletsClusterTriangle
                [meshlet->meshletData.triangle_offset + j] + meshlet->meshletData.vertex_offset];
                if(uniqueIndex.insert(index).second)
                {
                    meshlet->meshletVertexBuffer.emplace_back(prevLod.lodVertexBuffer[index]);
                }
                meshlet->meshletIndexBuffer.emplace_back(index);
                groupIndexBuffer.emplace_back(index);    
            }  
        }
          
    }

    size_t PhoenixMesh::Simplify(std::vector<uint32_t>& groupIndexBuffer, 
    const std::vector<MINERVA_VERTEX>& groupVertexBuffer, 
    LOD& currentLod,
    float& outError)
    {
        float interValue = currentLod.lod / MAX_LOD_NUMBER;
        float targetError = 0.99f * interValue + 0.01f * (1.0f - interValue); 
        float threshold =  0.5f;

        targetError *= meshopt_simplifyScale(&groupVertexBuffer[0].pos.x,
        groupVertexBuffer.size(), sizeof(MINERVA_VERTEX));
        
        size_t targetIndexCount = groupIndexBuffer.size() * threshold;
        unsigned int options = meshopt_SimplifySparse | meshopt_SimplifyLockBorder 
        | meshopt_SimplifyErrorAbsolute;

        auto simplifiedSize = meshopt_simplify(groupIndexBuffer.data(), groupIndexBuffer.data(), 
        groupIndexBuffer.size(), &currentLod.lodVertexBuffer[0].pos.x, currentLod.lodVertexBuffer.size(),
        sizeof(Minerva::Mesh::Vertex),targetIndexCount, targetError, options ,&outError);
        
        groupIndexBuffer.resize(simplifiedSize);

        
        
        return simplifiedSize;
    }



    void PhoenixMesh::Split(LOD& currentLod, std::vector<uint32_t> groupIndexBuffer, float error, LOD* prevLod, 
    MeshletGroup* group, float& maxChildrenError)
    {
        const float cone_weight = 0.0f;
        
        std::unordered_set<uint32_t> uniqueIndex;
        std::vector<MINERVA_VERTEX> groupVertexBuffer;
        for(auto index : groupIndexBuffer)
        {
            if(uniqueIndex.insert(index).second)
            {
                groupVertexBuffer.emplace_back(currentLod.lodVertexBuffer[index]);
            }
        }

        size_t max_meshlets = meshopt_buildMeshletsBound(groupIndexBuffer.size(), MESHLET_VERTICES_NUMBER,
        MESHLET_TRIANGLE_NUMBER); 
        
        std::vector<meshopt_Meshlet> localmeshlets;
        std::vector<uint32_t> localMeshletsClusterIndex;
        std::vector<unsigned char> localMeshletsClusterTriangle;
        localmeshlets.resize(max_meshlets);
        localMeshletsClusterIndex.resize(max_meshlets * MESHLET_VERTICES_NUMBER);
        localMeshletsClusterTriangle.resize(max_meshlets * MESHLET_TRIANGLE_NUMBER * 3);

        size_t vertexArrayOffset = currentLod.lodMeshletsClusterIndex.size();
        size_t triangleArrayOffset = currentLod.lodMeshletsClusterTriangle.size();
        //I set the offset of global meshlet array
        size_t meshletArrayOffset  = currentLod.lodVerticesMeshlets.size();

        size_t meshletCount = meshopt_buildMeshlets(localmeshlets.data(),localMeshletsClusterIndex.data(), 
        localMeshletsClusterTriangle.data(), groupIndexBuffer.data(), groupIndexBuffer.size(),&currentLod.lodVertexBuffer[0].pos.x,
        currentLod.lodVertexBuffer.size(),sizeof(Minerva::Mesh::Vertex), MESHLET_VERTICES_NUMBER,
        MESHLET_TRIANGLE_NUMBER, cone_weight);
        
        const meshopt_Meshlet& last = localmeshlets[meshletCount - 1];
        localMeshletsClusterIndex.resize(last.vertex_offset  + last.vertex_count);
        localMeshletsClusterTriangle.resize(last.triangle_offset  + ((last.triangle_count * 3 + 3) & ~3));
        localmeshlets.resize(meshletCount);

        currentLod.lodMeshletsClusterIndex.resize(vertexArrayOffset + localMeshletsClusterIndex.size());
        currentLod.lodMeshletsClusterTriangle.resize(triangleArrayOffset + localMeshletsClusterTriangle.size());

        //I resize the lodVerticesMeshlets
        currentLod.lodVerticesMeshlets.resize(meshletArrayOffset + localmeshlets.size());

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

        std::vector<glm::vec3> meshletCenters;
        //Fill the phoenixMeshlets
        for(size_t i = meshletArrayOffset; i < localmeshlets.size() + meshletArrayOffset; i++)
        {
            meshopt_Meshlet currentMeshlet = localmeshlets[i - meshletArrayOffset];
            
            //I compute the meshlet vertex and triangle offset 
            currentMeshlet.vertex_offset = static_cast<uint32_t>(vertexArrayOffset) + currentMeshlet.vertex_offset;
            currentMeshlet.triangle_offset = static_cast<uint32_t>(triangleArrayOffset) + currentMeshlet.triangle_offset;
            
            currentLod.lodVerticesMeshlets[i].meshletData = currentMeshlet;
            currentLod.lodVerticesMeshlets[i].error = maxChildrenError + error;
            currentLod.lodVerticesMeshlets[i].meshletID = meshletID;
            meshletID++;
            currentLod.lodVerticesMeshlets[i].lod = currentLod.lod;
            meshopt_Bounds meshletBound = meshopt_computeMeshletBounds(
                currentLod.lodMeshletsClusterIndex.data(),
                currentLod.lodMeshletsClusterTriangle.data(),
                currentMeshlet.triangle_count,
                &currentLod.lodVertexBuffer[0].pos.x,
                currentLod.lodVertexBuffer.size(),
                sizeof(MINERVA_VERTEX));

            glm::vec3 center{meshletBound.center[0], meshletBound.center[1],
            meshletBound.center[2]};
            meshletCenters.emplace_back(center);
            
        }
        PhoenixBound currentBound;
        for(size_t i = meshletArrayOffset; i < localmeshlets.size() + meshletArrayOffset; i++)
        {
            currentBound = welzl.ExecuteWelzl(meshletCenters, {},
            static_cast<int>(meshletCenters.size()));
            currentLod.lodVerticesMeshlets[i].bound = currentBound;
        }

        //Parent setup. "group->meshlets" refers to the meshlets of the prev lod group
        for(const auto& meshlet : group->meshlets)
        {
            prevLod->lodVerticesMeshlets[meshlet].parentError = maxChildrenError + error;
            prevLod->lodVerticesMeshlets[meshlet].parentBound = currentBound;
        }
             
    }

    void PhoenixMesh::Split(LOD &firstLod, std::vector<uint32_t> indexBuffer)
    {
        const float cone_weight = 0.0f;
        
        std::unordered_set<uint32_t> uniqueIndex;
        std::vector<MINERVA_VERTEX> groupVertexBuffer;
        for(auto index : indexBuffer)
        {
            if(uniqueIndex.insert(index).second)
            {
                groupVertexBuffer.emplace_back(firstLod.lodVertexBuffer[index]);
            }
        }

        size_t max_meshlets = meshopt_buildMeshletsBound(indexBuffer.size(), MESHLET_VERTICES_NUMBER,
        MESHLET_TRIANGLE_NUMBER);
        
        std::vector<meshopt_Meshlet> localmeshlets;
        std::vector<uint32_t> localMeshletsClusterIndex;
        std::vector<unsigned char> localMeshletsClusterTriangle;
        localmeshlets.resize(max_meshlets);
        localMeshletsClusterIndex.resize(max_meshlets * MESHLET_VERTICES_NUMBER);
        localMeshletsClusterTriangle.resize(max_meshlets * MESHLET_TRIANGLE_NUMBER * 3);

        size_t meshletCount = meshopt_buildMeshlets(localmeshlets.data(),localMeshletsClusterIndex.data(), 
        localMeshletsClusterTriangle.data(), indexBuffer.data(), indexBuffer.size(),&firstLod.lodVertexBuffer[0].pos.x,
        firstLod.lodVertexBuffer.size(),sizeof(Minerva::Mesh::Vertex), MESHLET_VERTICES_NUMBER,
        MESHLET_TRIANGLE_NUMBER, cone_weight);
        
        const meshopt_Meshlet& last = localmeshlets[meshletCount - 1];
        localMeshletsClusterIndex.resize(last.vertex_offset  + last.vertex_count);
        localMeshletsClusterTriangle.resize(last.triangle_offset  + ((last.triangle_count * 3 + 3) & ~3));
        localmeshlets.resize(meshletCount);

        firstLod.lodMeshletsClusterIndex.resize(localMeshletsClusterIndex.size());
        firstLod.lodMeshletsClusterTriangle.resize(localMeshletsClusterTriangle.size());

        //I resize the lodVerticesMeshlets
        firstLod.lodVerticesMeshlets.resize(localmeshlets.size());

        //Fill the lodMeshletsClusterIndex
        for(size_t i = 0; i < localMeshletsClusterIndex.size(); i++)
        {
            firstLod.lodMeshletsClusterIndex[i] = localMeshletsClusterIndex[i];
        }

        //Fill the lodMeshletsClusterTriangle
        for(size_t i = 0; i < localMeshletsClusterTriangle.size(); i++)
        {
            firstLod.lodMeshletsClusterTriangle[i] = localMeshletsClusterTriangle[i];
        }        
        
        std::vector<glm::vec3> meshletCenters;
        //Fill the phoenixMeshlets
        for(size_t i = 0; i < localmeshlets.size(); i++)
        {
            meshopt_Meshlet currentMeshlet = localmeshlets[i];
            
            firstLod.lodVerticesMeshlets[i].meshletData = currentMeshlet;
            firstLod.lodVerticesMeshlets[i].error = 0.0f;
            firstLod.lodVerticesMeshlets[i].meshletID = meshletID;
            meshletID++;
            meshopt_Bounds meshletBound = meshopt_computeMeshletBounds(
                firstLod.lodMeshletsClusterIndex.data(),
                firstLod.lodMeshletsClusterTriangle.data(),
                currentMeshlet.triangle_count,
                &firstLod.lodVertexBuffer[0].pos.x,
                firstLod.lodVertexBuffer.size(),
                sizeof(MINERVA_VERTEX));

            glm::vec3 center{meshletBound.center[0], meshletBound.center[1],
            meshletBound.center[2]};
            meshletCenters.emplace_back(center);     
        }

        PhoenixBound currentBound;
        for(size_t i = 0; i < localmeshlets.size(); i++)
        {
            currentBound = welzl.ExecuteWelzl(meshletCenters, {},
            static_cast<int>(meshletCenters.size()));
            firstLod.lodVerticesMeshlets[i].bound = currentBound;
        }
    }

    void PhoenixMesh::ColourGroups(PhoenixMeshlet& meshlet, std::vector<MINERVA_VERTEX> &vertices)
    {        
        for(int i = 0; i < meshlet.meshletIndexBuffer.size(); i++)
        {
            uint32_t index = meshlet.meshletIndexBuffer[i];
            if(index >= vertices.size())
                continue;
            vertices[index].color = meshlet.meshletColor;
            
        }
    }

    void PhoenixMesh::SetColor(PhoenixMeshlet& meshlet)
    {
        std::random_device rd;
        std::mt19937 gen(rd()); 

        float min = 0.15f; 
        float max = 1.0f; 
        std::uniform_real_distribution<float> dist(min, max);
        
        float r = dist(gen);
        float g = dist(gen);
        float b = dist(gen);
        meshlet.meshletColor = glm::vec3(r, g, b);
    }
}

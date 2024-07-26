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
        //Split operation of the first lod
        Split(firstLod, indices);
        //I group the meshlets of the first lod
        Group(firstLod);
        
        //I add the first lod in the LODs array
        lods.emplace_back(firstLod);
        int lodIndex = 1;
        //At the beginning the max error is zero, because I didn't perform any simplify operation 
        float maxChildrenError = 0.0f;
        //I create lods until I reach the max lod number
        while (lodIndex < MAX_LOD_NUMBER)
        {
            //Next LODs creation
            //The prev lod will be very useful to compute the current lod
            LOD* prevLod = &lods[lodIndex - 1];
            /*The current lod is the lod that I compute right now. To compute it
            I use the data contained in the prev lod*/
            LOD currentLod;
            currentLod.lod = lodIndex;
            //The vertex buffer dosen't change
            currentLod.lodVertexBuffer = prevLod->lodVertexBuffer;
            //I loop the prev lod's groups
            for(int i = 0; i < prevLod->groups.size(); i++) 
            {
                std::vector<uint32_t> groupIndexBuffer;
                MeshletGroup* group = &prevLod->groups[i];
                float outError = 0.0f;

                /*In this case the merge operation fills the group Index Buffers using the indices
                of the current group's meshlets*/
                Merge(*group,*prevLod,groupIndexBuffer); 
                auto simplifiedCount = Simplify(groupIndexBuffer,vertices, currentLod.lod, outError);
                Split(currentLod, groupIndexBuffer,outError,prevLod, group, maxChildrenError); 
            }
            //I fill the "totalMeshlets" array
            for(const auto& pMeshlet : prevLod->lodVerticesMeshlets)
            {
                totalMeshlets.emplace_back(pMeshlet);
            }

            //I do this if for sure, if my number of meshelets are below to 1 I stop the build operation
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
            Group(currentLod);
            lods.emplace_back(currentLod);
            lodIndex++;
        }

        // I fill the meshlet index buffer of the last lod's meshlets
        LOD* lastLod =  &lods[lods.size() - 1];
        
        for(auto& group : lastLod->groups) 
        {
            std::vector<uint32_t> groupIndexBuffer;            
            Merge(group,*lastLod,groupIndexBuffer);
        }

        for(const auto& pMeshlet : lastLod->lodVerticesMeshlets)
        {
            totalMeshlets.emplace_back(pMeshlet);
        }
        std::cout << "\n";
    }

 
    std::vector<MeshletGroup> PhoenixMesh::Group(LOD& currentLod)
    {
        /*It is a limit case, if the number of meshlets are below 8 I simply group the meshlets
        by inserting the indices in the array*/
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
        /*In these instructions I simply create the graph useful for partition.
        Each meshlet is a node of graph and a shared vertex between two different meshlets is 
        and edge*/
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
                    /*Here I create an edge between two vertices. In this case we can have an edge
                    composed by two vertices which belong to the same meshlet. These type of edge
                    will be removed soon*/
                    
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
        
        // I remove edges which are not connected to 2 different meshlets
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


        /*Based on METIS manual: "The adjacency structure of the graph is stored using the compressed 
        storage format (CSR).The CSR format is a widely used scheme for storing sparse graphs. In this 
        format the adjacency structure of a graph with n vertices and m edges is represented using two 
        arrays xadj and adjncy."*/

        // xadj: It contains the size of edgeAdjacency at each step 
        std::vector<idx_t> xadjacency;
        xadjacency.reserve(vertexCount + 1);

        // adjncy: It contains the adjancent meshlet of the current meshlet
        std::vector<idx_t> edgeAdjacency;
        // weight of each edge
        std::vector<idx_t> edgeWeights;

        for(size_t meshletIndex = 0; meshletIndex < currentLod.lodVerticesMeshlets.size(); meshletIndex++) 
        {
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

    void PhoenixMesh::Merge(const MeshletGroup& group, LOD& prevLod, std::vector<uint32_t>& groupIndexBuffer)
    {
        
        std::vector<glm::vec3> meshletCenters;
        for(const auto& meshletIndex : group.meshlets) 
        {
            glm::vec3 meshletCenter {0.0f};
            PhoenixMeshlet* meshlet = &prevLod.lodVerticesMeshlets[meshletIndex];
            std::unordered_set<uint32_t> uniqueIndex;
            //I multiply for 3 because each triangle has 3 vertices 
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
            /*Optimize the vertex cache to use it efficently. To do that it re-orders the index buffer 
             to maximize the locality of reused vertex. Based on meshoptmizer github page:
             "When the GPU renders the mesh, it has to run the vertex shader for each vertex; usually GPUs 
             have a built-in fixed size cache that stores the transformed vertices (the result of running the 
             vertex shader), and uses this cache to reduce the number of vertex shader invocations. This cache 
             is usually small, 16-32 vertices, and can have different replacement policies; to use this cache 
             efficiently, you have to reorder your triangles to maximize the locality of reused vertex"*/
            meshopt_optimizeVertexCache(meshlet->meshletIndexBuffer.data(), meshlet->meshletIndexBuffer.data(), 
            meshlet->meshletIndexBuffer.size(), prevLod.lodVertexBuffer.size());
        }
          
    }

    size_t PhoenixMesh::Simplify(std::vector<uint32_t>& groupIndexBuffer, 
    const std::vector<MINERVA_VERTEX>& vertices, 
    int currentLodLevel, float& outError)
    {
        float interValue = currentLodLevel / MAX_LOD_NUMBER;
        float targetError = 0.99f * interValue + 0.01f * (1.0f - interValue); 
        float threshold =  0.5f;

        targetError *= meshopt_simplifyScale(&vertices[0].pos.x, vertices.size(), sizeof(MINERVA_VERTEX));
        
        size_t targetIndexCount = groupIndexBuffer.size() * threshold;
        /*this simplification error as how much the shape of the mesh has been modified, as in how 
        much change the mesh got in terms of its size.*/
        auto simplifiedSize = meshopt_simplify(groupIndexBuffer.data(), groupIndexBuffer.data(), 
        groupIndexBuffer.size(), &vertices[0].pos.x, vertices.size(),
        sizeof(Minerva::Mesh::Vertex),targetIndexCount, targetError, simplifyOptions ,&outError);
        
        groupIndexBuffer.resize(simplifiedSize);

        
        
        return simplifiedSize;
    }



    void PhoenixMesh::Split(LOD& currentLod, std::vector<uint32_t> groupIndexBuffer, float error, LOD* prevLod, 
    MeshletGroup* group, float& maxChildrenError)
    {
        const float cone_weight = 0.0f;

        size_t max_meshlets = meshopt_buildMeshletsBound(groupIndexBuffer.size(), MESHLET_VERTICES_NUMBER,
        MESHLET_TRIANGLE_NUMBER); 
        
        std::vector<meshopt_Meshlet> localmeshlets;
        std::vector<uint32_t> localMeshletsClusterIndex;
        std::vector<unsigned char> localMeshletsClusterTriangle;
        localmeshlets.resize(max_meshlets);
        localMeshletsClusterIndex.resize(max_meshlets * MESHLET_VERTICES_NUMBER);
        localMeshletsClusterTriangle.resize(max_meshlets * MESHLET_TRIANGLE_NUMBER * 3);

        //I compute the offsets useful to set correctly the offsets in the lodVerticesMeshlets array 
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
        //This just keeps the triangles array aligned by 4 bytes (or by 4 indices since each index is 1 byte)
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
            
            //I compute the meshlet's vertex and triangle offset 
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
            std::vector tempCenters = meshletCenters;
            currentBound = welzl.ExecuteWelzl(tempCenters, {},
            static_cast<int>(tempCenters.size()));
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
        //This just keeps the triangles array aligned by 4 bytes (or by 4 indices since each index is 1 byte)
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
            std::vector tempCenters = meshletCenters;
            currentBound = welzl.ExecuteWelzl(tempCenters, {},
            static_cast<int>(tempCenters.size()));
            firstLod.lodVerticesMeshlets[i].bound = currentBound;
        }
    }
}

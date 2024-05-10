#pragma once
#include "vector"
#include "Meshoptimizer/src/meshoptimizer.h"
#include "Minerva/Mesh.h"
#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include "metis.h"
#define MESHLET_VERTICES_NUMBER 128
#define MESHLET_TRIANGLE_NUMBER 256
#define MAX_LOD_NUMBER 5
#define MAX_GROUP_NUMBER 4
#define MINERVA_VERTEX Minerva::Mesh::Vertex
/*Merge and Group methods are based on: https://jglrxavpok.github.io/2024/01/19/recreating-nanite-lod-generation.html
post*/
namespace Phoenix
{

    struct MeshletEdge 
    {
        MeshletEdge(size_t a, size_t b): first((std::min)(a, b)), second((std::max)(a, b)) {}

        bool operator==(const MeshletEdge& other) const = default;

        const size_t first;
        const size_t second;
    };

    struct MeshletEdgeHasher 
    {
        //I need to define an hasing function
        size_t operator()(const MeshletEdge& edge) const 
        {
            return glm::mix(edge.first, edge.second, 0.5f);
        }
    };

    struct MeshletGroup
    {
        std::vector<size_t> meshlets;
        std::vector<uint32_t> localGroupIndexBuffer;
        std::vector<size_t> nextMeshlets;
    };

    struct LOD
    {
        std::vector<meshopt_Meshlet> lodVerticesMeshlets;
        std::vector<uint32_t> lodMeshletsClusterIndex;
        std::vector<unsigned char> lodMeshletsClusterTriangle;
        std::vector<MeshletGroup> groups;
        std::vector<uint32_t> lodIndexBuffer;
        std::vector<MINERVA_VERTEX> lodVertexBuffer;
        std::unordered_map<size_t, idx_t> meshletToGroup;
        std::uint32_t lod = 0;
        float lodError = 0.0f;
        
    };

    class PhoenixMesh
    {
    public:
        std::vector<LOD> lods;
        std::vector<int> vertexRemap;
        
        void ColourMeshelets(MeshletGroup& group, std::vector<MINERVA_VERTEX>& vertices);
        void BuildLodsHierarchy(std::vector<MINERVA_VERTEX>& vertices, std::vector<uint32_t> &indices);
        std::vector<MeshletGroup> Group(LOD& currentLod);
        void Merge(const MeshletGroup& group, const LOD& currentLod, 
        std::vector<uint32_t>& localGroupIndexBuffer);
        size_t Simplify(std::vector<uint32_t>& localGroupIndexBuffer, LOD& currentLod, float& outError);
        std::vector<size_t> Split(LOD& currentLod, std::vector<uint32_t> indexBuffer);
        
        PhoenixMesh() = default;
        ~PhoenixMesh() = default;
    };
}



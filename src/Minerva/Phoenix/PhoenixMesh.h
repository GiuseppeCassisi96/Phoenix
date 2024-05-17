#pragma once
#include "vector"
#include "Meshoptimizer/src/meshoptimizer.h"
#include "Minerva/Mesh.h"
#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include "metis.h"
#include <random>
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
            std::hash<size_t> t;
            return (t._Do_hash(edge.first)) | (t._Do_hash(edge.second));
        }
    };
    
    struct PhoenixMeshlet
    {
        meshopt_Meshlet meshletData;
        meshopt_Bounds bound;
    };

    struct MeshletGroup
    {
        std::vector<size_t> meshlets;
        std::vector<uint32_t> localGroupIndexBuffer;
        std::unordered_set<idx_t> parentsGroup;
        float groupError = 0.0f;
    };

    struct LOD
    {
        std::vector<PhoenixMeshlet> lodVerticesMeshlets;
        std::vector<uint32_t> lodMeshletsClusterIndex;
        std::vector<unsigned char> lodMeshletsClusterTriangle;
        std::vector<MeshletGroup> groups;
        std::vector<uint32_t> lodIndexBuffer;
        std::vector<MINERVA_VERTEX> lodVertexBuffer;
        std::unordered_map<size_t, idx_t> meshletToGroup;
        std::unordered_set<uint32_t> vertexNumber;
        uint32_t lod = 0;
        float lodError;
        
    };

    class PhoenixMesh
    {
    public:
        std::vector<LOD> lods;
        int lodIndex = 0;
        int groupNumber = MAX_GROUP_NUMBER;
        
        void ColourMeshelets(MeshletGroup& group, std::vector<MINERVA_VERTEX>& vertices);
        void BuildLodsHierarchy(std::vector<MINERVA_VERTEX>& vertices, std::vector<uint32_t> &indices);
        std::vector<MeshletGroup> Group(LOD& currentLod, LOD* prevLod = nullptr);
        void Merge(MeshletGroup& group, LOD& currentLod, std::vector<uint32_t>& localGroupIndexBuffer);
        size_t Simplify(std::vector<uint32_t>& localGroupIndexBuffer, LOD& currentLod, float& outError);
        void Split(LOD& currentLod, std::vector<uint32_t> indexBuffer, 
        LOD* prevLod = nullptr, idx_t groupIndex = 0);
        void ApplyReduction(std::vector<uint32_t>& simplifiedIndexBuffer, const LOD& currentLod);
        
        PhoenixMesh() = default;
        ~PhoenixMesh() = default;
    };
}



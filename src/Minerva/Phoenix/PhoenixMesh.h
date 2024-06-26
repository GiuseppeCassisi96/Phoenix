#pragma once
#include "PhoenixWelzl.h"
#include "vector"
#include "Meshoptimizer/src/meshoptimizer.h"
#include "Minerva/Mesh.h"
#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include "metis.h"
#include <random>
#include <map>
#include <set>
#include "robin_hood.h"
#define MESHLET_VERTICES_NUMBER 255
#define MESHLET_TRIANGLE_NUMBER 512
#define MAX_LOD_NUMBER 5
#define MAX_GROUP_NUMBER 4
#define MINERVA_VERTEX Minerva::Mesh::Vertex
/*Merge and Group methods are based on: https://jglrxavpok.github.io/2024/01/19/recreating-nanite-lod-generation.html
post*/
namespace Phoenix
{

    static void HashCombine(std::size_t& seed, const std::size_t& v) 
    {
        seed ^= robin_hood::hash_int(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
    }
    struct Edge 
    {
        Edge(size_t a, size_t b): firstVertex((std::min)(a, b)), secondVertex((std::max)(a, b)) {}

        bool operator==(const Edge& other) const = default;

        const size_t firstVertex;
        const size_t secondVertex;
    };

    struct EdgeHasher 
    {
        //I need to define an hasing function
        size_t operator()(const Edge& edge) const 
        {

            std::size_t h = edge.firstVertex;
            HashCombine(h, edge.secondVertex);
            return h;
        }
    };
    
    struct PhoenixMeshlet
    {
        std::vector<uint32_t> meshletIndexBuffer;
        std::vector<MINERVA_VERTEX> meshletVertexBuffer;
        meshopt_Meshlet meshletData;
        idx_t meshletID;  
        PhoenixBound parentBound;
        PhoenixBound bound;
        float error = 0.0f;
        float parentError = 0.0f;
        int lod = 0;
        glm::vec3 meshletColor;
    };

    struct MeshletGroup
    {
        std::unordered_set<size_t> meshlets;
    };


    struct LOD
    {
        std::vector<PhoenixMeshlet> lodVerticesMeshlets;
        std::vector<uint32_t> lodMeshletsClusterIndex;
        std::vector<unsigned char> lodMeshletsClusterTriangle;
        std::vector<MeshletGroup> groups;
        std::vector<MINERVA_VERTEX> lodVertexBuffer;
        uint32_t lod = 0;
    };

    class PhoenixMesh
    {
    public:
        std::vector<LOD> lods;
        PhoenixWelzl welzl;
        int meshletID = 0;
        std::vector<PhoenixMeshlet> totalMeshlets;
        std::unordered_set<uint32_t> uniqueIndex;
        
        void ColourGroups(PhoenixMeshlet& meshlet, std::vector<MINERVA_VERTEX>& vertices);
        void SetColor(PhoenixMeshlet& meshlet);
        void BuildLodsHierarchy(std::vector<MINERVA_VERTEX>& vertices, std::vector<uint32_t> &indices);
        std::vector<MeshletGroup> Group(LOD& currentLod, LOD* prevLod = nullptr);
        void Merge(const MeshletGroup& group, LOD& prevLod, std::vector<uint32_t>& groupIndexBuffer, 
        std::unordered_set<uint32_t>& uniqueIndex);
        size_t Simplify(std::vector<uint32_t>& groupIndexBuffer, const std::vector<MINERVA_VERTEX>& groupVertexBuffer, 
        LOD& currentLod, float& outError);
        void Split(LOD& currentLod, std::vector<uint32_t> groupIndexBuffer, float error, LOD* prevLod, 
        MeshletGroup* group, float& maxChildrenError);
        void Split(LOD& firstLod, std::vector<uint32_t> indexBuffer);
        PhoenixMesh() = default;
        ~PhoenixMesh() = default;
    };
}



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
    
    /// @brief Represent my version of meshlet 
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
    };

    /// @brief Represent a group of meshlets 
    struct MeshletGroup
    {
        /// @brief A group of meshlets is rapresented as a collection of ID 
        std::unordered_set<size_t> meshlets;
    };

    /// @brief Rapresents a LOD 
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
        /// @brief The instance of the PhoenixWelzl class that executes the Welzl algorithm 
        PhoenixWelzl welzl;
        int meshletID = 0;
        //It is an array that contains the meshlets of all levels. It is, in certain sense, my version of DAG 
        std::vector<PhoenixMeshlet> totalMeshlets;
        unsigned int simplifyOptions = meshopt_SimplifySparse | meshopt_SimplifyLockBorder 
        | meshopt_SimplifyErrorAbsolute;
        
        
        /// @brief It builds the lods hierarchy at the end of this processing the 'totalMeshlets' 
        //vector  will be completely fill
        /// @param vertices Is the vertex buffer of the loaded mesh
        /// @param indices Is the index buffer of the loaded mesh
        void BuildLodsHierarchy(std::vector<MINERVA_VERTEX>& vertices, std::vector<uint32_t> &indices);
        /// @brief It groups each meshlets based on his adjacency in groups of 4 meshlets. 
        /// @param currentLod The current lod which contains the meshlets that I want to group
        /// @return The group of meshlets
        std::vector<MeshletGroup> Group(LOD& currentLod);
        /// @brief Create an index buffer of the grouped meshlets. The merge buffer optimize also the index buffer
        /// @param group The group which contains the meshlets. The group belongs to the prev lod
        /// @param prevLod The previus lod which contains the vertices indices created in the split operation
        /// @param groupIndexBuffer The group index buffer that I want create
        void Merge(const MeshletGroup& group, LOD& prevLod, std::vector<uint32_t>& groupIndexBuffer);
        /// @brief It simplifies the group index buffer, it locks the border of the grouped meshlets 
        /// to avoid cracks during lod selection 
        /// @param groupIndexBuffer The group index buffer that I want simplify
        /// @param vertices Vertex buffer of the loaded mesh, useful to compute the scaling factor 
        /// @param currentLodLevel It is the current lod index useful to interpolate the target error  
        /// @param outError The result error 
        /// @return The dim of the simplify index buffer 
        size_t Simplify(std::vector<uint32_t>& groupIndexBuffer, const std::vector<MINERVA_VERTEX>& vertices, 
        int currentLodLevel, float& outError);
        /// @brief It subdivides the simplified index buffer in meshlets 
        /// @param currentLod I pass the current LOD in order to fill: the meshlet array, the vertices index array and
        /// the triangle index array
        /// @param groupIndexBuffer The simplified index buffer, useful to compute meshlets
        /// @param error It is the simplification error, useful for the meshlet error
        /// @param prevLod Is the previus LOD, it is useful to create parent relathionship
        /// @param group It is the current group, which referes to the previus LOD
        /// @param maxChildrenError is the max meshlet error of the previous LOD
        void Split(LOD& currentLod, std::vector<uint32_t> groupIndexBuffer, float error, LOD* prevLod, 
        MeshletGroup* group, float& maxChildrenError);
        /// @brief It subdivide the initial index buffer in meshlets 
        /// @param firstLod I pass the first LOD in order to fill: the meshlet array, the vertices index array and
        /// the triangle index array
        /// @param indexBuffer The initial index buffer, useful to compute meshlets
        void Split(LOD& firstLod, std::vector<uint32_t> indexBuffer);
        PhoenixMesh() = default;
        ~PhoenixMesh() = default;
    };
}



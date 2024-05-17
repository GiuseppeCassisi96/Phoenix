#pragma once
#include "PhoenixMesh.h"
#include <vector>

namespace Phoenix
{
    struct ComputeMeshletGroup
    {
        std::vector<size_t> meshletsRef;
        std::vector<idx_t> parentsGroup;       
    };
    struct ComputeLOD
    {
        std::vector<ComputeMeshletGroup> groupsData;
        std::vector<PhoenixMeshlet> meshlets;
    };

    struct DataToCompute
    {
        std::vector<ComputeLOD> lodsToCompute;
    };
    
    class LODSelectionDispatcher
    {
    public:
        DataToCompute computeData;
        void PrepareComputeData(const std::vector<LOD>& computeLods);
        void LodSelector(const glm::mat4& modelViewMatrix);
        LODSelectionDispatcher() = default;
        ~LODSelectionDispatcher() = default;
    };
}





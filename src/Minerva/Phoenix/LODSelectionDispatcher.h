#pragma once
#include "PhoenixMesh.h"
#include "Minerva/EngineCamera.h"
#include "Minerva/ModelLoader.h"
#include <vector>
#include <cmath>
#include <map>

namespace Phoenix
{
    struct ComputeMeshletGroup
    {
        idx_t groupID;
        std::vector<idx_t> parentsGroupID;  
    };
    struct ComputeLOD
    {
        std::vector<ComputeMeshletGroup> groupsData;
    };

    struct DataToCompute
    {
        std::vector<ComputeLOD> lodsToCompute;
    };


    class LODSelectionDispatcher
    {
    public:

        int count = 0;
        float currentAvgLOD = 0.0f;
        DataToCompute computeData;
        float errorThreshold = 0.0f;
        float lastAvgLod = 0.0f;
        void PrepareComputeData(std::vector<PhoenixMeshlet>& totalMeshlet, Minerva::SampleType currentSample);

        std::vector<uint32_t> LodSelector(std::vector<PhoenixMeshlet>&  totalMeshlets, int width, float hFov,
        const glm::vec3& instancePos, float& avgLOD, std::vector<MINERVA_VERTEX>& vertexBuffer,
        Minerva::Transformation& tr, PhoenixMesh& mesh, int& vertexCount);

        float ComputeScreenSpaceError(PhoenixBound bound,float groupError,int width, 
        float hFov, const glm::vec3& instancePos, float distanceMul, const glm::mat4& modelView);

        LODSelectionDispatcher() = default;
        ~LODSelectionDispatcher() = default;
    };
}





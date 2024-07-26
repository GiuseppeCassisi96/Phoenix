#pragma once
#include "PhoenixMesh.h"
#include "Minerva/EngineCamera.h"
#include "Minerva/ModelLoader.h"
#include <vector>
#include <cmath>
#include <map>

namespace Phoenix
{

    struct InputComputeData
    {
        alignas(16) glm::vec3 boundCenter{0.0f};
        alignas(16) glm::vec3 parentBoundCenter{0.0f};
        int meshletID = 0;
        float error = 0.0f;
        float parentError = 0.0f;
        int lod = 0;
        int numberOfInstance = -1;
        
    };

    struct ConstantData
    {
        alignas(16) glm::vec3 instancesPos;
        int numberOfMeshlet = 0;
        int numberOfInstances = -1;
        int height = 0;
        float hfov = 0.0f;
        float errorThreshold = 0.0f;
    };

    struct alignas(16) OutputData
    {
        int index = 0;
        int ID = -1;
        int instanceNumber = 0;
    };


    class LODSelectionDispatcher
    {
    public:

        int count = 0;
        float errorThreshold = 0.0f;
        float lastAvgLod = 0.0f;
        std::vector<InputComputeData> inputData;
        std::vector<ConstantData> constantData;
        std::vector<OutputData> outputData;
        std::vector<PhoenixMeshlet> meshletForSelection;
        void PrepareComputeData(const std::vector<PhoenixMeshlet>&  totalMeshlets, float hFOV, int height);

        LODSelectionDispatcher() = default;
        ~LODSelectionDispatcher() = default;
    };
}





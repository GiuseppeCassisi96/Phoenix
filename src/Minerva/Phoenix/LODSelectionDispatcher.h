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
        float errorThreshold = 0.0f;
        int lod = 0;
        int width = 0;
        float hfov = 0.0f;
        bool isSelected = false;
    };

    class LODSelectionDispatcher
    {
    public:

        int count = 0;
        float errorThreshold = 0.0f;
        float lastAvgLod = 0.0f;
        std::vector<InputComputeData> inputData;
        std::vector<PhoenixMeshlet> meshletForSelection;
        

        void PrepareComputeData(const std::vector<PhoenixMeshlet>&  totalMeshlets, float hFOV, int width);
        std::vector<uint32_t> LodSelector(int width, float hFov,
        const glm::vec3& instancePos, std::vector<MINERVA_VERTEX>& vertexBuffer,
        Minerva::Transformation& tr, PhoenixMesh& mesh, int& vertexCount);

        float ComputeScreenSpaceError(PhoenixBound bound,float groupError,int width, 
        float hFov, const glm::vec3& instancePos, const glm::mat4& modelView);

        LODSelectionDispatcher() = default;
        ~LODSelectionDispatcher() = default;
    };
}





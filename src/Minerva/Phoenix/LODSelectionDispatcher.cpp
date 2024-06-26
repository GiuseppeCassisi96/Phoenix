#include "LODSelectionDispatcher.h"
#include <iostream>
namespace Phoenix
{
    void LODSelectionDispatcher::PrepareComputeData(std::vector<PhoenixMeshlet>& totalMeshlet)
    {
        /*Data:
        --GroupID
        --GroupError
        --ParentError
        --GroupBound(center and radius)
        --ParentBound(center and radius)
        --ErrorThreshold
        --InstancePos
        */
    }
    std::vector<uint32_t> LODSelectionDispatcher::LodSelector(std::vector<PhoenixMeshlet>&  totalMeshlets, 
    int width, float hFov, const LOD& lastLOD, const glm::vec3& instancePos,float& avgLOD, 
    std::vector<MINERVA_VERTEX>& vertexBuffer, int groupSelector, Minerva::Transformation& tr,
    Minerva::SampleType currentSample)
    {
        std::vector<uint32_t> newIndexBuffer;
        
        float distanceMul = 2.0f;
        errorThreshold = currentSample.tError;
        std::unordered_set<idx_t> meshletsSelected;
        
        for(const auto& meshlet : totalMeshlets) 
        {
            glm::mat4 groupMatrix = tr.ubo.view * tr.ubo.model;
            float currentError = ComputeScreenSpaceError(meshlet.bound, meshlet.error, width,
            hFov, instancePos, distanceMul, groupMatrix);

            if(meshlet.lod >= MAX_LOD_NUMBER - 1)
            {
                if(currentError <= errorThreshold)
                {
                    meshletsSelected.insert(meshlet.meshletID);
                }  
                continue;
            } 

            glm::mat4 parentGroupMatrix = tr.ubo.view * tr.ubo.model;
            float parentError = ComputeScreenSpaceError(meshlet.parentBound, meshlet.parentError,
            width, hFov, instancePos, distanceMul, parentGroupMatrix);

            if(currentError <= errorThreshold && parentError > errorThreshold)
            {
                meshletsSelected.insert(meshlet.meshletID);
            }
            

                         
        } 
        //groupsSelected.insert(groupSelector % lastLOD.groups.size());
        //CPU side 
        for(const auto& meshletID : meshletsSelected)
        {
            PhoenixMeshlet* currentPMeshlet = &totalMeshlets[meshletID];    
            newIndexBuffer.insert(newIndexBuffer.end(), currentPMeshlet->meshletIndexBuffer.begin(),
            currentPMeshlet->meshletIndexBuffer.end());  
            avgLOD += currentPMeshlet->lod;
        }
        avgLOD /= meshletsSelected.size();
        if(newIndexBuffer.size() <= 0)
            avgLOD = -1.0f;
        
        currentAvgLOD = avgLOD;

        return newIndexBuffer;
    }

    float LODSelectionDispatcher::ComputeScreenSpaceError(PhoenixBound bound,float groupError,int width, 
    float hFov, const glm::vec3& instancePos, float distanceMul, const glm::mat4& modelView)
    {
        bound.center += instancePos;
        glm::vec4 viewCenter = glm::vec4(bound.center, 1.0f);
        //I transform the center in view-space
        viewCenter = modelView * viewCenter;
        bound.center.x = viewCenter.x;
        bound.center.y = viewCenter.y;
        bound.center.z = viewCenter.z;
        bound.radius = glm::length(glm::vec3 {modelView * glm::vec4{groupError, 0, 0, 0 }});
  
        //https://stackoverflow.com/questions/21648630/radius-of-projected-sphere-in-screen-space
        const float cotHalfFov = 1.0f / glm::tan(hFov / 2.0f);
        const float d2 = glm::dot(bound.center, bound.center);
        const float r = bound.radius;
        const float div = glm::sqrt(d2 - r*r);
        float screenSpaceError = (width / 2.0f * cotHalfFov * r) / div;
        return screenSpaceError;

        
        //Bound center in View space 
        /* float d = bound.center.z - bound.radius;
        float screenSpaceError =  (groupError * width) / (2 * d * tan(hFov/2));
        return glm::abs(screenSpaceError); */

    }
}
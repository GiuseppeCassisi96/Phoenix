#include "LODSelectionDispatcher.h"
namespace Phoenix
{
    void LODSelectionDispatcher::PrepareComputeData(const std::vector<LOD> &computeLods)
    {
        computeData.lodsToCompute.resize(computeLods.size());
        for(int i = 0; i < computeLods.size(); i++)
        {
            ComputeLOD currentComputeLod;
            ComputeMeshletGroup computeGroup;
            for(int j = 0; j < computeLods[i].groups.size(); j++)
            {
                MeshletGroup group = computeLods[i].groups[j];
                computeGroup.meshletsRef = group.meshlets;
                computeGroup.parentsGroup.insert(computeGroup.parentsGroup.begin(), 
                group.parentsGroup.begin(), group.parentsGroup.end());
                
            }
            currentComputeLod.groupsData.emplace_back(computeGroup);
            currentComputeLod.meshlets = computeLods[i].lodVerticesMeshlets;
            computeData.lodsToCompute.emplace_back(currentComputeLod);
        }
    }
    void LODSelectionDispatcher::LodSelector(const glm::mat4 &modelViewMatrix)
    {
        ComputeLOD startingLod = computeData.lodsToCompute[0];
        for(auto group : startingLod.groupsData)
        {
            
        } 
    }
}
#include "AnimationManager.h"
#include "Bone.h"
#include "Mesh.h"
#include "ModelLoader.h"
#include "EngineVars.h"
#include <iostream>

namespace Minerva
{
    void Animation::CreateAnimation(const std::string &animationPath, ModelLoader* model)
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
        assert(scene && scene->mRootNode);
        auto animation = scene->mAnimations[0];
        duration = static_cast<float>(animation->mDuration);
        ticksPerSecond = static_cast<int>(animation->mTicksPerSecond);
        ReadHeirarchyData(rootNode, scene->mRootNode);
        ReadMissingBones(animation, *model);
    }
    Bone *Animation::FindBone(const std::string &name)
    {
       auto iter = std::find_if(bones.begin(), bones.end(),
            [&](const Bone& Bone)
            {
                return Bone.name == name;
            }
        );
        if (iter == bones.end()) return nullptr;
        else return &(*iter);
    }
    void Animation::ReadMissingBones(const aiAnimation *animation, ModelLoader &model)
    {
        int size = animation->mNumChannels;

        auto& boneInfoMap = model.infoBoneMap;//getting infoBoneMap from ModelLoader class
        int& boneCount = model.boneNumber; //getting the boneNumber from ModelLoader class

        //reading channels(bones engaged in an animation and their keyframes)
        for (int i = 0; i < size; i++)
        {
            auto channel = animation->mChannels[i];
            std::string boneName = channel->mNodeName.data;

            if (boneInfoMap.find(boneName) == boneInfoMap.end())
            {
                boneInfoMap[boneName].id = boneCount;
                boneCount++;
            }
            bones.push_back(Bone(channel->mNodeName.data,
                boneInfoMap[channel->mNodeName.data].id, channel));
        }

        animBoneInfoMap = boneInfoMap;
    }
    void Animation::ReadHeirarchyData(AssimpNodeData &dest, const aiNode *src)
    {
        assert(src);

        dest.name = src->mName.data;
        dest.transformation = ModelLoader::ConvertMatrixToGLMFormat(src->mTransformation);
        dest.childrenCount = src->mNumChildren;

        for (unsigned int i = 0; i < src->mNumChildren; i++)
        {
            AssimpNodeData newData;
            ReadHeirarchyData(newData, src->mChildren[i]);
            dest.children.push_back(newData);
        }
    }

    void Animator::CreateAnimator(Animation *Animation)
    {
        currentTime = 0.0;
        currentAnimation = Animation;

        for (int i = 0; i < MAX_BONES; i++)
            engineRenderer.UNBoneMatrices.finalBoneMatrices[i] = glm::mat4(1.0f);
    }

    void Animator::UpdateAnimation(float dt)
    {
        deltaTime = dt;
        if (currentAnimation)
        {
            currentTime += currentAnimation->ticksPerSecond * dt;
            currentTime = fmod(currentTime, currentAnimation->duration);
            CalculateBoneTransform(&currentAnimation->rootNode, glm::mat4(1.0f));
            
        }
    }

    void Animator::PlayAnimation(Animation *pAnimation)
    {
        currentAnimation = pAnimation;
        currentTime = 0.0f;
    }
    void Animator::CalculateBoneTransform(const AssimpNodeData *node, glm::mat4 parentTransform)
    {
        std::string nodeName = node->name;
        glm::mat4 nodeTransform = node->transformation;
	
        Bone* Bone = currentAnimation->FindBone(nodeName);
	
        if(Bone)
        {
            Bone->Update(currentTime);
            nodeTransform = Bone->localTransform;
        }
	
        glm::mat4 globalTransformation = parentTransform * nodeTransform;
	
        auto& boneInfoMap = currentAnimation->animBoneInfoMap;
        if (boneInfoMap.find(nodeName) != boneInfoMap.end())
        {
            int index = boneInfoMap[nodeName].id;
            glm::mat4 offset = boneInfoMap[nodeName].offset;
            engineRenderer.UNBoneMatrices.finalBoneMatrices[index] = globalTransformation * offset;
            
        }
	
        for (int i = 0; i < node->childrenCount; i++)
            CalculateBoneTransform(&node->children[i], globalTransformation);
    }
}
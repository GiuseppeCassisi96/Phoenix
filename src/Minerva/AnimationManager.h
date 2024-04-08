#pragma once
#include "string"
#include "Bone.h"
#include "Renderer.h"
#include "Mesh.h"
#include <map>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Minerva
{
    struct AssimpNodeData
    {
        glm::mat4 transformation;
        std::string name;
        int childrenCount;
        std::vector<AssimpNodeData> children;
    };


    class ModelLoader;

    class Animation
    {
    public:
        float duration;
        int ticksPerSecond;
        std::vector<Bone> bones;
        AssimpNodeData rootNode;
        std::map<std::string, Mesh::BoneInfo> animBoneInfoMap;
        Animation() = default; 
        void CreateAnimation(const std::string& animationPath, ModelLoader* model);
        Bone* FindBone(const std::string& name);
        void ReadMissingBones(const aiAnimation* animation, ModelLoader& model);
        void ReadHeirarchyData(AssimpNodeData& dest, const aiNode* src);
    };

    class Animator
    {
    public: 
       
        Animation* currentAnimation;
        float currentTime;
        float deltaTime;
        Animator() = default;
        void CreateAnimator(Animation* Animation);
        void UpdateAnimation(float dt);
        void PlayAnimation(Animation* pAnimation);
        void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform);

    };
}
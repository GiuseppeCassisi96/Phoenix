
#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <map>
#include "Mesh.h"
namespace Minerva
{
    struct SampleType
    {
        std::string textureName;
        std::string modelName;
        std::vector<std::string> animName;
        int animNumber;
        float scale;
        int rowDim;
    };

    struct MeshInfo
    {
        int numberOfVertices;
        int numberOfPolygons;
        int numberOfBones;
    };
    class ModelLoader
    {
    public:
        MeshInfo info;
        int instanceNumber;
        std::vector<InstanceData> instancesData;
        Mesh::InstanceBuffer instanceBuffer;
        std::vector<Mesh> sceneMeshes;
        std::map<std::string, Mesh::BoneInfo> infoBoneMap;
        int boneNumber = 0;
        void LoadModel(std::string fileName);
        void ProcessAssimpNode(aiNode *node, const aiScene *scene);
        Mesh ProcessAssimpMesh(aiMesh *mesh, const aiScene *scene);
        void PrepareInstanceData(SampleType type);
        void SetupBoneData(Minerva::Mesh::Vertex& currentVertex, int boneID = -1, float weight = 0.0f);
        void ExtractBoneWeightForVertices(std::vector<Mesh::Vertex>& vertices, aiMesh* mesh, const aiScene* scene);
        static glm::mat4 ConvertMatrixToGLMFormat(const aiMatrix4x4&from);
        ModelLoader() = default;
        ~ModelLoader();
        
        ModelLoader(const ModelLoader& other) = delete;
        ModelLoader& operator=(const ModelLoader& other) = delete;

        ModelLoader (ModelLoader&& other) noexcept;
        ModelLoader& operator=(ModelLoader&& other) noexcept;
    private:
        
        const std::string MODELS_PATH = "C:/UNIMI/TESI/Phoenix/src/Minerva/Models/";
    };
    
}
#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Mesh.h"
namespace Minerva
{
    class ModelLoader
    {
    public:
        int instanceNumber = 100;
        std::vector<InstanceData> instancesData;
        Mesh::InstanceBuffer instanceBuffer;
        std::vector<Mesh> sceneMeshes;
        void LoadModel(std::string fileName);
        void ProcessAssimpNode(aiNode *node, const aiScene *scene);
        Mesh ProcessAssimpMesh(aiMesh *mesh, const aiScene *scene);
        void PrepareInstanceData();
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
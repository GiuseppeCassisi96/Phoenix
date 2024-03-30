#include "Mesh.h"
#include "EngineVars.h"
#include <unordered_map>

#include <iostream>

namespace Minerva
{
    void Mesh::LoadModel(std::string fileName)
    {
        Assimp::Importer importer;

        const aiScene *scene = importer.ReadFile((MODELS_PATH + fileName).c_str(), 
        aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);

        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
        {
            std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
            return;
        }
        ProcessAssimpNode(scene->mRootNode, scene);
        
    }

    void Mesh::ProcessAssimpNode(aiNode *node, const aiScene *scene)
    {
         // process all the node's meshes (if any)
        for(unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            sceneMeshes.emplace_back(std::move(ProcessAssimpMesh(mesh, scene)));
        }
        // then do the same for each of its children
        for(unsigned int i = 0; i < node->mNumChildren; i++)
        {
            ProcessAssimpNode(node->mChildren[i], scene);
        }
    }

    Mesh Mesh::ProcessAssimpMesh(aiMesh *mesh, const aiScene *scene)
    {

        std::vector<Vertex> tempVertices;
        std::vector<uint32_t> tempIndices;

        for(int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex currentVertex;

            //Position loading
            currentVertex.pos.x = mesh->mVertices[i].x;
            currentVertex.pos.y = mesh->mVertices[i].y;
            currentVertex.pos.z = mesh->mVertices[i].z;


            currentVertex.color = glm::vec3 {1.0f};

            //UVCoord loading
            if(mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
            {
                currentVertex.texCoord.x = mesh->mTextureCoords[0][i].x; 
                currentVertex.texCoord.y = mesh->mTextureCoords[0][i].y;
            }
            else
            {
                currentVertex.texCoord = glm::vec2(0.0f, 0.0f);  
            }
                
            tempVertices.emplace_back(currentVertex);
        }


        for(unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for(unsigned int j = 0; j < face.mNumIndices; j++)
                tempIndices.emplace_back(face.mIndices[j]);
        }
        std::cout << "END\n";
        return Mesh(tempVertices, tempIndices);
    }

    void Transformation::Move(const glm::vec3 &dir, glm::mat4 model)
    {
        ubo.model = glm::translate(model, dir);
    }
    void Transformation::Scale(const glm::vec3& dim, glm::mat4 model)
    {
        ubo.model = glm::scale(model, dim);
    }
    void Transformation::Rotate(const float& angle, const glm::vec3& axis, glm::mat4 model)
    {
        ubo.model = glm::rotate(model, glm::radians(angle), axis);
    }
    void Mesh::PrepareInstanceData()
    {
        int counter = 0;
        float row = 0.0f;
        instancesData.resize(instanceNumber);

        for(int i = 0; i < instanceNumber; i++)
        {   
            counter++;
            instancesData[i].instancePos = glm::vec3(counter * 30.0f,-row * 30.0f, 0.0f);
            if(counter >= 10)
            {
                counter = 0;
                row++;
            }
            instancesData[i].instanceScale = 10.0f;
            
        }

        instanceBuffer.size = instancesData.size() * sizeof(InstanceData);
    }

    Mesh::Mesh(std::vector<Vertex> inputVertices, std::vector<uint32_t> inputIndices): vertices(inputVertices), 
    indices(inputIndices)
    {}

    Mesh::~Mesh()
    {
        vkDestroyBuffer(engineDevice.logicalDevice, instanceBuffer.buffer, nullptr);
        vkFreeMemory(engineDevice.logicalDevice, instanceBuffer.memory, nullptr);
    }
}
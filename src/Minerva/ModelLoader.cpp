#include "ModelLoader.h"

#include "EngineVars.h"


namespace Minerva
{
    void ModelLoader::LoadModel(std::string fileName)
    {
        Assimp::Importer importer;

        const aiScene *scene = importer.ReadFile((MODELS_PATH + fileName).c_str(), 
        aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices );

        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
        {
            std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
            return;
        }
        ProcessAssimpNode(scene->mRootNode, scene);
        if(sceneMeshes[0].vertices[0].boneID[0] == -1)
            sceneMeshes[0].typeOfMesh = Mesh::MeshType::Static;
        else
            sceneMeshes[0].typeOfMesh = Mesh::MeshType::Skeletal;

    }
    void ModelLoader::ProcessAssimpNode(aiNode *node, const aiScene *scene)
    {
         // process all the node's meshes (if any)
        for(unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            sceneMeshes.emplace_back(ProcessAssimpMesh(mesh, scene));
        }
        // then do the same for each of its children
        for(unsigned int i = 0; i < node->mNumChildren; i++)
        {
            ProcessAssimpNode(node->mChildren[i], scene);
        }

    }
    Mesh ModelLoader::ProcessAssimpMesh(aiMesh *mesh, const aiScene *scene)
    {
        std::vector<Minerva::Mesh::Vertex> tempVertices;
        std::vector<uint32_t> tempIndices;
        Mesh createdMesh;
        
        
        for(unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Minerva::Mesh::Vertex currentVertex;

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
            
            SetupBoneData(currentVertex);
            tempVertices.emplace_back(currentVertex);
        }


        for(unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for(unsigned int j = 0; j < face.mNumIndices; j++)
                tempIndices.emplace_back(face.mIndices[j]);
        }
       
        createdMesh.vertices = tempVertices;
        createdMesh.indices = tempIndices;
        if(mesh->mNumBones > 0)
            ExtractBoneWeightForVertices(createdMesh.vertices, mesh, scene);
        
        info.numberOfBones = mesh->mNumBones;
        info.numberOfPolygons = mesh->mNumFaces;
        info.numberOfVertices = mesh->mNumVertices;
        return createdMesh;
    }
    void ModelLoader::PrepareInstanceData(SampleType type)
    {
        int counter = 0;
        float row = 0.0f;
        instancesData.resize(instanceNumber);

        for(int i = 0; i < instanceNumber; i++)
        {   
            counter++;
            instancesData[i].instancePos = glm::vec3(counter * 30.0f,-row * 30.0f, 0.0f);
            if(counter >= type.rowDim)
            {
                counter = 0;
                row++;
            }
            instancesData[i].instanceScale = type.scale;
            
        }

        instanceBuffer.size = instancesData.size() * sizeof(InstanceData);
    }

    void ModelLoader::SetupBoneData(Minerva::Mesh::Vertex& currentVertex, int boneID, float weight)
    {
        for(int i = 0; i < MAX_BONE_PER_VERTEX; i++)
        {
            currentVertex.boneID[i] = boneID;
            currentVertex.weight[i] = weight; 
        }
    }

    void ModelLoader::ExtractBoneWeightForVertices(std::vector<Mesh::Vertex> &vertices, aiMesh *mesh, const aiScene *scene)
    {
        std::vector<float> boneweight;
        for (unsigned int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
        {
            int boneID = -1;
            std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
            if (infoBoneMap.find(boneName) == infoBoneMap.end())
            {
                Mesh::BoneInfo newBoneInfo;
                newBoneInfo.id = boneNumber;
                newBoneInfo.offset = ConvertMatrixToGLMFormat(
                    mesh->mBones[boneIndex]->mOffsetMatrix);
                infoBoneMap[boneName] = newBoneInfo;
                boneID = boneNumber;
                boneNumber++;
            }
            else
            {
                boneID = infoBoneMap[boneName].id;
            }
            assert(boneID != -1);
            auto weights = mesh->mBones[boneIndex]->mWeights;
            int numWeights = mesh->mBones[boneIndex]->mNumWeights;
            float weight;
            for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
            {
                int vertexId = weights[weightIndex].mVertexId;
                weight = weights[weightIndex].mWeight;
                assert(vertexId <= vertices.size());
                SetupBoneData(vertices[vertexId], boneID, weight);
                for(int i = 0; i < MAX_BONE_PER_VERTEX; i++)
                {
                    vertices[vertexId].boneID[i] = boneID;
                    vertices[vertexId].weight[i] = weight; 
                }
            }
            boneweight.emplace_back(weight);
        }

    }

    glm::mat4 ModelLoader::ConvertMatrixToGLMFormat(const aiMatrix4x4 &from)
    {
        glm::mat4 to;
		//the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
		to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
		to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
		to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
		to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
		return to;
    }

    ModelLoader::~ModelLoader()
    {
        vkDestroyBuffer(engineDevice.logicalDevice, instanceBuffer.buffer, nullptr);
        vkFreeMemory(engineDevice.logicalDevice, instanceBuffer.memory, nullptr);
    }

    ModelLoader::ModelLoader(ModelLoader &&other) noexcept
    {
        instanceNumber = std::move(other.instanceNumber);
        instanceBuffer = std::move(other.instanceBuffer);
        instancesData = std::move(other.instancesData);
        sceneMeshes = std::move(other.sceneMeshes);

        other.instanceNumber = 0;
        vkDestroyBuffer(engineDevice.logicalDevice, other.instanceBuffer.buffer, nullptr);
        vkFreeMemory(engineDevice.logicalDevice, other.instanceBuffer.memory, nullptr);
        free(other.instancesData.data());
        free(other.sceneMeshes.data());
        
    }
    ModelLoader &ModelLoader::operator=(ModelLoader &&other) noexcept
    {
        instanceNumber = std::move(other.instanceNumber);
        instanceBuffer = std::move(other.instanceBuffer);
        instancesData = std::move(other.instancesData);
        sceneMeshes = std::move(other.sceneMeshes);

        other.instanceNumber = 0;
        vkDestroyBuffer(engineDevice.logicalDevice, other.instanceBuffer.buffer, nullptr);
        vkFreeMemory(engineDevice.logicalDevice, other.instanceBuffer.memory, nullptr);
        free(other.instancesData.data());
        free(other.sceneMeshes.data());
        return *this;
    }
}
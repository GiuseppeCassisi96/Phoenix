#include "EngineStartup.h"
#include <iostream>
#include "Phoenix/PhoenixMesh.h"
#include "Phoenix/PhoenixSelection.h"
#include <chrono>
#include <algorithm>


namespace Minerva
{
    VulkanInstance engineInstance;
    Window windowInstance;
    DebugManager debugLayer;
    Device engineDevice;
    EnginePipeline enginePipeline;
    Renderer engineRenderer;
    Transformation engineTransform;
    TextureManager texture;
    EngineCamera camera;
    MinervaUI engineUI;
    ModelLoader engineModLoader;
    Phoenix::LODSelectionDispatcher dispatcher;
    Phoenix::PhoenixMesh phoenixMesh; 

    template<typename T>
    void CreateUniformBuffers(UniformBuffers& UNBuffers)
    {
        VkDeviceSize bufferSize = sizeof(T);

        UNBuffers.uniformBuffers.resize(engineRenderer.MAX_FRAMES_IN_FLIGHT);
        UNBuffers.uniformBuffersMemory.resize(engineRenderer.MAX_FRAMES_IN_FLIGHT);
        UNBuffers.uniformBuffersMapped.resize(engineRenderer.MAX_FRAMES_IN_FLIGHT);

        for (size_t i = 0; i < engineRenderer.MAX_FRAMES_IN_FLIGHT; i++) {
            engineRenderer.CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
            UNBuffers.uniformBuffers[i], UNBuffers.uniformBuffersMemory[i]);

            vkMapMemory(engineDevice.logicalDevice, UNBuffers.uniformBuffersMemory[i], 0,
            bufferSize, 0, &UNBuffers.uniformBuffersMapped[i]);
        }
    }

    void EngineStartup::RunEngine()
    {
        std::cout << "                                          -----------------MINERVA ENGINE-----------------\n\n";
        Start();
        Loop();
        debugLayer.DestroyDebugUtilsMessengerEXT(engineInstance.instance,debugLayer.debugMessenger,nullptr);
        vkDestroyBuffer(engineDevice.logicalDevice, engineModLoader.sceneMeshes[0].meshBuffer.vertexBuffer, nullptr);
        vkFreeMemory(engineDevice.logicalDevice, engineModLoader.sceneMeshes[0].meshBuffer.vertexBufferMemory, nullptr);
    }

    void EngineStartup::Start()
    {
        samplesTest["0"].modelName = "submarine.fbx";
        samplesTest["0"].textureName = "submarineColor.png";
        samplesTest["0"].scale = 600.0f;
        samplesTest["0"].rowDim = 5;
        samplesTest["0"].distanceMultiplier = 2100.0f;
        samplesTest["0"].tError = 0.3f;

        samplesTest["1"].modelName = "dancer.obj";
        samplesTest["1"].textureName = "dancerColor.jpg";
        samplesTest["1"].scale = 1.0f;
        samplesTest["1"].rowDim = 5;
        samplesTest["1"].distanceMultiplier = 2100.0f;
        samplesTest["1"].tError = 0.6f;

        samplesTest["2"].modelName = "moonRock.obj";
        samplesTest["2"].textureName = "rockColor.jpeg";
        samplesTest["2"].scale = 1600.0f;
        samplesTest["2"].rowDim = 5;
        samplesTest["2"].distanceMultiplier = 1200.0f;
        samplesTest["2"].tError = 0.1f;

        samplesTest["3"].animNumber = 3;
        samplesTest["3"].animName.emplace_back("monsterIdle.fbx");
        samplesTest["3"].animName.emplace_back("monsterWalk.fbx");
        samplesTest["3"].animName.emplace_back("monsterRun.fbx");
        samplesTest["3"].modelName = "monster.fbx";
        samplesTest["3"].textureName = "monsterColor.png";
        samplesTest["3"].scale = 1.0f;
        samplesTest["3"].rowDim = 30;
        samplesTest["3"].distanceMultiplier = 1000.0f;
        samplesTest["3"].tError = 0.3f;
   
        std::string key;
        std::string choose;

        std::cout << "Choose the model which you want rendered: \n"
        << "Insert '0' to render the submarine static model\n"
        << "Insert '1' to render the dancer static model\n"
        << "Insert '2' to render the rock static model\n"
        << "Insert '3' to render the skeletal model\n" ;
        std::cin >> key;
        assert(key == "1" || key == "0" || key == "2" || key == "3");
        std::cout << "Select the instance number: ";
        std::cin >> engineModLoader.instanceNumber;
        std::cout << "Do you want use the algorithm (y/n): ";
        std::cin >> choose;
        assert(choose == "y" || choose == "n");
        if(choose == "y")
        {
            engineRenderer.renderMode = Mode::Phoenix;
        }
        else if(choose == "n")
        {
            engineRenderer.renderMode = Mode::Normal;
        }

        choosenSample = samplesTest[key];
        dispatcher.errorThreshold = choosenSample.tError;

        windowInstance.EngineInitWindow(windowInstance.WIDTH, windowInstance.HEIGHT);
        engineInstance.CreateInstance();
        debugLayer.SetupDebugMessenger(engineInstance.instance);
        windowInstance.CreateWindowSurface(engineInstance.instance);
        engineDevice.PickMostSuitableDevice(engineInstance.instance, windowInstance.windowSurface);
        engineDevice.PrintInfoDeviceSelected();
        engineDevice.CreateLogicalDevice(debugLayer, windowInstance.windowSurface);
        engineDevice.CreateSwapChain();
        engineDevice.CreateImageViews();
        engineRenderer.CreateRenderPass();
        engineRenderer.CreateDescriptorSetLayout();
        enginePipeline.CreatePipeline("vert", "frag");
        enginePipeline.CreateComputePipeline("comp");
        engineRenderer.CreateCommandPool();
        engineRenderer.CreateColorResources();
        engineRenderer.CreateDepthResources();
        engineRenderer.CreateFramebuffers();
        texture.CreateTextureImage(choosenSample.textureName);
        texture.CreateTextureImageView();
        texture.CreateTextureSampler();
        
        engineModLoader.LoadModel(choosenSample.modelName, choosenSample);

        if(engineModLoader.sceneMeshes[0].typeOfMesh == Mesh::MeshType::Skeletal)
        {
            
            for(int i = 0; i < choosenSample.animNumber; i++)
            {
                Animation currentAnim;
                currentAnim.CreateAnimation("C:/UNIMI/TESI/Phoenix/src/Minerva/Animations/" 
                + choosenSample.animName[i], &engineModLoader);
                animations.emplace_back(currentAnim);
            }
            animator.CreateAnimator(&animations[0]);
        }
        
        if(engineRenderer.renderMode == Mode::Phoenix)
        {
            auto startTime = std::chrono::high_resolution_clock::now();
            phoenixMesh.BuildLodsHierarchy(engineModLoader.sceneMeshes[0].vertices, engineModLoader.sceneMeshes[0].indices);
            auto endTime = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> amount = endTime - startTime;
            std::cout << "S: " << amount << "\n";
        }
        else
        {
            phoenixMesh.totalMeshlets.resize(1);
        }
        

        engineModLoader.PrepareInstanceData(choosenSample);
        engineRenderer.PrepareIndirectData(engineModLoader.sceneMeshes[0].indices, engineModLoader.sceneMeshes[0].vertices);
        dispatcher.PrepareComputeData(phoenixMesh.totalMeshlets, glm::radians(45.0f), windowInstance.HEIGHT);
        
        engineRenderer.CreateVertexBuffer();
        engineRenderer.CreateInstanceBuffer();
        engineRenderer.CreateIndexBuffer();
        CreateUniformBuffers<UniformBufferObject>(engineRenderer.transformationUBuffers);
        CreateUniformBuffers<BoneMatricesUniformType>(engineRenderer.animUBuffers);
        engineRenderer.CreateDescriptorPool();
        engineRenderer.CreateDescriptorSets(phoenixMesh.totalMeshlets.size());
        engineRenderer.CreateCommandBuffer();
        engineRenderer.CreateComputeCommandBuffer();
        engineRenderer.CreateSyncObjects();
    
        
        glfwSetInputMode(windowInstance.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        camera.SetupViewMatrix(engineTransform.ubo.view);
        glfwSetCursorPosCallback(windowInstance.window, [](GLFWwindow* window, double xpos, double ypos)
        {
            if(windowInstance.isCursorDisabled)
                camera.MouseCallback(window, xpos, ypos);
        });
        engineUI.SetupUI(*this);
        glfwSetKeyCallback(windowInstance.window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            windowInstance.KeyPressCallback(window, key, scancode, action, mods);
        });
          
    }
    void EngineStartup::Loop()
    {
        engineRenderer.UpdateUniformBuffer(engineRenderer.currentFrame);
        size_t indexMaxSize = engineModLoader.sceneMeshes[0].indices.size();
        size_t singleVertexSize = engineModLoader.sceneMeshes[0].vertices.size() 
        / engineModLoader.instanceNumber;
        std::vector<uint32_t> constantIndexBuffer = engineModLoader.sceneMeshes[0].indices;
        std::vector<std::vector<uint32_t>> instanceIndexBuffer; 
        instanceIndexBuffer.resize(engineModLoader.instanceNumber, std::vector<uint32_t>(0));
        for (size_t i = 0; i < engineModLoader.instanceNumber; i++)
        {
            instanceIndexBuffer[i].reserve(indexMaxSize);
        }
        
        std::vector<Phoenix::OutputData> selectedMeshlet;
        selectedMeshlet.resize(phoenixMesh.totalMeshlets.size() * engineModLoader.instanceNumber);
        engineRenderer.InitialDispatchCompute(phoenixMesh.totalMeshlets.size(), selectedMeshlet.data());


        //GAME LOOP
        while (!glfwWindowShouldClose(windowInstance.window)) 
        {
            engineModLoader.sceneMeshes[0].indices.clear();
            engineRenderer.UpdateUniformBuffer(engineRenderer.currentFrame);
            int numberOfVertex = 0;

            glfwPollEvents();
            if(engineModLoader.sceneMeshes[0].typeOfMesh == Mesh::MeshType::Skeletal)
                animator.UpdateAnimation(camera.deltaTime);
            camera.ProcessUserInput(windowInstance.window);

            int indexOffset = 0;
            int vertexOffset = 0;
            if(engineRenderer.renderMode == Mode::Phoenix)
            {
                //LOD selection per-meshlet 
                engineRenderer.DispatchCompute(phoenixMesh.totalMeshlets.size());
                int currentVertexCount = 0;
                int currentTriangleCount = 0;
                
                
                for(int i = 0; i < selectedMeshlet[0].index; i++)
                {
                    
                    int instanceIndex = selectedMeshlet[i].instanceNumber;
                    const Phoenix::PhoenixMeshlet* currentPMeshlet = &phoenixMesh.totalMeshlets[selectedMeshlet[i].ID]; 
                    instanceIndexBuffer[instanceIndex].insert(instanceIndexBuffer[instanceIndex].end(), 
                    currentPMeshlet->meshletIndexBuffer.begin(), currentPMeshlet->meshletIndexBuffer.end()); 

                    currentVertexCount += currentPMeshlet->meshletData.vertex_count;
                    currentTriangleCount += currentPMeshlet->meshletData.triangle_count;
                    
                }
                
                for (size_t i = 0; i < engineModLoader.instanceNumber; i++)
                {
                    std::vector<uint32_t>* currentIndex = &instanceIndexBuffer[i];
                    VkDrawIndexedIndirectCommand* currentIndirectCommand = &engineRenderer.indirectCommands[i];
                    if(currentIndex->size() > indexMaxSize)
                        currentIndex->resize(indexMaxSize);
                    //Indirect data update     
                    currentIndirectCommand->firstIndex = indexOffset;
                    currentIndirectCommand->vertexOffset = vertexOffset;
                    currentIndirectCommand->indexCount = static_cast<uint32_t>(currentIndex->size());
                    indexOffset += static_cast<uint32_t>(currentIndex->size());
                    vertexOffset += static_cast<uint32_t>(singleVertexSize);
                    
                    engineModLoader.sceneMeshes[0].indices.insert(engineModLoader.sceneMeshes[0].indices.end(), 
                    currentIndex->begin(), currentIndex->end());

                    engineModLoader.info.numberOfPolygons += (currentIndex->size() / 3);

                    
                    currentIndex->clear();
   
                }
                //UI scene info update
                engineModLoader.info.numberOfVertices = currentVertexCount;
                engineModLoader.info.numberOfPolygons = currentTriangleCount;
                engineRenderer.DrawFrame();

                selectedMeshlet[0].index = 0;  
                memcpy(engineRenderer.OutputMappedSSBO[engineRenderer.currentComputeFrame], 
                selectedMeshlet.data(), sizeof(int));

                VkDeviceSize outBufferSize = (sizeof(Phoenix::OutputData) * phoenixMesh.totalMeshlets.size()) 
                * engineModLoader.instanceNumber;

                vkWaitForFences(engineDevice.logicalDevice, 1, &engineRenderer.computeInFlightFences[
                (engineRenderer.currentComputeFrame + 1) % engineRenderer.MAX_FRAMES_IN_FLIGHT], VK_TRUE, UINT64_MAX); 

                memcpy(selectedMeshlet.data(), engineRenderer.OutputMappedSSBO[(engineRenderer.currentComputeFrame + 1)
                % engineRenderer.MAX_FRAMES_IN_FLIGHT], outBufferSize);
                
                engineRenderer.currentComputeFrame = (engineRenderer.currentComputeFrame + 1) % 
                engineRenderer.MAX_FRAMES_IN_FLIGHT;
                engineModLoader.info.numberOfVertices = 0;
                engineModLoader.info.numberOfPolygons = 0;
                
            }
            else
            {
                for(int i = 0; i < engineModLoader.instanceNumber; i++)
                {  
                            
                    //Indirect data update     
                    engineRenderer.indirectCommands[i].firstIndex = indexOffset;
                    engineRenderer.indirectCommands[i].vertexOffset = vertexOffset;
                    engineRenderer.indirectCommands[i].indexCount = static_cast<uint32_t>(constantIndexBuffer.size());
                    indexOffset += static_cast<uint32_t>(constantIndexBuffer.size());
                    vertexOffset += static_cast<uint32_t>(singleVertexSize);
                    
                    engineModLoader.sceneMeshes[0].indices.insert(engineModLoader.sceneMeshes[0].indices.end(), 
                    constantIndexBuffer.begin(), constantIndexBuffer.end());
                    
                }
                engineRenderer.DrawFrame();
            }
            
            
            
        }
        std::cout << "Avg framerate: " << engineUI.sumFramerates / engineUI.frame << "\n";
        vkDeviceWaitIdle(engineDevice.logicalDevice);

        
    }

    
}

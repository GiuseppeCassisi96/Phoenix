#include "EngineStartup.h"
#include <iostream>
#include "Phoenix/PhoenixMesh.h"
#include "Phoenix/LODSelectionDispatcher.h"
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
    }

    void EngineStartup::Start()
    {
        samplesTest["0"].modelName = "submarine.fbx";
        samplesTest["0"].textureName = "submarineColor.png";
        samplesTest["0"].scale = 500.0f;
        samplesTest["0"].rowDim = 5;
        samplesTest["0"].distanceMultiplier = 600.0f;
        samplesTest["0"].tError = 0.5f;

        samplesTest["1"].modelName = "dancer.obj";
        samplesTest["1"].textureName = "dancerColor.jpg";
        samplesTest["1"].scale = 1.0f;
        samplesTest["1"].rowDim = 20;
        samplesTest["1"].distanceMultiplier = 1000.0f;
        samplesTest["1"].tError = 1.0f;

        samplesTest["2"].modelName = "teapot.fbx";
        samplesTest["2"].textureName = "teepotColor.png";
        samplesTest["2"].scale = 20.0f;
        samplesTest["2"].rowDim = 20;
        samplesTest["2"].distanceMultiplier = 600.0f;
        samplesTest["2"].tError = 0.2f;

        samplesTest["3"].animNumber = 3;
        samplesTest["3"].animName.emplace_back("monsterIdle.fbx");
        samplesTest["3"].animName.emplace_back("monsterWalk.fbx");
        samplesTest["3"].animName.emplace_back("monsterRun.fbx");
        samplesTest["3"].modelName = "monster.fbx";
        samplesTest["3"].textureName = "monsterColor.png";
        samplesTest["3"].scale = 1.0f;
        samplesTest["3"].rowDim = 40;
        samplesTest["3"].distanceMultiplier = 400.0f;
        samplesTest["3"].tError = 10.5f;
   
        std::string key;

        std::cout << "Choose the model which you want rendered: \n"
        << "Insert '0' to render the submarine static model\n"
        << "Insert '1' to render the dancer static model\n"
        << "Insert '2' to render the teapot static model\n"
        << "Insert '3' to render the skeletal model\n" ;
        std::cin >> key;
        assert(key == "1" || key == "0" || key == "2" || key == "3");
        std::cout << "Select the instance number: ";
        std::cin >> engineModLoader.instanceNumber;

        choosenSample = samplesTest[key];

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
        
        
        auto startTime = std::chrono::high_resolution_clock::now();
        phoenixMesh.BuildLodsHierarchy(engineModLoader.sceneMeshes[0].vertices, engineModLoader.sceneMeshes[0].indices);
        auto endTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> amount = endTime - startTime;
        std::cout << "S: " << amount << "\n";

        dispatcher.PrepareComputeData(phoenixMesh.totalMeshlets, choosenSample);
        engineRenderer.PrepareIndirectData(engineModLoader.sceneMeshes[0].indices, engineModLoader.sceneMeshes[0].vertices);
        engineModLoader.PrepareInstanceData(choosenSample);
        engineRenderer.CreateVertexBuffer();
        engineRenderer.CreateInstanceBuffer();
        engineRenderer.CreateIndexBuffer();
        CreateUniformBuffers<UniformBufferObject>(engineRenderer.transformationUBuffers);
        CreateUniformBuffers<BoneMatricesUniformType>(engineRenderer.animUBuffers);
        engineRenderer.CreateDescriptorPool();
        engineRenderer.CreateDescriptorSets();
        engineRenderer.CreateCommandBuffer();
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
        size_t indexMaxSize = engineModLoader.sceneMeshes[0].indices.size();
        std::vector<Mesh::Vertex> instanceVertexBuffer = engineModLoader.sceneMeshes[0].vertices;
        
        while (!glfwWindowShouldClose(windowInstance.window)) 
        {
            engineModLoader.sceneMeshes[0].indices.clear();
            engineModLoader.sceneMeshes[0].vertices.clear();
            glfwPollEvents();
            if(engineModLoader.sceneMeshes[0].typeOfMesh == Mesh::MeshType::Skeletal)
                animator.UpdateAnimation(camera.deltaTime);
            camera.ProcessUserInput(windowInstance.window);
            std::vector<size_t> multiOffset;
            multiOffset.emplace_back(0);
            int indexOffset = 0;
            int vertexOffset = 0;
            for(int i = 0; i < engineModLoader.instanceNumber; i++)
            {   
                std::vector<uint32_t> instanceIndexBuffer;
                int numberOfVertex = 0;
                //LOD selection LOD per meshlet 
                instanceIndexBuffer = dispatcher.LodSelector(
                phoenixMesh.totalMeshlets, windowInstance.WIDTH, glm::radians(45.0f),
                engineModLoader.instancesData[i].instancePos,avgLOD, instanceVertexBuffer, 
                engineTransform,phoenixMesh, numberOfVertex);
                if(instanceIndexBuffer.size() > indexMaxSize)
                    instanceIndexBuffer.resize(indexMaxSize);

                //Indirect data update     
                engineRenderer.indirectCommands[i].firstIndex = indexOffset;
                engineRenderer.indirectCommands[i].vertexOffset = vertexOffset;
                engineRenderer.indirectCommands[i].indexCount = static_cast<uint32_t>(instanceIndexBuffer.size());
                indexOffset += static_cast<uint32_t>(instanceIndexBuffer.size());
                vertexOffset += static_cast<uint32_t>(instanceVertexBuffer.size());

                //Global Index Buffer computation 
                engineModLoader.sceneMeshes[0].indices.insert(engineModLoader.sceneMeshes[0].indices.end(), 
                instanceIndexBuffer.begin(), instanceIndexBuffer.end());

                //Global Vertex Buffer computation 
                engineModLoader.sceneMeshes[0].vertices.insert(engineModLoader.sceneMeshes[0].vertices.end(), 
                instanceVertexBuffer.begin(), instanceVertexBuffer.end());
                
                //UI scene info update
                engineModLoader.info.numberOfVertices += numberOfVertex;
                engineModLoader.info.numberOfPolygons += instanceIndexBuffer.size() / static_cast<size_t>(3);
            }
            engineRenderer.DrawFrame();
            
            engineModLoader.info.numberOfVertices = 0;
            engineModLoader.info.numberOfPolygons = 0;
            avgLOD = 0.0f;
            
        }
        vkDeviceWaitIdle(engineDevice.logicalDevice);
    }

    
}

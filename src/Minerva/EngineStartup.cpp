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
        samplesTest["0"].modelName = "bunny.obj";
        samplesTest["0"].textureName = "statueColor.jpeg";
        samplesTest["0"].scale = 130.0f;
        samplesTest["0"].rowDim = 20;
        samplesTest["0"].distanceMultiplier = 30.0f;

        samplesTest["1"].modelName = "lucy.obj";
        samplesTest["1"].textureName = "statueColor.jpeg";
        samplesTest["1"].scale = 0.13f;
        samplesTest["1"].rowDim = 20;
        samplesTest["1"].distanceMultiplier = 60.0f;

        samplesTest["2"].modelName = "happy.obj";
        samplesTest["2"].textureName = "statueColor.jpeg";
        samplesTest["2"].scale = 300.0f;
        samplesTest["2"].rowDim = 20;
        samplesTest["2"].distanceMultiplier = 60.0f;

        samplesTest["3"].animNumber = 3;
        samplesTest["3"].animName.emplace_back("monsterIdle.fbx");
        samplesTest["3"].animName.emplace_back("monsterWalk.fbx");
        samplesTest["3"].animName.emplace_back("monsterRun.fbx");
        samplesTest["3"].modelName = "monster.fbx";
        samplesTest["3"].textureName = "monsterColor.png";
        samplesTest["3"].scale = 0.2f;
        samplesTest["3"].rowDim = 40;
        samplesTest["3"].distanceMultiplier = 45.0f;

        

         
        std::string key;
        int lodSelected = 0;

        std::cout << "Choose the model which you want rendered: \n"
        << "Insert '0' to render the bunny static model\n"
        << "Insert '1' to render the lucy static model\n"
        << "Insert '2' to render the happy budda static model\n"
        << "Insert '3' to render the skeletal model\n" ;
        std::cin >> key;
        assert(key == "1" || key == "0" || key == "2" || key == "3");
        std::cout << "Select the instance number: ";
        std::cin >> engineModLoader.instanceNumber;
        std::cout << "Select the LOD between 0 and 5: ";
        assert(lodSelected >= 0 && lodSelected <= 5);
        std::cin >> lodSelected;

        SampleType choosenSample = samplesTest[key];

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
        
        engineModLoader.LoadModel(choosenSample.modelName);

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
        Phoenix::PhoenixMesh phoenixMesh; 
        Phoenix::LODSelectionDispatcher dispatcher;
        auto startTime = std::chrono::high_resolution_clock::now();
        phoenixMesh.BuildLodsHierarchy(engineModLoader.sceneMeshes[0].vertices, 
        engineModLoader.sceneMeshes[0].indices);
        auto endTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> amount = endTime - startTime;
        std::cout << "S: " << amount << "\n";
        
        engineModLoader.sceneMeshes[0].indices = phoenixMesh.lods[lodSelected].lodIndexBuffer;
        for(auto group : phoenixMesh.lods[lodSelected].groups)
        {
            phoenixMesh.ColourMeshelets(group,engineModLoader.sceneMeshes[0].vertices);
        }

        engineModLoader.info.numberOfVertices =  phoenixMesh.lods[lodSelected].vertexNumber.size();
        dispatcher.PrepareComputeData(phoenixMesh.lods);
        engineRenderer.PrepareIndirectData(phoenixMesh.lods[lodSelected]);
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
        
        while (!glfwWindowShouldClose(windowInstance.window)) {
            
            glfwPollEvents();
            if(engineModLoader.sceneMeshes[0].typeOfMesh == Mesh::MeshType::Skeletal)
                animator.UpdateAnimation(camera.deltaTime);
            camera.ProcessUserInput(windowInstance.window);
            engineRenderer.DrawFrame();
            
            
        }
        vkDeviceWaitIdle(engineDevice.logicalDevice);
    }

    
}

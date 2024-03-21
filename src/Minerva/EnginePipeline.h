#pragma once
#include <vector>
#include <string>
#include "vulkan/vulkan.h"

namespace Minerva
{
    class EnginePipeline
    {
        
    public:
        VkPipeline graphicsPipeline;
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        /// @brief Creates the graphics pipeline
        /// @param vertShaderName The name of vertex shader
        /// @param fragShaderName The name of fragment shader
        void CreatePipeline(const std::string& vertShaderName, const std::string& fragShaderName);
        ~EnginePipeline();
    private:     
        const std::string SHADERS_PATH = "C:/UNIMI/TESI/Phoenix/src/Minerva/Shaders/";
        const std::string FILE_TYPE = ".spv";
        

        /// @brief Reads the compiled shader file 
        /// @param filename The path of the file 
        /// @return The shader code
        static std::vector<char> ReadFile(const std::string& filename);
        /// @brief Creates a shader module using the shader code
        /// @return The VkShaderModule handle 
        VkShaderModule CreateShaderModule(const std::vector<char>& code);
        
    };
}
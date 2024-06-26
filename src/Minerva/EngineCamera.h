#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

namespace Minerva
{
    struct UniformBufferObject 
    {
        glm::mat4 model {1.0f};
        glm::mat4 view {1.0f};
        glm::mat4 proj {1.0f};
    };
    class Transformation
    {
        public:
            UniformBufferObject ubo {};
            void Move(const glm::vec3& dir, glm::mat4 model = glm::mat4(1.0f));
            void Scale(const glm::vec3& dim, glm::mat4 model = glm::mat4(1.0f));
            void Rotate(const float& angle, const glm::vec3& axis, glm::mat4 model = glm::mat4(1.0f));
    };

    class EngineCamera
    {
    public:
        glm::vec3 cameraPos;
        glm::vec3 cameraRight, cameraForward, cameraUp;
        //glm::vec3 cameraDir;
        float deltaTime, lastFrame;
        bool firstMouse = true;
        double yaw = -90.0f, pitch = 0.0f;
        double lastX = 400.0f, lastY = 300.0f;
        void SetupViewMatrix(glm::mat4& viewMatrix);
        void ProcessUserInput(GLFWwindow *window);
        void UpdateViewMatrix(glm::mat4& viewMatrix);
        void MouseCallback(GLFWwindow* window, double xpos, double ypos);
    };
    
}
#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

namespace Minerva
{
    class EngineCamera
    {
    public:
        glm::vec3 cameraPos;
        glm::vec3 cameraRight, cameraForward, cameraUp;
        glm::vec3 cameraDir;
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
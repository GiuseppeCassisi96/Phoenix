#include "EngineCamera.h"
#include <chrono>
namespace Minerva
{
    void EngineCamera::SetupViewMatrix(glm::mat4 &viewMatrix)
    {
        cameraPos = glm::vec3(0.0f, 3.0f, 0.0f);
        cameraForward = glm::vec3(0.0f, -1.0f, 0.0f);
        cameraUp = glm::vec3(0.0f, 0.0f, 1.0f);

        viewMatrix = glm::lookAt(cameraPos, cameraPos + cameraForward, cameraUp);
    }
    void EngineCamera::ProcessUserInput(GLFWwindow *window)
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;  
        const float cameraSpeed = 0.6f; // adjust accordingly

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            cameraPos += cameraSpeed * cameraForward * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            cameraPos -= cameraSpeed * cameraForward * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            cameraPos -= glm::normalize(glm::cross(cameraForward, cameraUp)) * cameraSpeed * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            cameraPos += glm::normalize(glm::cross(cameraForward, cameraUp)) * cameraSpeed * deltaTime;
        }
    void EngineCamera::UpdateViewMatrix(glm::mat4 &viewMatrix)
    {
        viewMatrix = glm::lookAt(cameraPos, cameraPos + cameraForward, cameraUp);
    }
    void EngineCamera::MouseCallback(GLFWwindow *window, double xpos, double ypos)
    {
        if (firstMouse)
        {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }
    
        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos; 
        lastX = xpos;
        lastY = ypos;

        float sensitivity = 0.1f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        yaw   += xoffset;
        pitch += yoffset;

        if(pitch > 89.0f)
            pitch = 89.0f;
        if(pitch < -89.0f)
            pitch = -89.0f;

        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.z = sin(glm::radians(pitch));
        direction.x *= -1.0f;
        cameraForward = glm::normalize(direction);
    }
}
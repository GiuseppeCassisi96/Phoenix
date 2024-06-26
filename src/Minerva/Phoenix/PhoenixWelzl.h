#pragma once
#include "Minerva/Mesh.h"
#include <vector>

namespace Phoenix
{
    struct PhoenixBound
    {
        glm::vec3 center;
        float radius;
    };
 
    class PhoenixWelzl
    {
    public:
     
        PhoenixBound ExecuteWelzl(std::vector<glm::vec3> &points, std::vector<glm::vec3 > rPoints, 
        int vertexCount);
        PhoenixWelzl() = default;
        ~PhoenixWelzl() = default;
    private:
        float Distance(const glm::vec3 & p1, const glm::vec3 & p2);
        bool IsInside(const PhoenixBound& bound, const glm::vec3 & point);
        PhoenixBound ComputeBoundFrom3(const glm::vec3 & p1, const glm::vec3 & p2, const glm::vec3 & p3); 
        PhoenixBound ComputeBoundFrom2(const glm::vec3 & p1, const glm::vec3 & p2);
        bool IsValidCircle(const PhoenixBound& bound, const std::vector<glm::vec3 >& points);
        PhoenixBound MinimumCircle(std::vector<glm::vec3>& points);
    };
    
}
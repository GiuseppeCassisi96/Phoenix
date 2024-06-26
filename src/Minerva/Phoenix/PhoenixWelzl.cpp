#include "PhoenixWelzl.h"
#include "glm/gtx/norm.hpp"
#include <iostream>
//https://www.geeksforgeeks.org/minimum-enclosing-circle-using-welzls-algorithm/
namespace Phoenix
{
    float PhoenixWelzl::Distance(const glm::vec3 &p1, const glm::vec3 &p2)
    {
        return glm::distance2(p1, p2);
    }
    bool PhoenixWelzl::IsInside(const PhoenixBound &bound, const glm::vec3 &point)
    {
        return Distance(bound.center, point) <= bound.radius;
    }
    PhoenixBound PhoenixWelzl::ComputeBoundFrom2(const glm::vec3 &p1, const glm::vec3 &p2)
    {
        PhoenixBound bound;
        bound.center = (p1 + p2) / 2.0f;
        bound.radius = Distance(p1, p2) / 2.0f;
 
        return bound;
    }
    bool PhoenixWelzl::IsValidCircle(const PhoenixBound &bound, const std::vector<glm::vec3> &points)
    {
        for(auto point : points)
        {
            if(!IsInside(bound, point))
            {
                return false;
            }
        }
        return true;
    }
    PhoenixBound PhoenixWelzl::MinimumCircle(std::vector<glm::vec3> &points)
    {
        assert(points.size() <= 3);
        if(points.empty())
        {
            glm::vec3 point {0.0f};
            return PhoenixBound{point, 0};
        }
        else if(points.size() == 1)
        {   
            return PhoenixBound{points[0], 0.0f};
        }
        else if(points.size() == 2)
        {
            return ComputeBoundFrom2(points[0], points[1]);
        }

        for(int i = 0; i < 3; i++)
        {
            for(int j = i; j < 3; j++)
            {
                PhoenixBound bound = ComputeBoundFrom2(points[i], points[j]);
                if(IsValidCircle(bound, points))
                {
                    return bound;
                }
            }
        }
        return ComputeBoundFrom3(points[0], points[1], points[2]);
    }
    PhoenixBound PhoenixWelzl::ExecuteWelzl(std::vector<glm::vec3> &points, std::vector<glm::vec3> rPoints, 
    int vertexCount)
    {
        if(vertexCount == 0 || rPoints.size() == 3)
        {
            return MinimumCircle(rPoints);
        }
        assert(vertexCount > 0);
        int index = rand() % vertexCount;
        glm::vec3 point = points[index];
        std::swap(points[index], points[vertexCount - 1]);

        PhoenixBound bound = ExecuteWelzl(points, rPoints, vertexCount-1);
        if(IsInside(bound, point))
        {
            return bound;
        }
        rPoints.emplace_back(point);

        return ExecuteWelzl(points, rPoints, vertexCount-1);
    }
    //https://stackoverflow.com/questions/13977354/build-circle-from-3-points-in-3d-space-implementation-in-c-or-c
    PhoenixBound PhoenixWelzl::ComputeBoundFrom3(const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3)
    {
        glm::vec3 edge1 = p2 - p1; 
        glm::vec3 edge2 = p3 - p1; 
        glm::vec3 edge3 = p3 - p2; 

        glm::vec3 up = glm::cross(edge1, edge2);
        float upSquareLen = glm::dot(up, up);

        float upSquareLen2 = 1.0 / (2.0 * upSquareLen);
        float edge1SquareLen = glm::dot(edge1, edge1);
        float edge2SquareLen = glm::dot(edge2, edge2);
        float edge3SquareLen = glm::dot(edge3, edge3);

        PhoenixBound bound;
        bound.center = (p1 + p2 + p3) / 3.0f;
        bound.radius = sqrt(edge1SquareLen * edge2SquareLen * edge3SquareLen * upSquareLen2 * 0.5f);

        return bound;
    }
}
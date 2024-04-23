#pragma once
#include "vector"
#include "Meshoptimizer/src/meshoptimizer.h"
#include "Minerva/Mesh.h"

namespace Phoenix
{
    struct PhoenixMeshlet
    {
        meshopt_Meshlet meshlet;
        std::vector<Minerva::Mesh::Vertex> meshletVertices;
    };
    
    class PhoenixMesh
    {
    private:
    public:
        std::vector<PhoenixMeshlet> phoenixMeshlets;
        void SimplifyMeshlet(std::vector<Minerva::Mesh::Vertex>& vertices,  
        std::vector<unsigned int>& indicesMeshlet);
        void MeshletGeneration(std::vector<Minerva::Mesh::Vertex>& vertices,  
        std::vector<unsigned int> &indices);
        void ColourTheMesh(std::vector<meshopt_Meshlet>& meshlets, 
        std::vector<Minerva::Mesh::Vertex>& vertices, std::vector<unsigned int>& meshlet_vertices);
        PhoenixMesh() = default;
        ~PhoenixMesh() = default;
    };
}



#include "PhoenixMesh.h"
#include <iostream>
#include <random>
namespace Phoenix
{
    void PhoenixMesh::SimplifyMeshlet(std::vector<Minerva::Mesh::Vertex>& vertices, 
    std::vector<unsigned int>& indicesMeshlet)
    {
        float threshold = 0.2f;
        size_t target_index_count = size_t(indicesMeshlet.size() * threshold);
        float target_error = 0.2f;

        //                             OUT INDICES            IN INDICES
        auto count = meshopt_simplify(indicesMeshlet.data(), indicesMeshlet.data(), indicesMeshlet.size(), 
        &vertices[0].pos.x, vertices.size(), 
        sizeof(Minerva::Mesh::Vertex), target_index_count, target_error);

        indicesMeshlet.resize(count);
    }
    void PhoenixMesh::MeshletGeneration(std::vector<Minerva::Mesh::Vertex> &vertices, 
    std::vector<unsigned int> &indices)
    {
        const size_t max_vertices = 64;
        const size_t max_triangles = 128;
        const float cone_weight = 0.5f;

        size_t max_meshlets = meshopt_buildMeshletsBound(indices.size(), max_vertices, max_triangles);
        std::vector<meshopt_Meshlet> meshlets(max_meshlets);
        std::vector<unsigned int> meshlet_vertices(max_meshlets * max_vertices);
        std::vector<unsigned char> meshlet_triangles(max_meshlets * max_triangles * 3);
        std::cout<< "Max meshlets: " << max_meshlets << "\n";

        auto meshletCount = meshopt_buildMeshlets(meshlets.data(),meshlet_vertices.data(), 
        meshlet_triangles.data(), indices.data(), indices.size(),&vertices[0].pos.x,vertices.size(), 
        sizeof(Minerva::Mesh::Vertex),max_vertices,max_triangles, 1.0f);
        std::cout<<"END\n";
        const meshopt_Meshlet& last = meshlets[meshletCount - 1];
        meshlet_vertices.resize(last.vertex_offset + last.vertex_count);
        meshlet_triangles.resize(last.triangle_offset + ((last.triangle_count * 3 + 3) & ~3));
        meshlets.resize(meshletCount);
        phoenixMeshlets.reserve(meshletCount);
        ColourTheMesh(meshlets,vertices, meshlet_vertices);
    }
    void PhoenixMesh::ColourTheMesh(std::vector<meshopt_Meshlet>& meshlets, 
    std::vector<Minerva::Mesh::Vertex> &vertices, std::vector<unsigned int>& meshlet_vertices)
    {
        
        std::random_device rd;
        std::mt19937 gen(rd()); 

        
        float min = 0.0; 
        float max = 1.0; 
        std::uniform_real_distribution<float> dist(min, max);
        int offset = 0;
        for(int i = 0; i < meshlets.size(); i++)
        {
            PhoenixMeshlet currentMeshlet;
            offset = meshlets[i].vertex_offset;
            float r = dist(gen);
            float g = dist(gen);
            float b = dist(gen);
            currentMeshlet.meshletVertices.reserve(meshlets[i].vertex_count);
            for(int j = offset; j < meshlets[i].vertex_count + offset; j++)
            {
                currentMeshlet.meshletVertices.emplace_back(vertices[meshlet_vertices[j]]);
                vertices[meshlet_vertices[j]].color = glm::vec3(r, g, b);
            }
            phoenixMeshlets.emplace_back(currentMeshlet);
        }

    }
}

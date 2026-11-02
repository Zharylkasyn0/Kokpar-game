#pragma once

#include <vector>

#include <glad/glad.h>
#include <glm/glm.hpp>

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
};

// Immutable GPU mesh (VAO + VBO + EBO). Move-only. Requires a current GL context.
class Mesh
{
public:
    Mesh() = default;
    Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
    ~Mesh();

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;

    void draw() const;
    bool valid() const { return vao_ != 0; }

private:
    void release();

    GLuint vao_ = 0;
    GLuint vbo_ = 0;
    GLuint ebo_ = 0;
    GLsizei indexCount_ = 0;
};

// ---- Procedural primitives (all centered at the origin) ------------------
// Unit sphere: radius 1.
Mesh MakeSphereMesh(int stacks, int slices);
// Capsule along the Y axis: total length = cylinderLength + 2 * radius.
Mesh MakeCapsuleMesh(float radius, float cylinderLength, int slices, int capRings);
// Cylinder along Y: radius 1, height 1.
Mesh MakeCylinderMesh(int slices);
// Cone along Y: base radius 1 at y = -0.5, apex at y = +0.5.
Mesh MakeConeMesh(int slices);
// Unit cube: edge length 1, flat normals.
Mesh MakeBoxMesh();

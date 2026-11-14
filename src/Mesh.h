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
// Tapered capsule along Y: top cap radius rTop, bottom cap radius rBottom, middle height.
Mesh MakeTaperedCapsuleMesh(float rBottom, float rTop, float height, int slices = 20, int capRings = 6);
// 5-point customizable leg segment along Y: top, upper-mid, center, lower-mid, bottom radii, and segment length.
Mesh MakeCustomLegMesh(float rTop, float rUpperMid, float rCenter, float rLowerMid, float rBottom,
                       float length, int slices = 20, int capRings = 6);
// Torus centered in XZ plane: major radius (ring), minor radius (tube thickness).
Mesh MakeTorusMesh(float majorR, float minorR, int majorSlices = 24, int minorSlices = 12);
// Cohesive horse torso: anatomical tube from rump to sternal keel, in horse-root space.
Mesh MakeHorseBodyMesh();
// Cohesive horse neck: arched muscular cervical column from withers/chest to poll.
Mesh MakeHorseNeckMesh();
// Cohesive horse head: skull from poll to muzzle tip, local +Y toward muzzle, +Z = dorsal.
Mesh MakeHorseHeadMesh();
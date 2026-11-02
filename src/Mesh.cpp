#include "Mesh.h"

#include <cmath>
#include <cstdint>
#include <utility>

#include "Math.h"

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices)
{
    indexCount_ = static_cast<GLsizei>(indices.size());

    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glGenBuffers(1, &ebo_);

    glBindVertexArray(vao_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)),
                 vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(indices.size() * sizeof(unsigned int)),
                 indices.data(), GL_STATIC_DRAW);

    // location 0: position, location 1: normal (tightly packed 2 x vec3)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(static_cast<uintptr_t>(sizeof(glm::vec3))));

    glBindVertexArray(0);
}

Mesh::~Mesh() { release(); }

Mesh::Mesh(Mesh&& other) noexcept
    : vao_(other.vao_), vbo_(other.vbo_), ebo_(other.ebo_), indexCount_(other.indexCount_)
{
    other.vao_ = other.vbo_ = other.ebo_ = 0;
    other.indexCount_ = 0;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept
{
    if (this != &other)
    {
        release();
        vao_ = other.vao_;
        vbo_ = other.vbo_;
        ebo_ = other.ebo_;
        indexCount_ = other.indexCount_;
        other.vao_ = other.vbo_ = other.ebo_ = 0;
        other.indexCount_ = 0;
    }
    return *this;
}

void Mesh::release()
{
    if (ebo_ != 0) glDeleteBuffers(1, &ebo_);
    if (vbo_ != 0) glDeleteBuffers(1, &vbo_);
    if (vao_ != 0) glDeleteVertexArrays(1, &vao_);
    vao_ = vbo_ = ebo_ = 0;
    indexCount_ = 0;
}

void Mesh::draw() const
{
    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLES, indexCount_, GL_UNSIGNED_INT, nullptr);
}

// ---------------------------------------------------------------------------
// Procedural geometry
// ---------------------------------------------------------------------------
namespace
{
// A "ring" of a surface of revolution around the Y axis.
// Normal at angle phi is (nr*cos(phi), ny, nr*sin(phi)).
struct Ring
{
    float y;
    float r;
    float nr;
    float ny;
};

Mesh BuildRevolved(const std::vector<Ring>& rings, int slices)
{
    std::vector<Vertex> vertices;
    vertices.reserve(rings.size() * static_cast<size_t>(slices + 1));

    for (const Ring& ring : rings)
    {
        for (int j = 0; j <= slices; ++j)
        {
            const float phi = kTwoPi * static_cast<float>(j) / static_cast<float>(slices);
            const float c = std::cos(phi);
            const float s = std::sin(phi);
            Vertex v;
            v.position = glm::vec3(ring.r * c, ring.y, ring.r * s);
            v.normal = glm::vec3(ring.nr * c, ring.ny, ring.nr * s);
            vertices.push_back(v);
        }
    }

    std::vector<unsigned int> indices;
    const unsigned int stride = static_cast<unsigned int>(slices + 1);
    for (size_t i = 0; i + 1 < rings.size(); ++i)
    {
        for (int j = 0; j < slices; ++j)
        {
            const unsigned int a = static_cast<unsigned int>(i) * stride + static_cast<unsigned int>(j);
            const unsigned int b = a + stride;
            indices.push_back(a);
            indices.push_back(a + 1);
            indices.push_back(b);
            indices.push_back(a + 1);
            indices.push_back(b + 1);
            indices.push_back(b);
        }
    }
    return Mesh(vertices, indices);
}
}

Mesh MakeSphereMesh(int stacks, int slices)
{
    std::vector<Ring> rings;
    for (int i = 0; i <= stacks; ++i)
    {
        const float theta = kPi * static_cast<float>(i) / static_cast<float>(stacks);
        rings.push_back({std::cos(theta), std::sin(theta), std::sin(theta), std::cos(theta)});
    }
    return BuildRevolved(rings, slices);
}

Mesh MakeCapsuleMesh(float radius, float cylinderLength, int slices, int capRings)
{
    const float half = cylinderLength * 0.5f;
    std::vector<Ring> rings;

    // Top hemisphere: pole -> equator.
    for (int i = 0; i <= capRings; ++i)
    {
        const float theta = kHalfPi * static_cast<float>(i) / static_cast<float>(capRings);
        rings.push_back({radius * std::cos(theta) + half, radius * std::sin(theta),
                         std::sin(theta), std::cos(theta)});
    }
    // Bottom hemisphere: equator -> pole. The jump between the two equator
    // rings forms the cylindrical band.
    for (int i = 0; i <= capRings; ++i)
    {
        const float theta = kHalfPi + kHalfPi * static_cast<float>(i) / static_cast<float>(capRings);
        rings.push_back({radius * std::cos(theta) - half, radius * std::sin(theta),
                         std::sin(theta), std::cos(theta)});
    }
    return BuildRevolved(rings, slices);
}

Mesh MakeCylinderMesh(int slices)
{
    const std::vector<Ring> rings = {
        {+0.5f, 0.0f, 0.0f, +1.0f},   // top cap centre
        {+0.5f, 1.0f, 0.0f, +1.0f},   // top cap rim (up normal)
        {+0.5f, 1.0f, 1.0f, 0.0f},    // side top (side normal)
        {-0.5f, 1.0f, 1.0f, 0.0f},    // side bottom
        {-0.5f, 1.0f, 0.0f, -1.0f},   // bottom cap rim (down normal)
        {-0.5f, 0.0f, 0.0f, -1.0f},   // bottom cap centre
    };
    return BuildRevolved(rings, slices);
}

Mesh MakeConeMesh(int slices)
{
    // Side normal of a cone with radius 1 and height 1 is (1, 1) / sqrt(2) in (radial, y).
    const float k = 0.70710678f;
    const std::vector<Ring> rings = {
        {+0.5f, 0.0f, k, k},          // apex
        {-0.5f, 1.0f, k, k},          // base rim (side normal)
        {-0.5f, 1.0f, 0.0f, -1.0f},   // base rim (down normal)
        {-0.5f, 0.0f, 0.0f, -1.0f},   // base centre
    };
    return BuildRevolved(rings, slices);
}

Mesh MakeBoxMesh()
{
    const glm::vec3 normals[6] = {
        glm::vec3(1, 0, 0), glm::vec3(-1, 0, 0),
        glm::vec3(0, 1, 0), glm::vec3(0, -1, 0),
        glm::vec3(0, 0, 1), glm::vec3(0, 0, -1),
    };

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    for (int f = 0; f < 6; ++f)
    {
        const glm::vec3 n = normals[f];
        const glm::vec3 helper = (std::abs(n.y) > 0.5f) ? glm::vec3(1, 0, 0) : glm::vec3(0, 1, 0);
        const glm::vec3 u = glm::normalize(glm::cross(n, helper));
        const glm::vec3 v = glm::cross(n, u);

        const unsigned int base = static_cast<unsigned int>(vertices.size());
        const glm::vec3 corners[4] = {
            n * 0.5f + (-u - v) * 0.5f,
            n * 0.5f + (u - v) * 0.5f,
            n * 0.5f + (u + v) * 0.5f,
            n * 0.5f + (-u + v) * 0.5f,
        };
        for (const glm::vec3& c : corners)
        {
            Vertex vert;
            vert.position = c;
            vert.normal = n;
            vertices.push_back(vert);
        }
        const unsigned int quad[6] = {0, 1, 2, 0, 2, 3};
        for (unsigned int q : quad) indices.push_back(base + q);
    }
    return Mesh(vertices, indices);
}

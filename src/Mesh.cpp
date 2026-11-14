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

Mesh MakeTaperedCapsuleMesh(float rBottom, float rTop, float height, int slices, int capRings)
{
    const float half = height * 0.5f;
    const float dr = rBottom - rTop;
    const float dy = height;
    const float hyp = std::sqrt(dr * dr + dy * dy);
    const float sideNr = (hyp > 0.0001f) ? (dy / hyp) : 1.0f;
    const float sideNy = (hyp > 0.0001f) ? (dr / hyp) : 0.0f;

    std::vector<Ring> rings;

    // Top hemisphere: pole -> equator
    for (int i = 0; i <= capRings; ++i)
    {
        const float theta = kHalfPi * static_cast<float>(i) / static_cast<float>(capRings);
        rings.push_back({ rTop * std::cos(theta) + half, rTop * std::sin(theta),
                         std::sin(theta), std::cos(theta) });
    }

    // Intermediate ring on the conical side
    rings.push_back({ 0.0f, (rTop + rBottom) * 0.5f, sideNr, sideNy });

    // Bottom hemisphere: equator -> pole
    for (int i = 0; i <= capRings; ++i)
    {
        const float theta = kHalfPi + kHalfPi * static_cast<float>(i) / static_cast<float>(capRings);
        rings.push_back({ rBottom * std::cos(theta) - half, rBottom * std::sin(theta),
                         std::sin(theta), std::cos(theta) });
    }

    return BuildRevolved(rings, slices);
}

Mesh MakeCustomLegMesh(float rTop, float rUpperMid, float rCenter, float rLowerMid, float rBottom,
                       float length, int slices, int capRings)
{
    const float safeLen = std::max(length, 0.001f);
    const float half = safeLen * 0.5f;

    // Ensure non-negative radii
    rTop = std::max(rTop, 0.0005f);
    rUpperMid = std::max(rUpperMid, 0.0005f);
    rCenter = std::max(rCenter, 0.0005f);
    rLowerMid = std::max(rLowerMid, 0.0005f);
    rBottom = std::max(rBottom, 0.0005f);

    const float R[5] = { rTop, rUpperMid, rCenter, rLowerMid, rBottom };

    std::vector<Ring> rings;

    // 1. Top hemisphere cap: pole (+Y) down to equator (y = +half)
    // Avoid duplicate equator ring: run from i = 0 to capRings - 1
    for (int i = 0; i < capRings; ++i)
    {
        const float theta = kHalfPi * static_cast<float>(i) / static_cast<float>(capRings);
        rings.push_back({ rTop * std::cos(theta) + half, rTop * std::sin(theta),
                          std::sin(theta), std::cos(theta) });
    }

    // 2. 5-point leg shaft: Catmull-Rom spline interpolation through the 5 radii
    constexpr int kSubStepsPerSeg = 3;
    constexpr int kTotalSteps = 4 * kSubStepsPerSeg;

    for (int step = 0; step <= kTotalSteps; ++step)
    {
        const float s = 4.0f * static_cast<float>(step) / static_cast<float>(kTotalSteps);
        const int seg = std::clamp(static_cast<int>(s), 0, 3);
        const float u = s - static_cast<float>(seg);

        const float p0 = (seg == 0) ? (2.0f * R[0] - R[1]) : R[seg - 1];
        const float p1 = R[seg];
        const float p2 = R[seg + 1];
        const float p3 = (seg >= 2) ? (2.0f * R[4] - R[3]) : R[seg + 2];

        // Catmull-Rom cubic evaluation
        const float r = 0.5f * ((2.0f * p1) +
                                (-p0 + p2) * u +
                                (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * (u * u) +
                                (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * (u * u * u));

        // Derivative dr/du
        const float dr_du = 0.5f * ((-p0 + p2) +
                                    2.0f * (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * u +
                                    3.0f * (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * (u * u));

        // Position y along leg shaft
        const float y = half * (1.0f - 0.5f * s);
        const float dy_du = -half * 0.5f;

        // Outward normal in (r, y) plane: tangent T = (dr_du, dy_du), normal N = (-dy_du, dr_du) normalized
        const float hyp = std::sqrt(dr_du * dr_du + dy_du * dy_du);
        const float nr = (hyp > 0.0001f) ? (-dy_du / hyp) : 1.0f;
        const float ny = (hyp > 0.0001f) ? (dr_du / hyp) : 0.0f;

        rings.push_back({ y, std::max(r, 0.0005f), nr, ny });
    }

    // 3. Bottom hemisphere cap: equator (y = -half) down to bottom pole (-Y)
    // Avoid duplicate equator ring: run from i = 1 to capRings
    for (int i = 1; i <= capRings; ++i)
    {
        const float theta = kHalfPi + kHalfPi * static_cast<float>(i) / static_cast<float>(capRings);
        rings.push_back({ rBottom * std::cos(theta) - half, rBottom * std::sin(theta),
                          std::sin(theta), std::cos(theta) });
    }

    return BuildRevolved(rings, slices);
}

Mesh MakeTorusMesh(float majorR, float minorR, int majorSlices, int minorSlices)
{
    std::vector<Vertex> vertices;
    vertices.reserve(static_cast<size_t>((majorSlices + 1) * (minorSlices + 1)));

    for (int i = 0; i <= majorSlices; ++i)
    {
        const float phi = kTwoPi * static_cast<float>(i) / static_cast<float>(majorSlices);
        const float cosPhi = std::cos(phi);
        const float sinPhi = std::sin(phi);

        for (int j = 0; j <= minorSlices; ++j)
        {
            const float psi = kTwoPi * static_cast<float>(j) / static_cast<float>(minorSlices);
            const float cosPsi = std::cos(psi);
            const float sinPsi = std::sin(psi);

            Vertex v;
            v.position = glm::vec3((majorR + minorR * cosPsi) * cosPhi,
                minorR * sinPsi,
                (majorR + minorR * cosPsi) * sinPhi);
            v.normal = glm::vec3(cosPsi * cosPhi, sinPsi, cosPsi * sinPhi);
            vertices.push_back(v);
        }
    }

    std::vector<unsigned int> indices;
    const unsigned int stride = static_cast<unsigned int>(minorSlices + 1);
    for (int i = 0; i < majorSlices; ++i)
    {
        for (int j = 0; j < minorSlices; ++j)
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

Mesh MakeHorseBodyMesh()
{
    struct BodyStn {
        float z;        // position along horse Z axis
        float yDorsal;  // spine / withers height (+Y)
        float yVentral; // belly / chest underline height (-Y)
        float rW;       // dorsal (top spine) half-width
        float rBW;      // barrel / rib (mid-height) half-width
    };

    // --- Stations: rump → sternal keel (broad, deep, barrel-chested horse) ---
    static const BodyStn kS[] = {
        // z        yUH      yBH         rW      rBW
        {-0.80f,    1.54f,   1.02f,    0.270f, 0.180f}, // 0  tail dock / buttocks
        {-0.60f,    1.62f,   0.93f,    0.260f, 0.280f}, // 1  rounded croup & haunches
        {-0.4f,    1.60f,   0.88f,    0.245f, 0.270f}, // 2  loin & flank tuck
        {-0.24f,    1.58f,   0.82f,    0.260f, 0.300f}, // 3  saddle dip & rear barrel
        { 0.06f,    1.55f,   0.78f,    0.250f, 0.315f}, // 4  mid barrel (well-sprung ribs)
        { 0.22f,    1.52f,   0.76f,    0.220f, 0.285f}, // 5  withers & deep girth
        { 0.46f,    1.60f,   0.78f,    0.230f, 0.255f}, // 6  shoulder & chest
        { 0.68f,    1.60f,   0.84f,    0.080f, 0.30f}, // 7  pectoral front
        { 0.79f,    1.56f,   1.0f,     0.080f,   0.250f}, // 8  breast / neck root
    };
    constexpr int kNS = 9;
    constexpr int kSl = 14;

    auto bodyPos = [&](int si, float phi) -> glm::vec3 {
        const int i = std::clamp(si, 0, kNS - 1);
        const BodyStn& s = kS[i];
        const float c = std::cos(phi);
        const float sn = std::sin(phi);
        const float top = 0.5f + 0.5f * sn;          // 1 at spine, 0 at belly
        const float rX = s.rW * top + s.rBW * (1.0f - top);
        const float yCenter = 0.5f * (s.yDorsal + s.yVentral);
        const float rY = 0.5f * (s.yDorsal - s.yVentral);
        return glm::vec3(rX * c, yCenter + rY * sn, s.z);
    };

    std::vector<Vertex> verts;
    verts.reserve((kSl + 1) * kNS + 2);

    const float dphi = kTwoPi / static_cast<float>(kSl * 10);

    for (int si = 0; si < kNS; ++si)
    {
        for (int j = 0; j <= kSl; ++j)
        {
            const float phi = kTwoPi * static_cast<float>(j) / static_cast<float>(kSl);
            const glm::vec3 P = bodyPos(si, phi);

            const glm::vec3 Tc = glm::normalize(bodyPos(si, phi + dphi) - bodyPos(si, phi - dphi));

            glm::vec3 Tl;
            if (si == 0)      Tl = bodyPos(1, phi) - bodyPos(0, phi);
            else if (si == kNS - 1)  Tl = bodyPos(kNS - 1, phi) - bodyPos(kNS - 2, phi);
            else                   Tl = bodyPos(si + 1, phi) - bodyPos(si - 1, phi);
            Tl = glm::normalize(Tl);

            glm::vec3 N = glm::normalize(glm::cross(Tc, Tl));
            const float yCenter = 0.5f * (kS[si].yDorsal + kS[si].yVentral);
            const glm::vec3 toP(P.x, P.y - yCenter, 0.0f);
            if (glm::dot(N, toP) < 0.0f) N = -N;

            verts.push_back({ P, N });
        }
    }

    std::vector<unsigned int> idx;
    idx.reserve(kSl * (kNS - 1) * 6 + kSl * 6);
    const unsigned int stride = static_cast<unsigned int>(kSl + 1);
    for (int si = 0; si < kNS - 1; ++si)
    {
        for (int j = 0; j < kSl; ++j)
        {
            const unsigned int a = static_cast<unsigned int>(si) * stride + static_cast<unsigned int>(j);
            const unsigned int b = a + stride;
            idx.push_back(a);   idx.push_back(a + 1); idx.push_back(b);
            idx.push_back(a + 1); idx.push_back(b + 1); idx.push_back(b);
        }
    }

    // Watertight end caps: close rump (rear) and chest front
    const unsigned int rumpIdx = static_cast<unsigned int>(verts.size());
    const float yCenterRump = 0.5f * (kS[0].yDorsal + kS[0].yVentral);
    verts.push_back({ glm::vec3(0.0f, yCenterRump, kS[0].z - 0.04f), glm::vec3(0.0f, 0.0f, -1.0f) });
    for (int j = 0; j < kSl; ++j)
    {
        idx.push_back(rumpIdx);
        idx.push_back(static_cast<unsigned int>(j + 1));
        idx.push_back(static_cast<unsigned int>(j));
    }

    const unsigned int chestIdx = static_cast<unsigned int>(verts.size());
    const float yCenterChest = 0.5f * (kS[kNS - 1].yDorsal + kS[kNS - 1].yVentral);
    const glm::vec3 frontN = glm::normalize(glm::vec3(0.0f, -0.2f, 0.98f));
    verts.push_back({ glm::vec3(0.0f, yCenterChest, kS[kNS - 1].z + 0.04f), frontN });
    const unsigned int frontRingBase = static_cast<unsigned int>(kNS - 1) * stride;
    for (int j = 0; j < kSl; ++j)
    {
        idx.push_back(chestIdx);
        idx.push_back(frontRingBase + static_cast<unsigned int>(j));
        idx.push_back(frontRingBase + static_cast<unsigned int>(j + 1));
    }

    return Mesh(verts, idx);
}

Mesh MakeHorseNeckMesh()
{
    struct NeckStn {
        float y;      // position along neck axis
        float zc;     // center offset along Z (+Z = toward crest)
        float rX;     // lateral half-width
        float rZc;    // crest dorsal half-depth (+Z)
        float rZt;    // throat ventral half-depth (-Z)
    };

    static const NeckStn kN[] = {
        // y       zc     width      down     up
        {0.00f, -0.030f, 0.160f, 0.250f, 0.250f},
        {0.20f, -0.030f, 0.160f, 0.250f, 0.18f},
        {0.400f, -0.030f, 0.120f, 0.210f, 0.180f},
        {0.600f, -0.030f, 0.10f, 0.125f, 0.20f},
        {0.700f, -0.030f, 0.10f, 0.125f, 0.18f},
        {0.80f, -0.030f, 0.050f, 0.00f, 0.15f},
    };
    constexpr int kNN = 6;
    constexpr int kSl = 12;

    auto neckPos = [&](int si, float phi) -> glm::vec3 {
        const int i = std::clamp(si, 0, kNN - 1);
        const NeckStn& n = kN[i];
        const float c = std::cos(phi);
        const float sn = std::sin(phi);
        const float crestMix = 0.5f + 0.5f * c; // 1 at crest (+Z), 0 at throat (-Z)
        const float rZ = n.rZc * crestMix + n.rZt * (1.0f - crestMix);
        return glm::vec3(n.rX * sn, n.y, n.zc + rZ * c);
    };

    std::vector<Vertex> verts;
    verts.reserve((kSl + 1) * kNN + 2);

    const float dphi = kTwoPi / static_cast<float>(kSl * 10);

    for (int si = 0; si < kNN; ++si)
    {
        for (int j = 0; j <= kSl; ++j)
        {
            const float phi = kTwoPi * static_cast<float>(j) / static_cast<float>(kSl);
            const glm::vec3 P = neckPos(si, phi);

            const glm::vec3 Tc = glm::normalize(neckPos(si, phi + dphi) - neckPos(si, phi - dphi));

            glm::vec3 Tl;
            if (si == 0)      Tl = neckPos(1, phi) - neckPos(0, phi);
            else if (si == kNN - 1)  Tl = neckPos(kNN - 1, phi) - neckPos(kNN - 2, phi);
            else                   Tl = neckPos(si + 1, phi) - neckPos(si - 1, phi);
            Tl = glm::normalize(Tl);

            glm::vec3 N = glm::normalize(glm::cross(Tc, Tl));
            const glm::vec3 toP(P.x, 0.0f, P.z - kN[si].zc);
            if (glm::dot(N, toP) < 0.0f) N = -N;

            verts.push_back({ P, N });
        }
    }

    std::vector<unsigned int> idx;
    idx.reserve(kSl * (kNN - 1) * 6 + kSl * 6);
    const unsigned int stride = static_cast<unsigned int>(kSl + 1);
    for (int si = 0; si < kNN - 1; ++si)
    {
        for (int j = 0; j < kSl; ++j)
        {
            const unsigned int a = static_cast<unsigned int>(si) * stride + static_cast<unsigned int>(j);
            const unsigned int b = a + stride;
            idx.push_back(a);   idx.push_back(a + 1); idx.push_back(b);
            idx.push_back(a + 1); idx.push_back(b + 1); idx.push_back(b);
        }
    }

    // Watertight end caps: base (Y=0) and poll (Y=0.44)
    const unsigned int baseIdx = static_cast<unsigned int>(verts.size());
    verts.push_back({ glm::vec3(0.0f, -0.02f, kN[0].zc), glm::vec3(0.0f, -1.0f, 0.0f) });
    for (int j = 0; j < kSl; ++j)
    {
        idx.push_back(baseIdx);
        idx.push_back(static_cast<unsigned int>(j));
        idx.push_back(static_cast<unsigned int>(j + 1));
    }

    const unsigned int topIdx = static_cast<unsigned int>(verts.size());
    verts.push_back({ glm::vec3(0.0f, kN[kNN - 1].y + 0.02f, kN[kNN - 1].zc), glm::vec3(0.0f, 1.0f, 0.0f) });
    const unsigned int topRingBase = static_cast<unsigned int>(kNN - 1) * stride;
    for (int j = 0; j < kSl; ++j)
    {
        idx.push_back(topIdx);
        idx.push_back(topRingBase + static_cast<unsigned int>(j + 1));
        idx.push_back(topRingBase + static_cast<unsigned int>(j));
    }

    return Mesh(verts, idx);
}

Mesh MakeHorseHeadMesh()
{
    struct HeadStn {
        float y;        // position along head axis (0=poll, 0.52=muzzle)
        float rX;       // lateral half-width
        float zDorsal;  // front / forehead / nasal bridge (+Z)
        float zVentral; // jaw / throat underside (-Z)
    };

    static const HeadStn kH[] = {
        {0.000f, 0.100f, 0.100f, -0.090f},
        {0.080f, 0.140f, 0.210f, -0.190f},
        {0.200f, 0.140f, 0.210f, -0.190f},
        {0.350f, 0.080f, 0.130f, -0.115f},
        {0.520f, 0.065f, 0.065f, -0.065f},
        {0.540f, 0.025f, 0.025f, -0.025f},
        {0.545f, 0.001f, 0.001f, -0.001f},
    };
    constexpr int kNH = static_cast<int>(sizeof(kH) / sizeof(kH[0]));
    constexpr int kSl = 12;

    auto headPos = [&](int si, float phi) -> glm::vec3 {
        const int i = std::clamp(si, 0, kNH - 1);
        const HeadStn& h = kH[i];
        const float c = std::cos(phi);
        const float sn = std::sin(phi);
        const float zCenter = 0.5f * (h.zDorsal + h.zVentral);
        const float rZ = 0.5f * (h.zDorsal - h.zVentral);
        return glm::vec3(h.rX * sn, h.y, zCenter + rZ * c);
    };

    std::vector<Vertex> verts;
    verts.reserve((kSl + 1) * kNH + 2);

    const float dphi = kTwoPi / static_cast<float>(kSl * 10);

    for (int si = 0; si < kNH; ++si)
    {
        for (int j = 0; j <= kSl; ++j)
        {
            const float phi = kTwoPi * static_cast<float>(j) / static_cast<float>(kSl);
            const glm::vec3 P = headPos(si, phi);

            const glm::vec3 Tc = glm::normalize(headPos(si, phi + dphi) - headPos(si, phi - dphi));

            glm::vec3 Tl;
            if (si == 0)      Tl = headPos(1, phi) - headPos(0, phi);
            else if (si == kNH - 1)  Tl = headPos(kNH - 1, phi) - headPos(kNH - 2, phi);
            else                   Tl = headPos(si + 1, phi) - headPos(si - 1, phi);
            Tl = glm::normalize(Tl);

            glm::vec3 N = glm::normalize(glm::cross(Tc, Tl));
            const float zCenter = 0.5f * (kH[si].zDorsal + kH[si].zVentral);
            const glm::vec3 toP(P.x, 0.0f, P.z - zCenter);
            if (glm::dot(N, toP) < 0.0f) N = -N;

            verts.push_back({ P, N });
        }
    }

    std::vector<unsigned int> idx;
    idx.reserve(kSl * (kNH - 1) * 6 + kSl * 6);
    const unsigned int stride = static_cast<unsigned int>(kSl + 1);
    for (int si = 0; si < kNH - 1; ++si)
    {
        for (int j = 0; j < kSl; ++j)
        {
            const unsigned int a = static_cast<unsigned int>(si) * stride + static_cast<unsigned int>(j);
            const unsigned int b = a + stride;
            idx.push_back(a);   idx.push_back(a + 1); idx.push_back(b);
            idx.push_back(a + 1); idx.push_back(b + 1); idx.push_back(b);
        }
    }

    // Watertight end caps: close poll (rear) and muzzle tip
    const unsigned int pollIdx = static_cast<unsigned int>(verts.size());
    const float zCenterPoll = 0.5f * (kH[0].zDorsal + kH[0].zVentral);
    verts.push_back({ glm::vec3(0.0f, -0.015f, zCenterPoll), glm::vec3(0.0f, -1.0f, 0.0f) });
    for (int j = 0; j < kSl; ++j)
    {
        idx.push_back(pollIdx);
        idx.push_back(static_cast<unsigned int>(j));
        idx.push_back(static_cast<unsigned int>(j + 1));
    }

    const unsigned int muzzleIdx = static_cast<unsigned int>(verts.size());
    const float zCenterMuzzle = 0.5f * (kH[kNH - 1].zDorsal + kH[kNH - 1].zVentral);
    verts.push_back({ glm::vec3(0.0f, kH[kNH - 1].y + 0.015f, zCenterMuzzle), glm::vec3(0.0f, 1.0f, 0.0f) });
    const unsigned int muzzleRingBase = static_cast<unsigned int>(kNH - 1) * stride;
    for (int j = 0; j < kSl; ++j)
    {
        idx.push_back(muzzleIdx);
        idx.push_back(muzzleRingBase + static_cast<unsigned int>(j + 1));
        idx.push_back(muzzleRingBase + static_cast<unsigned int>(j));
    }

    return Mesh(verts, idx);
}

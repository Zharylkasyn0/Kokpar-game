#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "Assets.h"
#include "Locations.h"
#include "Renderer.h"

struct Obstacle
{
    enum class Type { Rock, Yurt, Tree, Fence, Spire };

    Type type = Type::Rock;
    glm::vec3 position = glm::vec3(0.0f);
    float radius = 1.0f;   // collision radius in the XZ plane
    float yaw = 0.0f;
    float scale = 1.0f;
    glm::vec3 color = glm::vec3(1.0f);
};

struct ScoreZone
{
    glm::vec3 center = glm::vec3(0.0f);
    float radius = 5.0f;

    bool contains(const glm::vec3& p) const
    {
        const float dx = p.x - center.x;
        const float dz = p.z - center.z;
        return dx * dx + dz * dz <= radius * radius;
    }
};

// Procedural steppe environments for all 8 Kazakhstan locations:
// ground, arena turf, track, obstacles, vegetation tufts, score zone (Kazanlyk),
// and unique perimeter landmark scenery.
class World
{
public:
    void init(const Assets& assets);
    void setLocation(LocationId id);
    LocationId currentLocation() const { return locationId_; }

    void update(float dt) { time_ += dt; }
    void draw(Renderer& renderer) const;

    // Circle-vs-world test in the XZ plane.
    glm::vec2 resolveCircle(const glm::vec3& center, float radius) const;

    const ScoreZone& zone() const { return zone_; }
    float arenaRadius() const { return arenaRadius_; }

    void setZoneHighlight(bool on) { zoneHighlight_ = on; }

private:
    struct Tuft
    {
        glm::vec3 position;
        float height;
        float phase;
        glm::vec3 color;
    };

    void buildLocationWorld(LocationId id);

    void drawZone(Renderer& r) const;
    void drawYurt(Renderer& r, const Obstacle& o) const;
    void drawRock(Renderer& r, const Obstacle& o) const;
    void drawTree(Renderer& r, const Obstacle& o) const;
    void drawFence(Renderer& r, const Obstacle& o) const;
    void drawSpire(Renderer& r, const Obstacle& o) const;

    // Location-specific unique perimeter scenery
    void drawCharynCanyonPerimeter(Renderer& r) const;
    void drawAlatauPerimeter(Renderer& r) const;
    void drawBozzhyraPerimeter(Renderer& r) const;
    void drawBurabayPerimeter(Renderer& r) const;
    void drawKolsaiPerimeter(Renderer& r) const;
    void drawUstyurtPerimeter(Renderer& r) const;
    void drawKatonKaragayPerimeter(Renderer& r) const;
    void drawQazaqAuylPerimeter(Renderer& r) const;

    void drawBoundaryPosts(Renderer& r) const;

    const Assets* assets_ = nullptr;
    LocationId locationId_ = LocationId::QazaqAuyl;

    std::vector<Obstacle> obstacles_;
    std::vector<Tuft> tufts_;
    ScoreZone zone_;
    float arenaRadius_ = 60.0f;
    float time_ = 0.0f;
    bool zoneHighlight_ = false;

    // Cached location palette
    glm::vec3 groundColor_ = glm::vec3(0.56f, 0.62f, 0.28f);
    glm::vec3 arenaColor_ = glm::vec3(0.65f, 0.72f, 0.34f);
    glm::vec3 trackColor_ = glm::vec3(0.68f, 0.60f, 0.42f);
    glm::vec3 boundaryColor_ = glm::vec3(0.00f, 0.64f, 0.82f);
};

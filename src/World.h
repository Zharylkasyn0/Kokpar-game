#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "Assets.h"
#include "Renderer.h"

struct Obstacle
{
    enum class Type { Rock, Yurt };

    Type type = Type::Rock;
    glm::vec3 position = glm::vec3(0.0f);
    float radius = 1.0f;   // collision radius in the XZ plane
    float yaw = 0.0f;
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

// Static steppe environment: ground, arena, obstacles (collision circles), score zone, decoration.
class World
{
public:
    void init(const Assets& assets);
    void update(float dt) { time_ += dt; }
    void draw(Renderer& renderer) const;

    // Circle-vs-world test in the XZ plane. Returns the push-out vector (x, z) that
    // moves the circle out of every obstacle and back inside the arena. (0, 0) = no contact.
    glm::vec2 resolveCircle(const glm::vec3& center, float radius) const;

    const ScoreZone& zone() const { return zone_; }
    float arenaRadius() const { return arenaRadius_; }

    // Makes the score zone pulse when the player is carrying the goat.
    void setZoneHighlight(bool on) { zoneHighlight_ = on; }

private:
    struct Tuft
    {
        glm::vec3 position;
        float height;
        float phase;
        glm::vec3 color;
    };

    void drawZone(Renderer& r) const;
    void drawYurt(Renderer& r, const Obstacle& o) const;
    void drawRock(Renderer& r, const Obstacle& o) const;
    void drawMountains(Renderer& r) const;
    void drawBoundaryPosts(Renderer& r) const;

    const Assets* assets_ = nullptr;
    std::vector<Obstacle> obstacles_;
    std::vector<Tuft> tufts_;
    ScoreZone zone_;
    float arenaRadius_ = 60.0f;
    float time_ = 0.0f;
    bool zoneHighlight_ = false;
};

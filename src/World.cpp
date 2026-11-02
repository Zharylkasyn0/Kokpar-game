#include "World.h"

#include <cmath>

#include "Math.h"

namespace
{
const glm::vec3 kGroundColor(0.58f, 0.64f, 0.30f);
const glm::vec3 kArenaColor(0.68f, 0.74f, 0.36f);
const glm::vec3 kRockColor(0.58f, 0.58f, 0.62f);
const glm::vec3 kFeltColor(0.94f, 0.91f, 0.82f);
const glm::vec3 kKazakhBlue(0.00f, 0.62f, 0.80f);
const glm::vec3 kGold(1.00f, 0.80f, 0.15f);
}

void World::init(const Assets& assets)
{
    assets_ = &assets;

    zone_.center = glm::vec3(0.0f, 0.0f, 40.0f);
    zone_.radius = 5.0f;

    auto addObstacle = [this](Obstacle::Type type, float x, float z, float radius, float yaw) {
        Obstacle o;
        o.type = type;
        o.position = glm::vec3(x, 0.0f, z);
        o.radius = radius;
        o.yaw = yaw;
        obstacles_.push_back(o);
    };

    // Yurts (big collision circles)
    addObstacle(Obstacle::Type::Yurt, -22.0f, 18.0f, 3.3f, 0.6f);
    addObstacle(Obstacle::Type::Yurt, 24.0f, 6.0f, 3.3f, 2.4f);
    addObstacle(Obstacle::Type::Yurt, -30.0f, -18.0f, 3.3f, 1.1f);

    // Rocks
    addObstacle(Obstacle::Type::Rock, -8.0f, -6.0f, 1.3f, 0.3f);
    addObstacle(Obstacle::Type::Rock, 10.0f, -14.0f, 1.0f, 1.2f);
    addObstacle(Obstacle::Type::Rock, 14.0f, 18.0f, 1.5f, 2.2f);
    addObstacle(Obstacle::Type::Rock, -14.0f, 6.0f, 1.1f, 0.9f);
    addObstacle(Obstacle::Type::Rock, 30.0f, -25.0f, 1.6f, 0.1f);
    addObstacle(Obstacle::Type::Rock, -35.0f, 5.0f, 1.4f, 1.7f);
    addObstacle(Obstacle::Type::Rock, 5.0f, 10.0f, 0.9f, 2.8f);
    addObstacle(Obstacle::Type::Rock, -4.0f, 30.0f, 1.2f, 0.5f);

    // Grass tufts: deterministic scatter, avoiding obstacles and the score zone.
    Rng rng(1337u);
    while (tufts_.size() < 140)
    {
        const float angle = rng.range(0.0f, kTwoPi);
        const float dist = rng.range(3.0f, arenaRadius_ + 25.0f);
        const glm::vec3 p(std::cos(angle) * dist, 0.0f, std::sin(angle) * dist);

        bool blocked = zone_.contains(glm::vec3(p.x, 0.0f, p.z)) ||
                       glm::length(glm::vec2(p.x - zone_.center.x, p.z - zone_.center.z)) < zone_.radius + 1.0f;
        for (const Obstacle& o : obstacles_)
        {
            if (glm::length(glm::vec2(p.x - o.position.x, p.z - o.position.z)) < o.radius + 0.6f) blocked = true;
        }
        if (blocked) continue;

        Tuft t;
        t.position = p;
        t.height = rng.range(0.5f, 1.1f);
        t.phase = rng.range(0.0f, kTwoPi);
        const float shade = rng.range(0.85f, 1.15f);
        t.color = glm::vec3(0.36f, 0.58f, 0.22f) * shade;
        tufts_.push_back(t);
    }
}

glm::vec2 World::resolveCircle(const glm::vec3& center, float radius) const
{
    glm::vec2 push(0.0f);
    const glm::vec2 c(center.x, center.z);

    for (const Obstacle& o : obstacles_)
    {
        const glm::vec2 d = c - glm::vec2(o.position.x, o.position.z);
        const float dist = glm::length(d);
        const float minDist = o.radius + radius;
        if (dist < minDist)
        {
            const glm::vec2 n = (dist > 1e-4f) ? d / dist : glm::vec2(1.0f, 0.0f);
            push += n * (minDist - dist);
        }
    }

    // Arena boundary (circle).
    const float limit = arenaRadius_ - radius;
    const float cd = glm::length(c);
    if (cd > limit && cd > 1e-4f)
    {
        push -= (c / cd) * (cd - limit);
    }
    return push;
}

void World::draw(Renderer& r) const
{
    const Assets& A = *assets_;

    // Ground + arena disc
    r.draw(A.box(), Translate(0.0f, -0.5f, 0.0f) * Scale(900.0f, 1.0f, 900.0f), kGroundColor);
    r.draw(A.cylinder(), Translate(0.0f, 0.0f, 0.0f) * Scale(arenaRadius_ + 2.0f, 0.02f, arenaRadius_ + 2.0f), kArenaColor);

    drawZone(r);

    for (const Obstacle& o : obstacles_)
    {
        if (o.type == Obstacle::Type::Yurt) drawYurt(r, o);
        else drawRock(r, o);
    }

    // Swaying grass tufts
    for (const Tuft& t : tufts_)
    {
        const float sway = std::sin(time_ * 1.8f + t.phase) * 0.14f;
        const glm::mat4 m = Translate(t.position) * RotZ(sway) * RotX(sway * 0.6f) *
                            Translate(0.0f, t.height * 0.5f, 0.0f) * Scale(0.22f, t.height, 0.22f);
        r.draw(A.cone(), m, t.color);
    }

    drawBoundaryPosts(r);
    drawMountains(r);
}

void World::drawZone(Renderer& r) const
{
    const Assets& A = *assets_;
    const glm::vec3 c = zone_.center;

    float glow = 1.0f;
    if (zoneHighlight_) glow = 1.0f + 0.25f * std::sin(time_ * 6.0f);

    const glm::vec3 outer = glm::vec3(0.86f, 0.32f, 0.20f) * glow;
    const glm::vec3 inner = glm::vec3(0.96f, 0.86f, 0.55f) * glow;
    const glm::vec3 core = glm::vec3(0.80f, 0.20f, 0.15f) * glow;

    const float pulse = 1.0f + (zoneHighlight_ ? 0.06f * std::sin(time_ * 6.0f) : 0.0f);

    r.draw(A.cylinder(), Translate(c.x, 0.015f, c.z) * Scale(zone_.radius, 0.02f, zone_.radius), outer);
    r.draw(A.cylinder(), Translate(c.x, 0.025f, c.z) * Scale(zone_.radius * 0.84f * pulse, 0.02f, zone_.radius * 0.84f * pulse), inner);
    r.draw(A.cylinder(), Translate(c.x, 0.035f, c.z) * Scale(zone_.radius * 0.32f, 0.02f, zone_.radius * 0.32f), core);

    // Four flag poles with waving pennants.
    for (int i = 0; i < 4; ++i)
    {
        const float a = kPi * 0.25f + kHalfPi * static_cast<float>(i);
        const float px = c.x + std::cos(a) * (zone_.radius + 0.4f);
        const float pz = c.z + std::sin(a) * (zone_.radius + 0.4f);
        r.draw(A.cylinder(), Translate(px, 1.6f, pz) * Scale(0.08f, 3.2f, 0.08f), glm::vec3(0.45f, 0.30f, 0.18f));
        const float wave = std::sin(time_ * 3.0f + static_cast<float>(i) * 1.3f) * 0.35f;
        const glm::vec3 flagColor = (i % 2 == 0) ? kKazakhBlue : kGold;
        r.draw(A.box(), Translate(px, 2.85f, pz) * RotY(wave + a) * Translate(0.45f, 0.0f, 0.0f) * Scale(0.9f, 0.5f, 0.05f), flagColor);
        r.draw(A.sphere(), Translate(px, 3.25f, pz) * Scale(0.12f), kGold);
    }
}

void World::drawYurt(Renderer& r, const Obstacle& o) const
{
    const Assets& A = *assets_;
    const glm::vec3 p = o.position;

    r.draw(A.cylinder(), Translate(p.x, 1.1f, p.z) * Scale(3.0f, 2.2f, 3.0f), kFeltColor);
    r.draw(A.cylinder(), Translate(p.x, 1.5f, p.z) * Scale(3.03f, 0.30f, 3.03f), kKazakhBlue);
    r.draw(A.cone(), Translate(p.x, 3.05f, p.z) * Scale(3.5f, 1.7f, 3.5f), glm::vec3(0.86f, 0.78f, 0.62f));
    r.draw(A.sphere(), Translate(p.x, 3.95f, p.z) * Scale(0.25f), kGold);
    r.draw(A.box(), Translate(p.x, 0.85f, p.z) * RotY(o.yaw) * Translate(0.0f, 0.0f, 3.0f) * Scale(1.2f, 1.7f, 0.18f),
           glm::vec3(0.50f, 0.25f, 0.12f));
}

void World::drawRock(Renderer& r, const Obstacle& o) const
{
    const Assets& A = *assets_;
    const float s = o.radius * 1.05f;
    r.draw(A.sphere(), Translate(o.position.x, s * 0.55f, o.position.z) * RotY(o.yaw) * Scale(s, s * 0.72f, s * 0.88f), kRockColor);
}

void World::drawMountains(Renderer& r) const
{
    const Assets& A = *assets_;
    const int count = 12;
    for (int i = 0; i < count; ++i)
    {
        const float fi = static_cast<float>(i);
        const float angle = kTwoPi * fi / static_cast<float>(count) + std::sin(fi * 7.0f) * 0.2f;
        const float dist = 190.0f + 20.0f * std::sin(fi * 3.1f);
        const float height = 40.0f + 25.0f * std::abs(std::sin(fi * 1.7f));
        const float rad = 55.0f + 20.0f * std::abs(std::sin(fi * 2.3f));
        const float x = std::cos(angle) * dist;
        const float z = std::sin(angle) * dist;
        r.draw(A.cone(), Translate(x, height * 0.5f, z) * Scale(rad, height, rad), glm::vec3(0.45f, 0.52f, 0.68f));
        r.draw(A.cone(), Translate(x, height * 0.85f, z) * Scale(rad * 0.3f, height * 0.3f, rad * 0.3f), glm::vec3(0.95f, 0.96f, 1.0f));
    }
}

void World::drawBoundaryPosts(Renderer& r) const
{
    const Assets& A = *assets_;
    const int count = 36;
    for (int i = 0; i < count; ++i)
    {
        const float a = kTwoPi * static_cast<float>(i) / static_cast<float>(count);
        const float x = std::cos(a) * (arenaRadius_ + 0.5f);
        const float z = std::sin(a) * (arenaRadius_ + 0.5f);
        r.draw(A.cylinder(), Translate(x, 0.6f, z) * Scale(0.16f, 1.2f, 0.16f), glm::vec3(0.50f, 0.34f, 0.20f));
        const glm::vec3 cap = (i % 2 == 0) ? kKazakhBlue : kGold;
        r.draw(A.sphere(), Translate(x, 1.3f, z) * Scale(0.22f), cap);
    }
}

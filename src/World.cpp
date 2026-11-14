#include "World.h"

#include <cmath>

#include "Math.h"

namespace
{
const glm::vec3 kGroundColor(0.56f, 0.62f, 0.28f);       // Expansive steppe grass
const glm::vec3 kArenaColor(0.65f, 0.72f, 0.34f);        // Inside arena playing turf
const glm::vec3 kDirtTrackColor(0.68f, 0.60f, 0.42f);    // Trodden perimeter gallop trail
const glm::vec3 kRockColor(0.54f, 0.55f, 0.58f);         // Steppe granite boulder
const glm::vec3 kRockDark(0.42f, 0.43f, 0.46f);          // Secondary shaded rock
const glm::vec3 kFeltColor(0.94f, 0.91f, 0.82f);         // Natural sheep-wool felt
const glm::vec3 kFeltWarm(0.88f, 0.82f, 0.68f);          // Weathered roof felt
const glm::vec3 kKazakhBlue(0.00f, 0.64f, 0.82f);        // Traditional turquoise
const glm::vec3 kGold(1.00f, 0.80f, 0.15f);              // Steppe sun gold
const glm::vec3 kWood(0.45f, 0.26f, 0.14f);              // Carved birch / pine wood
const glm::vec3 kRuby(0.85f, 0.22f, 0.18f);              // Traditional crimson
}

void World::init(const Assets& assets)
{
    assets_ = &assets;

    zone_.center = glm::vec3(0.0f, 0.0f, 40.0f);
    zone_.radius = 5.2f;

    auto addObstacle = [this](Obstacle::Type type, float x, float z, float radius, float yaw) {
        Obstacle o;
        o.type = type;
        o.position = glm::vec3(x, 0.0f, z);
        o.radius = radius;
        o.yaw = yaw;
        obstacles_.push_back(o);
    };

    // Traditional felt Yurts (spaced strategically across the steppe arena)
    addObstacle(Obstacle::Type::Yurt, -22.0f, 18.0f, 3.4f, 0.6f);
    addObstacle(Obstacle::Type::Yurt, 24.0f, 6.0f, 3.4f, 2.4f);
    addObstacle(Obstacle::Type::Yurt, -30.0f, -18.0f, 3.4f, 1.1f);

    // Steppe rock clusters
    addObstacle(Obstacle::Type::Rock, -8.0f, -6.0f, 1.35f, 0.3f);
    addObstacle(Obstacle::Type::Rock, 10.0f, -14.0f, 1.1f, 1.2f);
    addObstacle(Obstacle::Type::Rock, 14.0f, 18.0f, 1.55f, 2.2f);
    addObstacle(Obstacle::Type::Rock, -14.0f, 6.0f, 1.15f, 0.9f);
    addObstacle(Obstacle::Type::Rock, 30.0f, -25.0f, 1.65f, 0.1f);
    addObstacle(Obstacle::Type::Rock, -35.0f, 5.0f, 1.45f, 1.7f);
    addObstacle(Obstacle::Type::Rock, 5.0f, 10.0f, 0.95f, 2.8f);
    addObstacle(Obstacle::Type::Rock, -4.0f, 30.0f, 1.25f, 0.5f);

    // Feather grass tufts (Stipa / Селеу): deterministic scatter with varied heights and wind phases
    Rng rng(1337u);
    while (tufts_.size() < 160)
    {
        const float angle = rng.range(0.0f, kTwoPi);
        const float dist = rng.range(3.0f, arenaRadius_ + 28.0f);
        const glm::vec3 p(std::cos(angle) * dist, 0.0f, std::sin(angle) * dist);

        bool blocked = zone_.contains(glm::vec3(p.x, 0.0f, p.z)) ||
                       glm::length(glm::vec2(p.x - zone_.center.x, p.z - zone_.center.z)) < zone_.radius + 1.2f;
        for (const Obstacle& o : obstacles_)
        {
            if (glm::length(glm::vec2(p.x - o.position.x, p.z - o.position.z)) < o.radius + 0.8f) blocked = true;
        }
        if (blocked) continue;

        Tuft t;
        t.position = p;
        t.height = rng.range(0.45f, 1.15f);
        t.phase = rng.range(0.0f, kTwoPi);
        const float shade = rng.range(0.85f, 1.20f);
        t.color = (rng.range(0.0f, 1.0f) > 0.4f) ?
                  glm::vec3(0.38f, 0.58f, 0.22f) * shade :
                  glm::vec3(0.68f, 0.65f, 0.32f) * shade; // Golden feather grass seed heads
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

    // Arena circular boundary
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

    // 1. Vast steppe ground plane
    r.draw(A.box(), Translate(0.0f, -0.5f, 0.0f) * Scale(950.0f, 1.0f, 950.0f), kGroundColor);

    // 2. Outer arena perimeter track (trodden dirt ring)
    r.draw(A.cylinder(), Translate(0.0f, 0.005f, 0.0f) * Scale(arenaRadius_ + 3.0f, 0.015f, arenaRadius_ + 3.0f), kDirtTrackColor);

    // 3. Central competition green turf
    r.draw(A.cylinder(), Translate(0.0f, 0.012f, 0.0f) * Scale(arenaRadius_ - 1.2f, 0.015f, arenaRadius_ - 1.2f), kArenaColor);

    // 4. Score zone (Kazanlyk)
    drawZone(r);

    // 5. Obstacles (Yurts and Rock clusters)
    for (const Obstacle& o : obstacles_)
    {
        if (o.type == Obstacle::Type::Yurt) drawYurt(r, o);
        else drawRock(r, o);
    }

    // 6. Swaying feather grass tufts
    for (const Tuft& t : tufts_)
    {
        const float sway = std::sin(time_ * 2.2f + t.phase) * 0.16f;
        const glm::mat4 m = Translate(t.position) * RotZ(sway) * RotX(sway * 0.5f) *
                            Translate(0.0f, t.height * 0.5f, 0.0f) * Scale(0.20f, t.height, 0.20f);
        r.draw(A.cone(), m, t.color);
    }

    // 7. Perimeter boundary posts and flags
    drawBoundaryPosts(r);

    // 8. Distant multi-layered Tian Shan mountain ridges
    drawMountains(r);
}

void World::drawZone(Renderer& r) const
{
    const Assets& A = *assets_;
    const glm::vec3 c = zone_.center;

    float glow = 1.0f;
    if (zoneHighlight_) glow = 1.0f + 0.30f * std::sin(time_ * 7.0f);

    const glm::vec3 outer = glm::vec3(0.88f, 0.32f, 0.18f) * glow;
    const glm::vec3 inner = glm::vec3(0.98f, 0.88f, 0.52f) * glow;
    const glm::vec3 core = glm::vec3(0.85f, 0.18f, 0.14f) * glow;

    const float pulse = 1.0f + (zoneHighlight_ ? 0.07f * std::sin(time_ * 7.0f) : 0.0f);

    // Raised rim of the Kazanlyk goal
    r.draw(A.cylinder(), Translate(c.x, 0.02f, c.z) * Scale(zone_.radius, 0.03f, zone_.radius), outer);
    r.draw(A.cylinder(), Translate(c.x, 0.035f, c.z) * Scale(zone_.radius * 0.85f * pulse, 0.03f, zone_.radius * 0.85f * pulse), inner);
    r.draw(A.cylinder(), Translate(c.x, 0.05f, c.z) * Scale(zone_.radius * 0.35f, 0.03f, zone_.radius * 0.35f), core);

    // Four corner ceremonial flagpoles with waving pennants
    for (int i = 0; i < 4; ++i)
    {
        const float a = kPi * 0.25f + kHalfPi * static_cast<float>(i);
        const float px = c.x + std::cos(a) * (zone_.radius + 0.5f);
        const float pz = c.z + std::sin(a) * (zone_.radius + 0.5f);
        r.draw(A.cylinder(), Translate(px, 1.75f, pz) * Scale(0.09f, 3.5f, 0.09f), kWood);
        const float wave = std::sin(time_ * 3.4f + static_cast<float>(i) * 1.4f) * 0.38f;
        const glm::vec3 flagColor = (i % 2 == 0) ? kKazakhBlue : kGold;
        r.draw(A.box(), Translate(px, 3.1f, pz) * RotY(wave + a) * Translate(0.50f, 0.0f, 0.0f) * Scale(1.0f, 0.55f, 0.04f), flagColor);
        r.draw(A.sphere(), Translate(px, 3.55f, pz) * Scale(0.14f), kGold);
    }
}

void World::drawYurt(Renderer& r, const Obstacle& o) const
{
    const Assets& A = *assets_;
    const glm::vec3 p = o.position;

    // Yurt main felt wall cylinder (Kerege)
    r.draw(A.cylinder(), Translate(p.x, 1.15f, p.z) * Scale(3.1f, 2.3f, 3.1f), kFeltColor);

    // Decorative woven yurt band (Baskur) in Kazakh turquoise and gold
    r.draw(A.cylinder(), Translate(p.x, 1.65f, p.z) * Scale(3.14f, 0.28f, 3.14f), kKazakhBlue);
    r.draw(A.cylinder(), Translate(p.x, 1.85f, p.z) * Scale(3.13f, 0.08f, 3.13f), kGold);
    r.draw(A.cylinder(), Translate(p.x, 1.45f, p.z) * Scale(3.13f, 0.08f, 3.13f), kGold);

    // Sloped felt roof dome (Uzuk)
    r.draw(A.cone(), Translate(p.x, 3.15f, p.z) * Scale(3.6f, 1.8f, 3.6f), kFeltWarm);

    // Crown of the yurt (Shanyrak) - sacred circular roof ring
    r.draw(A.cylinder(), Translate(p.x, 4.02f, p.z) * Scale(0.65f, 0.12f, 0.65f), kWood);
    r.draw(A.sphere(), Translate(p.x, 4.12f, p.z) * Scale(0.26f), kGold);

    // Wooden doorway frame & felt entrance flap
    const glm::mat4 doorRot = Translate(p.x, 0.0f, p.z) * RotY(o.yaw);
    // Door posts & lintel
    r.draw(A.box(), doorRot * Translate(0.0f, 0.95f, 3.08f) * Scale(1.25f, 1.9f, 0.16f), kWood);
    // Red decorative door curtain
    r.draw(A.box(), doorRot * Translate(0.0f, 0.90f, 3.16f) * Scale(0.85f, 1.6f, 0.08f), kRuby);
}

void World::drawRock(Renderer& r, const Obstacle& o) const
{
    const Assets& A = *assets_;
    const float s = o.radius * 1.05f;
    const glm::vec3 p = o.position;

    // Compound stylized granite boulder cluster (primary boulder + companion rocks)
    r.draw(A.sphere(), Translate(p.x, s * 0.55f, p.z) * RotY(o.yaw) * Scale(s, s * 0.74f, s * 0.88f), kRockColor);
    r.draw(A.sphere(), Translate(p.x + s * 0.45f, s * 0.32f, p.z - s * 0.30f) * RotY(o.yaw + 1.2f) * Scale(s * 0.55f, s * 0.45f, s * 0.50f), kRockDark);
    r.draw(A.sphere(), Translate(p.x - s * 0.40f, s * 0.25f, p.z + s * 0.35f) * RotY(o.yaw - 0.8f) * Scale(s * 0.48f, s * 0.38f, s * 0.42f), kRockColor);
}

void World::drawMountains(Renderer& r) const
{
    const Assets& A = *assets_;

    // Layer 1: Towering Tian Shan / Alatau snow-capped mountain peaks (distant, majestic)
    const int countHigh = 16;
    for (int i = 0; i < countHigh; ++i)
    {
        const float fi = static_cast<float>(i);
        const float angle = kTwoPi * fi / static_cast<float>(countHigh) + std::sin(fi * 5.7f) * 0.15f;
        const float dist = 220.0f + 25.0f * std::sin(fi * 3.3f);
        const float height = 62.0f + 30.0f * std::abs(std::sin(fi * 2.1f));
        const float rad = 65.0f + 22.0f * std::abs(std::sin(fi * 1.9f));
        const float x = std::cos(angle) * dist;
        const float z = std::sin(angle) * dist;

        // Mountain rock body
        r.draw(A.cone(), Translate(x, height * 0.5f, z) * Scale(rad, height, rad), glm::vec3(0.40f, 0.46f, 0.62f));
        // Gleaming alpine snow cap
        r.draw(A.cone(), Translate(x, height * 0.84f, z) * Scale(rad * 0.34f, height * 0.34f, rad * 0.34f), glm::vec3(0.97f, 0.98f, 1.0f));
    }

    // Layer 2: Rolling foothills (closer, warm sage / slate steppe hills)
    const int countLow = 14;
    for (int i = 0; i < countLow; ++i)
    {
        const float fi = static_cast<float>(i);
        const float angle = kTwoPi * (fi + 0.5f) / static_cast<float>(countLow) + std::sin(fi * 4.2f) * 0.2f;
        const float dist = 165.0f + 18.0f * std::sin(fi * 2.7f);
        const float height = 26.0f + 14.0f * std::abs(std::sin(fi * 1.5f));
        const float rad = 45.0f + 16.0f * std::abs(std::sin(fi * 2.5f));
        const float x = std::cos(angle) * dist;
        const float z = std::sin(angle) * dist;

        r.draw(A.cone(), Translate(x, height * 0.45f, z) * Scale(rad, height, rad), glm::vec3(0.46f, 0.52f, 0.36f));
    }
}

void World::drawBoundaryPosts(Renderer& r) const
{
    const Assets& A = *assets_;
    const int count = 40;
    for (int i = 0; i < count; ++i)
    {
        const float a = kTwoPi * static_cast<float>(i) / static_cast<float>(count);
        const float x = std::cos(a) * (arenaRadius_ + 0.6f);
        const float z = std::sin(a) * (arenaRadius_ + 0.6f);
        r.draw(A.cylinder(), Translate(x, 0.65f, z) * Scale(0.18f, 1.3f, 0.18f), kWood);
        const glm::vec3 cap = (i % 2 == 0) ? kKazakhBlue : kGold;
        r.draw(A.sphere(), Translate(x, 1.35f, z) * Scale(0.24f), cap);
    }
}

#include "World.h"

#include <algorithm>
#include <cmath>

#include "Math.h"

namespace
{
const glm::vec3 kWood(0.45f, 0.26f, 0.14f);              // Carved birch / pine wood
const glm::vec3 kDarkWood(0.30f, 0.18f, 0.10f);          // Weathered wood
const glm::vec3 kKazakhBlue(0.00f, 0.64f, 0.82f);        // Traditional turquoise
const glm::vec3 kGold(1.00f, 0.80f, 0.15f);              // Steppe sun gold
const glm::vec3 kRuby(0.85f, 0.22f, 0.18f);              // Traditional crimson
const glm::vec3 kFeltColor(0.94f, 0.91f, 0.82f);         // Natural sheep-wool felt
const glm::vec3 kFeltWarm(0.88f, 0.82f, 0.68f);          // Weathered roof felt
const glm::vec3 kSpruceGreen(0.18f, 0.38f, 0.20f);       // Tien Shan spruce needle green
const glm::vec3 kPineGreen(0.22f, 0.44f, 0.22f);         // Burabay pine green
const glm::vec3 kLarchGreen(0.25f, 0.50f, 0.24f);        // Altai Siberian larch
const glm::vec3 kSnowWhite(0.97f, 0.98f, 1.00f);         // Glacier snow
const glm::vec3 kChalkWhite(0.92f, 0.90f, 0.84f);        // Mangystau chalk
const glm::vec3 kTerracotta(0.78f, 0.38f, 0.22f);        // Charyn sandstone
const glm::vec3 kLakeTurquoise(0.05f, 0.58f, 0.68f);     // Kolsai turquoise water
const glm::vec3 kLakeBlue(0.10f, 0.42f, 0.75f);          // Burabay lake blue
}

void World::init(const Assets& assets)
{
    assets_ = &assets;
    setLocation(LocationId::QazaqAuyl);
}

void World::setLocation(LocationId id)
{
    locationId_ = id;
    const LocationInfo& info = LocationManager::get(id);

    groundColor_ = info.groundColor;
    arenaColor_ = info.arenaColor;
    trackColor_ = info.trackColor;
    boundaryColor_ = info.boundaryColor;

    buildLocationWorld(id);
}

void World::buildLocationWorld(LocationId id)
{
    obstacles_.clear();
    tufts_.clear();
	// The arena is always a 100m x 100m square, centered at (0,0) in world space
    // Kazanlyk scoring goal remains authoritative across all locations
    zone_.center = glm::vec3(0.0f, 0.0f, 40.0f);
    zone_.radius = 5.2f;

    auto addObstacle = [this](Obstacle::Type type, float x, float z, float radius, float yaw, float scale = 1.0f, const glm::vec3& col = glm::vec3(1.0f)) {
        Obstacle o;
        o.type = type;
        o.position = glm::vec3(x, 0.0f, z);
        o.radius = radius;
        o.yaw = yaw;
        o.scale = scale;
        o.color = col;
        obstacles_.push_back(o);
    };

    switch (id)
    {
    case LocationId::CharynCanyon:
    {
        // Red sandstone pinnacles & layered boulder pillars inside arena
        addObstacle(Obstacle::Type::Spire, -20.0f, 15.0f, 2.8f, 0.4f, 1.3f, kTerracotta);
        addObstacle(Obstacle::Type::Spire, 22.0f, 8.0f, 2.6f, 1.2f, 1.2f, kTerracotta);
        addObstacle(Obstacle::Type::Rock, -12.0f, -14.0f, 1.6f, 0.7f, 1.2f, glm::vec3(0.70f, 0.34f, 0.18f));
        addObstacle(Obstacle::Type::Rock, 14.0f, -16.0f, 1.5f, 2.1f, 1.1f, glm::vec3(0.68f, 0.32f, 0.17f));
        addObstacle(Obstacle::Type::Rock, 28.0f, -22.0f, 1.8f, 0.3f, 1.4f, glm::vec3(0.72f, 0.35f, 0.19f));
        addObstacle(Obstacle::Type::Rock, -28.0f, -20.0f, 1.7f, 1.5f, 1.3f, glm::vec3(0.70f, 0.34f, 0.18f));
        addObstacle(Obstacle::Type::Rock, 4.0f, 12.0f, 1.2f, 2.6f, 1.0f, glm::vec3(0.74f, 0.36f, 0.20f));
        break;
    }

    case LocationId::TransIliAlatau:
    {
        // Mountain spruce trees & granite boulders
        addObstacle(Obstacle::Type::Tree, -22.0f, 18.0f, 2.2f, 0.2f, 1.3f, kSpruceGreen);
        addObstacle(Obstacle::Type::Tree, 24.0f, 6.0f, 2.2f, 1.8f, 1.2f, kSpruceGreen);
        addObstacle(Obstacle::Type::Tree, -28.0f, -16.0f, 2.2f, 0.9f, 1.4f, kSpruceGreen);
        addObstacle(Obstacle::Type::Tree, 18.0f, -24.0f, 2.2f, 2.5f, 1.25f, kSpruceGreen);
        addObstacle(Obstacle::Type::Rock, -8.0f, -8.0f, 1.4f, 0.5f, 1.1f, glm::vec3(0.48f, 0.50f, 0.54f));
        addObstacle(Obstacle::Type::Rock, 10.0f, -12.0f, 1.3f, 1.4f, 1.0f, glm::vec3(0.46f, 0.48f, 0.52f));
        addObstacle(Obstacle::Type::Rock, 12.0f, 16.0f, 1.5f, 2.1f, 1.2f, glm::vec3(0.50f, 0.52f, 0.56f));
        break;
    }

    case LocationId::Bozzhyra:
    {
        // White limestone spires & chalk columns
        addObstacle(Obstacle::Type::Spire, -24.0f, 12.0f, 3.2f, 0.5f, 1.6f, kChalkWhite);
        addObstacle(Obstacle::Type::Spire, 24.0f, 12.0f, 3.2f, 1.6f, 1.6f, kChalkWhite);
        addObstacle(Obstacle::Type::Spire, 0.0f, -24.0f, 2.8f, 2.8f, 1.4f, kChalkWhite);
        addObstacle(Obstacle::Type::Rock, -12.0f, -10.0f, 1.4f, 0.8f, 1.1f, glm::vec3(0.82f, 0.80f, 0.74f));
        addObstacle(Obstacle::Type::Rock, 14.0f, -12.0f, 1.3f, 1.9f, 1.0f, glm::vec3(0.80f, 0.78f, 0.72f));
        addObstacle(Obstacle::Type::Rock, -18.0f, -22.0f, 1.5f, 0.3f, 1.2f, glm::vec3(0.84f, 0.82f, 0.76f));
        break;
    }

    case LocationId::Burabay:
    {
        // Pine trees & rounded granite boulders
        addObstacle(Obstacle::Type::Tree, -20.0f, 16.0f, 2.0f, 0.3f, 1.3f, kPineGreen);
        addObstacle(Obstacle::Type::Tree, 22.0f, 8.0f, 2.0f, 1.7f, 1.2f, kPineGreen);
        addObstacle(Obstacle::Type::Tree, -26.0f, -18.0f, 2.0f, 1.0f, 1.4f, kPineGreen);
        addObstacle(Obstacle::Type::Tree, 16.0f, -22.0f, 2.0f, 2.4f, 1.25f, kPineGreen);
        addObstacle(Obstacle::Type::Rock, -10.0f, -6.0f, 1.6f, 0.4f, 1.3f, glm::vec3(0.52f, 0.50f, 0.48f));
        addObstacle(Obstacle::Type::Rock, 12.0f, -14.0f, 1.4f, 1.2f, 1.1f, glm::vec3(0.50f, 0.48f, 0.46f));
        addObstacle(Obstacle::Type::Rock, 14.0f, 16.0f, 1.7f, 2.3f, 1.4f, glm::vec3(0.54f, 0.52f, 0.50f));
        break;
    }

    case LocationId::KolsaiLakes:
    {
        // Spruce forest groves & alpine stream rocks
        addObstacle(Obstacle::Type::Tree, -24.0f, 14.0f, 2.2f, 0.6f, 1.4f, kSpruceGreen);
        addObstacle(Obstacle::Type::Tree, 24.0f, 14.0f, 2.2f, 1.9f, 1.4f, kSpruceGreen);
        addObstacle(Obstacle::Type::Tree, -20.0f, -20.0f, 2.2f, 1.2f, 1.3f, kSpruceGreen);
        addObstacle(Obstacle::Type::Tree, 20.0f, -20.0f, 2.2f, 2.6f, 1.3f, kSpruceGreen);
        addObstacle(Obstacle::Type::Rock, -8.0f, 6.0f, 1.3f, 0.7f, 1.0f, glm::vec3(0.44f, 0.46f, 0.48f));
        addObstacle(Obstacle::Type::Rock, 8.0f, -8.0f, 1.4f, 1.5f, 1.1f, glm::vec3(0.42f, 0.44f, 0.46f));
        break;
    }

    case LocationId::UstyurtPlateau:
    {
        // Wind-carved table rocks & dry clay spires
        addObstacle(Obstacle::Type::Rock, -18.0f, 14.0f, 1.8f, 0.2f, 1.4f, glm::vec3(0.74f, 0.56f, 0.38f));
        addObstacle(Obstacle::Type::Rock, 20.0f, 8.0f, 1.7f, 1.4f, 1.3f, glm::vec3(0.72f, 0.54f, 0.36f));
        addObstacle(Obstacle::Type::Rock, -24.0f, -16.0f, 1.9f, 2.5f, 1.5f, glm::vec3(0.76f, 0.58f, 0.40f));
        addObstacle(Obstacle::Type::Rock, 22.0f, -22.0f, 1.8f, 0.8f, 1.4f, glm::vec3(0.74f, 0.56f, 0.38f));
        addObstacle(Obstacle::Type::Rock, 0.0f, -12.0f, 1.4f, 1.7f, 1.1f, glm::vec3(0.70f, 0.52f, 0.34f));
        break;
    }

    case LocationId::KatonKaragay:
    {
        // Siberian larch trees & Altai boulders
        addObstacle(Obstacle::Type::Tree, -22.0f, 16.0f, 2.2f, 0.4f, 1.4f, kLarchGreen);
        addObstacle(Obstacle::Type::Tree, 22.0f, 12.0f, 2.2f, 1.8f, 1.3f, kLarchGreen);
        addObstacle(Obstacle::Type::Tree, -26.0f, -18.0f, 2.2f, 1.1f, 1.4f, kLarchGreen);
        addObstacle(Obstacle::Type::Tree, 18.0f, -22.0f, 2.2f, 2.5f, 1.3f, kLarchGreen);
        addObstacle(Obstacle::Type::Rock, -10.0f, -8.0f, 1.5f, 0.6f, 1.2f, glm::vec3(0.46f, 0.48f, 0.50f));
        addObstacle(Obstacle::Type::Rock, 12.0f, -12.0f, 1.4f, 1.6f, 1.1f, glm::vec3(0.44f, 0.46f, 0.48f));
        break;
    }

    case LocationId::QazaqAuyl:
    default:
    {
        // Traditional felt Yurts and wooden corrals / hitching posts
        addObstacle(Obstacle::Type::Yurt, -22.0f, 18.0f, 3.4f, 0.6f);
        addObstacle(Obstacle::Type::Yurt, 24.0f, 6.0f, 3.4f, 2.4f);
        addObstacle(Obstacle::Type::Yurt, -30.0f, -18.0f, 3.4f, 1.1f);
        addObstacle(Obstacle::Type::Fence, 22.0f, -20.0f, 2.5f, 0.3f);
        addObstacle(Obstacle::Type::Fence, -12.0f, 26.0f, 2.5f, 1.5f);
        addObstacle(Obstacle::Type::Rock, -8.0f, -6.0f, 1.35f, 0.3f);
        addObstacle(Obstacle::Type::Rock, 10.0f, -14.0f, 1.1f, 1.2f);
        addObstacle(Obstacle::Type::Rock, 14.0f, 18.0f, 1.55f, 2.2f);
        break;
    }
    }

    // Vegetation scatter: Deterministic seeds per location
    Rng rng(202400u + static_cast<unsigned int>(id) * 37u);
    tufts_.clear();
    while (tufts_.size() < 160)
    {
        const float angle = rng.range(0.0f, kTwoPi);
        const float dist = rng.range(3.0f, arenaRadius_ + 28.0f);
        const glm::vec3 p(std::cos(angle) * dist, 0.0f, std::sin(angle) * dist);

        bool blocked = zone_.contains(p) ||
                       glm::length(glm::vec2(p.x - zone_.center.x, p.z - zone_.center.z)) < zone_.radius + 1.2f;
        for (const Obstacle& o : obstacles_)
        {
            if (glm::length(glm::vec2(p.x - o.position.x, p.z - o.position.z)) < o.radius + 0.8f) blocked = true;
        }
        if (blocked) continue;

        Tuft t;
        t.position = p;
        t.height = rng.range(0.40f, 1.10f);
        t.phase = rng.range(0.0f, kTwoPi);

        const float shade = rng.range(0.85f, 1.20f);
        if (id == LocationId::CharynCanyon)
        {
            t.color = glm::vec3(0.68f, 0.48f, 0.22f) * shade; // Dry canyon brush
        }
        else if (id == LocationId::Bozzhyra || id == LocationId::UstyurtPlateau)
        {
            t.color = glm::vec3(0.72f, 0.68f, 0.42f) * shade; // Desert wormwood (жусан)
        }
        else
        {
            t.color = (rng.range(0.0f, 1.0f) > 0.4f) ?
                      glm::vec3(0.38f, 0.58f, 0.22f) * shade :
                      glm::vec3(0.68f, 0.65f, 0.32f) * shade; // Steppe feather grass
        }
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

    // 1. Vast terrain ground plane
    r.draw(A.box(), Translate(0.0f, -0.5f, 0.0f) * Scale(950.0f, 1.0f, 950.0f), groundColor_);

    // 2. Outer arena perimeter track (trodden dirt/sand trail)
    r.draw(A.cylinder(), Translate(0.0f, 0.005f, 0.0f) * Scale(arenaRadius_ + 3.0f, 0.015f, arenaRadius_ + 3.0f), trackColor_);

    // 3. Central competition green/clay turf
    r.draw(A.cylinder(), Translate(0.0f, 0.012f, 0.0f) * Scale(arenaRadius_ - 1.2f, 0.015f, arenaRadius_ - 1.2f), arenaColor_);

    // 4. Kazanlyk score zone
    drawZone(r);

    // 5. In-arena thematic obstacles
    for (const Obstacle& o : obstacles_)
    {
        switch (o.type)
        {
        case Obstacle::Type::Yurt:   drawYurt(r, o); break;
        case Obstacle::Type::Rock:   drawRock(r, o); break;
        case Obstacle::Type::Tree:   drawTree(r, o); break;
        case Obstacle::Type::Fence:  drawFence(r, o); break;
        case Obstacle::Type::Spire:  drawSpire(r, o); break;
        }
    }

    // 6. Swaying vegetation tufts
    for (const Tuft& t : tufts_)
    {
        const float sway = std::sin(time_ * 2.2f + t.phase) * 0.16f;
        const glm::mat4 m = Translate(t.position) * RotZ(sway) * RotX(sway * 0.5f) *
                            Translate(0.0f, t.height * 0.5f, 0.0f) * Scale(0.20f, t.height, 0.20f);
        r.draw(A.cone(), m, t.color);
    }

    // 7. Perimeter boundary posts
    drawBoundaryPosts(r);

    // 8. Location-specific unique landmark perimeter scenery
    switch (locationId_)
    {
    case LocationId::CharynCanyon:   drawCharynCanyonPerimeter(r); break;
    case LocationId::TransIliAlatau: drawAlatauPerimeter(r); break;
    case LocationId::Bozzhyra:       drawBozzhyraPerimeter(r); break;
    case LocationId::Burabay:        drawBurabayPerimeter(r); break;
    case LocationId::KolsaiLakes:    drawKolsaiPerimeter(r); break;
    case LocationId::UstyurtPlateau: drawUstyurtPerimeter(r); break;
    case LocationId::KatonKaragay:   drawKatonKaragayPerimeter(r); break;
    case LocationId::QazaqAuyl:
    default:                         drawQazaqAuylPerimeter(r); break;
    }
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

    r.draw(A.cylinder(), Translate(c.x, 0.02f, c.z) * Scale(zone_.radius, 0.03f, zone_.radius), outer);
    r.draw(A.cylinder(), Translate(c.x, 0.035f, c.z) * Scale(zone_.radius * 0.85f * pulse, 0.03f, zone_.radius * 0.85f * pulse), inner);
    r.draw(A.cylinder(), Translate(c.x, 0.05f, c.z) * Scale(zone_.radius * 0.35f, 0.03f, zone_.radius * 0.35f), core);

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

    // Yurt main wall cylinder (Kerege)
    r.draw(A.cylinder(), Translate(p.x, 1.15f, p.z) * Scale(3.1f, 2.3f, 3.1f), kFeltColor);

    // Decorative woven bands (Baskur) in turquoise and gold
    r.draw(A.cylinder(), Translate(p.x, 1.65f, p.z) * Scale(3.14f, 0.28f, 3.14f), kKazakhBlue);
    r.draw(A.cylinder(), Translate(p.x, 1.85f, p.z) * Scale(3.13f, 0.08f, 3.13f), kGold);
    r.draw(A.cylinder(), Translate(p.x, 1.45f, p.z) * Scale(3.13f, 0.08f, 3.13f), kGold);

    // Sloped roof dome (Uzuk)
    r.draw(A.cone(), Translate(p.x, 3.15f, p.z) * Scale(3.6f, 1.8f, 3.6f), kFeltWarm);

    // Sacred crown ring (Shanyrak)
    r.draw(A.cylinder(), Translate(p.x, 4.02f, p.z) * Scale(0.65f, 0.12f, 0.65f), kWood);
    r.draw(A.sphere(), Translate(p.x, 4.12f, p.z) * Scale(0.26f), kGold);

    // Doorway frame & red felt curtain
    const glm::mat4 doorRot = Translate(p.x, 0.0f, p.z) * RotY(o.yaw);
    r.draw(A.box(), doorRot * Translate(0.0f, 0.95f, 3.08f) * Scale(1.25f, 1.9f, 0.16f), kWood);
    r.draw(A.box(), doorRot * Translate(0.0f, 0.90f, 3.16f) * Scale(0.85f, 1.6f, 0.08f), kRuby);
}

void World::drawRock(Renderer& r, const Obstacle& o) const
{
    const Assets& A = *assets_;
    const float s = o.radius * 1.05f * o.scale;
    const glm::vec3 p = o.position;
    const glm::vec3 col = (o.color != glm::vec3(1.0f)) ? o.color : glm::vec3(0.54f, 0.55f, 0.58f);

    r.draw(A.sphere(), Translate(p.x, s * 0.55f, p.z) * RotY(o.yaw) * Scale(s, s * 0.74f, s * 0.88f), col);
    r.draw(A.sphere(), Translate(p.x + s * 0.45f, s * 0.32f, p.z - s * 0.30f) * RotY(o.yaw + 1.2f) * Scale(s * 0.55f, s * 0.45f, s * 0.50f), col * 0.82f);
    r.draw(A.sphere(), Translate(p.x - s * 0.40f, s * 0.25f, p.z + s * 0.35f) * RotY(o.yaw - 0.8f) * Scale(s * 0.48f, s * 0.38f, s * 0.42f), col);
}

void World::drawTree(Renderer& r, const Obstacle& o) const
{
    const Assets& A = *assets_;
    const glm::vec3 p = o.position;
    const float s = o.scale;
    const glm::vec3 foliageCol = (o.color != glm::vec3(1.0f)) ? o.color : kSpruceGreen;

    // Wooden trunk
    r.draw(A.cylinder(), Translate(p.x, 1.5f * s, p.z) * Scale(0.35f * s, 3.0f * s, 0.35f * s), kWood);

    // Stacked conical needle tiers
    r.draw(A.cone(), Translate(p.x, 3.4f * s, p.z) * Scale(2.6f * s, 2.6f * s, 2.6f * s), foliageCol * 0.85f);
    r.draw(A.cone(), Translate(p.x, 5.0f * s, p.z) * Scale(2.1f * s, 2.4f * s, 2.1f * s), foliageCol * 0.95f);
    r.draw(A.cone(), Translate(p.x, 6.4f * s, p.z) * Scale(1.5f * s, 2.2f * s, 1.5f * s), foliageCol * 1.08f);
}

void World::drawFence(Renderer& r, const Obstacle& o) const
{
    const Assets& A = *assets_;
    const glm::mat4 m = Translate(o.position) * RotY(o.yaw);

    // 3 wooden posts
    r.draw(A.cylinder(), m * Translate(-1.6f, 0.75f, 0.0f) * Scale(0.14f, 1.5f, 0.14f), kWood);
    r.draw(A.cylinder(), m * Translate( 0.0f, 0.75f, 0.0f) * Scale(0.14f, 1.5f, 0.14f), kWood);
    r.draw(A.cylinder(), m * Translate( 1.6f, 0.75f, 0.0f) * Scale(0.14f, 1.5f, 0.14f), kWood);

    // 2 horizontal crossbars
    r.draw(A.box(), m * Translate(0.0f, 0.55f, 0.0f) * Scale(3.5f, 0.12f, 0.08f), kDarkWood);
    r.draw(A.box(), m * Translate(0.0f, 1.15f, 0.0f) * Scale(3.5f, 0.12f, 0.08f), kDarkWood);
}

void World::drawSpire(Renderer& r, const Obstacle& o) const
{
    const Assets& A = *assets_;
    const glm::vec3 p = o.position;
    const float s = o.scale;
    const glm::vec3 col = (o.color != glm::vec3(1.0f)) ? o.color : kTerracotta;

    // Stepped layered sandstone/chalk spire
    r.draw(A.cylinder(), Translate(p.x, 1.8f * s, p.z) * Scale(1.8f * s, 3.6f * s, 1.8f * s), col * 0.92f);
    r.draw(A.cone(), Translate(p.x, 4.8f * s, p.z) * Scale(1.6f * s, 3.2f * s, 1.6f * s), col);
    r.draw(A.cone(), Translate(p.x, 6.8f * s, p.z) * Scale(0.9f * s, 2.0f * s, 0.9f * s), col * 1.08f);
}

// ---------------------------------------------------------------------------
// 1. Charyn Canyon: Layered red sandstone walls & castellated rock bluffs
// ---------------------------------------------------------------------------
void World::drawCharynCanyonPerimeter(Renderer& r) const
{
    const Assets& A = *assets_;
    const int count = 22;
    for (int i = 0; i < count; ++i)
    {
        const float fi = static_cast<float>(i);
        const float angle = kTwoPi * fi / static_cast<float>(count) + std::sin(fi * 4.3f) * 0.12f;
        const float dist = 145.0f + 25.0f * std::sin(fi * 3.1f);
        const float x = std::cos(angle) * dist;
        const float z = std::sin(angle) * dist;

        const float h = 38.0f + 22.0f * std::abs(std::sin(fi * 2.7f));
        const float w = 36.0f + 16.0f * std::abs(std::cos(fi * 1.8f));

        // Tiered canyon rim towers ("Valley of Castles")
        r.draw(A.box(), Translate(x, h * 0.4f, z) * RotY(angle) * Scale(w, h * 0.8f, w * 0.7f), kTerracotta * 0.85f);
        r.draw(A.cylinder(), Translate(x, h * 0.85f, z) * Scale(w * 0.45f, h * 0.35f, w * 0.45f), kTerracotta * 1.05f);
        r.draw(A.cone(), Translate(x, h * 1.1f, z) * Scale(w * 0.35f, h * 0.25f, w * 0.35f), kTerracotta * 1.15f);
    }
}

// ---------------------------------------------------------------------------
// 2. Trans-Ili Alatau: Majestic snow-capped Tian Shan peaks & spruce belts
// ---------------------------------------------------------------------------
void World::drawAlatauPerimeter(Renderer& r) const
{
    const Assets& A = *assets_;
    // High snow summits
    const int countHigh = 16;
    for (int i = 0; i < countHigh; ++i)
    {
        const float fi = static_cast<float>(i);
        const float angle = kTwoPi * fi / static_cast<float>(countHigh);
        const float dist = 220.0f + 25.0f * std::sin(fi * 3.3f);
        const float h = 68.0f + 32.0f * std::abs(std::sin(fi * 2.1f));
        const float rad = 65.0f + 20.0f * std::abs(std::sin(fi * 1.9f));
        const float x = std::cos(angle) * dist;
        const float z = std::sin(angle) * dist;

        r.draw(A.cone(), Translate(x, h * 0.5f, z) * Scale(rad, h, rad), glm::vec3(0.38f, 0.44f, 0.58f));
        r.draw(A.cone(), Translate(x, h * 0.84f, z) * Scale(rad * 0.35f, h * 0.35f, rad * 0.35f), kSnowWhite);
    }

    // Lower spruce forested ridges
    const int countForest = 20;
    for (int i = 0; i < countForest; ++i)
    {
        const float fi = static_cast<float>(i);
        const float angle = kTwoPi * (fi + 0.5f) / static_cast<float>(countForest);
        const float dist = 120.0f + 15.0f * std::sin(fi * 2.5f);
        const float x = std::cos(angle) * dist;
        const float z = std::sin(angle) * dist;

        r.draw(A.cone(), Translate(x, 14.0f, z) * Scale(18.0f, 28.0f, 18.0f), kSpruceGreen * 0.85f);
    }
}

// ---------------------------------------------------------------------------
// 3. Bozzhyra: Iconic white limestone fangs & chalk plateau escarpments
// ---------------------------------------------------------------------------
void World::drawBozzhyraPerimeter(Renderer& r) const
{
    const Assets& A = *assets_;

    // The legendary twin fangs of Bozzhyra (Azul) rising majestically to the North-West
    const glm::vec3 fang1(-110.0f, 0.0f, 120.0f);
    const glm::vec3 fang2(-85.0f, 0.0f, 130.0f);
    for (const glm::vec3& fp : {fang1, fang2})
    {
        r.draw(A.cone(), Translate(fp.x, 32.0f, fp.z) * Scale(24.0f, 64.0f, 18.0f), kChalkWhite);
        r.draw(A.cone(), Translate(fp.x, 62.0f, fp.z) * Scale(10.0f, 32.0f, 8.0f), glm::vec3(0.98f, 0.96f, 0.92f));
    }

    // Vast sheer white chalk cliffs (chinks) encircling the horizon
    const int count = 16;
    for (int i = 0; i < count; ++i)
    {
        const float fi = static_cast<float>(i);
        const float angle = kTwoPi * fi / static_cast<float>(count);
        const float dist = 160.0f + 25.0f * std::sin(fi * 2.8f);
        const float x = std::cos(angle) * dist;
        const float z = std::sin(angle) * dist;

        const float h = 32.0f + 16.0f * std::abs(std::sin(fi * 3.4f));
        const float w = 48.0f;

        // Flat-topped chalk table mountain bluffs
        r.draw(A.box(), Translate(x, h * 0.5f, z) * RotY(angle) * Scale(w, h, 28.0f), kChalkWhite * 0.95f);
        r.draw(A.box(), Translate(x, h + 2.0f, z) * RotY(angle) * Scale(w * 0.9f, 4.0f, 24.0f), glm::vec3(0.85f, 0.82f, 0.74f));
    }
}

// ---------------------------------------------------------------------------
// 4. Burabay: Sapphire lake crescent, granite Okzhetpes monolith & pine ridges
// ---------------------------------------------------------------------------
void World::drawBurabayPerimeter(Renderer& r) const
{
    const Assets& A = *assets_;

    // The iconic Okzhetpes ("Arrow Cannot Reach") granite monolith crag
    const glm::vec3 okzhetpes(90.0f, 0.0f, -120.0f);
    r.draw(A.sphere(), Translate(okzhetpes.x, 35.0f, okzhetpes.z) * Scale(32.0f, 70.0f, 26.0f), glm::vec3(0.50f, 0.48f, 0.46f));
    r.draw(A.cone(), Translate(okzhetpes.x, 68.0f, okzhetpes.z) * Scale(16.0f, 30.0f, 14.0f), glm::vec3(0.54f, 0.52f, 0.50f));

    // Scenic blue Burabay lake crescent
    r.draw(A.cylinder(), Translate(-105.0f, 0.02f, 45.0f) * Scale(85.0f, 0.02f, 55.0f), kLakeBlue);

    // Surrounding pine forested granite hills
    const int count = 18;
    for (int i = 0; i < count; ++i)
    {
        const float fi = static_cast<float>(i);
        const float angle = kTwoPi * fi / static_cast<float>(count);
        const float dist = 140.0f + 20.0f * std::sin(fi * 3.2f);
        const float x = std::cos(angle) * dist;
        const float z = std::sin(angle) * dist;

        r.draw(A.sphere(), Translate(x, 18.0f, z) * Scale(38.0f, 36.0f, 38.0f), glm::vec3(0.48f, 0.46f, 0.44f));
        r.draw(A.cone(), Translate(x, 32.0f, z) * Scale(24.0f, 28.0f, 24.0f), kPineGreen);
    }
}

// ---------------------------------------------------------------------------
// 5. Kolsai Lakes: Turquoise mountain lake crescent, steep spruce slopes & snowy ridges
// ---------------------------------------------------------------------------
void World::drawKolsaiPerimeter(Renderer& r) const
{
    const Assets& A = *assets_;

    // Turquoise alpine lake crescent outside southern playing field
    r.draw(A.cylinder(), Translate(0.0f, 0.02f, -115.0f) * Scale(95.0f, 0.02f, 45.0f), kLakeTurquoise);

    // Deep mountain gorge slopes lined with dense Tien Shan spruce
    const int count = 18;
    for (int i = 0; i < count; ++i)
    {
        const float fi = static_cast<float>(i);
        const float angle = kTwoPi * fi / static_cast<float>(count);
        const float dist = 135.0f + 22.0f * std::sin(fi * 2.9f);
        const float x = std::cos(angle) * dist;
        const float z = std::sin(angle) * dist;

        const float h = 45.0f + 25.0f * std::abs(std::sin(fi * 2.3f));
        r.draw(A.cone(), Translate(x, h * 0.45f, z) * Scale(42.0f, h, 42.0f), glm::vec3(0.24f, 0.40f, 0.22f));

        // Snow on high valley rims
        if (h > 55.0f)
        {
            r.draw(A.cone(), Translate(x, h * 0.85f, z) * Scale(14.0f, h * 0.3f, 14.0f), kSnowWhite);
        }
    }
}

// ---------------------------------------------------------------------------
// 6. Ustyurt Plateau: Vast table mountains (chinks) & dramatic clay horizon
// ---------------------------------------------------------------------------
void World::drawUstyurtPerimeter(Renderer& r) const
{
    const Assets& A = *assets_;

    // Grand flat-topped table mountains & stepped escarpments
    const int count = 14;
    for (int i = 0; i < count; ++i)
    {
        const float fi = static_cast<float>(i);
        const float angle = kTwoPi * fi / static_cast<float>(count);
        const float dist = 175.0f + 20.0f * std::sin(fi * 2.4f);
        const float x = std::cos(angle) * dist;
        const float z = std::sin(angle) * dist;

        const float h = 28.0f + 14.0f * std::abs(std::sin(fi * 3.1f));
        const float len = 65.0f;

        r.draw(A.box(), Translate(x, h * 0.5f, z) * RotY(angle) * Scale(len, h, 34.0f), glm::vec3(0.70f, 0.54f, 0.35f));
        r.draw(A.box(), Translate(x, h + 1.5f, z) * RotY(angle) * Scale(len * 0.95f, 3.0f, 30.0f), glm::vec3(0.82f, 0.68f, 0.44f));
    }
}

// ---------------------------------------------------------------------------
// 7. Katon-Karagay: Rich Siberian larch taiga belts & Mount Belukha peaks
// ---------------------------------------------------------------------------
void World::drawKatonKaragayPerimeter(Renderer& r) const
{
    const Assets& A = *assets_;

    // Distant twin snowy Belukha peaks to the East
    const glm::vec3 belukha1(140.0f, 0.0f, 150.0f);
    const glm::vec3 belukha2(165.0f, 0.0f, 135.0f);
    r.draw(A.cone(), Translate(belukha1.x, 50.0f, belukha1.z) * Scale(55.0f, 100.0f, 55.0f), glm::vec3(0.38f, 0.44f, 0.54f));
    r.draw(A.cone(), Translate(belukha1.x, 82.0f, belukha1.z) * Scale(24.0f, 45.0f, 24.0f), kSnowWhite);

    r.draw(A.cone(), Translate(belukha2.x, 45.0f, belukha2.z) * Scale(50.0f, 90.0f, 50.0f), glm::vec3(0.38f, 0.44f, 0.54f));
    r.draw(A.cone(), Translate(belukha2.x, 75.0f, belukha2.z) * Scale(22.0f, 40.0f, 22.0f), kSnowWhite);

    // Multi-tiered Siberian larch and cedar forest ridges
    const int count = 20;
    for (int i = 0; i < count; ++i)
    {
        const float fi = static_cast<float>(i);
        const float angle = kTwoPi * fi / static_cast<float>(count);
        const float dist = 125.0f + 18.0f * std::sin(fi * 2.7f);
        const float x = std::cos(angle) * dist;
        const float z = std::sin(angle) * dist;

        r.draw(A.cone(), Translate(x, 18.0f, z) * Scale(28.0f, 36.0f, 28.0f), kLarchGreen * 0.90f);
    }
}

// ---------------------------------------------------------------------------
// 8. Qazaq Auyl: Encampment of nomadic felt yurts, corrals & rolling steppe hills
// ---------------------------------------------------------------------------
void World::drawQazaqAuylPerimeter(Renderer& r) const
{
    const Assets& A = *assets_;

    // Village yurt clusters outside the playing field (North and East)
    const glm::vec3 auylYurts[] = {
        glm::vec3(-80.0f, 0.0f, 75.0f),
        glm::vec3(-95.0f, 0.0f, 85.0f),
        glm::vec3(-70.0f, 0.0f, 95.0f),
        glm::vec3(85.0f, 0.0f, 80.0f),
        glm::vec3(100.0f, 0.0f, 92.0f),
    };

    for (const glm::vec3& yp : auylYurts)
    {
        // Yurt base cylinder
        r.draw(A.cylinder(), Translate(yp.x, 1.8f, yp.z) * Scale(4.2f, 3.6f, 4.2f), kFeltColor);
        // Ornamental turquoise baskur
        r.draw(A.cylinder(), Translate(yp.x, 2.7f, yp.z) * Scale(4.25f, 0.45f, 4.25f), kKazakhBlue);
        // Roof cone
        r.draw(A.cone(), Translate(yp.x, 4.8f, yp.z) * Scale(4.8f, 2.8f, 4.8f), kFeltWarm);
        // Shanyrak crown
        r.draw(A.cylinder(), Translate(yp.x, 6.2f, yp.z) * Scale(0.9f, 0.2f, 0.9f), kWood);
        r.draw(A.sphere(), Translate(yp.x, 6.4f, yp.z) * Scale(0.35f), kGold);
    }

    // Traditional Kazakh ceremonial tughs (tall standards with horsehair/silk pennants)
    for (int i = 0; i < 6; ++i)
    {
        const float a = kTwoPi * static_cast<float>(i) / 6.0f + 0.3f;
        const float x = std::cos(a) * 75.0f;
        const float z = std::sin(a) * 75.0f;

        r.draw(A.cylinder(), Translate(x, 4.5f, z) * Scale(0.16f, 9.0f, 0.16f), kWood);
        r.draw(A.sphere(), Translate(x, 9.2f, z) * Scale(0.45f), kGold);
        // Waving turquoise banner
        const float wave = std::sin(time_ * 3.2f + static_cast<float>(i)) * 0.4f;
        r.draw(A.box(), Translate(x, 8.2f, z) * RotY(wave + a) * Translate(1.0f, 0.0f, 0.0f) * Scale(2.0f, 1.1f, 0.06f), kKazakhBlue);
    }

    // Rolling gentle steppe foothills in the background
    const int countHills = 16;
    for (int i = 0; i < countHills; ++i)
    {
        const float fi = static_cast<float>(i);
        const float angle = kTwoPi * fi / static_cast<float>(countHills);
        const float dist = 160.0f + 25.0f * std::sin(fi * 2.3f);
        const float x = std::cos(angle) * dist;
        const float z = std::sin(angle) * dist;

        r.draw(A.cone(), Translate(x, 16.0f, z) * Scale(55.0f, 32.0f, 55.0f), glm::vec3(0.48f, 0.54f, 0.28f));
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
        const glm::vec3 cap = (i % 2 == 0) ? boundaryColor_ : kGold;
        r.draw(A.sphere(), Translate(x, 1.35f, z) * Scale(0.24f), cap);
    }
}

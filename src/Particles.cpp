#include "Particles.h"

#include <algorithm>
#include <cmath>

#include "Assets.h"
#include "Math.h"
#include "Renderer.h"

void Particles::init(const Assets& assets)
{
    assets_ = &assets;
    pool_.resize(kMaxParticles);
    clear();
}

void Particles::clear()
{
    for (auto& p : pool_)
    {
        p.active = false;
    }
}

void Particles::update(float dt)
{
    dt = std::max(dt, 1e-4f);
    for (auto& p : pool_)
    {
        if (!p.active) continue;

        p.life += dt;
        if (p.life >= p.maxLife)
        {
            p.active = false;
            continue;
        }

        // Apply velocity, drag, and gravity
        p.vel.y -= p.gravity * dt;
        p.vel *= std::max(0.0f, 1.0f - 2.8f * dt);
        p.pos += p.vel * dt;

        // Ground collision bounce / slide
        if (p.pos.y < 0.05f)
        {
            p.pos.y = 0.05f;
            p.vel.y = -p.vel.y * 0.3f;
            p.vel.x *= 0.7f;
            p.vel.z *= 0.7f;
        }
    }
}

void Particles::draw(Renderer& r) const
{
    if (!assets_) return;
    const Assets& A = *assets_;

    for (const auto& p : pool_)
    {
        if (!p.active) continue;

        const float progress = std::clamp(p.life / p.maxLife, 0.0f, 1.0f);
        // Grow over life then fade
        const float curSize = p.size + (p.maxSize - p.size) * progress;
        const float alpha = (1.0f - progress * progress) * 0.55f;

        const glm::mat4 m = Translate(p.pos) * Scale(curSize);
        r.drawUnlit(A.sphere(), m, p.color, alpha);
    }
}

void Particles::emitDust(const glm::vec3& groundPos, const glm::vec3& horseVel, float intensity)
{
    if (intensity < 0.05f) return;

    int count = static_cast<int>(1.0f + intensity * 2.0f);
    for (int c = 0; c < count; ++c)
    {
        for (auto& p : pool_)
        {
            if (p.active) continue;

            p.active = true;
            p.pos = groundPos + glm::vec3(rng_.range(-0.15f, 0.15f), 0.06f, rng_.range(-0.15f, 0.15f));
            // Drift slightly backward relative to horse movement
            p.vel = -horseVel * 0.12f + glm::vec3(rng_.range(-0.35f, 0.35f), rng_.range(0.6f, 1.4f), rng_.range(-0.35f, 0.35f));
            p.color = glm::vec3(0.72f, 0.68f, 0.48f) * rng_.range(0.9f, 1.1f); // Warm steppe dust
            p.size = rng_.range(0.10f, 0.18f);
            p.maxSize = rng_.range(0.35f, 0.55f);
            p.life = 0.0f;
            p.maxLife = rng_.range(0.35f, 0.55f);
            p.gravity = 0.6f;
            break;
        }
    }
}

void Particles::emitPickup(const glm::vec3& pos)
{
    const int count = 18;

    for (int i = 0; i < count; ++i)
    {
        for (auto& p : pool_)
        {
            if (p.active) continue;

            p.active = true;
            p.pos = pos + glm::vec3(rng_.range(-0.2f, 0.2f), rng_.range(0.0f, 0.4f), rng_.range(-0.2f, 0.2f));
            const float angle = rng_.range(0.0f, kTwoPi);
            const float speed = rng_.range(1.5f, 4.0f);
            p.vel = glm::vec3(std::cos(angle) * speed, rng_.range(2.0f, 4.5f), std::sin(angle) * speed);
            p.color = (i % 2 == 0) ? glm::vec3(1.0f, 0.85f, 0.20f) : glm::vec3(1.0f, 1.0f, 0.85f); // Golden sparkles
            p.size = rng_.range(0.12f, 0.18f);
            p.maxSize = rng_.range(0.22f, 0.30f);
            p.life = 0.0f;
            p.maxLife = rng_.range(0.40f, 0.65f);
            p.gravity = 3.5f;
            break;
        }
    }
}

void Particles::emitScore(const glm::vec3& center)
{
    const int count = 48;

    const glm::vec3 colors[] = {
        glm::vec3(1.0f, 0.80f, 0.10f), // Gold
        glm::vec3(0.0f, 0.65f, 0.85f), // Kazakh Turquoise
        glm::vec3(0.95f, 0.95f, 0.20f), // Ruby Red
        glm::vec3(0.20f, 0.85f, 0.45f), // Emerald Green
        glm::vec3(1.0f, 0.95f, 0.70f)  // Warm Ivory
    };

    for (int i = 0; i < count; ++i)
    {
        for (auto& p : pool_)
        {
            if (p.active) continue;

            p.active = true;
            const float rad = rng_.range(0.2f, 2.5f);
            const float theta = rng_.range(0.0f, kTwoPi);
            p.pos = center + glm::vec3(std::cos(theta) * rad, rng_.range(0.2f, 1.0f), std::sin(theta) * rad);

            const float speed = rng_.range(2.0f, 6.5f);
            const float phi = rng_.range(0.2f, 1.2f);
            p.vel = glm::vec3(std::cos(theta) * speed * std::cos(phi),
                              speed * std::sin(phi) + rng_.range(3.0f, 7.0f),
                              std::sin(theta) * speed * std::cos(phi));

            p.color = colors[i % 5];
            p.size = rng_.range(0.15f, 0.25f);
            p.maxSize = rng_.range(0.30f, 0.45f);
            p.life = 0.0f;
            p.maxLife = rng_.range(0.70f, 1.25f);
            p.gravity = 6.0f;
            break;
        }
    }
}

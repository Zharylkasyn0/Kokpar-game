#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "Math.h"

class Assets;
class Renderer;

// ===========================================================================
// Lightweight Particle System (v0.3)
//
// Zero-allocation per-frame particle pool for:
// - Hoof dust puffs during trotting and galloping
// - Golden sparkle burst upon goat pickup
// - Colorful celebratory confetti/fireworks upon scoring
// ===========================================================================
class Particles
{
public:
    void init(const Assets& assets);
    void update(float dt);
    void draw(Renderer& renderer) const;
    void clear();

    // Spawns a hoof dust puff at the given ground position
    void emitDust(const glm::vec3& groundPos, const glm::vec3& horseVel, float intensity);

    // Spawns golden pickup feedback stars/sparkles
    void emitPickup(const glm::vec3& pos);

    // Spawns colorful celebration particles upon scoring
    void emitScore(const glm::vec3& center);

private:
    struct Particle
    {
        glm::vec3 pos = glm::vec3(0.0f);
        glm::vec3 vel = glm::vec3(0.0f);
        glm::vec3 color = glm::vec3(1.0f);
        float size = 0.2f;
        float maxSize = 0.5f;
        float life = 0.0f;
        float maxLife = 0.5f;
        float gravity = 0.0f;
        bool active = false;
    };

    static constexpr size_t kMaxParticles = 192;
    std::vector<Particle> pool_;
    const Assets* assets_ = nullptr;
    mutable Rng rng_{1337u};
};

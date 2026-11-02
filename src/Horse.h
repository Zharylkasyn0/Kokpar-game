#pragma once

#include <glm/glm.hpp>

#include "Assets.h"
#include "Input.h"
#include "Math.h"
#include "Renderer.h"
#include "World.h"

// Player: capsule-built horse with a rider. Local space: +Z forward, +Y up, -X is the rider's right.
// Heading: forward = (sin(yaw), 0, cos(yaw)). Pressing A increases yaw (turn left).
class Horse
{
public:
    void init(Assets& assets);
    void reset(const glm::vec3& position, float yaw);

    void update(float dt, const Input& input, const World& world);
    void draw(Renderer& renderer) const;

    // Called by Game: visual state of the rider's arm / celebration.
    void setCarrying(bool carrying) { carrying_ = carrying; }
    void cheer() { squash_.v -= 5.0f; }   // quick stretch-jump

    const glm::vec3& position() const { return position_; }
    float yaw() const { return yaw_; }
    float speed() const { return speed_; }
    float speed01() const { return std::min(1.0f, std::abs(speed_) / kSprintSpeed); }
    float bobHeight() const { return bobY_; }
    glm::vec3 forward() const { return glm::vec3(std::sin(yaw_), 0.0f, std::cos(yaw_)); }
    glm::vec3 right() const { return glm::vec3(-std::cos(yaw_), 0.0f, std::sin(yaw_)); }
    glm::vec3 velocity() const { return forward() * speed_; }

    // World transform of the point where the goat is held (right hip of the rider).
    glm::mat4 carryAnchor() const { return rigid_ * Translate(-0.80f, 1.62f, 0.10f); }

private:
    static constexpr float kWalkSpeed = 7.0f;
    static constexpr float kSprintSpeed = 12.0f;
    static constexpr float kReverseSpeed = 3.0f;
    static constexpr float kAcceleration = 14.0f;
    static constexpr float kBrake = 22.0f;
    static constexpr float kTurnRate = 2.6f;
    static constexpr float kProbeRadius = 0.6f;
    static constexpr float kProbeOffset = 0.65f;

    const Assets* assets_ = nullptr;
    const Mesh* bodyMesh_ = nullptr;
    const Mesh* legMesh_ = nullptr;
    const Mesh* neckMesh_ = nullptr;
    const Mesh* headMesh_ = nullptr;
    const Mesh* tailMesh_ = nullptr;
    const Mesh* torsoMesh_ = nullptr;
    const Mesh* riderLegMesh_ = nullptr;
    const Mesh* armMesh_ = nullptr;

    // Kinematics
    glm::vec3 position_ = glm::vec3(0.0f);
    float yaw_ = 0.0f;
    float speed_ = 0.0f;
    float prevSpeed_ = 0.0f;
    float accelSmooth_ = 0.0f;
    float turnInput_ = 0.0f;
    float bumpCooldown_ = 0.0f;
    bool carrying_ = false;

    // Animation
    float time_ = 0.0f;
    float gait_ = 0.0f;       // gallop phase
    float legAmp_ = 0.0f;     // leg swing amplitude
    float bobY_ = 0.0f;       // body hop
    Spring lean_;             // rider pitch
    Spring bounce_;           // rider vertical lag
    Spring roll_;             // body roll when turning
    Spring tail_;
    Spring ear_;
    Spring squash_;           // squash & stretch on bumps / cheers

    glm::mat4 rigid_ = glm::mat4(1.0f);  // position + yaw + roll (no scale)
};

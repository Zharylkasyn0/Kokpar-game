#pragma once

#include <glm/glm.hpp>

#include "Assets.h"
#include "Horse.h"
#include "Math.h"
#include "Renderer.h"
#include "World.h"

// Cartoon goat built from capsules/spheres/cones, with four spring-driven floppy legs.
// Local space: origin = body centre, +Z forward, +Y up.
// No ragdoll: legs are independent damped springs driven by the carrier's acceleration.
class Goat
{
public:
    enum class State { Idle, Carried, Thrown };

    static constexpr float kBodyHeight = 0.62f;   // body-centre height when standing on the ground

    void init(Assets& assets);

    void respawn(const glm::vec3& groundPosition, float yaw);
    void pickUp(const Horse& horse);
    void drop(const glm::vec3& velocity);

    void update(float dt, const Horse& horse, const World& world);
    void draw(Renderer& renderer, bool pickupHint) const;

    State state() const { return state_; }
    const glm::vec3& position() const { return position_; }
    glm::vec3 groundPosition() const { return glm::vec3(position_.x, 0.0f, position_.z); }
    float heightAboveGround() const { return std::max(0.0f, position_.y - kBodyHeight); }

private:
    void updateIdle(float dt);
    void updateCarried(float dt, const Horse& horse);
    void updateThrown(float dt, const World& world);

    const Mesh* bodyMesh_ = nullptr;
    const Mesh* legMesh_ = nullptr;
    const Assets* assets_ = nullptr;

    State state_ = State::Idle;
    glm::vec3 position_ = glm::vec3(0.0f, kBodyHeight, 0.0f);
    glm::vec3 velocity_ = glm::vec3(0.0f);
    float yaw_ = 0.0f;
    float time_ = 0.0f;
    float popT_ = 1.0f;               // spawn "pop" animation progress 0..1

    // Carry state
    glm::vec3 pickupFrom_ = glm::vec3(0.0f);
    float pickupYaw_ = 0.0f;
    float carryBlend_ = 0.0f;
    int warmup_ = 0;
    glm::vec3 prevAnchor_ = glm::vec3(0.0f);
    glm::vec3 prevVelocity_ = glm::vec3(0.0f);
    glm::vec3 accel_ = glm::vec3(0.0f);
    float carrierSpeed01_ = 0.0f;

    // Secondary motion
    Spring legX_[4];     // fore/aft swing
    Spring legZ_[4];     // side swing
    Spring swayX_;       // whole-body pitch
    Spring swayZ_;       // whole-body roll
    Spring head_;        // head nod
    Spring ear_;         // ear flop
    Spring tumble_;      // flip angle while thrown / wobble after landing
};

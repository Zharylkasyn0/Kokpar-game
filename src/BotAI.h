#pragma once

#include <array>
#include <string>
#include <glm/glm.hpp>

#include "Goat.h"
#include "Horse.h"
#include "World.h"

// ===========================================================================
// Autonomous Free-for-All Bot AI
//
// Lightweight Finite State Machine:
// - SEEK_GOAT: Navigate to free serke on ground, approach smoothly, pick up
// - CHASE_CARRIER: Pursue opponent carrying goat with interception & flank offsets
// - STEAL_GOAT: Execute FIFA-style steal in range & facing angle with cooldown
// - CARRY_TO_GOAL: Gallop towards Kazanlyk to score while avoiding obstacles
// - RECOVER: Unstick from arena boundaries, obstacles, or collisions
//
// Drives the existing Horse movement system identically to a human player.
// ===========================================================================
class BotAI
{
public:
    enum class State
    {
        SeekGoat,
        ChaseCarrier,
        StealGoat,
        CarryToGoal,
        Recover
    };

    struct Output
    {
        HorseControlInput ctrl;
        bool wantPickup = false;
        bool wantSteal = false;
    };

    void init(int botIndex, float personalityOffset = 0.0f);
    void reset();

    Output update(float dt,
                  const Horse& selfHorse,
                  int carrierIndex,
                  const Goat& goat,
                  const std::array<Horse, 4>& allHorses,
                  const World& world);

    State state() const { return state_; }
    const char* stateName() const;
    int botIndex() const { return botIndex_; }

    void triggerStealCooldown(float duration = 1.5f) { stealCooldown_ = duration; }
    bool canSteal() const { return stealCooldown_ <= 0.0f; }

    static constexpr float kStealRange = 3.2f;
    static constexpr float kStealAngleMax = 1.92f; // ~110 degrees total frontal cone
    static constexpr float kStealCooldown = 1.5f;

private:
    int botIndex_ = 1;                     // Index in horses array: 1, 2, or 3
    State state_ = State::SeekGoat;
    float personalityOffset_ = 0.0f;       // Flank offset angle for chasing (-0.3f, 0.0f, +0.3f)
    float stealCooldown_ = 0.0f;
    glm::vec3 currentNavTarget_ = glm::vec3(0.0f);

    // Stuck / Recover detection
    float stuckTimer_ = 0.0f;
    float recoverTimer_ = 0.0f;
    float recoverTurnSign_ = 1.0f;
};

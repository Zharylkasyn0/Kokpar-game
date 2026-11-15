#include "BotAI.h"

#include <algorithm>
#include <cmath>

#include "Math.h"

void BotAI::init(int botIndex, float personalityOffset)
{
    botIndex_ = botIndex;
    personalityOffset_ = personalityOffset;
    reset();
}

void BotAI::reset()
{
    state_ = State::SeekGoat;
    stealCooldown_ = 0.0f;
    stuckTimer_ = 0.0f;
    recoverTimer_ = 0.0f;
    recoverTurnSign_ = 1.0f;
    currentNavTarget_ = glm::vec3(0.0f);
}

const char* BotAI::stateName() const
{
    switch (state_)
    {
    case State::SeekGoat:     return "SEEK_GOAT";
    case State::ChaseCarrier: return "CHASE_CARRIER";
    case State::StealGoat:    return "STEAL_GOAT";
    case State::CarryToGoal:  return "CARRY_TO_GOAL";
    case State::Recover:      return "RECOVER";
    default:                  return "UNKNOWN";
    }
}

BotAI::Output BotAI::update(float dt,
                            const Horse& selfHorse,
                            int carrierIndex,
                            const Goat& goat,
                            const std::array<Horse, 4>& allHorses,
                            const World& world)
{
    Output out;
    stealCooldown_ = std::max(0.0f, stealCooldown_ - dt);

    const glm::vec3 selfPos = selfHorse.position();
    const glm::vec3 selfFwd = selfHorse.forward();
    const float distFromCenter = glm::length(glm::vec2(selfPos.x, selfPos.z));

    // ---- 1. State Transitions --------------------------------------------
    if (recoverTimer_ > 0.0f)
    {
        state_ = State::Recover;
    }
    else if (carrierIndex == botIndex_)
    {
        state_ = State::CarryToGoal;
    }
    else if (carrierIndex >= 0 && carrierIndex < static_cast<int>(allHorses.size()))
    {
        // An opponent carries the goat
        const glm::vec3 carrierPos = allHorses[carrierIndex].position();
        const glm::vec3 toCarrier = carrierPos - selfPos;
        const float distToCarrier = glm::length(glm::vec2(toCarrier.x, toCarrier.z));

        const float cosHalfAngle = std::cos(kStealAngleMax * 0.5f);
        float dotFwd = 0.0f;
        if (distToCarrier > 1e-4f)
        {
            const glm::vec3 dir = toCarrier / distToCarrier;
            dotFwd = glm::dot(selfFwd, dir);
        }

        const bool inStealRange = (distToCarrier <= kStealRange);
        const bool inStealCone = (dotFwd >= cosHalfAngle);

        if (inStealRange && inStealCone && stealCooldown_ <= 0.0f)
        {
            state_ = State::StealGoat;
        }
        else
        {
            state_ = State::ChaseCarrier;
        }
    }
    else
    {
        state_ = State::SeekGoat;
    }

    // ---- 2. Target Computation & Actions ---------------------------------
    glm::vec3 targetPos(0.0f);

    switch (state_)
    {
    case State::Recover:
    {
        recoverTimer_ -= dt;
        // Back up and turn away from obstacles
        out.ctrl.backward = true;
        if (recoverTurnSign_ > 0.0f) out.ctrl.left = true;
        else out.ctrl.right = true;
        return out;
    }

    case State::CarryToGoal:
    {
        // Destination is Kazanlyk center (0, 0, 0)
        targetPos = world.zone().center;
        break;
    }

    case State::StealGoat:
    {
        if (carrierIndex >= 0 && carrierIndex < static_cast<int>(allHorses.size()))
        {
            targetPos = allHorses[carrierIndex].position();
            out.wantSteal = true;
            stealCooldown_ = kStealCooldown;
        }
        break;
    }

    case State::ChaseCarrier:
    {
        if (carrierIndex >= 0 && carrierIndex < static_cast<int>(allHorses.size()))
        {
            const Horse& carrierHorse = allHorses[carrierIndex];
            const glm::vec3 carrierPos = carrierHorse.position();
            const glm::vec3 carrierVel = carrierHorse.velocity();
            const float dist = glm::length(glm::vec2(carrierPos.x - selfPos.x, carrierPos.z - selfPos.z));

            // Intercept prediction: lead target based on travel time
            const float leadTime = std::clamp(dist / 11.0f, 0.15f, 0.85f);
            glm::vec3 predicted = carrierPos + carrierVel * leadTime;

            // Flank offset: differentiate bot pursuit vectors
            const glm::vec3 flankOffset = carrierHorse.right() * (personalityOffset_ * 3.2f);
            targetPos = predicted + flankOffset;
        }
        break;
    }

    case State::SeekGoat:
    default:
    {
        targetPos = goat.position();
        const float distToGoat = glm::length(glm::vec2(targetPos.x - selfPos.x, targetPos.z - selfPos.z));
        if (distToGoat <= 2.8f && goat.state() == Goat::State::Idle)
        {
            out.wantPickup = true;
        }
        break;
    }
    }

    // ---- 3. Obstacle & Boundary Feeler Adjustments ------------------------
    // Arena boundary safety: steer toward center when close to fence
    if (distFromCenter > (world.arenaRadius() - 6.0f))
    {
        // Blend target strongly toward center
        const float t = (distFromCenter - (world.arenaRadius() - 6.0f)) / 6.0f;
        targetPos = glm::mix(targetPos, glm::vec3(0.0f), std::clamp(t, 0.0f, 1.0f));
    }

    // Feeler probe ahead of horse
    const glm::vec3 feelerPos = selfPos + selfFwd * 2.4f;
    const glm::vec2 push = world.resolveCircle(feelerPos, 0.65f);
    if (glm::length(push) > 1e-4f)
    {
        // Deflect target perpendicular to push normal
        targetPos += glm::vec3(push.x, 0.0f, push.y) * 4.0f;
    }

    // ---- 4. Steering and Locomotion --------------------------------------
    const glm::vec2 toTarget(targetPos.x - selfPos.x, targetPos.z - selfPos.z);
    const float targetDist = glm::length(toTarget);

    if (targetDist > 0.25f)
    {
        const float desiredYaw = std::atan2(toTarget.x, toTarget.y);
        const float angleDiff = WrapAngle(desiredYaw - selfHorse.yaw());

        // Steering
        if (angleDiff > 0.08f)
        {
            out.ctrl.left = true;
        }
        else if (angleDiff < -0.08f)
        {
            out.ctrl.right = true;
        }

        // Speed management
        const float absDiff = std::abs(angleDiff);
        if (absDiff > 1.6f)
        {
            // Turn sharply in place or slow walk
            out.ctrl.forward = (targetDist > 3.5f);
            out.ctrl.sprint = false;
        }
        else
        {
            out.ctrl.forward = true;

            // Sprint when carrying, chasing, or long-distance seeking
            if (state_ == State::CarryToGoal || state_ == State::ChaseCarrier || targetDist > 8.0f)
            {
                out.ctrl.sprint = (absDiff < 0.65f);
            }

            // Smooth approach when arriving at pickup point
            if (state_ == State::SeekGoat && targetDist < 3.0f)
            {
                out.ctrl.sprint = false;
            }
        }
    }

    // ---- 5. Stuck Detection ----------------------------------------------
    if (out.ctrl.forward && std::abs(selfHorse.speed()) < 0.6f && targetDist > 1.8f)
    {
        stuckTimer_ += dt;
        if (stuckTimer_ > 0.9f)
        {
            // Enter recovery mode
            recoverTimer_ = 0.85f;
            stuckTimer_ = 0.0f;
            recoverTurnSign_ = (selfPos.x >= 0.0f) ? 1.0f : -1.0f;
        }
    }
    else
    {
        stuckTimer_ = std::max(0.0f, stuckTimer_ - dt * 2.0f);
    }

    return out;
}

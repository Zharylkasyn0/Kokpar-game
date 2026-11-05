#pragma once

#include <algorithm>
#include <array>
#include <cmath>

#include <glm/glm.hpp>

#include "Assets.h"
#include "Input.h"
#include "Math.h"
#include "Renderer.h"
#include "World.h"

// ===========================================================================
// Realistic Equine Character (v0.2)
//
// Biomechanically grounded horse with hierarchical skeletal transformations,
// multi-segment articulated legs (knees, hocks, pasterns, hooves),
// distinct Walk / Trot / Gallop gaits with smooth speed-based blending,
// momentum physics, and dynamic rider stabilization.
// ===========================================================================
class Horse
{
public:
    void init(Assets& assets);
    void reset(const glm::vec3& position, float yaw);

    void update(float dt, const Input& input, const World& world);
    void draw(Renderer& renderer) const;

    // Called by Game: visual state of the rider's arm / celebration.
    void setCarrying(bool carrying) { carrying_ = carrying; }
    void cheer() { squash_.v -= 6.0f; }

    const glm::vec3& position() const { return position_; }
    float yaw() const { return yaw_; }
    float speed() const { return speed_; }
    float speed01() const { return std::min(1.0f, std::abs(speed_) / kSprintSpeed); }
    float bobHeight() const { return bobY_; }
    glm::vec3 forward() const { return glm::vec3(std::sin(yaw_), 0.0f, std::cos(yaw_)); }
    glm::vec3 right() const { return glm::vec3(-std::cos(yaw_), 0.0f, std::sin(yaw_)); }
    glm::vec3 velocity() const { return forward() * speed_; }

    // World transform of the point where the goat is held (right hip of the rider).
    glm::mat4 carryAnchor() const;

private:
    // Speed tiers for distinct equine gaits
    static constexpr float kWalkMaxSpeed  = 3.6f;
    static constexpr float kTrotMaxSpeed  = 7.4f;
    static constexpr float kSprintSpeed   = 13.5f;
    static constexpr float kReverseSpeed  = 2.8f;
    static constexpr float kAcceleration  = 9.5f;
    static constexpr float kBrake         = 20.0f;
    static constexpr float kTurnRate      = 2.5f;

    // Collision probes (chest and rump)
    static constexpr float kProbeRadius   = 0.65f;
    static constexpr float kProbeOffset   = 0.72f;

    // Leg indices for locomotion
    enum LegIndex {
        kLegFL = 0, // Fore Left
        kLegFR = 1, // Fore Right
        kLegBL = 2, // Hind Left
        kLegBR = 3  // Hind Right
    };

    struct LegAngles {
        float upper = 0.0f;    // Hip / shoulder pitch
        float mid = 0.0f;      // Stifle pitch (hind leg only)
        float lower = 0.0f;    // Knee (fore) / hock (hind) flexion
        float pastern = 0.0f;  // Fetlock joint extension / cushion
        float lift = 0.0f;     // Vertical foot clearance above ground
    };

    // Locomotion solvers for the three gaits
    void computeWalk(float phase, float legAmp, std::array<LegAngles, 4>& outAngles,
                     float& outBob, float& outPitch) const;
    void computeTrot(float phase, float legAmp, std::array<LegAngles, 4>& outAngles,
                     float& outBob, float& outPitch) const;
    void computeGallop(float phase, float legAmp, float sprintFactor, std::array<LegAngles, 4>& outAngles,
                      float& outBob, float& outPitch) const;

    // Drawing helper methods for anatomical sub-assemblies
    void drawTorso(Renderer& r, const glm::mat4& root) const;
    void drawNeckAndHead(Renderer& r, const glm::mat4& root, float s01, float gaitPitch) const;
    void drawForeleg(Renderer& r, const glm::mat4& root, int side, const LegAngles& angles) const;
    void drawHindleg(Renderer& r, const glm::mat4& root, int side, const LegAngles& angles) const;
    void drawTail(Renderer& r, const glm::mat4& root, float s01) const;
    void drawSaddleAndTack(Renderer& r, const glm::mat4& root) const;
    void drawRider(Renderer& r, const glm::mat4& root) const;

    const Assets* assets_ = nullptr;

    // Anatomical procedural mesh pointers
    const Mesh* chestMesh_ = nullptr;
    const Mesh* barrelMesh_ = nullptr;
    const Mesh* croupMesh_ = nullptr;
    const Mesh* neckLowerMesh_ = nullptr;
    const Mesh* neckUpperMesh_ = nullptr;
    const Mesh* headMesh_ = nullptr;
    const Mesh* muzzleMesh_ = nullptr;
    const Mesh* foreUpperMesh_ = nullptr;
    const Mesh* foreLowerMesh_ = nullptr;
    const Mesh* hindUpperMesh_ = nullptr;
    const Mesh* hindMidMesh_ = nullptr;
    const Mesh* hindLowerMesh_ = nullptr;
    const Mesh* pasternMesh_ = nullptr;
    const Mesh* tailMesh_ = nullptr;
    const Mesh* torsoMesh_ = nullptr;
    const Mesh* riderLegMesh_ = nullptr;
    const Mesh* armMesh_ = nullptr;

    // Kinematics and world placement
    glm::vec3 position_ = glm::vec3(0.0f);
    float yaw_ = 0.0f;
    float speed_ = 0.0f;
    float prevSpeed_ = 0.0f;
    float accelSmooth_ = 0.0f;
    float turnInput_ = 0.0f;
    float bumpCooldown_ = 0.0f;
    bool carrying_ = false;

    // Gait and locomotion state
    float time_ = 0.0f;
    float gaitPhase_ = 0.0f;     // Master gait cycle phase [0, 2*pi)
    float bobY_ = 0.0f;          // Procedural vertical bobbing
    float bodyPitch_ = 0.0f;     // Procedural pitch from gait & weight transfer
    std::array<LegAngles, 4> legAngles_{};

    // Secondary physics springs
    Spring roll_;               // Centrifugal banking into turns
    Spring lean_;               // Forward/backward inertia lean
    Spring neckPitch_;          // Neck secondary motion
    Spring neckYaw_;            // Neck turning lag
    Spring headPitch_;          // Head nod lag
    Spring headYaw_;            // Head looking into turns
    Spring ear_[2];             // Left / Right ear flop & alertness
    Spring tail_[3];            // 3-segment cascading tail lag
    Spring riderBounce_;        // Rider vertical lag / stabilization
    Spring riderLean_;          // Rider spine stabilization
    Spring squash_;             // Bump / celebration squash & stretch

    glm::mat4 rigid_ = glm::mat4(1.0f); // Position + yaw + roll + pitch
};

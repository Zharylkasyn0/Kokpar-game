#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

#include <glm/glm.hpp>

#include "Assets.h"
#include "Input.h"
#include "Math.h"
#include "Renderer.h"
#include "World.h"

// Control inputs for horse locomotion (usable by both player and AI bots)
struct HorseControlInput
{
    bool forward = false;
    bool backward = false;
    bool left = false;
    bool right = false;
    bool sprint = false;
    bool brake = false;
};

// ===========================================================================
// Anatomical Equine Character & Kipchak Warrior (v0.4)
//
// Biomechanically grounded horse with natural equine silhouette:
// sculpted withers, deep keel chest, muscular sloping scapulae,
// tapered athletic flank tuck-up, rounded croup and powerful haunches,
// arched crested neck with throatlatch, chiseled equine head with large
// ganache jowls, tapered muzzle with flared nostrils, and calibrated
// leg joints with flush ground-contact hooves.
// Rider features a medieval Kipchak conical iron helmet with bronze brow
// band, protective cheek guards, chainmail aventail, and bold moustache.
// ===========================================================================
class Horse
{
public:
    void init(Assets& assets);
    void reset(const glm::vec3& position, float yaw);

    void update(float dt, const Input& input, const World& world);
    void update(float dt, const HorseControlInput& ctrl, const World& world);
    void draw(Renderer& renderer) const;

    // Called by Game: visual state of the rider's arm / celebration.
    void setCarrying(bool carrying) { carrying_ = carrying; }
    bool isCarrying() const { return carrying_; }
    void cheer() { squash_.v -= 6.0f; }

    // Steal reach arm animation trigger
    void triggerStealAnim() { stealTimer_ = 0.40f; }
    bool isStealing() const { return stealTimer_ > 0.0f; }

    // Rider appearance customization (vest color)
    void setVestColor(const glm::vec3& col) { vestColor_ = col; }
    const glm::vec3& vestColor() const { return vestColor_; }

    // Displacement / Collision resolution
    void displace(const glm::vec2& d) { position_.x += d.x; position_.z += d.y; }
    void displace(const glm::vec3& d) { position_ += d; }
    void getCollisionProbes(glm::vec3& chest, glm::vec3& rump) const
    {
        const glm::vec3 fwd = forward();
        chest = position_ + fwd * kProbeOffset;
        rump  = position_ - fwd * kProbeOffset;
    }
    static constexpr float probeRadius() { return kProbeRadius; }
    static constexpr float probeOffset() { return kProbeOffset; }

    const glm::vec3& position() const { return position_; }
    float yaw() const { return yaw_; }
    float speed() const { return speed_; }
    float speed01() const { return std::min(1.0f, std::abs(speed_) / kSprintSpeed); }
    float bobHeight() const { return bobY_; }
    glm::vec3 forward() const { return glm::vec3(std::sin(yaw_), 0.0f, std::cos(yaw_)); }
    glm::vec3 right() const { return glm::vec3(std::cos(yaw_), 0.0f, -std::sin(yaw_)); }
    glm::vec3 velocity() const { return forward() * speed_; }

    const char* gaitName() const;
    void getHoofStrikes(std::vector<glm::vec3>& outStrikes) const { outStrikes = currentHoofStrikes_; }

    // World transform of the point where the goat is held (right hip of the rider).
    glm::mat4 carryAnchor() const;

private:
    // Speed tiers for distinct equine gaits
    static constexpr float kWalkMaxSpeed = 3.8f;
    static constexpr float kTrotMaxSpeed = 7.8f;
    static constexpr float kSprintSpeed = 13.8f;
    static constexpr float kReverseSpeed = 3.0f;
    static constexpr float kAcceleration = 10.5f;
    static constexpr float kBrake = 22.0f;
    static constexpr float kTurnRate = 2.7f;

    // Collision probes (chest and rump) calibrated for compact body
    static constexpr float kProbeRadius = 0.55f;
    static constexpr float kProbeOffset = 0.50f;

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

    // Locomotion solvers for the two gaits (WALK & GALLOP)
    void computeWalk(float phase, float legAmp, std::array<LegAngles, 4>& outAngles,
        float& outBob, float& outPitch) const;
    void computeGallop(float phase, float legAmp, float sprintFactor, std::array<LegAngles, 4>& outAngles,
        float& outBob, float& outPitch) const;

    // Drawing helper methods for anatomical sub-assemblies
    void drawTorso(Renderer& r, const glm::mat4& root) const;
    void drawNeckAndHead(Renderer& r, const glm::mat4& root, float s01, float gaitPitch, glm::mat4& outHead) const;
    void drawBridle(Renderer& r, const glm::mat4& head) const;
    void drawReins(Renderer& r, const glm::mat4& head, const glm::mat4& rider) const;
    void drawForeleg(Renderer& r, const glm::mat4& root, int side, const LegAngles& angles) const;
    void drawHindleg(Renderer& r, const glm::mat4& root, int side, const LegAngles& angles) const;
    void drawTail(Renderer& r, const glm::mat4& root, float s01) const;
    void drawSaddleAndTack(Renderer& r, const glm::mat4& root) const;
    void drawRider(Renderer& r, const glm::mat4& root, glm::mat4& outRider) const;

    const Assets* assets_ = nullptr;

    // Anatomical procedural mesh pointers
    const Mesh* horseBodyMesh_ = nullptr;
    const Mesh* horseNeckMesh_ = nullptr;
    const Mesh* horseHeadMesh_ = nullptr;
    const Mesh* chestMesh_ = nullptr;
    const Mesh* barrelMesh_ = nullptr;
    const Mesh* croupMesh_ = nullptr;
    const Mesh* scapulaMesh_ = nullptr;
    const Mesh* neckLowerMesh_ = nullptr;
    const Mesh* neckUpperMesh_ = nullptr;
    const Mesh* nasalMesh_ = nullptr;
    const Mesh* muzzleMesh_ = nullptr;
    const Mesh* foreUpperMesh_ = nullptr;
    const Mesh* forearmMesh_ = nullptr;
    const Mesh* foreLowerMesh_ = nullptr;
    const Mesh* hindUpperMesh_ = nullptr;
    const Mesh* hindMidMesh_ = nullptr;
    const Mesh* hindLowerMesh_ = nullptr;
    const Mesh* pasternMesh_ = nullptr;
    const Mesh* tailMesh_ = nullptr;
    const Mesh* bitRingMesh_ = nullptr;
    const Mesh* torsoMesh_ = nullptr;
    const Mesh* riderLegMesh_ = nullptr;
    const Mesh* armMesh_ = nullptr;
    const Mesh* moustacheMesh_ = nullptr;

    // Kinematics and world placement
    glm::vec3 position_ = glm::vec3(0.0f);
    float yaw_ = 0.0f;
    float speed_ = 0.0f;
    float prevSpeed_ = 0.0f;
    float accelSmooth_ = 0.0f;
    float turnInput_ = 0.0f;
    float bumpCooldown_ = 0.0f;
    float stealTimer_ = 0.0f;
    bool carrying_ = false;
    glm::vec3 vestColor_ = glm::vec3(0.80f, 0.14f, 0.14f);

    // Gait and locomotion state
    float time_ = 0.0f;
    float gaitPhase_ = 0.0f;     // Master gait cycle phase [0, 2*pi)
    float bobY_ = 0.0f;          // Procedural vertical bobbing
    float bodyPitch_ = 0.0f;     // Procedural pitch from gait & weight transfer
    std::array<LegAngles, 4> legAngles_{};
    std::array<float, 4> prevLift_{};
    mutable std::vector<glm::vec3> currentHoofStrikes_;

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
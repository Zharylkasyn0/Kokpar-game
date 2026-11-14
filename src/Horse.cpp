#include "Horse.h"

#include <GLFW/glfw3.h>

namespace
{
    // Natural Equine Coat & Steppe Anatomy Palette
    const glm::vec3 kCoat(0.54f, 0.28f, 0.14f);        // Rich warm steppe chestnut / bay
    const glm::vec3 kCoatDark(0.36f, 0.18f, 0.09f);    // Points, lower legs, muscle shading
    const glm::vec3 kCoatLight(0.72f, 0.48f, 0.32f);   // Flank highlight & soft undercarriage
    const glm::vec3 kMuzzle(0.22f, 0.17f, 0.15f);      // Soft dark equine muzzle tip
    const glm::vec3 kMane(0.10f, 0.08f, 0.08f);        // Dark flowing mane, forelock & tail
    const glm::vec3 kHoof(0.20f, 0.18f, 0.16f);        // Keratin hooves
    const glm::vec3 kHoofBand(0.68f, 0.58f, 0.48f);    // Coronet band above hooves
    const glm::vec3 kWhiteMarking(0.92f, 0.90f, 0.88f);// White facial star on forehead

    // Nomadic Saddle & Traditional Tack
    const glm::vec3 kSaddle(0.14f, 0.48f, 0.52f);      // Kazakh turquoise leather saddle
    const glm::vec3 kSaddlePad(0.82f, 0.20f, 0.16f);   // Traditional crimson felt shabrack
    const glm::vec3 kStirrup(0.75f, 0.72f, 0.68f);     // Forged iron stirrups
    const glm::vec3 kReins(0.24f, 0.16f, 0.10f);       // Leather bridle & reins

    // Medieval Kipchak Warrior Armor & Features
    const glm::vec3 kVest(0.80f, 0.14f, 0.14f);        // Traditional steppe vest
    const glm::vec3 kSkin(0.92f, 0.74f, 0.60f);        // Tanned weathered nomad skin
    const glm::vec3 kIronHelmet(0.24f, 0.26f, 0.28f);   // Forged medieval iron helmet skull
    const glm::vec3 kBronzeTrim(0.82f, 0.66f, 0.25f);   // Polished bronze brow band & finial spire
    const glm::vec3 kAventail(0.28f, 0.30f, 0.33f);     // Chainmail neck aventail (camail)
    const glm::vec3 kMoustache(0.08f, 0.06f, 0.06f);    // Thick stylized dark warrior moustache
    const glm::vec3 kDark(0.10f, 0.08f, 0.08f);        // Eyes, eyebrows, riding boots, trim
    const glm::vec3 kGold(1.00f, 0.80f, 0.15f);        // Belt & ornamental bronze studs

    glm::mat4 AlignY(const glm::vec3& dir)
    {
        const glm::vec3 vFrom(0.0f, 1.0f, 0.0f);
        const float len = glm::length(dir);
        if (len < 0.0001f) return glm::mat4(1.0f);
        const glm::vec3 vTo = dir / len;
        const float d = glm::dot(vFrom, vTo);
        if (d > 0.9999f) return glm::mat4(1.0f);
        if (d < -0.9999f) return glm::rotate(glm::mat4(1.0f), kPi, glm::vec3(1.0f, 0.0f, 0.0f));
        const glm::vec3 axis = glm::normalize(glm::cross(vFrom, vTo));
        const float angle = std::acos(std::clamp(d, -1.0f, 1.0f));
        return glm::rotate(glm::mat4(1.0f), angle, axis);
    }

    // =========================================================================
    // HORSE ANIMATION SETTINGS
    // Edit values here to tweak the walking and galloping animations.
    // All angles are in radians.
    // =========================================================================
    struct HorseAnimationSettings
    {
        // --- 1. WALK ANIMATION PARAMETERS -------------------------------------

        // How far each leg swings forward and backward at the hip/shoulder (radians).
        // Larger value = bigger stride length.
        float walkForeLegSwing          = 0.45f; // Front legs forward/back reach
        float walkHindLegSwing          = 0.45f; // Hind legs forward/back reach

        // Front leg joint bending angles during swing phase (radians).
        float walkForeKneeBend          = 0.55f; // Knee (carpus) bends backward toward belly
        float walkForeElbowFold         = 0.25f; // Forearm fold at elbow
        // Front knee natural walk motion shaping:
        // Adjusts how early the knee flexes on lift-off and how smoothly it extends for placement.
        float walkForeKneeEarlyFlex     = 0.22f; // Early flexion shaping (0.0 = symmetric sine, 0.22 = natural equine cadence)
        float walkForePlacementEase     = 0.16f; // Ground contact cushioning factor

        // Hind leg joint bending angles during swing phase (radians).
        float walkHindStifleBend        = 0.40f; // Stifle fold (brings gaskin forward)
        float walkHindHockBend          = 0.52f; // Hock flex forward (tucks cannon under belly)

        // Hoof vertical ground clearance during swing phase (meters).
        float walkForeHoofLift          = 0.12f; // Front hoof lift height
        float walkHindHoofLift          = 0.12f; // Hind hoof lift height

        // Body movement during walk.
        float walkBodyBob               = 0.018f;// Subtle vertical body bob per stride (meters)
        float walkBodyPitch             = 0.015f;// Subtle forward/backward body tilt with stride (radians)
        float walkPasternFlex           = 0.18f; // Pastern cushion on hoof impact (radians)

        // --- 2. GALLOP ANIMATION PARAMETERS -----------------------------------

        // Hip and shoulder swing reach during full gallop stride (radians).
        float gallopForeLegSwing        = 0.82f; // Front legs reach
        float gallopHindLegSwing        = 0.88f; // Hind legs reach

        // Front leg joint bending angles during airborne recovery (radians).
        float gallopForeKneeBend        = 0.85f; // High knee bend during gathering
        float gallopForeElbowFold       = 0.35f; // Elbow fold during recovery

        // Hind leg joint bending angles during airborne recovery (radians).
        float gallopHindStifleBend      = 0.80f; // Stifle tuck under belly
        float gallopHindHockBend        = 0.95f; // Hock flexion under abdomen

        // Hoof vertical ground clearance during gallop (meters).
        float gallopForeHoofLift        = 0.22f; // Front hoof clearance
        float gallopHindHoofLift        = 0.22f; // Hind hoof clearance

        // Body movement during gallop.
        float gallopBodyBob             = 0.060f;// Body rise during airborne suspension phase (meters)
        float gallopBodyPitch           = 0.12f; // Rocking-horse pitch oscillation (radians)
        float gallopPasternFlex         = 0.38f; // Pastern shock absorption on landing (radians)

        // --- 3. GAIT SWITCHING THRESHOLDS -------------------------------------

        // Speed (m/s) below which the horse uses 100% walk animation.
        float walkMaxSpeed              = 3.8f;

        // Speed (m/s) at which gallop animation reaches 100%.
        // The game smoothly blends from walk to gallop between walkMaxSpeed and gallopFullSpeed.
        float gallopFullSpeed           = 7.0f;

        // --- 4. RIDER MOVEMENT PARAMETERS -------------------------------------

        // Subtle forward-backward rider sway in saddle matching stride rhythm (meters).
        float riderForwardBackwardMotion = 0.035f;

        // Multiplier for rider torso pitch lag (spine balancing reaction).
        float riderLeanScale            = 0.80f;

        // Multiplier for rider vertical bounce absorption from saddle bob.
        float riderBounceScale          = 0.65f;
    };

    // Global settings instance - modify parameters above to customize animations.
    const HorseAnimationSettings kAnim;

} // anonymous namespace

void Horse::init(Assets& assets)
{
    assets_ = &assets;

    // Cohesive low-poly polygonal meshes
    horseBodyMesh_ = &assets.horseBody();
    horseNeckMesh_ = &assets.horseNeck();
    horseHeadMesh_ = &assets.horseHead();

    // Muscular equine torso segments (compact, athletic steppe horse proportions)
    chestMesh_ = &assets.taperedCapsule(0.28f, 0.22f, 0.40f);
    barrelMesh_ = &assets.capsule(0.32f, 0.38f);
    croupMesh_ = &assets.taperedCapsule(0.30f, 0.25f, 0.36f);
    scapulaMesh_ = &assets.taperedCapsule(0.095f, 0.065f, 0.28f);

    // Neck & Head: Natural convex arched curve, forward angle, tapered throatlatch
    neckLowerMesh_ = &assets.taperedCapsule(0.18f, 0.135f, 0.28f);
    neckUpperMesh_ = &assets.taperedCapsule(0.135f, 0.105f, 0.20f);
    nasalMesh_ = &assets.taperedCapsule(0.090f, 0.070f, 0.15f);
    muzzleMesh_ = &assets.taperedCapsule(0.070f, 0.058f, 0.10f);

    // -------------------------------------------------------------------------
    // 5-Point Customizable Leg Thickness System:
    // Parameters: customLeg(rTop, rUpperMid, rCenter, rLowerMid, rBottom, length)
    //   - rTop:      Radius at proximal/upper joint
    //   - rUpperMid: Radius at 25% down the segment
    //   - rCenter:   Radius at 50% midpoint
    //   - rLowerMid: Radius at 75% down the segment
    //   - rBottom:   Radius at distal/lower joint
    //   - length:    Total segment length
    // -------------------------------------------------------------------------

    // Foreleg segments:
    // Humerus / Upper arm (shoulder to elbow)
    foreUpperMesh_ = &assets.customLeg(
        0.07f,   // rTop (proximal shoulder)
        0.15f,   // rUpperMid
        0.155f,   // rCenter
        0.08f,   // rLowerMid
        0.05f,   // rBottom (elbow joint)
        0.7f     // length
    );

    // Forearm / Radius (elbow to carpus/knee)
    forearmMesh_ = &assets.customLeg(
        0.046f,   // rTop (elbow)
        0.0525f,  // rUpperMid
        0.0590f,  // rCenter
        0.0655f,  // rLowerMid
        0.0720f,  // rBottom (knee joint)
        0.42f     // length
    );

    // Fore cannon bone (knee to fetlock)
    foreLowerMesh_ = &assets.customLeg(
        0.038f,   // rTop (knee)
        0.039f,   // rUpperMid
        0.040f,   // rCenter
        0.041f,   // rLowerMid
        0.042f,   // rBottom (fetlock)
        0.34f     // length
    );

    // Hindleg segments:
    // Femur / Thigh (hip to stifle)
    hindUpperMesh_ = &assets.customLeg(
        0.205f,   // rTop (hip)
        0.17f, // rUpperMid
        0.1425f,  // rCenter
        0.1125f, // rLowerMid
        0.10f,   // rBottom (stifle joint)
        0.32f     // length
    );

    // Gaskin / Tibia (stifle to hock)
    hindMidMesh_ = &assets.customLeg(
        0.052f,   // rTop (stifle)
        0.064f,   // rUpperMid
        0.076f,   // rCenter
        0.088f,   // rLowerMid
        0.100f,   // rBottom (hock)
        0.32f     // length
    );

    // Hind cannon bone (hock to fetlock)
    hindLowerMesh_ = &assets.customLeg(
        0.038f,   // rTop (hock)
        0.0395f,  // rUpperMid
        0.041f,   // rCenter
        0.0425f,  // rLowerMid
        0.044f,   // rBottom (fetlock)
        0.26f     // length
    );

    // Pastern & tail
    pasternMesh_ = &assets.taperedCapsule(0.038f, 0.034f, 0.10f);
    tailMesh_ = &assets.taperedCapsule(0.070f, 0.050f, 0.30f);

    // Kazakh bridle bit rings (torus)
    bitRingMesh_ = &assets.torus(0.038f, 0.006f);

    // Kipchak warrior meshes
    torsoMesh_ = &assets.capsule(0.19f, 0.30f);
    riderLegMesh_ = &assets.capsule(0.085f, 0.26f);
    armMesh_ = &assets.capsule(0.065f, 0.19f);
    moustacheMesh_ = &assets.capsule(0.018f, 0.055f);
}

void Horse::reset(const glm::vec3& position, float yaw)
{
    position_ = position;
    yaw_ = yaw;
    speed_ = prevSpeed_ = accelSmooth_ = turnInput_ = 0.0f;
    bumpCooldown_ = 0.0f;
    carrying_ = false;

    gaitPhase_ = 0.0f;
    bobY_ = 0.0f;
    bodyPitch_ = 0.0f;
    legAngles_.fill(LegAngles{});

    roll_.reset();
    lean_.reset();
    neckPitch_.reset();
    neckYaw_.reset();
    headPitch_.reset();
    headYaw_.reset();
    ear_[0].reset();
    ear_[1].reset();
    tail_[0].reset();
    tail_[1].reset();
    tail_[2].reset();
    riderBounce_.reset();
    riderLean_.reset();
    squash_.reset();
    prevLift_.fill(0.0f);
    currentHoofStrikes_.clear();

    rigid_ = Translate(position_) * RotY(yaw_);
}

// ---------------------------------------------------------------------------
// Biomechanical Locomotion Solvers
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// WALK SOLVER  - Four-beat lateral sequence: BL -> FL -> BR -> FR
// ---------------------------------------------------------------------------
// legAmp: 0..1 amplitude scale (driven by speed)
// outAngles[0]=FL, [1]=FR, [2]=BL, [3]=BR  (see LegIndex enum)
void Horse::computeWalk(float phase, float legAmp,
    std::array<LegAngles, 4>& outAngles,
    float& outBob, float& outPitch) const
{
    const auto& P = kAnim;

    // Classic 4-beat lateral walk footfall sequence: BL -> FL -> BR -> FR
    // Regular 25% phase spacing ensures steady cadence with no abrupt transitions.
    const float offsets[4] = {
        0.25f * kTwoPi,  // FL (Fore Left)
        0.75f * kTwoPi,  // FR (Fore Right)
        0.00f * kTwoPi,  // BL (Hind Left)
        0.50f * kTwoPi   // BR (Hind Right)
    };
    const bool isFore[4] = { true, true, false, false };

    constexpr float kStanceFraction = 0.62f; // 62% stance: always 2-3 feet grounded
    constexpr float kSwingFraction  = 1.0f - kStanceFraction; // 38% swing

    for (int i = 0; i < 4; ++i)
    {
        const float lp = std::fmod(phase + offsets[i] + kTwoPi * 4.0f, kTwoPi) / kTwoPi;
        LegAngles a{};

        const float swing    = isFore[i] ? P.walkForeLegSwing : P.walkHindLegSwing;
        const float hoofLift = isFore[i] ? P.walkForeHoofLift : P.walkHindHoofLift;

        if (lp < kStanceFraction)
        {
            // ---- STANCE PHASE (hoof planted firmly on ground) ----------------
            // Leg sweeps from forward touchdown (-0.5*swing) to backward push (+0.5*swing)
            // relative to the horse body as the horse advances forward over the ground.
            const float u = lp / kStanceFraction;
            a.upper = (u - 0.5f) * swing * legAmp;

            // Elastic cushion and weight support under body load
            const float wtBell = std::sin(u * kPi);
            if (isFore[i])
            {
                a.mid   = -0.03f * wtBell;
                a.lower =  0.03f * wtBell;
            }
            else
            {
                a.mid   = -0.04f * wtBell;
                a.lower =  0.03f * wtBell;
            }
            a.pastern = (-0.10f + P.walkPasternFlex * wtBell);
            a.lift    = 0.0f; // Firm ground contact
        }
        else
        {
            // ---- SWING PHASE (hoof airborne, folding and swinging forward) ---
            // Continuous transition: starts at push-off (+0.5*swing) and returns to touchdown (-0.5*swing) 
            const float s    = (lp - kStanceFraction) / kSwingFraction;
            const float flex = std::sin(s * kPi); // Bell curve for joint flexion

            a.upper = (0.5f - s) * swing * legAmp;

            if (isFore[i])
            {
                // Asymmetric equine flexion: prompt lift-off flexion followed by progressive forward extension
                const float foreFlex = std::sin(s * kPi) + P.walkForeKneeEarlyFlex * std::sin(2.0f * s * kPi);
                // Smooth placement cushion as hoof reaches toward the ground
                const float placementFactor = (1.0f - s * 0.25f);
                a.mid   = -P.walkForeElbowFold * foreFlex * placementFactor;
                a.lower =  P.walkForeKneeBend  * std::max(0.0f, foreFlex);
                a.pastern = (0.10f * std::sin(s * kPi) - P.walkForePlacementEase * std::sin(s * s * kPi));
                a.lift    = hoofLift * std::sin(s * kPi);
            }
            else
            {
                // Hindleg stifle draws gaskin forward while hock tucks cannon forward under belly
                a.mid   = -P.walkHindStifleBend * flex;
                a.lower =  P.walkHindHockBend   * flex;
            }
            a.pastern = 0.12f * flex;
            a.lift    = hoofLift * flex;
        }
        outAngles[static_cast<size_t>(i)] = a;
    }

    // Subtle 4-beat body cadence
    outBob   = std::abs(std::sin(phase * 2.0f)) * P.walkBodyBob * legAmp;
    outPitch = std::sin(phase) * P.walkBodyPitch * legAmp;
}

// ---------------------------------------------------------------------------
// GALLOP SOLVER  - Asymmetric four-beat with true suspension phase
// ---------------------------------------------------------------------------
// Right-lead sequence: BL -> BR -> FL -> FR -> (suspension)
void Horse::computeGallop(float phase, float legAmp, float sprintFactor,
    std::array<LegAngles, 4>& outAngles,
    float& outBob, float& outPitch) const
{
    const auto& P = kAnim;

    // Asymmetric footfall timing for right-lead gallop
    const float offsets[4] = {
        0.35f * kTwoPi,  // FL trailing foreleg
        0.50f * kTwoPi,  // FR leading foreleg (reaches furthest forward)
        0.00f * kTwoPi,  // BL trailing hindleg (initiates propulsion)
        0.16f * kTwoPi   // BR leading hindleg
    };
    const bool isFore[4] = { true, true, false, false };
    const bool isLead[4] = { false, true, false, true };

    constexpr float kStanceFraction = 0.36f; // ~36% ground contact
    constexpr float kSwingFraction  = 1.0f - kStanceFraction; // ~64% swing / suspension

    for (int i = 0; i < 4; ++i)
    {
        const float lp = std::fmod(phase + offsets[i] + kTwoPi * 4.0f, kTwoPi) / kTwoPi;
        LegAngles a{};

        const float baseSwing = isFore[i] ? P.gallopForeLegSwing : P.gallopHindLegSwing;
        const float leadAdd   = isLead[i] ? 0.12f : 0.0f;
        const float swing     = baseSwing + leadAdd;
        const float hoofLift  = isFore[i] ? P.gallopForeHoofLift : P.gallopHindHoofLift;

        if (lp < kStanceFraction)
        {
            // ---- GROUND CONTACT & DRIVE PHASE -------------------------------
            const float u    = lp / kStanceFraction;
            const float bell = std::sin(u * kPi);

            // Forward touchdown (-0.5*swing) sweeping to backward propulsion (+0.5*swing)
            a.upper = (u - 0.5f) * swing * legAmp;

            if (isFore[i])
            {
                a.mid     = -0.06f * bell;
                a.lower   =  0.08f + 0.12f * bell;
                a.pastern = -P.gallopPasternFlex + (P.gallopPasternFlex * 1.8f) * bell;
            }
            else
            {
                // Hindlegs push backward forcefully with hock extension
                a.mid     = -0.06f - 0.10f * bell;
                a.lower   =  0.05f + 0.10f * bell;
                a.pastern = -P.gallopPasternFlex + (P.gallopPasternFlex * 1.8f) * bell;
            }
            a.lift = 0.0f;
        }
        else
        {
            // ---- AIRBORNE & GATHERING PHASE ---------------------------------
            const float s    = (lp - kStanceFraction) / kSwingFraction;
            const float flex = std::sin(s * kPi);

            // Airborne swing back to forward reach
            a.upper = (0.5f - s) * swing * legAmp;

            if (isFore[i])
            {
                // Forelegs fold knee backward during early recovery
                a.mid     = -P.gallopForeElbowFold * flex;
                a.lower   =  P.gallopForeKneeBend  * flex;
                a.pastern =  0.22f * flex;
            }
            else
            {
                // Hindlegs tuck powerfully under abdomen
                a.mid     = -P.gallopHindStifleBend * flex;
                a.lower   =  P.gallopHindHockBend   * flex;
                a.pastern =  0.18f * flex;
            }
            a.lift = (hoofLift + sprintFactor * 0.06f) * flex;
        }
        outAngles[static_cast<size_t>(i)] = a;
    }

    // Suspension-phase vertical surge: body rises during airborne flight
    const float susp = std::sin(phase - 1.1f);
    outBob = (susp > 0.0f
                 ? susp * (P.gallopBodyBob + sprintFactor * 0.02f)
                 : susp * 0.025f) * legAmp;

    // Rocking-horse pitch: rear rises on hind push, nose dips on forehand landing
    outPitch = std::sin(phase - 0.4f) * (P.gallopBodyPitch + sprintFactor * 0.04f) * legAmp;
}

void Horse::update(float dt, const Input& in, const World& world)
{
    HorseControlInput ctrl;
    ctrl.forward = in.down(GLFW_KEY_W) || in.down(GLFW_KEY_UP);
    ctrl.backward = in.down(GLFW_KEY_S) || in.down(GLFW_KEY_DOWN);
    ctrl.left = in.down(GLFW_KEY_A) || in.down(GLFW_KEY_LEFT);
    ctrl.right = in.down(GLFW_KEY_D) || in.down(GLFW_KEY_RIGHT);
    ctrl.sprint = in.down(GLFW_KEY_LEFT_SHIFT) || in.down(GLFW_KEY_RIGHT_SHIFT);
    ctrl.brake = in.down(GLFW_KEY_SPACE);
    update(dt, ctrl, world);
}

void Horse::update(float dt, const HorseControlInput& ctrl, const World& world)
{
    dt = std::max(dt, 1e-4f);
    time_ += dt;
    bumpCooldown_ = std::max(0.0f, bumpCooldown_ - dt);
    stealTimer_ = std::max(0.0f, stealTimer_ - dt);

    // ---- 1. Control Input & Target Speed ---------------------------------
    const bool wantForward = ctrl.forward;
    const bool wantBack = ctrl.backward;
    const bool wantLeft = ctrl.left;
    const bool wantRight = ctrl.right;
    const bool sprint = ctrl.sprint;
    const bool quickBrake = ctrl.brake;

    float targetSpeed = 0.0f;
    if (quickBrake)
    {
        targetSpeed = 0.0f;
    }
    else if (wantForward && !wantBack)
    {
        // Smooth transition: holding W reaches trot; holding Shift gallops
        targetSpeed = sprint ? kSprintSpeed : kTrotMaxSpeed;
    }
    else if (wantBack && !wantForward)
    {
        targetSpeed = -kReverseSpeed;
    }

    turnInput_ = (wantLeft ? 1.0f : 0.0f) - (wantRight ? 1.0f : 0.0f);

    // ---- 2. Kinematics & Inertia -----------------------------------------
    const bool braking = quickBrake || (targetSpeed == 0.0f) || (targetSpeed * speed_ < 0.0f);
    const float accelRate = quickBrake ? (kBrake * 1.8f) : (braking ? kBrake : kAcceleration);
    speed_ = Approach(speed_, targetSpeed, accelRate * dt);

    // Speed-dependent turning radius (nimble at slow speeds, wide banking arcs at sprint)
    const float absSpeed = std::abs(speed_);
    const float s01 = speed01();
    float turnScale = 1.0f;
    if (absSpeed < 1.5f)
    {
        turnScale = 0.95f; // Responsive when stationary or slow walk
    }
    else
    {
        turnScale = std::clamp(1.0f - (absSpeed - 1.5f) * 0.038f, 0.52f, 1.0f);
    }
    yaw_ = WrapAngle(yaw_ + turnInput_ * kTurnRate * turnScale * dt);

    const glm::vec3 fwd = forward();
    position_ += fwd * (speed_ * dt);

    // ---- 3. World Collision: Dual Sphere Probes (Chest + Rump) -----------
    glm::vec2 totalPush(0.0f);
    for (int iter = 0; iter < 2; ++iter)
    {
        for (int probe = 0; probe < 2; ++probe)
        {
            const float offset = (probe == 0) ? kProbeOffset : -kProbeOffset;
            const glm::vec3 center = position_ + fwd * offset;
            const glm::vec2 push = world.resolveCircle(center, kProbeRadius);
            position_.x += push.x;
            position_.z += push.y;
            totalPush += push;
        }
    }
    position_.y = 0.0f;

    // Head-on collision absorbs speed and triggers cartoon recoil
    const float pushLen = glm::length(totalPush);
    if (pushLen > 1e-4f)
    {
        const glm::vec2 n = totalPush / pushLen;
        const float moveSign = (speed_ >= 0.0f) ? 1.0f : -1.0f;
        const float into = -(n.x * fwd.x + n.y * fwd.z) * moveSign;
        if (into > 0.2f)
        {
            speed_ *= 1.0f - std::clamp(into, 0.0f, 1.0f) * std::min(1.0f, 14.0f * dt);
            if (into > 0.5f && std::abs(prevSpeed_) > 3.5f && bumpCooldown_ <= 0.0f)
            {
                squash_.v += 3.5f;
                bumpCooldown_ = 0.35f;
            }
        }
    }

    // ---- 4. Speed & Acceleration Analysis --------------------------------
    const float rawAccel = (speed_ - prevSpeed_) / dt;
    prevSpeed_ = speed_;
    accelSmooth_ += (rawAccel - accelSmooth_) * ExpSmooth(10.0f, dt);

    // Stride frequency scales naturally with speed
    const float strideHz = std::clamp(1.1f + 0.16f * absSpeed, 1.1f, 3.4f);
    if (absSpeed > 0.05f)
    {
        const float dirSign = (speed_ >= 0.0f) ? 1.0f : -1.0f;
        gaitPhase_ = std::fmod(gaitPhase_ + kTwoPi * strideHz * dt * dirSign + kTwoPi * 10.0f, kTwoPi);
    }

    // Amplitude of leg swing (ramps up with speed)
    const float legAmp = std::clamp(absSpeed / 5.5f, 0.0f, 1.0f);

    // ---- 5. Two-Gait Blending: WALK <-> GALLOP ----------------------------
    // Smooth crossfade driven by kAnim speed thresholds.
    float wWalk = 1.0f, wGallop = 0.0f;
    if (absSpeed >= kAnim.gallopFullSpeed)
    {
        wWalk   = 0.0f;
        wGallop = 1.0f;
    }
    else if (absSpeed > kAnim.walkMaxSpeed)
    {
        const float t = (absSpeed - kAnim.walkMaxSpeed)
                      / (kAnim.gallopFullSpeed - kAnim.walkMaxSpeed);
        wWalk   = 1.0f - t;
        wGallop = t;
    }
    // else: pure walk (defaults above)

    std::array<LegAngles, 4> anglesWalk{}, anglesGallop{};
    float bobWalk = 0.0f, pitchWalk = 0.0f;
    float bobGallop = 0.0f, pitchGallop = 0.0f;

    computeWalk(gaitPhase_, legAmp, anglesWalk, bobWalk, pitchWalk);
    const float sprintFactor = std::clamp(
        (absSpeed - kAnim.walkMaxSpeed) / (kSprintSpeed - kAnim.walkMaxSpeed), 0.0f, 1.0f);
    computeGallop(gaitPhase_, legAmp, sprintFactor, anglesGallop, bobGallop, pitchGallop);

    // Blend leg angles
    for (size_t i = 0; i < 4; ++i)
    {
        legAngles_[i].upper   = wWalk * anglesWalk[i].upper   + wGallop * anglesGallop[i].upper;
        legAngles_[i].mid     = wWalk * anglesWalk[i].mid     + wGallop * anglesGallop[i].mid;
        legAngles_[i].lower   = wWalk * anglesWalk[i].lower   + wGallop * anglesGallop[i].lower;
        legAngles_[i].pastern = wWalk * anglesWalk[i].pastern + wGallop * anglesGallop[i].pastern;
        legAngles_[i].lift    = wWalk * anglesWalk[i].lift    + wGallop * anglesGallop[i].lift;
    }

    // Hoof contact tracking for dust effects
    currentHoofStrikes_.clear();
    for (size_t i = 0; i < 4; ++i)
    {
        if (prevLift_[i] >= 0.045f && legAngles_[i].lift < 0.045f && absSpeed > kWalkMaxSpeed * 0.85f)
        {
            glm::vec3 offset(0.0f);
            if (i == kLegFL) offset = fwd * 0.40f - right() * 0.20f;
            else if (i == kLegFR) offset = fwd * 0.40f + right() * 0.20f;
            else if (i == kLegBL) offset = -fwd * 0.45f - right() * 0.21f;
            else if (i == kLegBR) offset = -fwd * 0.45f + right() * 0.21f;

            currentHoofStrikes_.push_back(position_ + offset);
        }
        prevLift_[i] = legAngles_[i].lift;
    }

    bobY_ = wWalk * bobWalk + wGallop * bobGallop;
    const float gaitPitch = wWalk * pitchWalk + wGallop * pitchGallop;

    // Weight transfer pitch from acceleration/braking
    const float weightPitch = std::clamp(-accelSmooth_ * 0.018f, -0.25f, 0.25f);
    bodyPitch_ = gaitPitch + weightPitch;

    // ---- 6. Secondary Physics & Springs ----------------------------------
    // Centrifugal banking into turns
    roll_.step(-turnInput_ * s01 * 0.30f, 55.0f, 8.5f, dt);

    // Forward/backward inertia lean
    lean_.step(weightPitch + s01 * 0.06f, 65.0f, 8.0f, dt);

    // Neck secondary movement (reaches forward with speed, nods with stride)
    const float targetNeckPitch = s01 * 0.16f - bodyPitch_ * 0.6f;
    neckPitch_.step(targetNeckPitch, 50.0f, 6.5f, dt);
    neckYaw_.step(-turnInput_ * 0.15f * s01, 45.0f, 6.0f, dt);

    // Head nods and turns slightly into corners
    headPitch_.step(-bodyPitch_ * 0.8f + (gaitPitch * 0.5f), 70.0f, 7.5f, dt);
    headYaw_.step(-turnInput_ * 0.22f, 60.0f, 7.0f, dt);

    // Independent twitching ears
    ear_[0].step(s01 * 0.6f + std::sin(time_ * 3.5f) * 0.08f, 75.0f, 6.0f, dt);
    ear_[1].step(s01 * 0.6f + std::sin(time_ * 3.5f + 1.2f) * 0.08f, 75.0f, 6.0f, dt);

    // 3-segment cascading tail dynamics
    const float tailWag = std::sin(time_ * 5.0f) * 0.18f * (0.25f + s01);
    tail_[0].step(-bodyPitch_ * 0.5f + s01 * 0.35f, 40.0f, 4.0f, dt);
    tail_[1].step(tail_[0].x * 1.2f + tailWag, 35.0f, 3.5f, dt);
    tail_[2].step(tail_[1].x * 1.3f + tailWag * 1.4f, 30.0f, 3.0f, dt);

    // Rider stabilization: absorbs vertical shocks and balances torso
    riderBounce_.step(bobY_ * kAnim.riderBounceScale, 140.0f, 10.0f, dt);
    riderLean_.step((-bodyPitch_ * 0.8f + s01 * 0.25f) * kAnim.riderLeanScale, 65.0f, 7.5f, dt);

    // Squash & stretch recovery
    squash_.step(0.0f, 110.0f, 9.0f, dt);
    squash_.x = std::clamp(squash_.x, -1.2f, 1.2f);

    // Construct master rigid body frame
    rigid_ = Translate(position_.x, position_.y + bobY_, position_.z) *
        RotY(yaw_) *
        RotZ(roll_.x) *
        RotX(bodyPitch_);
}

glm::mat4 Horse::carryAnchor() const
{
    const float riderZ = -0.10f + std::sin(gaitPhase_) * kAnim.riderForwardBackwardMotion * speed01();
    // Right hip of the rider on horse
    return rigid_ * Translate(0.46f, 1.52f + riderBounce_.x, riderZ);
}

// ---------------------------------------------------------------------------
// Hierarchical Procedural Rendering
// ---------------------------------------------------------------------------

void Horse::draw(Renderer& r) const
{
    const float s01 = speed01();
    const float breathing = 1.0f + 0.015f * std::sin(time_ * 2.0f) * (1.0f - s01);
    const float sy = (1.0f - 0.10f * squash_.x) * breathing;
    const float sxz = 1.0f + 0.05f * squash_.x;
    const glm::mat4 root = rigid_ * Scale(sxz, sy, sxz);

    // 1. Torso, chest & hindquarters (compact, balanced equine anatomy)
    drawTorso(r, root);

    // 2. Articulated forelegs (Front Left & Front Right)
    drawForeleg(r, root, +1, legAngles_[kLegFR]);
    drawForeleg(r, root, -1, legAngles_[kLegFL]);

    // 3. Articulated hindlegs with prominent hocks (Back Left & Back Right)
    drawHindleg(r, root, +1, legAngles_[kLegBR]);
    drawHindleg(r, root, -1, legAngles_[kLegBL]);

    // 4. Cascading dynamic tail
    drawTail(r, root, s01);

    // 5. Saddle, shabrack pad & stirrups
    drawSaddleAndTack(r, root);

    // 6. Arched neck, broad sculpted head, muzzle & ears
    glm::mat4 headTransform(1.0f);
    drawNeckAndHead(r, root, s01, bodyPitch_, headTransform);

    // 7. Traditional Kazakh Bridle (Jügen)
    drawBridle(r, headTransform);

    // 8. Kipchak Warrior Rider
    glm::mat4 riderTransform(1.0f);
    drawRider(r, root, riderTransform);

    // 9. Flexible Leather Reins (Tizgin) with natural slack connecting bit to rider hands
    drawReins(r, headTransform, riderTransform);
}

void Horse::drawTorso(Renderer& r, const glm::mat4& root) const
{
    // 1. Cohesive, seamless low-poly anatomical horse torso mesh
    // Sculpted withers, deep keel chest, athletic flank tuck-up, and rounded croup.
    r.draw(*horseBodyMesh_, root * Translate(0.0f, 0.0f, 0.0f), kCoat);
}

void Horse::drawNeckAndHead(Renderer& r, const glm::mat4& root, float s01, float gaitPitch, glm::mat4& outHead) const
{
    const Assets& A = *assets_;

    // Move head and neck together
    const glm::mat4 headNeckGroup = root *
        Translate(0.00f, 0.4f, 1.15f) *
        RotX(-0.9f);

    // Neck emerges firmly from between withers and deep chest (Z = +0.34f, Y = 1.34f)
    const glm::mat4 withersBase = headNeckGroup * Translate(0.0f, 1.0f, 0.4f) * RotX(0.2f);

    // Natural forward-angled equine neck (~48 degrees forward from vertical)
    const glm::mat4 neck = withersBase *
        RotY(neckYaw_.x) *
        RotX(0.82f + neckPitch_.x + gaitPitch * 0.25f);

    // 1. Cohesive, continuous muscular equine neck mesh (arched crest, deep base)
    r.draw(*horseNeckMesh_, neck, kCoat);

    // 2. Head & Skull attached at poll / throatlatch (top of neck)
    // Head hangs forward-down at ~55 degrees from neck axis
    const glm::mat4 head = neck * Translate(0.00f, 0.8f, -0.15f) *
        RotY(headYaw_.x) *
        RotX(1.80f + headPitch_.x);
    outHead = head;

    // 3. Cohesive, low-poly equine skull mesh (broad forehead, prominent jowl, firm muzzle)
    r.draw(*horseHeadMesh_, head, kCoat);

    // Subtle dark muzzle shading at the tip
    r.draw(A.sphere(), head * Translate(0.0f, 0.490f, 0.015f) * Scale(0.056f, 0.038f, 0.045f), kMuzzle);

    // White nomad facial star marking on forehead
    r.draw(A.box(), head * Translate(0.0f, 0.130f, 0.082f) * Scale(0.036f, 0.045f, 0.006f), kWhiteMarking);

    // Subtle dark nostrils
    for (int side = -1; side <= 1; side += 2)
    {
        const float sx = static_cast<float>(side);
        r.draw(A.sphere(), head * Translate(sx * 0.045f, 0.480f, 0.058f) * Scale(0.014f, 0.020f, 0.012f), kDark);
    }

    // Small, dark expressive eyes on lateral orbital brow ridges
    for (int side = -1; side <= 1; side += 2)
    {
        const float sx = static_cast<float>(side);
        // Dark eye
        r.draw(A.sphere(), head * Translate(sx * 0.105f, 0.210f, 0.050f) * Scale(0.020f), kDark);
        // Specular highlight dot
        r.draw(A.sphere(), head * Translate(sx * 0.113f, 0.215f, 0.056f) * Scale(0.006f), glm::vec3(1.0f));
    }

    // Two small upright ears on poll (animated with ear springs)
    for (int side = -1; side <= 1; side += 2)
    {
        const float sx = static_cast<float>(side);
        const int earIdx = (side < 0) ? 0 : 1;
        const glm::mat4 ear = head * Translate(sx * 0.058f, 0.02f, -0.015f) *
            RotY(-sx * 0.14f) *
            RotX(-kHalfPi - ear_[earIdx].x) *
            Translate(0.003f, 0.075f, 0.003f) *
            Scale(0.038f, 0.13f, 0.038f);
        // Outer ear shell
        r.draw(A.cone(), ear, kCoat);
        // Inner ear hollow
        r.draw(A.cone(), ear * Scale(0.72f, 0.88f, 0.72f), glm::vec3(0.42f, 0.25f, 0.20f));
    }

    // Forelock tuft on poll
    r.draw(A.cone(), head * Translate(0.0f, 0.03f, 0.065f) * RotX(0.75f) * Scale(0.042f, 0.12f, 0.030f), kMane);

    // Stylized flowing mane along arched neck crest (+Z in neck space)
    const float maneSteps[6] = { 0.08f, 0.18f, 0.28f, 0.38f, 0.48f, 0.58f };
    for (float t : maneSteps)
    {
        const float flow = std::sin(time_ * 6.0f + t * 4.0f) * 0.022f * (0.3f + s01);
        r.draw(A.box(), neck * Translate(flow, t * 0.42f, 0.16f) *
            RotY(flow * 4.0f) *
            Scale(0.040f, 0.08f, 0.065f), kMane);
    }

}

void Horse::drawBridle(Renderer& r, const glm::mat4& head) const
{
    const Assets& A = *assets_;

    // 1. Bit Rings (Forged bronze/iron rings at corners of the mouth)
    for (int side = -1; side <= 1; side += 2)
    {
        const float sx = static_cast<float>(side);
        const glm::mat4 bitRing = head * Translate(sx * 0.066f, 0.44f, 0.010f) * RotZ(kHalfPi);
        r.draw(*bitRingMesh_, bitRing, kBronzeTrim);
    }

    // Bit mouthpiece bar through the mouth
    r.draw(A.cylinder(), head * Translate(0.0f, 0.44f, 0.010f) * RotZ(kHalfPi) * Scale(0.008f, 0.132f, 0.008f), kStirrup);

    // 2. Noseband (Тұмсықтық) - encircles bridge of nose and lower jaw
    // Nose bridge top strap
    r.draw(A.box(), head * Translate(0.0f, 0.36f, 0.072f) * Scale(0.136f, 0.020f, 0.016f), kReins);
    // Under-jaw strap
    r.draw(A.box(), head * Translate(0.0f, 0.36f, -0.067f) * Scale(0.124f, 0.018f, 0.016f), kReins);
    // Side connecting straps
    for (int side = -1; side <= 1; side += 2)
    {
        const float sx = static_cast<float>(side);
        r.draw(A.box(), head * Translate(sx * 0.066f, 0.36f, 0.002f) * Scale(0.016f, 0.020f, 0.140f), kReins);
        // Ornamental bronze studs
        r.draw(A.sphere(), head * Translate(sx * 0.052f, 0.36f, 0.078f) * Scale(0.011f), kBronzeTrim);
    }
    // Central ornamental bronze plaque (Құйма)
    r.draw(A.box(), head * Translate(0.0f, 0.36f, 0.080f) * Scale(0.032f, 0.022f, 0.010f), kBronzeTrim);

    // 3. Cheek Straps (Жақтау) - run from bit rings up each cheek to crownpiece
    for (int side = -1; side <= 1; side += 2)
    {
        const float sx = static_cast<float>(side);
        const glm::mat4 cheekStrap = head * Translate(sx * 0.070f, 0.24f, -0.010f) *
            RotX(0.10f) *
            Scale(0.015f, 0.40f, 0.018f);
        r.draw(A.box(), cheekStrap, kReins);

        // Bronze buckle at mid-cheek
        r.draw(A.box(), head * Translate(sx * 0.076f, 0.24f, -0.002f) * Scale(0.020f, 0.024f, 0.014f), kBronzeTrim);
    }

    // 4. Crownpiece / Headstall (Желкелік) - passes over the poll behind the ears
    r.draw(A.box(), head * Translate(0.0f, 0.00f, -0.025f) * Scale(0.165f, 0.022f, 0.018f), kReins);

    // 5. Browband (Маңдайлық) - across the broad forehead below the ears
    r.draw(A.box(), head * Translate(0.0f, 0.13f, 0.082f) * Scale(0.190f, 0.020f, 0.016f), kReins);

    // Central Kazakh bronze medallion (Солярлық өрнек / Күн таңба)
    r.draw(A.cylinder(), head * Translate(0.0f, 0.13f, 0.092f) * RotX(kHalfPi) * Scale(0.030f, 0.010f, 0.030f), kBronzeTrim);
    r.draw(A.sphere(), head * Translate(0.0f, 0.13f, 0.098f) * Scale(0.012f), kGold);

    // Nomadic felt/bead charm dangling beneath medallion (Кекілдік)
    r.draw(A.sphere(), head * Translate(0.0f, 0.105f, 0.088f) * Scale(0.012f, 0.020f, 0.010f), kSaddlePad);

    // 6. Throatlatch (Тамақтық) - under the throat to secure the bridle
    r.draw(A.box(), head * Translate(0.0f, 0.08f, -0.090f) * Scale(0.160f, 0.016f, 0.016f), kReins);
    // Bronze throatlatch buckle
    r.draw(A.box(), head * Translate(-0.065f, 0.08f, -0.090f) * Scale(0.016f, 0.018f, 0.014f), kBronzeTrim);
}

void Horse::drawReins(Renderer& r, const glm::mat4& head, const glm::mat4& rider) const
{
    const Assets& A = *assets_;
    const float s01 = speed01();

    // 1. Bit ring anchor points on the horse's head (at mouth corners)
    const glm::vec4 bitL_local(-0.066f, 0.44f, 0.010f, 1.0f);
    const glm::vec4 bitR_local(+0.066f, 0.44f, 0.010f, 1.0f);
    const glm::vec3 bitL = glm::vec3(head * bitL_local);
    const glm::vec3 bitR = glm::vec3(head * bitR_local);

    // 2. Rider hand anchor points
    glm::vec3 handL, handR;
    if (carrying_)
    {
        // When carrying the goat, right hand secures goat on hip; left hand holds BOTH reins over withers
        const glm::vec4 handGathered(-0.04f, 0.28f, 0.28f, 1.0f);
        handL = glm::vec3(rider * handGathered);
        handR = glm::vec3(rider * (handGathered + glm::vec4(0.02f, 0.0f, -0.01f, 0.0f)));
    }
    else
    {
        // Hands positioned over withers gripping reins
        const glm::vec4 handL_local(-0.08f, 0.28f, 0.28f, 1.0f);
        const glm::vec4 handR_local(+0.08f, 0.28f, 0.28f, 1.0f);
        handL = glm::vec3(rider * handL_local);
        handR = glm::vec3(rider * handR_local);
    }

    // Dynamic gravity sag and wind fluttering
    const float baseSag = 0.055f * (1.0f - 0.35f * s01);
    const float flutter = std::sin(time_ * 7.5f) * 0.006f * s01;

    // Draw left and right flexible leather reins
    for (int side = -1; side <= 1; side += 2)
    {
        const glm::vec3& pStart = (side < 0) ? bitL : bitR;
        const glm::vec3& pEnd = (side < 0) ? handL : handR;
        const float sx = static_cast<float>(side);

        constexpr int kSteps = 7;
        glm::vec3 prevPoint = pStart;

        for (int step = 1; step <= kSteps; ++step)
        {
            const float t = static_cast<float>(step) / static_cast<float>(kSteps);
            glm::vec3 curr = glm::mix(pStart, pEnd, t);

            // Natural catenary arc: parabolic / sine sag in gravity direction
            const float arc = std::sin(t * kPi);
            curr.y -= (baseSag * arc) + (flutter * arc);

            // Outward lateral curve around the horse's neck so reins don't clip through flesh
            curr.x += sx * 0.038f * arc;

            // Draw leather segment
            const glm::vec3 delta = curr - prevPoint;
            const float len = glm::length(delta);
            if (len > 0.0005f)
            {
                const glm::vec3 mid = (prevPoint + curr) * 0.5f;
                const glm::mat4 segTransform = Translate(mid) * AlignY(delta) * Scale(0.014f, len, 0.014f);
                r.draw(A.cylinder(), segTransform, kReins);
                // Joint smoothing bead
                r.draw(A.sphere(), Translate(curr) * Scale(0.015f), kReins);
            }
            prevPoint = curr;
        }

        // Ornamental bronze rein buckle near the bit
        const glm::vec3 bucklePos = glm::mix(pStart, pEnd, 0.16f);
        r.draw(A.box(), Translate(bucklePos) * Scale(0.022f, 0.018f, 0.024f), kBronzeTrim);
    }
}

void Horse::drawForeleg(Renderer& r, const glm::mat4& root, int side, const LegAngles& a) const
{
    const Assets& A = *assets_;
    const float sx = static_cast<float>(side);

    constexpr float kUpperLen = 0.18f;
    constexpr float kForeLen = 0.62f;
    constexpr float kCannonLen = 0.24f;
    constexpr float kPasternLen = 0.10f;

    // 1. Плечо
    const glm::mat4 shoulder = root * Translate(sx * 0.15f, 1.16f, 0.64f);

    // 2. Плечевая кость
    const glm::mat4 upper = shoulder * RotX(a.upper) * RotZ(-sx * 0.03f);
    r.draw(*foreUpperMesh_, upper * Translate(0.0f, -kUpperLen * 0.5f, -0.05f) * RotX(0.20f), kCoat);

    // 3. Локоть (a.mid > 0 -> предплечье назад)
    const glm::mat4 elbow = upper * Translate(0.0f, -kUpperLen, -0.14f) * RotX(a.mid);

    // 4. Предплечье (центр меша = половина длины)
    r.draw(*forearmMesh_, elbow * Translate(0.0f, -kForeLen * 0.5f, 0.02f) * RotX(-0.10f), kCoat);

    // 5. Запястье
    const glm::mat4 knee = elbow * Translate(0.0f, -kForeLen, 0.04f);
    r.draw(A.box(), knee * Scale(0.075f, 0.070f, 0.075f), kCoatDark);

    // 6. Cannon: +a.lower сгибает кость назад к брюху
    const glm::mat4 cannon = knee * RotX(a.lower);
    r.draw(*foreLowerMesh_, cannon * Translate(0.0f, -kCannonLen * 0.5f, 0.0f), kCoatDark);

    // 7. Фетлок
    const glm::mat4 fetlock = cannon * Translate(0.0f, -kCannonLen, 0.0f);
    r.draw(A.sphere(), fetlock * Scale(0.048f, 0.048f, 0.052f), kCoatDark);

    // 8. Бабка
    const glm::mat4 pastern = fetlock * RotX(-0.50f + a.pastern);
    r.draw(*pasternMesh_, pastern * Translate(0.0f, -kPasternLen * 0.5f, 0.025f), kCoatDark);

    // 9. Копыто
    const glm::mat4 hoof = pastern * Translate(0.0f, -kPasternLen, 0.03f);
    r.draw(A.sphere(), hoof * Translate(0.0f, 0.015f, 0.0f) * Scale(0.080f, 0.018f, 0.086f), kHoofBand);
    r.draw(A.cylinder(), hoof * Translate(0.0f, -0.020f, 0.008f) * Scale(0.082f, 0.058f, 0.090f), kHoof);
}

void Horse::drawHindleg(Renderer& r, const glm::mat4& root, int side, const LegAngles& a) const
{
    const Assets& A = *assets_;
    const float sx = static_cast<float>(side);

    constexpr float kThighLen = 0.37f;
    constexpr float kGaskinLen = 0.47f;
    constexpr float kCannonLen = 0.32f;
    constexpr float kPasternLen = 0.11f;

    constexpr float kThighRest = -0.35f;
    constexpr float kGaskinRest = 0.46f;
    constexpr float kCannonRest = -(kThighRest + kGaskinRest);   // -0.11

    // 1. Тазобедренный сустав
    const glm::mat4 hip = root * Translate(sx * 0.16f, 1.27f, -0.6f);

    // 2. Бедро
    const glm::mat4 thigh = hip * RotX(kThighRest + a.upper) * RotZ(-sx * 0.04f);
    r.draw(*hindUpperMesh_, thigh * Translate(0.0f, -kThighLen * 0.5f, 0.06f) * RotX(-0.14f), kCoat);

    // 3. Колено
    const glm::mat4 stifle = thigh * Translate(0.0f, -kThighLen, 0.12f);

    // 4. Голень
    const glm::mat4 gaskin = stifle * RotX(kGaskinRest + a.mid);
    r.draw(*hindMidMesh_, gaskin * Translate(0.0f, -kGaskinLen * 0.5f, -0.06f) * RotX(0.16f), kCoat);

    // 5. Скакательный сустав
    const glm::mat4 hock = gaskin * Translate(0.0f, -kGaskinLen, -0.12f);
    r.draw(A.box(), hock * Scale(0.070f, 0.085f, 0.078f), kCoatDark);
    r.draw(A.cone(), hock * Translate(0.0f, 0.030f, -0.052f) * RotX(kHalfPi) * Scale(0.035f, 0.070f, 0.035f), kCoatDark);

    // 6. Плюсна: вертикальна в покое; a.lower > 0 сгибает сустав
    const glm::mat4 cannon = hock * RotX(kCannonRest - a.lower);
    r.draw(*hindLowerMesh_, cannon * Translate(0.0f, -kCannonLen * 0.5f, 0.0f), kCoatDark);

    // 7. Путовый сустав
    const glm::mat4 fetlock = cannon * Translate(0.0f, -kCannonLen, 0.0f);
    r.draw(A.sphere(), fetlock * Scale(0.048f, 0.048f, 0.052f), kCoatDark);

    // 8. Бабка
    const glm::mat4 pastern = fetlock * RotX(-0.50f + a.pastern);
    r.draw(*pasternMesh_, pastern * Translate(0.0f, -kPasternLen * 0.5f, 0.025f), kCoatDark);

    // 9. Копыто
    const glm::mat4 hoof = pastern * Translate(0.0f, -kPasternLen, 0.03f);
    r.draw(A.sphere(), hoof * Translate(0.0f, 0.015f, 0.0f) * Scale(0.078f, 0.018f, 0.084f), kHoofBand);
    r.draw(A.cylinder(), hoof * Translate(0.0f, -0.020f, 0.008f) * Scale(0.080f, 0.058f, 0.088f), kHoof);
}void Horse::drawTail(Renderer& r, const glm::mat4& root, float s01) const
{
    const Assets& A = *assets_;

    // Dock of the tail at top of croup (Z = -0.56f, Y = 1.35f)
    const glm::mat4 tailDock = root * Translate(0.0f, 1.35f, -0.56f) *
        RotZ(tail_[0].x * 0.4f) *
        RotX(0.38f + tail_[0].x);

    // Base segment (arches back)
    r.draw(*tailMesh_, tailDock * Translate(0.0f, -0.14f, 0.0f), kMane);
    r.draw(A.sphere(), tailDock * Translate(0.0f, -0.28f, 0.0f) * Scale(0.11f), kMane);

    // Mid cascading segment (delayed secondary motion)
    const glm::mat4 tailMid = tailDock * Translate(0.0f, -0.28f, 0.0f) *
        RotZ(tail_[1].x * 0.5f) *
        RotX(0.22f + tail_[1].x);
    r.draw(*tailMesh_, tailMid * Translate(0.0f, -0.14f, 0.0f) * Scale(0.95f), kMane);
    r.draw(A.sphere(), tailMid * Translate(0.0f, -0.28f, 0.0f) * Scale(0.12f), kMane);

    // Lower plume flaring out dynamically at speed
    const glm::mat4 tailPlume = tailMid * Translate(0.0f, -0.26f, 0.0f) *
        RotZ(tail_[2].x * 0.6f) *
        RotX(0.28f + tail_[2].x + s01 * 0.45f);
    r.draw(*tailMesh_, tailPlume * Translate(0.0f, -0.16f, 0.0f) * Scale(1.08f, 1.15f, 1.08f), kMane);
    r.draw(A.sphere(), tailPlume * Translate(0.0f, -0.32f, 0.0f) * Scale(0.14f, 0.18f, 0.14f), kMane);
}

void Horse::drawSaddleAndTack(Renderer& r, const glm::mat4& root) const
{
    const Assets& A = *assets_;

    // Colorful saddle blanket (felt shabrack) centered on saddle dip (Z = -0.10f, Y = 1.435f)
    r.draw(A.box(), root * Translate(0.0f, 1.435f, -0.10f) * Scale(0.60f, 0.032f, 0.58f), kSaddlePad);

    // Contoured leather saddle nestled behind the withers
    r.draw(A.sphere(), root * Translate(0.0f, 1.465f, -0.10f) * Scale(0.40f, 0.08f, 0.40f), kSaddle);
    // Saddle pommel (front horn) & cantle (rear lip)
    r.draw(A.sphere(), root * Translate(0.0f, 1.505f, 0.06f) * Scale(0.10f, 0.07f, 0.07f), kSaddle);
    r.draw(A.sphere(), root * Translate(0.0f, 1.515f, -0.26f) * Scale(0.14f, 0.08f, 0.08f), kSaddle);

    // Stirrup leathers & iron stirrups hanging on both sides
    for (int side = -1; side <= 1; side += 2)
    {
        const float sx = static_cast<float>(side);
        r.draw(A.cylinder(), root * Translate(sx * 0.33f, 1.25f, -0.10f) * Scale(0.018f, 0.38f, 0.018f), kReins);
        r.draw(A.cylinder(), root * Translate(sx * 0.33f, 1.05f, -0.10f) * Scale(0.070f, 0.024f, 0.070f), kStirrup);
    }
}

void Horse::drawRider(Renderer& r, const glm::mat4& root, glm::mat4& outRider) const
{
    const Assets& A = *assets_;

    // Rider hips firmly anchored to saddle on horse back (Z = -0.10f, Y = 1.53f)
    // Subtle rhythmic forward-backward sway with gait phase
    const float riderZ = -0.10f + std::sin(gaitPhase_) * kAnim.riderForwardBackwardMotion * speed01();
    const glm::mat4 riderBase = root * Translate(0.0f, 1.53f + riderBounce_.x, riderZ);

    const float turnLean = -turnInput_ * speed01() * 0.28f;
    const float accelLean = -accelSmooth_ * 0.018f;
    const glm::mat4 rider = riderBase * RotX(riderLean_.x + accelLean) * RotZ(-roll_.x * 0.45f + turnLean);
    outRider = rider;

    // 1. Torso with traditional Kipchak tunic / vest
    r.draw(*torsoMesh_, rider * Translate(0.0f, 0.34f, 0.0f), vestColor_);
    // Bronze/Gold studded warrior belt
    r.draw(A.cylinder(), rider * Translate(0.0f, 0.19f, 0.0f) * Scale(0.205f, 0.060f, 0.205f), kGold);
    r.draw(A.sphere(), rider * Translate(0.0f, 0.19f, 0.21f) * Scale(0.042f, 0.042f, 0.024f), kBronzeTrim); // Belt buckle

    // 2. Rider Head & Kipchak Warrior Features
    const glm::mat4 riderHead = rider * Translate(0.0f, 0.82f, 0.0f);

    // Head / Facial structure
    r.draw(A.sphere(), riderHead * Scale(0.180f, 0.195f, 0.185f), kSkin);

    // Determined warrior eyes with dark brows
    for (int side = -1; side <= 1; side += 2)
    {
        const float sx = static_cast<float>(side);
        // Eyebrow
        r.draw(A.box(), riderHead * Translate(sx * 0.062f, 0.048f, 0.165f) * RotZ(-sx * 0.15f) * Scale(0.042f, 0.012f, 0.02f), kDark);
        // Eye
        r.draw(A.sphere(), riderHead * Translate(sx * 0.062f, 0.018f, 0.168f) * Scale(0.021f), kDark);
        r.draw(A.sphere(), riderHead * Translate(sx * 0.069f, 0.022f, 0.174f) * Scale(0.008f), glm::vec3(1.0f));
    }

    // Defined warrior nose
    r.draw(A.cone(), riderHead * Translate(0.0f, -0.01f, 0.180f) * RotX(kHalfPi) * Scale(0.025f, 0.072f, 0.034f), kSkin);

    // 3. Thick, Stylized Medieval Kipchak Moustache
    // Central chevron base below nose
    r.draw(A.box(), riderHead * Translate(0.0f, -0.042f, 0.182f) * Scale(0.046f, 0.025f, 0.030f), kMoustache);
    // Swept curved wings drooping boldly over the corners of the mouth
    for (int side = -1; side <= 1; side += 2)
    {
        const float sx = static_cast<float>(side);
        const glm::mat4 wing = riderHead * Translate(sx * 0.044f, -0.050f, 0.176f) *
            RotZ(-sx * 0.32f) *
            RotY(sx * 0.16f);
        r.draw(*moustacheMesh_, wing * RotZ(kHalfPi), kMoustache);

        const glm::mat4 droop = riderHead * Translate(sx * 0.078f, -0.074f, 0.158f) *
            RotZ(-sx * 1.15f);
        r.draw(A.cone(), droop * Scale(0.021f, 0.062f, 0.021f), kMoustache);
    }

    // 4. Medieval Kipchak Conical Iron Helmet
    const glm::mat4 helmBase = riderHead * Translate(0.0f, 0.09f, 0.0f);

    // Main conical iron skull tapering to a pointed apex
    r.draw(A.cone(), helmBase * Translate(0.0f, 0.15f, 0.0f) * Scale(0.220f, 0.31f, 0.230f), kIronHelmet);
    // Upper tapering spire
    r.draw(A.cone(), helmBase * Translate(0.0f, 0.29f, 0.0f) * Scale(0.120f, 0.21f, 0.120f), kIronHelmet);

    // Pointed top spire & bronze finial
    r.draw(A.sphere(), helmBase * Translate(0.0f, 0.40f, 0.0f) * Scale(0.030f), kBronzeTrim);
    r.draw(A.cone(), helmBase * Translate(0.0f, 0.45f, 0.0f) * Scale(0.015f, 0.075f, 0.015f), kBronzeTrim);

    // Metal Brow Band (Bronze rim encircling the helmet base with front nasal arch)
    r.draw(A.cylinder(), helmBase * Translate(0.0f, 0.01f, 0.0f) * Scale(0.225f, 0.034f, 0.235f), kBronzeTrim);
    // Nasal guard projection
    r.draw(A.box(), helmBase * Translate(0.0f, -0.015f, 0.220f) * Scale(0.028f, 0.042f, 0.024f), kBronzeTrim);

    // Protective Iron Cheek Guards
    for (int side = -1; side <= 1; side += 2)
    {
        const float sx = static_cast<float>(side);
        const glm::mat4 cheek = helmBase * Translate(sx * 0.180f, -0.065f, 0.032f) *
            RotZ(-sx * 0.15f) *
            RotY(-sx * 0.20f);
        r.draw(A.box(), cheek * Scale(0.024f, 0.125f, 0.078f), kIronHelmet);
        r.draw(A.sphere(), cheek * Translate(sx * 0.014f, -0.018f, 0.0f) * Scale(0.015f), kBronzeTrim);
    }

    // Chainmail Neck Protection (Aventail / Camail)
    r.draw(A.cylinder(), helmBase * Translate(0.0f, -0.075f, -0.028f) * Scale(0.205f, 0.135f, 0.205f), kAventail);
    r.draw(A.cylinder(), helmBase * Translate(0.0f, -0.18f, -0.038f) * Scale(0.245f, 0.115f, 0.235f), kAventail);
    r.draw(A.box(), helmBase * Translate(0.0f, -0.14f, -0.135f) * Scale(0.27f, 0.17f, 0.058f), kAventail);

    // 5. Rider Legs in Stirrups
    for (int side = -1; side <= 1; side += 2)
    {
        const float sx = static_cast<float>(side);
        const glm::mat4 hip = rider * Translate(sx * 0.28f, 0.17f, 0.0f) *
            RotZ(sx * 0.40f) *
            RotX(-0.32f);
        r.draw(*riderLegMesh_, hip * Translate(0.0f, -0.21f, 0.0f), glm::vec3(0.18f, 0.20f, 0.45f));
        // Leather riding boots
        r.draw(A.sphere(), hip * Translate(0.0f, -0.42f, 0.04f) * Scale(0.105f, 0.085f, 0.145f), kDark);
    }

    // 6. Rider Arms (Visibly holding reins / goat)
    for (int side = -1; side <= 1; side += 2)
    {
        const float sx = static_cast<float>(side);
        const glm::mat4 shoulder = rider * Translate(sx * 0.22f, 0.54f, 0.0f);
        glm::mat4 arm;
        if (side > 0 && carrying_)
        {
            // Reaching downward-right to secure the goat on the right hip
            arm = shoulder * RotZ(0.95f) * RotX(-0.25f);
        }
        else if (stealTimer_ > 0.0f)
        {
            // Steal grab/snatch reach animation: arm thrusts forward and down toward opponent
            const float progress = 1.0f - std::clamp(stealTimer_ / 0.40f, 0.0f, 1.0f);
            const float reach = std::sin(progress * kPi);
            arm = shoulder * RotZ(sx * 0.35f + (side > 0 ? 0.20f * reach : 0.0f)) *
                             RotX(-1.15f - 0.35f * reach);
        }
        else
        {
            // Hands reach forward and slightly down over withers, gripping reins firmly
            arm = shoulder * RotZ(sx * 0.20f) * RotX(-0.80f);
        }
        r.draw(*armMesh_, arm * Translate(0.0f, -0.16f, 0.0f), vestColor_);
        // Hands with closed grip around reins
        r.draw(A.sphere(), arm * Translate(0.0f, -0.34f, 0.0f) * Scale(0.072f), kSkin);
    }
    
}

const char* Horse::gaitName() const
{
    const float s = std::abs(speed_);
    if (s < 0.20f) return "IDLE";
    if (s <= kAnim.walkMaxSpeed) return "WALK";
    return "GALLOP";
}
#include "Horse.h"

#include <GLFW/glfw3.h>

namespace
{
// Traditional Kazakh Steppe colour palette
const glm::vec3 kCoat(0.56f, 0.30f, 0.15f);       // Rich bay / chestnut equine coat
const glm::vec3 kCoatDark(0.42f, 0.22f, 0.10f);   // Lower leg points & muscle shading
const glm::vec3 kCoatLight(0.78f, 0.54f, 0.38f);  // Muzzle, soft belly & flank highlight
const glm::vec3 kMane(0.12f, 0.08f, 0.06f);       // Dark, flowing mane and tail hair
const glm::vec3 kHoof(0.18f, 0.15f, 0.13f);       // Keratin hooves
const glm::vec3 kHoofBand(0.72f, 0.60f, 0.48f);   // Coronet band above hooves
const glm::vec3 kSaddle(0.14f, 0.48f, 0.52f);     // Teal / turquoise leather saddle
const glm::vec3 kSaddlePad(0.82f, 0.20f, 0.16f);  // Traditional red saddle blanket
const glm::vec3 kStirrup(0.75f, 0.72f, 0.68f);    // Iron stirrups
const glm::vec3 kReins(0.24f, 0.16f, 0.10f);      // Leather reins & bridle
const glm::vec3 kVest(0.80f, 0.14f, 0.14f);       // Rider vest (traditional red)
const glm::vec3 kSkin(1.00f, 0.80f, 0.66f);       // Rider skin
const glm::vec3 kFelt(0.96f, 0.95f, 0.90f);       // Traditional felt hat (kalpak)
const glm::vec3 kDark(0.10f, 0.08f, 0.08f);       // Eyes, boots, trim
const glm::vec3 kGold(1.00f, 0.80f, 0.15f);       // Belt, ornamental studs
}

void Horse::init(Assets& assets)
{
    assets_ = &assets;

    // Muscular equine torso segments
    chestMesh_      = &assets.capsule(0.38f, 0.52f);
    barrelMesh_     = &assets.capsule(0.35f, 0.48f);
    croupMesh_      = &assets.capsule(0.37f, 0.52f);

    // Arched neck & elongated head
    neckLowerMesh_  = &assets.capsule(0.22f, 0.46f);
    neckUpperMesh_  = &assets.capsule(0.18f, 0.42f);
    headMesh_       = &assets.capsule(0.15f, 0.28f);
    muzzleMesh_     = &assets.capsule(0.11f, 0.32f);

    // Articulated foreleg segments (front leg)
    foreUpperMesh_  = &assets.capsule(0.11f, 0.36f);
    foreLowerMesh_  = &assets.capsule(0.075f, 0.34f);

    // Articulated hindleg segments (back leg with prominent hock)
    hindUpperMesh_  = &assets.capsule(0.14f, 0.40f);
    hindMidMesh_    = &assets.capsule(0.11f, 0.36f);
    hindLowerMesh_  = &assets.capsule(0.075f, 0.34f);

    // Pastern & tail
    pasternMesh_    = &assets.capsule(0.06f, 0.12f);
    tailMesh_       = &assets.capsule(0.08f, 0.34f);

    // Rider meshes
    torsoMesh_      = &assets.capsule(0.20f, 0.32f);
    riderLegMesh_   = &assets.capsule(0.09f, 0.28f);
    armMesh_        = &assets.capsule(0.07f, 0.20f);
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

    rigid_ = Translate(position_) * RotY(yaw_);
}

// ---------------------------------------------------------------------------
// Biomechanical Locomotion Solvers
// ---------------------------------------------------------------------------

// 1. Four-beat lateral walking cycle (no suspension phase, 2-3 feet grounded)
void Horse::computeWalk(float phase, float legAmp, std::array<LegAngles, 4>& outAngles,
                        float& outBob, float& outPitch) const
{
    // Equine walk footfall sequence: BL -> FL -> BR -> FR
    const float offsets[4] = {
        0.25f * kTwoPi, // FL
        0.75f * kTwoPi, // FR
        0.00f * kTwoPi, // BL
        0.50f * kTwoPi  // BR
    };

    for (int i = 0; i < 4; ++i)
    {
        const float localPhase = std::fmod(phase + offsets[i] + kTwoPi * 2.0f, kTwoPi) / kTwoPi;
        LegAngles a;

        if (localPhase < 0.65f) // Stance phase (65% of cycle)
        {
            const float u = localPhase / 0.65f;
            // Linear progression from forward touch to backward push
            a.upper = (0.5f - u) * 0.44f * legAmp;
            // Subtle elastic flexion under weight
            a.lower = 0.06f + 0.08f * std::sin(u * kPi);
            a.mid   = -0.04f - 0.06f * std::sin(u * kPi);
            // Pastern absorbs impact
            a.pastern = -0.12f + 0.24f * std::sin(u * kPi);
            a.lift = 0.0f;
        }
        else // Swing phase (35% of cycle)
        {
            const float s = (localPhase - 0.65f) / 0.35f;
            a.upper = (-0.22f + s * 0.44f) * legAmp;
            // Knee and hock flex upward for ground clearance
            const float flex = std::sin(s * kPi);
            a.lower = 0.60f * flex;
            a.mid   = -0.45f * flex;
            a.pastern = 0.20f * flex;
            a.lift  = 0.11f * flex;
        }
        outAngles[static_cast<size_t>(i)] = a;
    }

    // Gentle 4-beat body oscillation
    outBob = std::abs(std::sin(phase * 2.0f)) * 0.025f * legAmp;
    outPitch = std::sin(phase) * 0.02f * legAmp;
}

// 2. Two-beat diagonal trot (diagonal pairs: FL+BR, FR+BL, springy suspension)
void Horse::computeTrot(float phase, float legAmp, std::array<LegAngles, 4>& outAngles,
                        float& outBob, float& outPitch) const
{
    // Diagonal pairs synchronized: Pair 0 (FL + BR), Pair 1 (FR + BL)
    const float offsets[4] = {
        0.00f, // FL
        kPi,   // FR
        kPi,   // BL
        0.00f  // BR
    };

    for (int i = 0; i < 4; ++i)
    {
        const float localPhase = std::fmod(phase + offsets[i] + kTwoPi * 2.0f, kTwoPi) / kTwoPi;
        LegAngles a;

        if (localPhase < 0.48f) // Stance phase (48% of cycle)
        {
            const float u = localPhase / 0.48f;
            a.upper = (0.5f - u) * 0.72f * legAmp;
            a.lower = 0.08f + 0.16f * std::sin(u * kPi);
            a.mid   = -0.06f - 0.12f * std::sin(u * kPi);
            a.pastern = -0.18f + 0.36f * std::sin(u * kPi);
            a.lift = 0.0f;
        }
        else // Swing phase (52% of cycle, includes moment of suspension)
        {
            const float s = (localPhase - 0.48f) / 0.52f;
            a.upper = (-0.36f + s * 0.72f) * legAmp;
            // High knee & hock action typical of a noble trotting horse
            const float flex = std::sin(s * kPi);
            a.lower = 0.95f * flex;
            a.mid   = -0.75f * flex;
            a.pastern = 0.32f * flex;
            a.lift  = 0.22f * flex;
        }
        outAngles[static_cast<size_t>(i)] = a;
    }

    // Springy double bounce per stride
    outBob = (std::sin(phase * 2.0f - kHalfPi) * 0.5f + 0.5f) * 0.075f * legAmp;
    outPitch = std::sin(phase * 2.0f) * 0.035f * legAmp;
}

// 3. Asymmetric Canter / Gallop with true suspension phase and rocking surge
void Horse::computeGallop(float phase, float legAmp, float sprintFactor,
                          std::array<LegAngles, 4>& outAngles,
                          float& outBob, float& outPitch) const
{
    // Right-lead gallop footfall sequence: BL -> BR -> FL -> FR -> Airborne suspension
    const float offsets[4] = {
        0.28f * kTwoPi, // FL (trailing foreleg)
        0.42f * kTwoPi, // FR (leading foreleg - reaches furthest forward)
        0.00f * kTwoPi, // BL (trailing hindleg - initiates drive)
        0.16f * kTwoPi  // BR (leading hindleg)
    };

    for (int i = 0; i < 4; ++i)
    {
        const float localPhase = std::fmod(phase + offsets[i] + kTwoPi * 2.0f, kTwoPi) / kTwoPi;
        const bool isFore = (i < 2);
        const bool isLead = (i == 1 || i == 3);
        LegAngles a;

        if (localPhase < 0.40f) // Ground strike & drive phase
        {
            const float u = localPhase / 0.40f;
            const float reach = (isLead ? 0.95f : 0.85f) * legAmp;
            a.upper = (0.5f - u) * reach;
            a.lower = 0.12f + 0.22f * std::sin(u * kPi);
            a.mid   = -0.10f - 0.18f * std::sin(u * kPi);
            a.pastern = -0.25f + 0.45f * std::sin(u * kPi);
            a.lift = 0.0f;
        }
        else // Airborne suspension & forward gathering
        {
            const float s = (localPhase - 0.40f) / 0.60f;
            const float flex = std::sin(s * kPi);
            const float leadBonus = isLead ? 0.15f : 0.0f;

            if (isFore)
            {
                // Forelegs reach far ahead during suspension
                a.upper = (-0.45f + s * (0.90f + leadBonus)) * legAmp;
                a.lower = (0.80f - s * 0.40f) * flex;
                a.mid   = 0.0f;
                a.pastern = 0.35f * flex;
            }
            else
            {
                // Hindlegs tuck powerfully under the belly to gather for the next push
                a.upper = (-0.55f + s * (0.95f + leadBonus)) * legAmp;
                a.lower = 1.15f * flex;
                a.mid   = -0.90f * flex;
                a.pastern = 0.28f * flex;
            }
            a.lift = (0.24f + sprintFactor * 0.08f) * flex;
        }
        outAngles[static_cast<size_t>(i)] = a;
    }

    // Dynamic vertical lift: body rises high during suspension
    const float susp = std::sin(phase - 1.2f);
    outBob = (susp > 0.0f ? susp * (0.10f + sprintFactor * 0.06f) : susp * 0.04f) * legAmp;

    // Rocking-horse pitch: rears up on hind drive (+), plunges down on forehand landing (-)
    outPitch = std::sin(phase - 0.4f) * (0.12f + sprintFactor * 0.05f) * legAmp;
}

void Horse::update(float dt, const Input& in, const World& world)
{
    dt = std::max(dt, 1e-4f);
    time_ += dt;
    bumpCooldown_ = std::max(0.0f, bumpCooldown_ - dt);

    // ---- 1. Player Input & Target Speed ----------------------------------
    const bool wantForward = in.down(GLFW_KEY_W) || in.down(GLFW_KEY_UP);
    const bool wantBack    = in.down(GLFW_KEY_S) || in.down(GLFW_KEY_DOWN);
    const bool wantLeft    = in.down(GLFW_KEY_A) || in.down(GLFW_KEY_LEFT);
    const bool wantRight   = in.down(GLFW_KEY_D) || in.down(GLFW_KEY_RIGHT);
    const bool sprint      = in.down(GLFW_KEY_LEFT_SHIFT) || in.down(GLFW_KEY_RIGHT_SHIFT);

    float targetSpeed = 0.0f;
    if (wantForward && !wantBack)
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
    const bool braking = (targetSpeed == 0.0f) || (targetSpeed * speed_ < 0.0f);
    const float accelRate = braking ? kBrake : kAcceleration;
    speed_ = Approach(speed_, targetSpeed, accelRate * dt);

    // Speed-dependent turning radius (wider arcs at full sprint)
    float turnScale = std::clamp(0.35f + std::abs(speed_) / 6.5f, 0.35f, 1.0f);
    if (std::abs(speed_) > 8.0f) turnScale *= 0.75f;
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
    const float absSpeed = std::abs(speed_);
    const float s01 = speed01();
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

    // Amplitude of leg swing
    const float legAmp = std::clamp(absSpeed / 6.0f, 0.0f, 1.0f);

    // ---- 5. Smooth Gait State Blending -----------------------------------
    float wWalk = 0.0f, wTrot = 0.0f, wGallop = 0.0f;
    if (absSpeed < 3.2f)
    {
        wWalk = 1.0f;
    }
    else if (absSpeed < 4.8f)
    {
        const float t = (absSpeed - 3.2f) / 1.6f;
        wWalk = 1.0f - t;
        wTrot = t;
    }
    else if (absSpeed < 7.6f)
    {
        wTrot = 1.0f;
    }
    else if (absSpeed < 9.6f)
    {
        const float t = (absSpeed - 7.6f) / 2.0f;
        wTrot = 1.0f - t;
        wGallop = t;
    }
    else
    {
        wGallop = 1.0f;
    }

    std::array<LegAngles, 4> anglesWalk{}, anglesTrot{}, anglesGallop{};
    float bobWalk = 0.0f, pitchWalk = 0.0f;
    float bobTrot = 0.0f, pitchTrot = 0.0f;
    float bobGallop = 0.0f, pitchGallop = 0.0f;

    computeWalk(gaitPhase_, legAmp, anglesWalk, bobWalk, pitchWalk);
    computeTrot(gaitPhase_, legAmp, anglesTrot, bobTrot, pitchTrot);
    const float sprintFactor = std::clamp((absSpeed - kTrotMaxSpeed) / (kSprintSpeed - kTrotMaxSpeed), 0.0f, 1.0f);
    computeGallop(gaitPhase_, legAmp, sprintFactor, anglesGallop, bobGallop, pitchGallop);

    // Blend all leg angles & body oscillations
    for (size_t i = 0; i < 4; ++i)
    {
        legAngles_[i].upper   = wWalk * anglesWalk[i].upper   + wTrot * anglesTrot[i].upper   + wGallop * anglesGallop[i].upper;
        legAngles_[i].mid     = wWalk * anglesWalk[i].mid     + wTrot * anglesTrot[i].mid     + wGallop * anglesGallop[i].mid;
        legAngles_[i].lower   = wWalk * anglesWalk[i].lower   + wTrot * anglesTrot[i].lower   + wGallop * anglesGallop[i].lower;
        legAngles_[i].pastern = wWalk * anglesWalk[i].pastern + wTrot * anglesTrot[i].pastern + wGallop * anglesGallop[i].pastern;
        legAngles_[i].lift    = wWalk * anglesWalk[i].lift    + wTrot * anglesTrot[i].lift    + wGallop * anglesGallop[i].lift;
    }

    bobY_ = wWalk * bobWalk + wTrot * bobTrot + wGallop * bobGallop;
    const float gaitPitch = wWalk * pitchWalk + wTrot * pitchTrot + wGallop * pitchGallop;

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
    riderBounce_.step(bobY_ * 0.65f, 140.0f, 10.0f, dt);
    riderLean_.step(-bodyPitch_ * 0.8f + s01 * 0.25f, 65.0f, 7.5f, dt);

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
    // Right hip of the rider
    return rigid_ * Translate(-0.52f, 1.58f + riderBounce_.x, 0.05f);
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

    // 1. Torso, chest & hindquarters
    drawTorso(r, root);

    // 2. Neck, head, muzzle, ears & mane
    drawNeckAndHead(r, root, s01, bodyPitch_);

    // 3. Articulated forelegs (Front Left & Front Right)
    drawForeleg(r, root, +1, legAngles_[kLegFR]);
    drawForeleg(r, root, -1, legAngles_[kLegFL]);

    // 4. Articulated hindlegs with prominent hocks (Back Left & Back Right)
    drawHindleg(r, root, +1, legAngles_[kLegBR]);
    drawHindleg(r, root, -1, legAngles_[kLegBL]);

    // 5. Cascading tail
    drawTail(r, root, s01);

    // 6. Saddle & tack
    drawSaddleAndTack(r, root);

    // 7. Stabilized rider
    drawRider(r, root);
}

void Horse::drawTorso(Renderer& r, const glm::mat4& root) const
{
    const Assets& A = *assets_;

    // Deep, powerful chest & withers (forequarters)
    const glm::mat4 chest = root * Translate(0.0f, 1.40f, 0.38f) * RotX(kHalfPi - 0.10f);
    r.draw(*chestMesh_, chest, kCoat);

    // Muscular withers ridge above shoulders
    r.draw(A.sphere(), root * Translate(0.0f, 1.56f, 0.44f) * Scale(0.18f, 0.14f, 0.28f), kCoat);

    // Midsection / ribcage (barrel)
    const glm::mat4 barrel = root * Translate(0.0f, 1.34f, -0.06f) * RotX(kHalfPi);
    r.draw(*barrelMesh_, barrel, kCoat);
    // Soft belly gradient
    r.draw(A.sphere(), root * Translate(0.0f, 1.08f, 0.02f) * Scale(0.33f, 0.22f, 0.65f), kCoatLight);

    // Muscular, rounded hindquarters (croup & rump)
    const glm::mat4 croup = root * Translate(0.0f, 1.38f, -0.48f) * RotX(kHalfPi + 0.10f);
    r.draw(*croupMesh_, croup, kCoat);

    // Rounded muscular croup curves on left & right sides
    for (int side = -1; side <= 1; side += 2)
    {
        const float sx = static_cast<float>(side);
        r.draw(A.sphere(), root * Translate(sx * 0.22f, 1.42f, -0.52f) * Scale(0.18f, 0.20f, 0.24f), kCoat);
    }
}

void Horse::drawNeckAndHead(Renderer& r, const glm::mat4& root, float s01, float gaitPitch) const
{
    const Assets& A = *assets_;

    // Neck base attaches seamlessly at the withers
    const glm::mat4 withersBase = root * Translate(0.0f, 1.50f, 0.46f);

    // Lower neck slopes upwards & forward at natural equine angle (~58 deg)
    const glm::mat4 neckLower = withersBase *
                                RotY(neckYaw_.x) *
                                RotX(0.85f + neckPitch_.x + gaitPitch * 0.3f);
    r.draw(*neckLowerMesh_, neckLower * Translate(0.0f, 0.24f, 0.0f), kCoat);

    // Muscular arched dorsal crest
    r.draw(A.sphere(), neckLower * Translate(0.0f, 0.26f, -0.10f) * Scale(0.14f, 0.26f, 0.14f), kCoat);

    // Upper neck / throatlatch
    const glm::mat4 neckUpper = neckLower * Translate(0.0f, 0.44f, 0.0f) * RotX(-0.25f);
    r.draw(*neckUpperMesh_, neckUpper * Translate(0.0f, 0.20f, 0.0f), kCoat);

    // Head (poll / cranium)
    const glm::mat4 head = neckUpper * Translate(0.0f, 0.40f, 0.0f) *
                           RotY(headYaw_.x) *
                           RotX(1.15f + headPitch_.x);
    r.draw(*headMesh_, head * Translate(0.0f, 0.10f, 0.0f), kCoat);

    // Broad forehead & sculpted cheek jowls (masseter muscles)
    for (int side = -1; side <= 1; side += 2)
    {
        const float sx = static_cast<float>(side);
        r.draw(A.sphere(), head * Translate(sx * 0.11f, 0.02f, -0.06f) * Scale(0.09f, 0.11f, 0.10f), kCoat);
    }

    // Elongated nasal bone tapering towards the muzzle
    const glm::mat4 muzzle = head * Translate(0.0f, 0.24f, 0.02f) * RotX(-0.08f);
    r.draw(*muzzleMesh_, muzzle * Translate(0.0f, 0.14f, 0.0f), kCoat);

    // Soft, rounded muzzle with chin & dark nostrils
    r.draw(A.sphere(), muzzle * Translate(0.0f, 0.30f, 0.01f) * Scale(0.12f, 0.10f, 0.11f), kCoatLight);
    r.draw(A.sphere(), muzzle * Translate(0.045f, 0.31f, 0.06f) * Scale(0.025f), kDark); // Right nostril
    r.draw(A.sphere(), muzzle * Translate(-0.045f, 0.31f, 0.06f) * Scale(0.025f), kDark); // Left nostril
    r.draw(A.sphere(), muzzle * Translate(0.0f, 0.26f, -0.04f) * Scale(0.07f, 0.05f, 0.06f), kCoat); // Chin

    // Lateral equine eyes with white sclera highlight
    for (int side = -1; side <= 1; side += 2)
    {
        const float sx = static_cast<float>(side);
        r.draw(A.sphere(), head * Translate(sx * 0.14f, 0.08f, 0.02f) * Scale(0.042f), kDark);
        r.draw(A.sphere(), head * Translate(sx * 0.155f, 0.09f, 0.015f) * Scale(0.015f), glm::vec3(1.0f));
    }

    // Alert, expressive equine ears at the poll
    for (int side = -1; side <= 1; side += 2)
    {
        const float sx = static_cast<float>(side);
        const int earIdx = (side < 0) ? 0 : 1;
        const glm::mat4 ear = head * Translate(sx * 0.09f, -0.06f, -0.08f) *
                              RotY(-0.20f * sx) *
                              RotX(-kHalfPi - ear_[earIdx].x) *
                              Translate(0.0f, 0.10f, 0.0f) *
                              Scale(0.065f, 0.20f, 0.065f);
        r.draw(A.cone(), ear, kCoat);
        r.draw(A.cone(), ear * Scale(0.85f), kCoatDark); // Inner ear shading
    }

    // Flowing dark mane along the neck crest
    const float maneSteps[6] = {0.10f, 0.24f, 0.38f, 0.52f, 0.66f, 0.80f};
    for (float t : maneSteps)
    {
        const float flow = std::sin(time_ * 6.0f + t * 4.0f) * 0.04f * (0.3f + s01);
        r.draw(A.sphere(), neckLower * Translate(flow, t * 0.52f, -0.16f) * Scale(0.09f, 0.12f, 0.09f), kMane);
    }
    // Forelock hanging between the ears
    r.draw(A.sphere(), head * Translate(0.0f, 0.02f, -0.12f) * Scale(0.09f, 0.10f, 0.06f), kMane);
}

void Horse::drawForeleg(Renderer& r, const glm::mat4& root, int side, const LegAngles& a) const
{
    const Assets& A = *assets_;
    const float sx = static_cast<float>(side);

    // Shoulder socket (cranial forequarter)
    const glm::mat4 shoulder = root * Translate(sx * 0.27f, 1.34f, 0.56f);
    r.draw(A.sphere(), shoulder * Scale(0.18f, 0.19f, 0.20f), kCoat); // Shoulder muscle

    // Upper foreleg (arm / brachium)
    const glm::mat4 upper = shoulder * RotX(a.upper) * RotZ(-sx * 0.04f);
    r.draw(*foreUpperMesh_, upper * Translate(0.0f, -0.18f, -0.02f) * RotX(0.10f), kCoat);

    // Distinct backward-bending knee joint (carpus)
    const glm::mat4 knee = upper * Translate(0.0f, -0.36f, -0.04f);
    r.draw(A.sphere(), knee * Scale(0.088f, 0.092f, 0.092f), kCoat);

    // Slender cannon bone (lower leg)
    const glm::mat4 cannon = knee * RotX(a.lower);
    r.draw(*foreLowerMesh_, cannon * Translate(0.0f, -0.18f, 0.0f), kCoatDark);

    // Fetlock joint (ankle bulb)
    const glm::mat4 fetlock = cannon * Translate(0.0f, -0.34f, 0.0f);
    r.draw(A.sphere(), fetlock * Scale(0.076f, 0.078f, 0.080f), kCoatDark);

    // Angled pastern
    const glm::mat4 pastern = fetlock * RotX(-0.48f + a.pastern);
    r.draw(*pasternMesh_, pastern * Translate(0.0f, -0.065f, 0.035f), kCoatDark);

    // Flared equine hoof resting flat on the ground (plus lift during stride)
    const glm::mat4 hoof = pastern * Translate(0.0f, -0.13f + a.lift, 0.07f);
    r.draw(A.cylinder(), hoof * Scale(0.098f, 0.075f, 0.105f), kHoof);
    r.draw(A.sphere(), hoof * Translate(0.0f, 0.035f, 0.0f) * Scale(0.092f, 0.025f, 0.098f), kHoofBand);
}

void Horse::drawHindleg(Renderer& r, const glm::mat4& root, int side, const LegAngles& a) const
{
    const Assets& A = *assets_;
    const float sx = static_cast<float>(side);

    // Pelvic hip socket (caudal hindquarter)
    const glm::mat4 hip = root * Translate(sx * 0.28f, 1.36f, -0.52f);
    r.draw(A.sphere(), hip * Scale(0.22f, 0.24f, 0.24f), kCoat); // Massive hip musculature

    // Upper thigh (slopes forward-down to stifle)
    const glm::mat4 thigh = hip * RotX(a.upper) * RotZ(-sx * 0.05f);
    r.draw(*hindUpperMesh_, thigh * Translate(0.0f, -0.20f, 0.08f) * RotX(-0.32f), kCoat);

    // Stifle joint (true knee - bends forward)
    const glm::mat4 stifle = thigh * Translate(0.0f, -0.38f, 0.16f);
    r.draw(A.sphere(), stifle * Scale(0.12f, 0.12f, 0.13f), kCoat);

    // Gaskin / second thigh (slopes backward-down to the hock)
    const glm::mat4 gaskin = stifle * RotX(a.mid);
    r.draw(*hindMidMesh_, gaskin * Translate(0.0f, -0.18f, -0.08f) * RotX(0.44f), kCoat);

    // HOCK JOINT (tarsus - the defining equine joint with backward calcaneus point)
    const glm::mat4 hock = gaskin * Translate(0.0f, -0.36f, -0.16f);
    r.draw(A.sphere(), hock * Scale(0.086f, 0.11f, 0.096f), kCoat);
    // Backward-pointing calcaneus heel
    r.draw(A.cone(), hock * Translate(0.0f, 0.03f, -0.06f) * RotX(kHalfPi) * Scale(0.05f, 0.09f, 0.05f), kCoat);

    // Lower cannon bone
    const glm::mat4 cannon = hock * RotX(a.lower);
    r.draw(*hindLowerMesh_, cannon * Translate(0.0f, -0.18f, 0.0f), kCoatDark);

    // Fetlock joint
    const glm::mat4 fetlock = cannon * Translate(0.0f, -0.34f, 0.0f);
    r.draw(A.sphere(), fetlock * Scale(0.076f, 0.078f, 0.080f), kCoatDark);

    // Pastern
    const glm::mat4 pastern = fetlock * RotX(-0.52f + a.pastern);
    r.draw(*pasternMesh_, pastern * Translate(0.0f, -0.065f, 0.035f), kCoatDark);

    // Hoof
    const glm::mat4 hoof = pastern * Translate(0.0f, -0.13f + a.lift, 0.07f);
    r.draw(A.cylinder(), hoof * Scale(0.098f, 0.075f, 0.105f), kHoof);
    r.draw(A.sphere(), hoof * Translate(0.0f, 0.035f, 0.0f) * Scale(0.092f, 0.025f, 0.098f), kHoofBand);
}

void Horse::drawTail(Renderer& r, const glm::mat4& root, float s01) const
{
    const Assets& A = *assets_;

    // Dock of the tail at the top of the croup
    const glm::mat4 tailDock = root * Translate(0.0f, 1.48f, -0.88f) *
                               RotZ(tail_[0].x * 0.4f) *
                               RotX(0.40f + tail_[0].x);

    // Base segment (arches back)
    r.draw(*tailMesh_, tailDock * Translate(0.0f, -0.16f, 0.0f), kMane);
    r.draw(A.sphere(), tailDock * Translate(0.0f, -0.32f, 0.0f) * Scale(0.12f), kMane);

    // Mid cascading segment (delayed secondary motion)
    const glm::mat4 tailMid = tailDock * Translate(0.0f, -0.32f, 0.0f) *
                              RotZ(tail_[1].x * 0.5f) *
                              RotX(0.25f + tail_[1].x);
    r.draw(*tailMesh_, tailMid * Translate(0.0f, -0.16f, 0.0f) * Scale(0.95f), kMane);
    r.draw(A.sphere(), tailMid * Translate(0.0f, -0.32f, 0.0f) * Scale(0.13f), kMane);

    // Lower plume flaring out dynamically at speed
    const glm::mat4 tailPlume = tailMid * Translate(0.0f, -0.30f, 0.0f) *
                                RotZ(tail_[2].x * 0.6f) *
                                RotX(0.30f + tail_[2].x + s01 * 0.45f);
    r.draw(*tailMesh_, tailPlume * Translate(0.0f, -0.18f, 0.0f) * Scale(1.1f, 1.2f, 1.1f), kMane);
    r.draw(A.sphere(), tailPlume * Translate(0.0f, -0.36f, 0.0f) * Scale(0.15f, 0.20f, 0.15f), kMane);
}

void Horse::drawSaddleAndTack(Renderer& r, const glm::mat4& root) const
{
    const Assets& A = *assets_;

    // Colorful saddle blanket (felt shabrack)
    r.draw(A.box(), root * Translate(0.0f, 1.54f, -0.02f) * Scale(0.68f, 0.035f, 0.72f), kSaddlePad);

    // Contoured leather saddle nestled behind the withers
    r.draw(A.sphere(), root * Translate(0.0f, 1.58f, -0.02f) * Scale(0.44f, 0.09f, 0.48f), kSaddle);
    // Saddle pommel (front horn) & cantle (rear lip)
    r.draw(A.sphere(), root * Translate(0.0f, 1.63f, 0.18f) * Scale(0.12f, 0.08f, 0.08f), kSaddle);
    r.draw(A.sphere(), root * Translate(0.0f, 1.64f, -0.22f) * Scale(0.16f, 0.09f, 0.09f), kSaddle);

    // Stirrup leathers & iron stirrups hanging on both sides
    for (int side = -1; side <= 1; side += 2)
    {
        const float sx = static_cast<float>(side);
        // Leather strap
        r.draw(A.cylinder(), root * Translate(sx * 0.38f, 1.34f, -0.02f) * Scale(0.02f, 0.38f, 0.02f), kReins);
        // Stirrup ring
        r.draw(A.cylinder(), root * Translate(sx * 0.38f, 1.14f, -0.02f) * Scale(0.075f, 0.025f, 0.075f), kStirrup);
    }
}

void Horse::drawRider(Renderer& r, const glm::mat4& root) const
{
    const Assets& A = *assets_;

    // Rider hips firmly anchored to saddle, smoothed by suspension spring
    const glm::mat4 riderBase = root * Translate(0.0f, 1.66f + riderBounce_.x, -0.02f);

    // Active spine stabilization: counters horse pitch and roll to keep torso balanced
    const glm::mat4 rider = riderBase * RotX(riderLean_.x) * RotZ(-roll_.x * 0.55f);

    // Torso with traditional vest
    r.draw(*torsoMesh_, rider * Translate(0.0f, 0.36f, 0.0f), kVest);
    r.draw(A.cylinder(), rider * Translate(0.0f, 0.20f, 0.0f) * Scale(0.215f, 0.06f, 0.215f), kGold); // Belt

    // Head, face & felt kalpak hat
    r.draw(A.sphere(), rider * Translate(0.0f, 0.86f, 0.0f) * Scale(0.19f), kSkin); // Head
    r.draw(A.cylinder(), rider * Translate(0.0f, 1.00f, 0.0f) * Scale(0.24f, 0.05f, 0.24f), kDark); // Brim
    r.draw(A.cone(), rider * Translate(0.0f, 1.17f, 0.0f) * Scale(0.20f, 0.30f, 0.20f), kFelt); // Kalpak
    r.draw(A.sphere(), rider * Translate(0.07f, 0.88f, 0.17f) * Scale(0.025f), kDark); // Eyes
    r.draw(A.sphere(), rider * Translate(-0.07f, 0.88f, 0.17f) * Scale(0.025f), kDark);

    // Rider legs hug the barrel cleanly into stirrups without body clipping
    for (int side = -1; side <= 1; side += 2)
    {
        const float sx = static_cast<float>(side);
        const glm::mat4 hip = rider * Translate(sx * 0.30f, 0.18f, 0.0f) *
                              RotZ(sx * 0.42f) *
                              RotX(-0.35f);
        r.draw(*riderLegMesh_, hip * Translate(0.0f, -0.22f, 0.0f), glm::vec3(0.18f, 0.20f, 0.45f));
        // Riding boots resting in stirrup position
        r.draw(A.sphere(), hip * Translate(0.0f, -0.44f, 0.04f) * Scale(0.11f, 0.09f, 0.15f), kDark);
    }

    // Arms: left hand holds reins forward; right reaches for goat or reins
    for (int side = -1; side <= 1; side += 2)
    {
        const float sx = static_cast<float>(side);
        const glm::mat4 shoulder = rider * Translate(sx * 0.24f, 0.58f, 0.0f);
        glm::mat4 arm;
        if (side < 0 && carrying_)
        {
            // Reaching downward-right to secure the goat
            arm = shoulder * RotZ(-0.95f) * RotX(-0.25f);
        }
        else
        {
            // Holding the reins over the withers
            arm = shoulder * RotZ(sx * 0.25f) * RotX(-0.85f);
        }
        r.draw(*armMesh_, arm * Translate(0.0f, -0.17f, 0.0f), kVest);
        r.draw(A.sphere(), arm * Translate(0.0f, -0.36f, 0.0f) * Scale(0.075f), kSkin);
    }
}

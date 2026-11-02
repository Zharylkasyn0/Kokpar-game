#include "Horse.h"

#include <GLFW/glfw3.h>

namespace
{
const glm::vec3 kCoat(0.62f, 0.36f, 0.18f);
const glm::vec3 kCoatLight(0.84f, 0.64f, 0.48f);
const glm::vec3 kMane(0.20f, 0.11f, 0.07f);
const glm::vec3 kHoof(0.14f, 0.11f, 0.10f);
const glm::vec3 kSaddle(0.08f, 0.56f, 0.60f);
const glm::vec3 kVest(0.80f, 0.14f, 0.14f);
const glm::vec3 kSkin(1.00f, 0.80f, 0.66f);
const glm::vec3 kFelt(0.96f, 0.95f, 0.90f);
const glm::vec3 kDark(0.10f, 0.08f, 0.08f);
const glm::vec3 kGold(1.00f, 0.80f, 0.15f);
}

void Horse::init(Assets& assets)
{
    assets_ = &assets;
    bodyMesh_ = &assets.capsule(0.45f, 1.10f);
    legMesh_ = &assets.capsule(0.13f, 0.70f);
    neckMesh_ = &assets.capsule(0.20f, 0.44f);
    headMesh_ = &assets.capsule(0.19f, 0.28f);
    tailMesh_ = &assets.capsule(0.08f, 0.45f);
    torsoMesh_ = &assets.capsule(0.20f, 0.32f);
    riderLegMesh_ = &assets.capsule(0.09f, 0.28f);
    armMesh_ = &assets.capsule(0.07f, 0.20f);
}

void Horse::reset(const glm::vec3& position, float yaw)
{
    position_ = position;
    yaw_ = yaw;
    speed_ = prevSpeed_ = accelSmooth_ = turnInput_ = 0.0f;
    bumpCooldown_ = 0.0f;
    carrying_ = false;
    gait_ = legAmp_ = bobY_ = 0.0f;
    lean_.reset();
    bounce_.reset();
    roll_.reset();
    tail_.reset();
    ear_.reset();
    squash_.reset();
    rigid_ = Translate(position_) * RotY(yaw_);
}

void Horse::update(float dt, const Input& in, const World& world)
{
    dt = std::max(dt, 1e-4f);
    time_ += dt;
    bumpCooldown_ = std::max(0.0f, bumpCooldown_ - dt);

    // ---- Input -----------------------------------------------------------
    const bool wantForward = in.down(GLFW_KEY_W) || in.down(GLFW_KEY_UP);
    const bool wantBack = in.down(GLFW_KEY_S) || in.down(GLFW_KEY_DOWN);
    const bool wantLeft = in.down(GLFW_KEY_A) || in.down(GLFW_KEY_LEFT);
    const bool wantRight = in.down(GLFW_KEY_D) || in.down(GLFW_KEY_RIGHT);
    const bool sprint = in.down(GLFW_KEY_LEFT_SHIFT) || in.down(GLFW_KEY_RIGHT_SHIFT);

    float targetSpeed = 0.0f;
    if (wantForward && !wantBack) targetSpeed = sprint ? kSprintSpeed : kWalkSpeed;
    else if (wantBack && !wantForward) targetSpeed = -kReverseSpeed;

    turnInput_ = (wantLeft ? 1.0f : 0.0f) - (wantRight ? 1.0f : 0.0f);

    // ---- Speed and heading ----------------------------------------------
    const bool braking = (targetSpeed == 0.0f) || (targetSpeed * speed_ < 0.0f);
    speed_ = Approach(speed_, targetSpeed, (braking ? kBrake : kAcceleration) * dt);

    float turnScale = std::clamp(0.30f + std::abs(speed_) / 6.0f, 0.30f, 1.0f);
    if (std::abs(speed_) > 9.0f) turnScale *= 0.8f;   // wider arcs at full gallop
    yaw_ = WrapAngle(yaw_ + turnInput_ * kTurnRate * turnScale * dt);

    const glm::vec3 fwd = forward();
    position_ += fwd * (speed_ * dt);

    // ---- Collision: two circles (chest + rump) vs world -----------------
    glm::vec2 totalPush(0.0f);
    for (int iteration = 0; iteration < 2; ++iteration)
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

    const float pushLength = glm::length(totalPush);
    if (pushLength > 1e-4f)
    {
        const glm::vec2 n = totalPush / pushLength;                       // points out of the obstacle
        const float moveSign = (speed_ >= 0.0f) ? 1.0f : -1.0f;
        const float into = -(n.x * fwd.x + n.y * fwd.z) * moveSign;       // 1 = head-on
        if (into > 0.2f)
        {
            speed_ *= 1.0f - std::clamp(into, 0.0f, 1.0f) * std::min(1.0f, 12.0f * dt);
            if (into > 0.5f && std::abs(prevSpeed_) > 4.0f && bumpCooldown_ <= 0.0f)
            {
                squash_.v += 3.0f;     // cartoon "boing"
                bumpCooldown_ = 0.35f;
            }
        }
    }

    // ---- Animation state -------------------------------------------------
    const float absSpeed = std::abs(speed_);
    const float s01 = speed01();

    const float strideHz = std::min(4.5f, 0.38f * absSpeed);
    gait_ = std::fmod(gait_ + kTwoPi * strideHz * dt, kTwoPi);
    legAmp_ = 0.85f * std::clamp(absSpeed / 8.0f, 0.0f, 1.0f);
    bobY_ = std::abs(std::sin(gait_)) * 0.09f * std::clamp(absSpeed / 8.0f, 0.0f, 1.0f);

    const float rawAccel = (speed_ - prevSpeed_) / dt;
    prevSpeed_ = speed_;
    accelSmooth_ += (rawAccel - accelSmooth_) * ExpSmooth(12.0f, dt);

    roll_.step(-turnInput_ * s01 * 0.28f, 60.0f, 9.0f, dt);
    lean_.step(std::clamp(-accelSmooth_ * 0.022f + s01 * 0.22f, -0.8f, 0.8f), 70.0f, 8.0f, dt);
    bounce_.step(bobY_, 160.0f, 10.0f, dt);
    tail_.step(std::clamp(accelSmooth_ * 0.03f, -0.8f, 0.8f) + std::sin(time_ * 2.5f) * 0.12f, 45.0f, 3.0f, dt);
    ear_.step(s01 * 0.7f + std::sin(time_ * 5.0f) * 0.05f, 80.0f, 6.0f, dt);
    squash_.step(0.0f, 120.0f, 9.0f, dt);
    squash_.x = std::clamp(squash_.x, -1.5f, 1.5f);

    rigid_ = Translate(position_.x, position_.y + bobY_, position_.z) * RotY(yaw_) * RotZ(roll_.x);
}

void Horse::draw(Renderer& r) const
{
    const Assets& A = *assets_;
    const float s01 = speed01();

    const float breathing = 1.0f + 0.012f * std::sin(time_ * 2.2f) * (1.0f - s01);
    const float sy = (1.0f - 0.12f * squash_.x) * breathing;
    const float sxz = 1.0f + 0.06f * squash_.x;
    const glm::mat4 root = rigid_ * Scale(sxz, sy, sxz);

    // ---- Body ------------------------------------------------------------
    r.draw(*bodyMesh_, root * Translate(0.0f, 1.35f, 0.0f) * RotX(kHalfPi), kCoat);
    // Light belly patch
    r.draw(A.sphere(), root * Translate(0.0f, 1.05f, 0.1f) * Scale(0.34f, 0.2f, 0.7f), kCoatLight);

    // ---- Legs (diagonal pairs) ------------------------------------------
    const float hx = 0.27f;
    const float hz = 0.62f;
    const glm::vec3 hips[4] = {
        glm::vec3(hx, 1.05f, hz), glm::vec3(-hx, 1.05f, hz),
        glm::vec3(hx, 1.05f, -hz), glm::vec3(-hx, 1.05f, -hz),
    };
    const float phaseOffset[4] = {0.0f, kPi, kPi, 0.0f};
    for (int i = 0; i < 4; ++i)
    {
        const float angle = std::sin(gait_ + phaseOffset[i]) * legAmp_;
        const glm::mat4 legRoot = root * Translate(hips[i]) * RotX(angle);
        r.draw(*legMesh_, legRoot * Translate(0.0f, -0.48f, 0.0f), kCoat);
        r.draw(A.sphere(), legRoot * Translate(0.0f, -0.93f, 0.0f) * Scale(0.15f, 0.10f, 0.17f), kHoof);
    }

    // ---- Neck and head ---------------------------------------------------
    const float headBob = std::sin(gait_ * 2.0f + 0.5f) * 0.06f * s01;
    const glm::mat4 neckBase = root * Translate(0.0f, 1.55f, 0.72f) * RotX(0.70f + headBob);
    r.draw(*neckMesh_, neckBase * Translate(0.0f, 0.42f, 0.0f), kCoat);

    const glm::mat4 headM = neckBase * Translate(0.0f, 0.80f, 0.0f) * RotX(1.05f) * Translate(0.0f, 0.28f, 0.0f);
    r.draw(*headMesh_, headM, kCoat);
    r.draw(A.sphere(), headM * Translate(0.0f, 0.27f, -0.01f) * Scale(0.17f, 0.16f, 0.16f), kCoatLight);   // muzzle

    for (int side = -1; side <= 1; side += 2)
    {
        const float sx = static_cast<float>(side);
        // Eyes with a tiny highlight
        r.draw(A.sphere(), headM * Translate(sx * 0.15f, 0.08f, -0.08f) * Scale(0.05f), kDark);
        r.draw(A.sphere(), headM * Translate(sx * 0.17f, 0.095f, -0.095f) * Scale(0.018f), glm::vec3(1.0f));
        // Ears: tilt back with speed (spring-driven flop)
        const glm::mat4 earM = headM * Translate(sx * 0.10f, -0.10f, -0.15f) * RotY(-0.25f * sx) *
                               RotX(-kHalfPi - ear_.x) * Translate(0.0f, 0.09f, 0.0f) * Scale(0.07f, 0.20f, 0.07f);
        r.draw(A.cone(), earM, kCoat);
    }

    // Mane along the back of the neck + forelock
    const float maneT[3] = {0.15f, 0.40f, 0.65f};
    for (float t : maneT)
    {
        r.draw(A.sphere(), neckBase * Translate(0.0f, t, -0.17f) * Scale(0.11f, 0.13f, 0.11f), kMane);
    }
    r.draw(A.sphere(), headM * Translate(0.0f, -0.02f, -0.19f) * Scale(0.12f, 0.12f, 0.08f), kMane);

    // ---- Tail ------------------------------------------------------------
    const float wag = std::sin(time_ * 4.0f) * 0.2f * (0.3f + s01);
    const glm::mat4 tailRoot = root * Translate(0.0f, 1.55f, -0.95f) * RotZ(wag) * RotX(0.45f + tail_.x);
    r.draw(*tailMesh_, tailRoot * Translate(0.0f, -0.32f, 0.0f), kMane);
    r.draw(A.sphere(), tailRoot * Translate(0.0f, -0.66f, 0.0f) * Scale(0.14f, 0.18f, 0.14f), kMane);

    // ---- Saddle ----------------------------------------------------------
    r.draw(A.sphere(), root * Translate(0.0f, 1.80f, -0.05f) * Scale(0.46f, 0.10f, 0.52f), kSaddle);

    // ---- Rider (lags behind the horse via springs) -----------------------
    const float riderOffset = std::clamp(bounce_.x - bobY_, -0.15f, 0.15f);
    const glm::mat4 rider = root * Translate(0.0f, 1.86f + riderOffset, -0.05f) * RotX(lean_.x);

    r.draw(*torsoMesh_, rider * Translate(0.0f, 0.36f, 0.0f), kVest);
    r.draw(A.cylinder(), rider * Translate(0.0f, 0.20f, 0.0f) * Scale(0.215f, 0.06f, 0.215f), kGold);   // belt
    r.draw(A.sphere(), rider * Translate(0.0f, 0.86f, 0.0f) * Scale(0.19f), kSkin);                   // head
    r.draw(A.cylinder(), rider * Translate(0.0f, 1.00f, 0.0f) * Scale(0.24f, 0.05f, 0.24f), kDark);   // hat brim
    r.draw(A.cone(), rider * Translate(0.0f, 1.17f, 0.0f) * Scale(0.20f, 0.30f, 0.20f), kFelt);       // hat
    r.draw(A.sphere(), rider * Translate(0.07f, 0.88f, 0.17f) * Scale(0.025f), kDark);
    r.draw(A.sphere(), rider * Translate(-0.07f, 0.88f, 0.17f) * Scale(0.025f), kDark);

    // Rider legs hang on both sides of the horse
    for (int side = -1; side <= 1; side += 2)
    {
        const float sx = static_cast<float>(side);
        const glm::mat4 hip = root * Translate(sx * 0.36f, 1.75f, -0.05f) * RotZ(sx * 0.45f) * RotX(-0.35f);
        r.draw(*riderLegMesh_, hip * Translate(0.0f, -0.23f, 0.0f), glm::vec3(0.18f, 0.20f, 0.45f));
        r.draw(A.sphere(), hip * Translate(0.0f, -0.46f, 0.0f) * Scale(0.11f, 0.09f, 0.14f), kDark);
    }

    // Arms: left holds the reins; right reaches for the goat when carrying
    for (int side = -1; side <= 1; side += 2)
    {
        const float sx = static_cast<float>(side);
        const glm::mat4 shoulder = rider * Translate(sx * 0.24f, 0.60f, 0.0f);
        glm::mat4 arm;
        if (side < 0 && carrying_) arm = shoulder * RotZ(-1.0f) * RotX(-0.2f);
        else arm = shoulder * RotZ(sx * 0.30f) * RotX(-0.9f);
        r.draw(*armMesh_, arm * Translate(0.0f, -0.17f, 0.0f), kVest);
        r.draw(A.sphere(), arm * Translate(0.0f, -0.36f, 0.0f) * Scale(0.075f), kSkin);
    }
}

#include "Goat.h"

namespace
{
const glm::vec3 kFur(0.96f, 0.93f, 0.84f);
const glm::vec3 kFurDark(0.80f, 0.74f, 0.62f);
const glm::vec3 kHorn(0.36f, 0.28f, 0.20f);
const glm::vec3 kNose(0.95f, 0.72f, 0.70f);
const glm::vec3 kDark(0.10f, 0.08f, 0.08f);
const glm::vec3 kMarker(1.00f, 0.82f, 0.18f);

float EaseOutBack(float t)
{
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;
    const float u = t - 1.0f;
    return 1.0f + c3 * u * u * u + c1 * u * u;
}
}

void Goat::init(Assets& assets)
{
    assets_ = &assets;
    bodyMesh_ = &assets.capsule(0.30f, 0.50f);
    legMesh_ = &assets.capsule(0.075f, 0.34f);
}

void Goat::respawn(const glm::vec3& groundPosition, float yaw)
{
    state_ = State::Idle;
    position_ = glm::vec3(groundPosition.x, kBodyHeight, groundPosition.z);
    velocity_ = glm::vec3(0.0f);
    yaw_ = yaw;
    popT_ = 0.0f;
    for (int i = 0; i < 4; ++i)
    {
        legX_[i].reset();
        legZ_[i].reset();
    }
    swayX_.reset();
    swayZ_.reset();
    head_.reset();
    ear_.reset();
    tumble_.reset();
}

void Goat::pickUp(const Horse& horse)
{
    (void)horse;
    state_ = State::Carried;
    pickupFrom_ = position_;
    pickupYaw_ = yaw_;
    carryBlend_ = 0.0f;
    warmup_ = 0;
    accel_ = glm::vec3(0.0f);
    prevVelocity_ = glm::vec3(0.0f);
    tumble_.reset();
}

void Goat::drop(const glm::vec3& velocity)
{
    state_ = State::Thrown;
    velocity_ = velocity;
    tumble_.x = 0.0f;
    tumble_.v = 0.0f;
}

void Goat::update(float dt, const Horse& horse, const World& world)
{
    dt = std::max(dt, 1e-4f);
    time_ += dt;
    popT_ = std::min(1.0f, popT_ + dt * 2.2f);

    switch (state_)
    {
    case State::Idle:    updateIdle(dt); break;
    case State::Carried: updateCarried(dt, horse); break;
    case State::Thrown:  updateThrown(dt, world); break;
    }
}

void Goat::updateIdle(float dt)
{
    position_.y = kBodyHeight;
    for (int i = 0; i < 4; ++i)
    {
        legX_[i].step(0.0f, 120.0f, 9.0f, dt);
        legZ_[i].step(0.0f, 120.0f, 9.0f, dt);
    }
    swayX_.step(0.0f, 60.0f, 8.0f, dt);
    swayZ_.step(0.0f, 60.0f, 8.0f, dt);
    head_.step(std::sin(time_ * 1.7f) * 0.10f, 60.0f, 6.0f, dt);
    ear_.step(std::sin(time_ * 3.0f) * 0.15f, 70.0f, 5.0f, dt);
    tumble_.step(0.0f, 70.0f, 6.0f, dt);   // wobbly settle after landing
}

void Goat::updateCarried(float dt, const Horse& horse)
{
    carryBlend_ = std::min(1.0f, carryBlend_ + dt * 4.0f);
    const float e = carryBlend_ * carryBlend_ * (3.0f - 2.0f * carryBlend_);

    const glm::vec3 anchorPos = glm::vec3(horse.carryAnchor()[3]);

    // Smoothly fly from the ground into the rider's arm (small arc).
    position_ = glm::mix(pickupFrom_, anchorPos, e);
    position_.y += std::sin(e * kPi) * 0.9f;
    yaw_ = pickupYaw_ + WrapAngle(horse.yaw() - pickupYaw_) * e;
    tumble_.x = 0.0f;
    tumble_.v = 0.0f;

    // Estimate the anchor's acceleration (warm-up avoids a huge first-frame spike).
    if (warmup_ == 0)
    {
        prevAnchor_ = anchorPos;
        warmup_ = 1;
        accel_ = glm::vec3(0.0f);
    }
    else
    {
        const glm::vec3 vel = (anchorPos - prevAnchor_) / dt;
        if (warmup_ == 1)
        {
            prevVelocity_ = vel;
            warmup_ = 2;
        }
        else
        {
            glm::vec3 a = (vel - prevVelocity_) / dt;
            const float len = glm::length(a);
            if (len > 60.0f) a *= 60.0f / len;
            accel_ += (a - accel_) * ExpSmooth(14.0f, dt);
            prevVelocity_ = vel;
        }
        prevAnchor_ = anchorPos;
    }

    carrierSpeed01_ = horse.speed01();

    // Acceleration in the carrier's local axes (forward / left).
    const float y = horse.yaw();
    const glm::vec3 fwd(std::sin(y), 0.0f, std::cos(y));
    const glm::vec3 left(std::cos(y), 0.0f, -std::sin(y));
    const float accF = glm::dot(accel_, fwd);
    const float accL = glm::dot(accel_, left);
    const float accY = accel_.y;

    // Pendulum-like legs: equilibrium shifts opposite to the acceleration, plus cartoon flailing.
    for (int i = 0; i < 4; ++i)
    {
        const float fi = static_cast<float>(i);
        const float side = (i % 2 == 0) ? 1.0f : -1.0f;
        const float flailX = std::sin(time_ * 11.0f + fi * 1.7f) * 0.30f * carrierSpeed01_;
        const float flailZ = std::sin(time_ * 9.0f + fi * 2.3f) * 0.15f * carrierSpeed01_;

        const float targetX = std::clamp(accF * 0.045f, -0.9f, 0.9f) + flailX;
        const float targetZ = std::clamp(-accL * 0.045f, -0.7f, 0.7f) + side * 0.10f + flailZ;

        legX_[i].step(targetX, 70.0f + fi * 6.0f, 4.5f, dt);
        legZ_[i].step(targetZ, 62.0f + fi * 5.0f, 4.0f, dt);
        legX_[i].x = std::clamp(legX_[i].x, -1.3f, 1.3f);
        legZ_[i].x = std::clamp(legZ_[i].x, -1.0f, 1.0f);
    }

    swayX_.step(std::clamp(-accF * 0.012f, -0.35f, 0.35f), 50.0f, 6.0f, dt);
    swayZ_.step(std::clamp(accL * 0.012f, -0.35f, 0.35f), 50.0f, 6.0f, dt);
    head_.step(std::clamp(-accF * 0.020f, -0.5f, 0.5f) + std::sin(time_ * 9.0f) * 0.10f * carrierSpeed01_, 60.0f, 5.0f, dt);
    ear_.step(0.4f + carrierSpeed01_ * 0.8f + std::clamp(-accY * 0.02f, -0.5f, 0.5f), 80.0f, 5.0f, dt);
}

void Goat::updateThrown(float dt, const World& world)
{
    velocity_.y -= 20.0f * dt;
    position_ += velocity_ * dt;
    yaw_ += dt * 3.0f;
    tumble_.x += dt * 9.0f;   // flip in the air

    // Keep it out of obstacles and inside the arena.
    const glm::vec2 push = world.resolveCircle(position_, 0.45f);
    position_.x += push.x;
    position_.z += push.y;

    for (int i = 0; i < 4; ++i)
    {
        const float fi = static_cast<float>(i);
        legX_[i].step(std::sin(time_ * 18.0f + fi * 1.3f) * 0.9f, 70.0f, 4.0f, dt);
        legZ_[i].step(std::sin(time_ * 15.0f + fi * 2.1f) * 0.5f, 60.0f, 4.0f, dt);
    }
    swayX_.step(0.0f, 50.0f, 6.0f, dt);
    swayZ_.step(0.0f, 50.0f, 6.0f, dt);
    ear_.step(0.9f, 80.0f, 5.0f, dt);
    head_.step(0.0f, 60.0f, 6.0f, dt);

    if (position_.y <= kBodyHeight && velocity_.y < 0.0f)
    {
        position_.y = kBodyHeight;
        if (velocity_.y < -4.0f)
        {
            // One small bounce.
            velocity_.y = -velocity_.y * 0.35f;
            velocity_.x *= 0.6f;
            velocity_.z *= 0.6f;
        }
        else
        {
            velocity_ = glm::vec3(0.0f);
            tumble_.x = std::remainder(tumble_.x, kTwoPi);   // settle with a wobble
            tumble_.v = 0.0f;
            state_ = State::Idle;
        }
    }
}

void Goat::draw(Renderer& r, bool pickupHint) const
{
    const Assets& A = *assets_;

    const float pop = std::max(EaseOutBack(popT_), 0.001f);   // never exactly 0 (singular matrix)
    const float idleWobble = (state_ == State::Idle) ? 0.02f * std::sin(time_ * 2.5f) : 0.0f;

    const glm::mat4 root = Translate(position_) * RotY(yaw_) * RotX(tumble_.x + swayX_.x) * RotZ(swayZ_.x) *
                           Scale(pop, pop * (1.0f + idleWobble), pop);

    // ---- Body ------------------------------------------------------------
    r.draw(*bodyMesh_, root * RotX(kHalfPi), kFur);

    // ---- Legs (spring-driven, hinged at the hips) -----------------------
    const glm::vec3 hips[4] = {
        glm::vec3(0.16f, -0.12f, 0.34f), glm::vec3(-0.16f, -0.12f, 0.34f),
        glm::vec3(0.16f, -0.12f, -0.34f), glm::vec3(-0.16f, -0.12f, -0.34f),
    };
    for (int i = 0; i < 4; ++i)
    {
        const glm::mat4 legRoot = root * Translate(hips[i]) * RotZ(legZ_[i].x) * RotX(legX_[i].x);
        r.draw(*legMesh_, legRoot * Translate(0.0f, -0.245f, 0.0f), kFurDark);
        r.draw(A.sphere(), legRoot * Translate(0.0f, -0.485f, 0.0f) * Scale(0.085f, 0.06f, 0.095f), kDark);
    }

    // ---- Head group (nods around a pivot at the neck) -------------------
    const glm::mat4 headRoot = root * Translate(0.0f, 0.15f, 0.45f) * RotX(head_.x) * Translate(0.0f, -0.15f, -0.45f);

    r.draw(A.sphere(), headRoot * Translate(0.0f, 0.32f, 0.62f) * Scale(0.20f, 0.20f, 0.22f), kFur);       // head
    r.draw(A.sphere(), headRoot * Translate(0.0f, 0.25f, 0.79f) * Scale(0.11f, 0.10f, 0.12f), kNose);      // muzzle
    r.draw(A.sphere(), headRoot * Translate(0.0f, 0.27f, 0.90f) * Scale(0.04f), kDark);                    // nose
    r.draw(A.cone(), headRoot * Translate(0.0f, 0.13f, 0.78f) * RotX(kPi) * Scale(0.05f, 0.14f, 0.05f), kFurDark);   // beard

    for (int side = -1; side <= 1; side += 2)
    {
        const float sx = static_cast<float>(side);
        // Eyes
        r.draw(A.sphere(), headRoot * Translate(sx * 0.10f, 0.37f, 0.74f) * Scale(0.035f), kDark);
        // Horns: swept back and slightly outward
        r.draw(A.cone(), headRoot * Translate(sx * 0.09f, 0.48f, 0.58f) * RotZ(-sx * 0.26f) * RotX(-0.6f) *
                             Translate(0.0f, 0.13f, 0.0f) * Scale(0.05f, 0.26f, 0.05f), kHorn);
        // Floppy ears
        r.draw(A.sphere(), headRoot * Translate(sx * 0.21f, 0.38f, 0.55f) * RotZ(-sx * (0.40f + ear_.x)) *
                               Scale(0.14f, 0.04f, 0.08f), kFurDark);
    }

    // ---- Tail ------------------------------------------------------------
    const float wag = std::sin(time_ * 7.0f) * 0.35f;
    r.draw(A.cone(), root * Translate(0.0f, 0.18f, -0.58f) * RotZ(wag) * RotX(-1.0f) *
                         Translate(0.0f, 0.07f, 0.0f) * Scale(0.07f, 0.16f, 0.07f), kFur);

    // ---- Pickup marker: bobbing golden arrow above an idle goat ---------
    if (state_ == State::Idle)
    {
        const float bob = std::sin(time_ * 3.0f) * 0.12f;
        const float size = pickupHint ? 1.35f : 1.0f;
        const glm::vec3 color = pickupHint ? glm::vec3(1.0f, 1.0f, 0.55f) : kMarker;
        r.draw(A.cone(), Translate(position_.x, 1.75f + bob, position_.z) * RotX(kPi) * Scale(0.30f * size, 0.45f * size, 0.30f * size), color);
    }
}

#include "Camera.h"

#include <GLFW/glfw3.h>

#include <algorithm>
#include <cmath>

#include <glm/gtc/matrix_transform.hpp>

#include "Input.h"
#include "Math.h"

void FollowCamera::resetOffsets()
{
    userYaw_ = 0.0f;
    userPitch_ = 0.18f;
    userDistance_ = 0.0f;
    userHeight_ = 0.0f;
    orbitYaw_ = 0.0f;
    mode_ = Mode::Follow;
}

void FollowCamera::setMode(Mode mode)
{
    mode_ = mode;
}

void FollowCamera::cycleMode(float currentYaw)
{
    switch (mode_)
    {
    case Mode::Follow:
        mode_ = Mode::FreeOrbit;
        orbitYaw_ = WrapAngle(currentYaw + userYaw_);
        userYaw_ = 0.0f;
        break;
    case Mode::FreeOrbit:
        mode_ = Mode::Cinematic;
        userYaw_ = WrapAngle(orbitYaw_ - currentYaw);
        break;
    case Mode::Cinematic:
        mode_ = Mode::Follow;
        break;
    }
}

const char* FollowCamera::modeName() const
{
    switch (mode_)
    {
    case Mode::Follow:    return "Follow";
    case Mode::FreeOrbit: return "Free Orbit";
    case Mode::Cinematic: return "Cinematic";
    }
    return "Unknown";
}

void FollowCamera::snapTo(const glm::vec3& target, float yaw)
{
    resetOffsets();
    orbitYaw_ = yaw;
    speed01_ = 0.0f;

    glm::vec3 desiredPos, desiredLook;
    float desiredFov = kBaseFov;
    computeTargetTransform(target, yaw, 0.0f, desiredPos, desiredLook, desiredFov);

    position_ = desiredPos;
    lookAt_ = desiredLook;
    fovDegrees_ = desiredFov;
    position_.y = std::max(position_.y, kMinTerrainClearance);
}

void FollowCamera::computeTargetTransform(const glm::vec3& target, float yaw, float speed01,
                                          glm::vec3& outPos, glm::vec3& outLook, float& outFov) const
{
    const glm::vec3 fwd(std::sin(yaw), 0.0f, std::cos(yaw));

    const glm::vec3 bodyTarget = target + glm::vec3(
        0.0f * std::cos(yaw),
        0.0f,
        0.0f * std::sin(yaw)
    );
    float dist = 0.0f;
    float baseH = 0.0f;
    float totalYaw = 0.0f;

    switch (mode_)
    {
    case Mode::Follow:
    default:
    {
        const float baseDist = kBaseDistance + speed01 * kExtraDistance;
        dist = std::clamp(baseDist + userDistance_, kMinDistance, kMaxDistance);
        baseH = kBaseHeight + speed01 * 0.45f;
        totalYaw = yaw + userYaw_;
        outLook = bodyTarget + fwd * (1.6f + speed01 * 1.4f) + glm::vec3(0.0f, kLookHeight + userHeight_ * 0.2f, 0.0f);
        outFov = kBaseFov + speed01 * kFovBoost;
        break;
    }
    case Mode::FreeOrbit:
    {
        dist = std::clamp(kBaseDistance + userDistance_, kMinDistance, kMaxDistance);
        baseH = kBaseHeight;
        totalYaw = orbitYaw_;
        outLook = bodyTarget + glm::vec3(0.0f, kLookHeight + userHeight_ * 0.2f, 0.0f);
        outFov = kBaseFov;
        break;
    }
    case Mode::Cinematic:
    {
        const float baseDist = 11.0f + speed01 * 1.6f;
        dist = std::clamp(baseDist + userDistance_, kMinDistance, kMaxDistance);
        baseH = 2.4f;
        totalYaw = yaw + userYaw_;
        outLook = bodyTarget + fwd * (1.8f + speed01 * 1.0f) + glm::vec3(0.0f, 1.35f + userHeight_ * 0.2f, 0.0f);
        outFov = 75.0f + speed01 * 5.0f;
        break;
    }
    }

    // Spherical orbit calculation: pitch alters horizontal radius and height elevation
    const float hDist = dist * std::cos(userPitch_);
    const float vDist = dist * std::sin(userPitch_);

    const float dx = -std::sin(totalYaw) * hDist;
    const float dz = -std::cos(totalYaw) * hDist;
    const float dy = std::max(kMinTerrainClearance, baseH + userHeight_ + vDist);

    outPos = bodyTarget + glm::vec3(dx, dy, dz);
}

void FollowCamera::update(float dt, const Input& in, const glm::vec3& target, float yaw, float speed01)
{
    // ---- 1. Camera Mode Toggle (C) ---------------------------------------
    if (in.pressed(GLFW_KEY_C))
    {
        cycleMode(yaw);
    }

    // ---- 2. Dedicated Camera Reset (Z or V) ------------------------------
    if (in.pressed(GLFW_KEY_Z) || in.pressed(GLFW_KEY_V))
    {
        resetOffsets();
    }

    // ---- 3. Modern Mouse Orbit (Enabled whenever cursor is captured) -----
    // Full 360-degree horizontal orbit and clamped vertical pitch
    if (in.isCursorCaptured())
    {
        const float dx = static_cast<float>(in.mouseDeltaX()) * mouseSensitivity_;
        const float dy = static_cast<float>(in.mouseDeltaY()) * mouseSensitivity_;

        if (mode_ == Mode::FreeOrbit)
        {
            orbitYaw_ = WrapAngle(orbitYaw_ - dx);
        }
        else
        {
            userYaw_ = WrapAngle(userYaw_ - dx);
        }

        userPitch_ += dy;
        userPitch_ = std::clamp(userPitch_, kMinPitch, kMaxPitch);
    }
    else if (in.mouseDown(GLFW_MOUSE_BUTTON_RIGHT))
    {
        // Fallback for uncaptured cursor: hold RMB to orbit
        const float dx = static_cast<float>(in.mouseDeltaX()) * mouseSensitivity_;
        const float dy = static_cast<float>(in.mouseDeltaY()) * mouseSensitivity_;

        if (mode_ == Mode::FreeOrbit)
        {
            orbitYaw_ = WrapAngle(orbitYaw_ - dx);
        }
        else
        {
            userYaw_ = WrapAngle(userYaw_ - dx);
        }

        userPitch_ += dy;
        userPitch_ = std::clamp(userPitch_, kMinPitch, kMaxPitch);
    }

    // ---- 4. Mouse Scroll Zoom --------------------------------------------
    const float scroll = in.scrollDeltaY();
    if (std::abs(scroll) > 1e-4f)
    {
        userDistance_ -= scroll * 0.95f;
        userDistance_ = std::clamp(userDistance_, -3.5f, 12.0f);
    }

    // ---- 5. Height Adjustment (Page Up / Page Down) ----------------------
    if (in.down(GLFW_KEY_PAGE_UP))
    {
        userHeight_ = std::min(userHeight_ + dt * 4.0f, kMaxHeightOffset);
    }
    if (in.down(GLFW_KEY_PAGE_DOWN))
    {
        userHeight_ = std::max(userHeight_ - dt * 4.0f, kMinHeightOffset);
    }

    // ---- 6. Smooth Interpolation towards Target Transform ----------------
    speed01_ += (speed01 - speed01_) * ExpSmooth(4.0f, dt);

    glm::vec3 desiredPos, desiredLook;
    float desiredFov = kBaseFov;
    computeTargetTransform(target, yaw, speed01_, desiredPos, desiredLook, desiredFov);

    float posRate = 7.0f;
    float lookRate = 11.0f;
    float fovRate = 4.0f;

    if (mode_ == Mode::FreeOrbit)
    {
        posRate = 9.0f;
        lookRate = 13.0f;
        fovRate = 5.0f;
    }
    else if (mode_ == Mode::Cinematic)
    {
        posRate = 3.2f;
        lookRate = 4.5f;
        fovRate = 2.5f;
    }

    position_ += (desiredPos - position_) * ExpSmooth(posRate, dt);
    lookAt_ += (desiredLook - lookAt_) * ExpSmooth(lookRate, dt);

    // Guaranteed terrain height clearance
    position_.y = std::max(position_.y, kMinTerrainClearance);

    fovDegrees_ += (desiredFov - fovDegrees_) * ExpSmooth(fovRate, dt);
}

glm::mat4 FollowCamera::view() const
{
    return glm::lookAt(position_, lookAt_, glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::mat4 FollowCamera::projection(float aspect) const
{
    return glm::perspective(glm::radians(fovDegrees_), aspect, 0.3f, 850.0f);
}

// ===========================================================================
// FreeCamera Implementation (F4)
// ===========================================================================
void FreeCamera::initFrom(const glm::vec3& pos, const glm::vec3& target)
{
    position_ = pos;
    const glm::vec3 dir = glm::normalize(target - pos);
    pitch_ = std::asin(std::clamp(dir.y, -0.999f, 0.999f));
    yaw_ = std::atan2(dir.x, dir.z);
    fovDegrees_ = 65.0f;
}

glm::vec3 FreeCamera::forward() const
{
    return glm::vec3(
        std::cos(pitch_) * std::sin(yaw_),
        std::sin(pitch_),
        std::cos(pitch_) * std::cos(yaw_)
    );
}

glm::vec3 FreeCamera::right() const
{
    return glm::vec3(std::cos(yaw_), 0.0f, -std::sin(yaw_));
}

void FreeCamera::update(float dt, const Input& in)
{
    yaw_ -= static_cast<float>(in.mouseDeltaX()) * mouseSensitivity_;
    pitch_ -= static_cast<float>(in.mouseDeltaY()) * mouseSensitivity_;
    pitch_ = std::clamp(pitch_, -1.50f, 1.50f);

    const bool sprint = in.down(GLFW_KEY_LEFT_SHIFT) || in.down(GLFW_KEY_RIGHT_SHIFT);
    const float speed = baseSpeed_ * (sprint ? 2.5f : 1.0f);

    const glm::vec3 fwd = forward();
    const glm::vec3 rgt = right();

    if (in.down(GLFW_KEY_W)) position_ += fwd * (speed * dt);
    if (in.down(GLFW_KEY_S)) position_ -= fwd * (speed * dt);
    if (in.down(GLFW_KEY_D)) position_ += rgt * (speed * dt);
    if (in.down(GLFW_KEY_A)) position_ -= rgt * (speed * dt);

    if (in.down(GLFW_KEY_E) || in.down(GLFW_KEY_SPACE)) position_.y += speed * dt;
    if (in.down(GLFW_KEY_Q) || in.down(GLFW_KEY_LEFT_CONTROL)) position_.y -= speed * dt;

    position_.y = std::max(position_.y, 0.35f);
}

glm::mat4 FreeCamera::view() const
{
    return glm::lookAt(position_, position_ + forward(), glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::mat4 FreeCamera::projection(float aspect) const
{
    return glm::perspective(glm::radians(fovDegrees_), aspect, 0.2f, 1000.0f);
}

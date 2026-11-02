#include "Camera.h"

#include <cmath>

#include <glm/gtc/matrix_transform.hpp>

#include "Math.h"

namespace
{
void ComputeDesired(const glm::vec3& target, float yaw, float speed01,
                    float baseDist, float extraDist, float baseHeight, float lookHeight,
                    glm::vec3& outPos, glm::vec3& outLook)
{
    const glm::vec3 fwd(std::sin(yaw), 0.0f, std::cos(yaw));
    const float dist = baseDist + speed01 * extraDist;
    outPos = target - fwd * dist + glm::vec3(0.0f, baseHeight + speed01 * 0.5f, 0.0f);
    outLook = target + fwd * (2.0f + speed01 * 2.5f) + glm::vec3(0.0f, lookHeight, 0.0f);
}
}

void FollowCamera::snapTo(const glm::vec3& target, float yaw)
{
    speed01_ = 0.0f;
    fovDegrees_ = kBaseFov;
    ComputeDesired(target, yaw, 0.0f, kBaseDistance, kExtraDistance, kBaseHeight, kLookHeight,
                   position_, lookAt_);
}

void FollowCamera::update(float dt, const glm::vec3& target, float yaw, float speed01)
{
    speed01_ += (speed01 - speed01_) * ExpSmooth(4.0f, dt);

    glm::vec3 desiredPos;
    glm::vec3 desiredLook;
    ComputeDesired(target, yaw, speed01_, kBaseDistance, kExtraDistance, kBaseHeight, kLookHeight,
                   desiredPos, desiredLook);

    position_ += (desiredPos - position_) * ExpSmooth(6.0f, dt);
    lookAt_ += (desiredLook - lookAt_) * ExpSmooth(10.0f, dt);
    position_.y = std::max(position_.y, 0.9f);

    const float desiredFov = kBaseFov + speed01_ * kFovBoost;
    fovDegrees_ += (desiredFov - fovDegrees_) * ExpSmooth(4.0f, dt);
}

glm::mat4 FollowCamera::view() const
{
    return glm::lookAt(position_, lookAt_, glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::mat4 FollowCamera::projection(float aspect) const
{
    return glm::perspective(glm::radians(fovDegrees_), aspect, 0.3f, 700.0f);
}

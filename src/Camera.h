#pragma once

#include <glm/glm.hpp>

// Smooth third-person follow camera. Position and look-at point are smoothed with
// frame-rate independent exponential filters; distance and FOV grow with speed.
class FollowCamera
{
public:
    void snapTo(const glm::vec3& target, float yaw);
    void update(float dt, const glm::vec3& target, float yaw, float speed01);

    glm::mat4 view() const;
    glm::mat4 projection(float aspect) const;
    const glm::vec3& position() const { return position_; }

private:
    glm::vec3 position_ = glm::vec3(0.0f, 4.0f, -8.0f);
    glm::vec3 lookAt_ = glm::vec3(0.0f);
    float speed01_ = 0.0f;
    float fovDegrees_ = 60.0f;

    static constexpr float kBaseDistance = 6.5f;
    static constexpr float kExtraDistance = 1.5f;
    static constexpr float kBaseHeight = 3.4f;
    static constexpr float kLookHeight = 1.6f;
    static constexpr float kBaseFov = 60.0f;
    static constexpr float kFovBoost = 12.0f;
};

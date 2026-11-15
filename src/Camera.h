#pragma once

#include <glm/glm.hpp>

class Input;

// ===========================================================================
// Modern Third-Person Orbital Camera (v0.3)
//
// Features:
// - Direct 360-degree horizontal orbit and vertical pitch via mouse movement
// - Smooth pitch clamping to prevent vertical flipping
// - Scroll wheel zoom with spring-damped distance interpolation
// - Speed-adaptive dynamic FOV expansion during sprints
// - Terrain ground avoidance (clamps above terrain)
// - Dedicated camera reset key ('Z' or 'V')
// - Cursor capture state compatibility (GLFW_CURSOR_DISABLED)
// ===========================================================================
class FollowCamera
{
public:
    enum class Mode
    {
        Follow,     // Default dynamic follow (heads behind horse with mouse offset)
        FreeOrbit,  // Full decoupled 360-degree free orbit
        Cinematic   // Wide panoramic presentation camera
    };

    void snapTo(const glm::vec3& target, float yaw);
    void update(float dt, const Input& input, const glm::vec3& target, float yaw, float speed01);

    void resetOffsets();
    void cycleMode(float currentYaw = 0.0f);
    void setMode(Mode mode);

    Mode mode() const { return mode_; }
    const char* modeName() const;

    float userYaw() const { return userYaw_; }
    float userPitch() const { return userPitch_; }
    float mouseSensitivity() const { return mouseSensitivity_; }
    void setMouseSensitivity(float s) { mouseSensitivity_ = s; }

    glm::mat4 view() const;
    glm::mat4 projection(float aspect) const;
    const glm::vec3& position() const { return position_; }
    const glm::vec3& lookAt() const { return lookAt_; }
    float fovDegrees() const { return fovDegrees_; }

private:
    void computeTargetTransform(const glm::vec3& target, float yaw, float speed01,
                                glm::vec3& outPos, glm::vec3& outLook, float& outFov) const;

    Mode mode_ = Mode::Follow;

    // Smoothed world camera state
    glm::vec3 position_ = glm::vec3(0.0f, 4.0f, -8.0f);
    glm::vec3 lookAt_ = glm::vec3(0.0f, 1.5f, 0.0f);
    float speed01_ = 0.0f;
    float fovDegrees_ = 60.0f;

    // User orbit offsets
    float userYaw_ = 0.0f;         // Relative yaw angle offset (radians)
    float userPitch_ = 0.18f;      // Vertical pitch angle offset (default slight elevated angle)
    float userDistance_ = 0.0f;    // Distance zoom offset
    float userHeight_ = 0.0f;      // Optional height offset from PageUp/PageDown

    // Free orbit absolute world heading
    float orbitYaw_ = 0.0f;

    // Mouse sensitivity setting
    float mouseSensitivity_ = 0.0022f;

    // Base camera dimensions
    static constexpr float kBaseDistance = 6.4f;
    static constexpr float kExtraDistance = 1.6f;
    static constexpr float kBaseHeight = 3.2f;
    static constexpr float kLookHeight = 1.65f;
    static constexpr float kBaseFov = 60.0f;
    static constexpr float kFovBoost = 12.0f;

    // Safety limits for pitch, distance, and height
    static constexpr float kMinPitch = -0.15f;  // ~ -8.5 degrees (ground skimming)
    static constexpr float kMaxPitch = 1.25f;   // ~ 71 degrees (top-down view)
    static constexpr float kMinDistance = 2.8f;
    static constexpr float kMaxDistance = 18.0f;
    static constexpr float kMinHeightOffset = -1.8f;
    static constexpr float kMaxHeightOffset = 5.0f;
    static constexpr float kMinTerrainClearance = 0.45f;
};

// ===========================================================================
// Decoupled Free-Fly Camera (F4)
// Allows flying anywhere across the Kazakhstan locations and inspecting frozen
// animations during simulation pause (P).
// ===========================================================================
class FreeCamera
{
public:
    void initFrom(const glm::vec3& pos, const glm::vec3& target);
    void update(float dt, const Input& input);

    glm::mat4 view() const;
    glm::mat4 projection(float aspect) const;
    const glm::vec3& position() const { return position_; }
    float yaw() const { return yaw_; }
    float pitch() const { return pitch_; }
    glm::vec3 forward() const;
    glm::vec3 right() const;

private:
    glm::vec3 position_ = glm::vec3(0.0f, 6.0f, -12.0f);
    float yaw_ = 0.0f;
    float pitch_ = -0.2f;
    float fovDegrees_ = 65.0f;
    float baseSpeed_ = 16.0f;
    float mouseSensitivity_ = 0.0028f;
};

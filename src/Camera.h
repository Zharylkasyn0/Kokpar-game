#pragma once

#include <glm/glm.hpp>

class Input;

// ===========================================================================
// Adjustable 3D Camera System
//
// Supports:
// 1. Mouse RMB orbit rotation (horizontal yaw, vertical pitch with safety clamping)
// 2. Mouse scroll wheel zoom (bounded distance with smooth interpolation)
// 3. Height adjustment via Page Up / Page Down keys
// 4. Multiple camera modes (Follow, Free Orbit, Cinematic) toggled with 'C'
// 5. Camera reset via 'R' (returns to default third-person view)
// 6. Frame-rate independent exponential smoothing for jitter-free tracking
// ===========================================================================
class FollowCamera
{
public:
    enum class Mode
    {
        Follow,     // Default third-person follow camera (auto-aligns with horse)
        FreeOrbit,  // Free orbit camera around horse (heading stays independent)
        Cinematic   // Wide panoramic cinematic camera (wide FOV, gentle drift)
    };

    void snapTo(const glm::vec3& target, float yaw);
    void update(float dt, const Input& input, const glm::vec3& target, float yaw, float speed01);

    void resetOffsets();
    void cycleMode();
    void setMode(Mode mode);

    Mode mode() const { return mode_; }
    const char* modeName() const;

    glm::mat4 view() const;
    glm::mat4 projection(float aspect) const;
    const glm::vec3& position() const { return position_; }

private:
    void computeTargetTransform(const glm::vec3& target, float yaw, float speed01,
                                glm::vec3& outPos, glm::vec3& outLook, float& outFov) const;

    Mode mode_ = Mode::Follow;

    // Smoothed world camera state
    glm::vec3 position_ = glm::vec3(0.0f, 4.0f, -8.0f);
    glm::vec3 lookAt_ = glm::vec3(0.0f, 1.5f, 0.0f);
    float speed01_ = 0.0f;
    float fovDegrees_ = 60.0f;

    // User adjustable offsets
    float userYaw_ = 0.0f;         // Relative yaw angle offset (radians)
    float userPitch_ = 0.0f;       // Vertical pitch angle offset (radians)
    float userDistance_ = 0.0f;    // Distance zoom offset
    float userHeight_ = 0.0f;      // Height offset from Page Up / Page Down

    // Free orbit absolute world yaw
    float orbitYaw_ = 0.0f;

    // Base camera dimensions
    static constexpr float kBaseDistance = 6.5f;
    static constexpr float kExtraDistance = 1.5f;
    static constexpr float kBaseHeight = 3.4f;
    static constexpr float kLookHeight = 1.6f;
    static constexpr float kBaseFov = 60.0f;
    static constexpr float kFovBoost = 12.0f;

    // Safety limits for pitch, distance, and height
    static constexpr float kMinPitch = -0.15f;  // ~ -8.5 degrees (low angle)
    static constexpr float kMaxPitch = 1.30f;   // ~ 75 degrees (top-down view)
    static constexpr float kMinDistance = 2.8f;
    static constexpr float kMaxDistance = 22.0f;
    static constexpr float kMinHeightOffset = -1.8f;
    static constexpr float kMaxHeightOffset = 6.0f;
};

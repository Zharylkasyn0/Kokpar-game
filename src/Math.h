#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

constexpr float kPi = 3.14159265358979f;
constexpr float kHalfPi = kPi * 0.5f;
constexpr float kTwoPi = kPi * 2.0f;

// ---- Matrix helpers (keep model-matrix code short and readable) ----------
inline glm::mat4 Translate(const glm::vec3& p) { return glm::translate(glm::mat4(1.0f), p); }
inline glm::mat4 Translate(float x, float y, float z) { return Translate(glm::vec3(x, y, z)); }
inline glm::mat4 Scale(const glm::vec3& s) { return glm::scale(glm::mat4(1.0f), s); }
inline glm::mat4 Scale(float x, float y, float z) { return Scale(glm::vec3(x, y, z)); }
inline glm::mat4 Scale(float s) { return Scale(glm::vec3(s, s, s)); }
inline glm::mat4 RotX(float a) { return glm::rotate(glm::mat4(1.0f), a, glm::vec3(1.0f, 0.0f, 0.0f)); }
inline glm::mat4 RotY(float a) { return glm::rotate(glm::mat4(1.0f), a, glm::vec3(0.0f, 1.0f, 0.0f)); }
inline glm::mat4 RotZ(float a) { return glm::rotate(glm::mat4(1.0f), a, glm::vec3(0.0f, 0.0f, 1.0f)); }

// ---- Scalar helpers ------------------------------------------------------
inline float Approach(float current, float target, float maxDelta)
{
    if (target > current) return std::min(current + maxDelta, target);
    return std::max(current - maxDelta, target);
}

// Frame-rate independent smoothing factor: x += (target - x) * ExpSmooth(rate, dt)
inline float ExpSmooth(float rate, float dt) { return 1.0f - std::exp(-rate * dt); }

// Wraps an angle into [-pi, pi].
inline float WrapAngle(float a) { return std::remainder(a, kTwoPi); }

// ---- Damped spring (used everywhere for secondary motion) ----------------
struct Spring
{
    float x = 0.0f;  // value
    float v = 0.0f;  // velocity

    // Semi-implicit Euler with sub-stepping so it stays stable on slow frames.
    void step(float target, float stiffness, float damping, float dt)
    {
        const int n = std::max(1, static_cast<int>(std::ceil(dt / 0.008f)));
        const float h = dt / static_cast<float>(n);
        for (int i = 0; i < n; ++i)
        {
            const float a = stiffness * (target - x) - damping * v;
            v += a * h;
            x += v * h;
        }
    }

    void reset() { x = 0.0f; v = 0.0f; }
};

// ---- Tiny deterministic RNG (xorshift32) ---------------------------------
struct Rng
{
    uint32_t state;
    explicit Rng(uint32_t seed = 2463534242u) : state(seed ? seed : 1u) {}

    uint32_t nextU32()
    {
        state ^= state << 13;
        state ^= state >> 17;
        state ^= state << 5;
        return state;
    }
    float next01() { return static_cast<float>(nextU32() & 0xFFFFFFu) / 16777216.0f; }
    float range(float a, float b) { return a + (b - a) * next01(); }
};

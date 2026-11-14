#pragma once

#include <string>
#include <glm/glm.hpp>

// ===========================================================================
// Dynamic Day/Night Lighting System & Celestial Mechanics
//
// Features:
// - Full 24-hour cycle (00:00 - 24:00) with smooth mathematical transitions
// - 7 Presets: DAWN, MORNING, NOON, AFTERNOON, SUNSET, NIGHT, MIDNIGHT
// - Real-time sun and moon directional light calculation
// - Hemispheric ambient lighting (sky & ground bounce)
// - Synchronized distance fog and horizon atmospheric color blending
// - Configurable real-time cycle speeds (OFF, 5m, 10m, 20m, 30m)
// - Manual sun elevation & azimuth controls with lighting preview
// ===========================================================================

enum class LightingPresetId
{
    Dawn,
    Morning,
    Noon,
    Afternoon,
    Sunset,
    Night,
    Midnight,
    Count
};

struct LightingState
{
    glm::vec3 toLight = glm::vec3(0.0f, 1.0f, 0.0f);       // Normalized vector toward light source
    glm::vec3 lightColor = glm::vec3(1.0f);                 // Direct sunlight or moonlight
    glm::vec3 ambientSky = glm::vec3(0.48f, 0.56f, 0.68f);  // Sky ambient
    glm::vec3 ambientGround = glm::vec3(0.34f, 0.30f, 0.22f); // Ground ambient
    glm::vec3 skyColor = glm::vec3(0.62f, 0.80f, 0.95f);   // Sky background clear color
    glm::vec3 horizonColor = glm::vec3(0.75f, 0.85f, 0.95f); // Horizon tint
    glm::vec3 fogColor = glm::vec3(0.62f, 0.80f, 0.95f);   // Distance fog
    float fogStart = 75.0f;
    float fogEnd = 320.0f;
    bool isNight = false;
    float starVisibility = 0.0f;                            // 0.0 (day) to 1.0 (midnight)
    glm::vec3 sunWorldDir = glm::vec3(0.0f, 1.0f, 0.0f);    // Direction to sun disc
    glm::vec3 moonWorldDir = glm::vec3(0.0f, -1.0f, 0.0f);  // Direction to moon disc
};

class LightingManager
{
public:
    void init();
    void update(float dt, bool isPaused);

    // Apply preset
    void applyPreset(LightingPresetId preset);
    LightingPresetId currentPreset() const { return currentPreset_; }
    const char* presetNameEn(LightingPresetId p) const;
    const char* presetNameKz(LightingPresetId p) const;

    // Time & Sun controls
    void setTimeOfDay(float hours); // 0.0f .. 24.0f
    float timeOfDay() const { return timeOfDay_; }

    void setManualAngles(float elevationDeg, float azimuthDeg);
    float sunElevation() const { return sunElevationDeg_; }
    float sunAzimuth() const { return sunAzimuthDeg_; }
    bool isManualOverride() const { return manualOverride_; }
    void setManualOverride(bool manual) { manualOverride_ = manual; }

    // Multipliers & intensities
    void setSunIntensity(float intensity) { sunIntensity_ = intensity; }
    float sunIntensity() const { return sunIntensity_; }

    void setAmbientIntensity(float intensity) { ambientIntensity_ = intensity; }
    float ambientIntensity() const { return ambientIntensity_; }

    // Auto-cycle progression
    enum class CycleDuration
    {
        Off,
        Min5,
        Min10,
        Min20,
        Min30,
        Count
    };
    void setCycleDuration(CycleDuration duration);
    CycleDuration cycleDuration() const { return cycleDuration_; }
    const char* cycleDurationName(CycleDuration d) const;
    bool isAutoCycleEnabled() const { return cycleDuration_ != CycleDuration::Off; }

    // Active lighting state for renderer & shaders
    const LightingState& state() const { return state_; }

    // Format time into "HH:MM"
    std::string formattedTime() const;

private:
    void recalculate();

    float timeOfDay_ = 12.0f;        // 12:00 (Noon)
    float sunElevationDeg_ = 68.0f;  // Elevation in degrees
    float sunAzimuthDeg_ = 180.0f;   // Azimuth in degrees
    bool manualOverride_ = false;

    float sunIntensity_ = 1.0f;
    float ambientIntensity_ = 1.0f;

    CycleDuration cycleDuration_ = CycleDuration::Off;
    LightingPresetId currentPreset_ = LightingPresetId::Noon;

    LightingState state_;
};

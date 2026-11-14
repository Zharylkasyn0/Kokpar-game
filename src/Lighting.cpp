#include "Lighting.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

#include "Math.h"

namespace
{
constexpr float kDegToRad = kPi / 180.0f;

// Calculate smooth sun celestial position for a given time of day (0.0 .. 24.0)
void CalculateSunTrajectory(float timeHours, float& outElevationDeg, float& outAzimuthDeg)
{
    // Solar noon is at 12:00, sunrise ~06:00, sunset ~18:00
    // Solar hour angle: -180 deg at midnight, 0 deg at noon, +180 deg at midnight
    const float hourAngle = (timeHours - 12.0f) * 15.0f; // 15 deg per hour
    // Latitude approximation for Kazakhstan (~45° N)
    const float latRad = 45.0f * kDegToRad;
    const float declRad = 15.0f * kDegToRad; // Spring/Summer steppe sun declination

    const float sinElev = std::sin(latRad) * std::sin(declRad) +
                          std::cos(latRad) * std::cos(declRad) * std::cos(hourAngle * kDegToRad);
    const float elevRad = std::asin(std::clamp(sinElev, -1.0f, 1.0f));
    outElevationDeg = elevRad / kDegToRad;

    // Azimuth: 0 = North, 90 = East, 180 = South, 270 = West
    const float cosAzim = (std::sin(declRad) - std::sin(latRad) * sinElev) /
                          (std::cos(latRad) * std::cos(elevRad) + 1e-5f);
    float azimRad = std::acos(std::clamp(cosAzim, -1.0f, 1.0f));
    if (std::sin(hourAngle * kDegToRad) > 0.0f)
    {
        azimRad = kTwoPi - azimRad;
    }
    outAzimuthDeg = 180.0f - (azimRad / kDegToRad);
    if (outAzimuthDeg < 0.0f) outAzimuthDeg += 360.0f;
    if (outAzimuthDeg >= 360.0f) outAzimuthDeg -= 360.0f;
}

// Convert spherical (elevation, azimuth in degrees) to direction vector (pointing TOWARD celestial body)
glm::vec3 CelestialDir(float elevationDeg, float azimuthDeg)
{
    const float elevRad = elevationDeg * kDegToRad;
    const float azimRad = azimuthDeg * kDegToRad;

    const float cosElev = std::cos(elevRad);
    const float sinElev = std::sin(elevRad);

    // X = East-West, Y = Up, Z = North-South
    const float x = cosElev * std::sin(azimRad);
    const float y = sinElev;
    const float z = cosElev * std::cos(azimRad);

    return glm::normalize(glm::vec3(x, y, z));
}
}

void LightingManager::init()
{
    applyPreset(LightingPresetId::Noon);
}

void LightingManager::update(float dt, bool isPaused)
{
    if (!isPaused && cycleDuration_ != CycleDuration::Off)
    {
        float cycleMinutes = 10.0f;
        switch (cycleDuration_)
        {
        case CycleDuration::Min5:  cycleMinutes = 5.0f; break;
        case CycleDuration::Min10: cycleMinutes = 10.0f; break;
        case CycleDuration::Min20: cycleMinutes = 20.0f; break;
        case CycleDuration::Min30: cycleMinutes = 30.0f; break;
        default: break;
        }

        // Advance time of day
        const float hoursPerSecond = 24.0f / (cycleMinutes * 60.0f);
        timeOfDay_ += dt * hoursPerSecond;
        if (timeOfDay_ >= 24.0f) timeOfDay_ = std::fmod(timeOfDay_, 24.0f);

        if (!manualOverride_)
        {
            CalculateSunTrajectory(timeOfDay_, sunElevationDeg_, sunAzimuthDeg_);
        }
        recalculate();
    }
}

void LightingManager::applyPreset(LightingPresetId preset)
{
    currentPreset_ = preset;
    manualOverride_ = false;

    switch (preset)
    {
    case LightingPresetId::Dawn:
        timeOfDay_ = 6.25f; // 06:15
        sunElevationDeg_ = 8.0f;
        sunAzimuthDeg_ = 82.0f;
        break;
    case LightingPresetId::Morning:
        timeOfDay_ = 9.0f;  // 09:00
        sunElevationDeg_ = 36.0f;
        sunAzimuthDeg_ = 125.0f;
        break;
    case LightingPresetId::Noon:
        timeOfDay_ = 12.5f; // 12:30
        sunElevationDeg_ = 72.0f;
        sunAzimuthDeg_ = 180.0f;
        break;
    case LightingPresetId::Afternoon:
        timeOfDay_ = 15.5f; // 15:30
        sunElevationDeg_ = 44.0f;
        sunAzimuthDeg_ = 230.0f;
        break;
    case LightingPresetId::Sunset:
        timeOfDay_ = 19.2f; // 19:12
        sunElevationDeg_ = 6.0f;
        sunAzimuthDeg_ = 282.0f;
        break;
    case LightingPresetId::Night:
        timeOfDay_ = 22.0f; // 22:00
        sunElevationDeg_ = -32.0f;
        sunAzimuthDeg_ = 320.0f;
        break;
    case LightingPresetId::Midnight:
        timeOfDay_ = 0.5f;  // 00:30
        sunElevationDeg_ = -65.0f;
        sunAzimuthDeg_ = 10.0f;
        break;
    default:
        break;
    }

    recalculate();
}

const char* LightingManager::presetNameEn(LightingPresetId p) const
{
    switch (p)
    {
    case LightingPresetId::Dawn:      return "DAWN";
    case LightingPresetId::Morning:   return "MORNING";
    case LightingPresetId::Noon:      return "NOON";
    case LightingPresetId::Afternoon: return "AFTERNOON";
    case LightingPresetId::Sunset:    return "SUNSET";
    case LightingPresetId::Night:     return "NIGHT";
    case LightingPresetId::Midnight:  return "MIDNIGHT";
    default:                          return "CUSTOM";
    }
}

const char* LightingManager::presetNameKz(LightingPresetId p) const
{
    switch (p)
    {
    case LightingPresetId::Dawn:      return "Таң";
    case LightingPresetId::Morning:   return "Таңертең";
    case LightingPresetId::Noon:      return "Талтүс";
    case LightingPresetId::Afternoon: return "Бесін";
    case LightingPresetId::Sunset:    return "Күн батуы";
    case LightingPresetId::Night:     return "Түн";
    case LightingPresetId::Midnight:  return "Түн ортасы";
    default:                          return "Арнайы";
    }
}

void LightingManager::setTimeOfDay(float hours)
{
    timeOfDay_ = std::fmod(hours + 24.0f, 24.0f);
    manualOverride_ = false;
    CalculateSunTrajectory(timeOfDay_, sunElevationDeg_, sunAzimuthDeg_);
    recalculate();
}

void LightingManager::setManualAngles(float elevationDeg, float azimuthDeg)
{
    manualOverride_ = true;
    sunElevationDeg_ = std::clamp(elevationDeg, -85.0f, 85.0f);
    sunAzimuthDeg_ = std::fmod(azimuthDeg + 360.0f, 360.0f);
    recalculate();
}

void LightingManager::setCycleDuration(CycleDuration duration)
{
    cycleDuration_ = duration;
}

const char* LightingManager::cycleDurationName(CycleDuration d) const
{
    switch (d)
    {
    case CycleDuration::Off:   return "OFF";
    case CycleDuration::Min5:  return "5 MINUTES";
    case CycleDuration::Min10: return "10 MINUTES";
    case CycleDuration::Min20: return "20 MINUTES";
    case CycleDuration::Min30: return "30 MINUTES";
    default:                   return "OFF";
    }
}

std::string LightingManager::formattedTime() const
{
    const int totalMins = static_cast<int>(timeOfDay_ * 60.0f) % (24 * 60);
    const int hours = totalMins / 60;
    const int mins = totalMins % 60;
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%02d:%02d", hours, mins);
    return std::string(buf);
}

void LightingManager::recalculate()
{
    // Sun direction vector in world space
    state_.sunWorldDir = CelestialDir(sunElevationDeg_, sunAzimuthDeg_);

    // Moon is positioned opposite the sun across the celestial sphere with an aesthetic tilt
    const float moonElevation = -sunElevationDeg_ * 0.85f + 12.0f;
    const float moonAzimuth = std::fmod(sunAzimuthDeg_ + 175.0f, 360.0f);
    state_.moonWorldDir = CelestialDir(std::clamp(moonElevation, -80.0f, 80.0f), moonAzimuth);

    // Daylight factor based on sun elevation angle
    // -6 deg (civil twilight start) to +12 deg (full day)
    const float dayFactor = std::clamp((sunElevationDeg_ + 6.0f) / 18.0f, 0.0f, 1.0f);
    state_.isNight = (sunElevationDeg_ < 0.0f);

    // Stars become visible when dayFactor < 0.35
    state_.starVisibility = std::clamp((0.35f - dayFactor) / 0.35f, 0.0f, 1.0f);

    // Golden hour factor: peak golden warm light near horizon (elevation 0° to 14°)
    const float horizonFactor = std::clamp(1.0f - std::abs(sunElevationDeg_ - 7.0f) / 12.0f, 0.0f, 1.0f);
    const bool isSunset = (timeOfDay_ > 12.0f);

    if (dayFactor > 0.05f)
    {
        // ------------------ DAY / DAWN / SUNSET LIGHTING ------------------
        state_.toLight = state_.sunWorldDir;

        // Base sun color shifts from warm peach/crimson at dawn/sunset to bright gold/white at noon
        glm::vec3 sunTint;
        if (isSunset)
        {
            // Sunset: fiery amber-red
            sunTint = glm::mix(glm::vec3(1.00f, 0.94f, 0.84f), glm::vec3(1.00f, 0.48f, 0.20f), horizonFactor);
        }
        else
        {
            // Dawn: warm soft peach-gold
            sunTint = glm::mix(glm::vec3(1.00f, 0.94f, 0.84f), glm::vec3(1.00f, 0.68f, 0.36f), horizonFactor);
        }
        state_.lightColor = sunTint * (0.35f + 0.95f * dayFactor) * sunIntensity_;

        // Ambient: Hemispheric sky / ground bounce
        const glm::vec3 daySkyAmbient = glm::mix(glm::vec3(0.50f, 0.60f, 0.72f), glm::vec3(0.62f, 0.42f, 0.52f), horizonFactor);
        const glm::vec3 dayGndAmbient = glm::mix(glm::vec3(0.36f, 0.32f, 0.24f), glm::vec3(0.38f, 0.24f, 0.18f), horizonFactor);
        state_.ambientSky = daySkyAmbient * (0.35f + 0.65f * dayFactor) * ambientIntensity_;
        state_.ambientGround = dayGndAmbient * (0.30f + 0.70f * dayFactor) * ambientIntensity_;

        // Sky & Fog: Clear azure daytime or vibrant dawn/sunset
        if (isSunset)
        {
            state_.skyColor = glm::mix(glm::vec3(0.52f, 0.72f, 0.92f), glm::vec3(0.36f, 0.28f, 0.52f), horizonFactor);
            state_.horizonColor = glm::mix(glm::vec3(0.70f, 0.84f, 0.96f), glm::vec3(0.98f, 0.45f, 0.20f), horizonFactor);
            state_.fogColor = glm::mix(state_.skyColor, glm::vec3(0.95f, 0.48f, 0.22f), horizonFactor * 0.85f);
        }
        else
        {
            state_.skyColor = glm::mix(glm::vec3(0.52f, 0.72f, 0.92f), glm::vec3(0.48f, 0.48f, 0.66f), horizonFactor);
            state_.horizonColor = glm::mix(glm::vec3(0.70f, 0.84f, 0.96f), glm::vec3(0.96f, 0.65f, 0.38f), horizonFactor);
            state_.fogColor = glm::mix(state_.skyColor, glm::vec3(0.92f, 0.62f, 0.36f), horizonFactor * 0.85f);
        }

        state_.fogStart = glm::mix(55.0f, 85.0f, dayFactor);
        state_.fogEnd = glm::mix(260.0f, 340.0f, dayFactor);
    }
    else
    {
        // ------------------ NIGHT / MIDNIGHT LIGHTING --------------------
        // Primary directional light becomes cool silver-blue moonlight
        state_.toLight = state_.moonWorldDir;
        if (state_.toLight.y < 0.15f) state_.toLight.y = 0.15f;
        state_.toLight = glm::normalize(state_.toLight);

        const float nightDepth = std::clamp((-sunElevationDeg_ - 6.0f) / 40.0f, 0.0f, 1.0f);

        // Moonlight: crisp silver-blue beam ensuring strong silhouettes and clear visibility
        state_.lightColor = glm::vec3(0.42f, 0.52f, 0.74f) * (0.65f + 0.35f * (1.0f - nightDepth)) * sunIntensity_;

        // Ambient: softly lit deep midnight sapphire (never pitch-black!)
        state_.ambientSky = glm::vec3(0.12f, 0.16f, 0.28f) * (0.85f + 0.15f * ambientIntensity_);
        state_.ambientGround = glm::vec3(0.07f, 0.09f, 0.15f) * (0.85f + 0.15f * ambientIntensity_);

        // Deep nocturnal sky & atmospheric haze
        state_.skyColor = glm::mix(glm::vec3(0.06f, 0.09f, 0.18f), glm::vec3(0.025f, 0.04f, 0.09f), nightDepth);
        state_.horizonColor = glm::mix(glm::vec3(0.10f, 0.14f, 0.25f), glm::vec3(0.04f, 0.06f, 0.13f), nightDepth);
        state_.fogColor = state_.skyColor;

        state_.fogStart = 65.0f;
        state_.fogEnd = 290.0f;
    }
}

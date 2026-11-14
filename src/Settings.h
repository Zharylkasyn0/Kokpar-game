#pragma once

#include <string>

// ===========================================================================
// Settings Manager & Local Configuration Persistence
//
// Manages:
// - Graphics quality (Low / Medium / High)
// - Screen resolution & Fullscreen mode
// - VSync toggle
// - Camera mouse orbit sensitivity
// - Default time of day & automatic cycle speed
// - Selected Kazakhstan location
// Saves to and loads from 'settings.cfg' in working directory.
// ===========================================================================

struct GameSettings
{
    int graphicsQuality = 1;       // 0 = Low, 1 = Medium, 2 = High
    int resolutionIndex = 0;       // 0 = 1280x720, 1 = 1600x900, 2 = 1920x1080
    bool fullscreen = false;
    bool vsync = true;

    float mouseSensitivity = 1.0f; // 0.5x, 1.0x, 1.5x, 2.0x
    int defaultTimePreset = 2;     // 0 = Dawn .. 2 = Noon .. 5 = Night
    int autoCycleIndex = 0;        // 0 = Off, 1 = 5m, 2 = 10m, 3 = 20m, 4 = 30m
    int selectedLocation = 7;      // Default to 7 (Qazaq Auyl)
};

class SettingsManager
{
public:
    void init();
    bool load();
    bool save() const;

    GameSettings& settings() { return settings_; }
    const GameSettings& settings() const { return settings_; }

    const char* qualityName() const;
    const char* resolutionName() const;
    void getResolution(int& outW, int& outH) const;

private:
    GameSettings settings_;
    std::string configPath_ = "settings.cfg";
};

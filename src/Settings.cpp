#include "Settings.h"

#include <cstdio>
#include <fstream>
#include <sstream>

void SettingsManager::init()
{
    load();
}

bool SettingsManager::load()
{
    std::ifstream file(configPath_);
    if (!file.is_open()) return false;

    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty() || line[0] == '#') continue;
        const auto eq = line.find('=');
        if (eq == std::string::npos) continue;

        const std::string key = line.substr(0, eq);
        const std::string val = line.substr(eq + 1);

        if (key == "graphicsQuality") settings_.graphicsQuality = std::stoi(val);
        else if (key == "resolutionIndex") settings_.resolutionIndex = std::stoi(val);
        else if (key == "fullscreen") settings_.fullscreen = (val == "1" || val == "true");
        else if (key == "vsync") settings_.vsync = (val == "1" || val == "true");
        else if (key == "mouseSensitivity") settings_.mouseSensitivity = std::stof(val);
        else if (key == "defaultTimePreset") settings_.defaultTimePreset = std::stoi(val);
        else if (key == "autoCycleIndex") settings_.autoCycleIndex = std::stoi(val);
        else if (key == "selectedLocation") settings_.selectedLocation = std::stoi(val);
    }
    return true;
}

bool SettingsManager::save() const
{
    std::ofstream file(configPath_);
    if (!file.is_open()) return false;

    file << "# KOKPAR: Wobbly Steppe Settings\n";
    file << "graphicsQuality=" << settings_.graphicsQuality << "\n";
    file << "resolutionIndex=" << settings_.resolutionIndex << "\n";
    file << "fullscreen=" << (settings_.fullscreen ? "1" : "0") << "\n";
    file << "vsync=" << (settings_.vsync ? "1" : "0") << "\n";
    file << "mouseSensitivity=" << settings_.mouseSensitivity << "\n";
    file << "defaultTimePreset=" << settings_.defaultTimePreset << "\n";
    file << "autoCycleIndex=" << settings_.autoCycleIndex << "\n";
    file << "selectedLocation=" << settings_.selectedLocation << "\n";
    return true;
}

const char* SettingsManager::qualityName() const
{
    switch (settings_.graphicsQuality)
    {
    case 0: return "LOW (PERFORMANCE)";
    case 1: return "MEDIUM (BALANCED)";
    case 2: return "HIGH (ENHANCED)";
    default: return "MEDIUM";
    }
}

const char* SettingsManager::resolutionName() const
{
    switch (settings_.resolutionIndex)
    {
    case 0: return "1280 x 720 (HD)";
    case 1: return "1600 x 900 (HD+)";
    case 2: return "1920 x 1080 (FHD)";
    default: return "1280 x 720";
    }
}

void SettingsManager::getResolution(int& outW, int& outH) const
{
    switch (settings_.resolutionIndex)
    {
    case 0: outW = 1280; outH = 720; break;
    case 1: outW = 1600; outH = 900; break;
    case 2: outW = 1920; outH = 1080; break;
    default: outW = 1280; outH = 720; break;
    }
}

#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>

#include "Assets.h"
#include "Lighting.h"
#include "Locations.h"
#include "Renderer.h"
#include "Settings.h"

// ===========================================================================
// Authentic Kazakh Cultural UI & Menu System
//
// Screens:
// - Main Menu (Play, Select Location, Time & Lighting, Settings, Exit)
// - Location Selection (8 Kazakhstan regions, lore, preview cards)
// - Time & Lighting (24h time, 7 presets, sun elevation/azimuth, auto-cycle)
// - Settings (Graphics, resolution, vsync, camera sensitivity, persistence)
// - Pause Menu (ESC in-game: Resume, Restart, Change Location, Settings, Main Menu)
// - In-Game Lighting Control Overlay (F6 in-game quick adjustments)
// ===========================================================================

struct MenuItem
{
    std::string labelEn;
    std::string labelKz;
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;
};

class MenuSystem
{
public:
    void init();
    void update(float dt) { time_ += dt; }

    // Screen Renderers
    void drawMainMenu(Renderer& r, const Assets& a, int width, int height,
                      int selectedIndex, double mouseX, double mouseY,
                      const LocationInfo& curLoc, const LightingManager& lighting);

    void drawLocationSelect(Renderer& r, const Assets& a, int width, int height,
                            const LocationInfo& loc, int locIndex, int totalLocs,
                            int selectedBtn, double mouseX, double mouseY);

    void drawTimeLightingMenu(Renderer& r, const Assets& a, int width, int height,
                              const LightingManager& lighting, const LocationInfo& loc,
                              int selectedBtn, double mouseX, double mouseY);

    void drawSettingsMenu(Renderer& r, const Assets& a, int width, int height,
                          const SettingsManager& settings,
                          int selectedRow, double mouseX, double mouseY);

    void drawPauseMenu(Renderer& r, const Assets& a, int width, int height,
                       int selectedIndex, double mouseX, double mouseY);

    void drawInGameLightingPanel(Renderer& r, const Assets& a, int width, int height,
                                 const LightingManager& lighting);

    // Kazakh ornamental vector motifs (Qoshqar myiz - ram horn patterns)
    static void drawKazakhHorn(Renderer& r, const Assets& a, float cx, float cy, float size, const glm::vec3& col, float alpha = 1.0f);
    static void drawKazakhBorder(Renderer& r, const Assets& a, float x, float y, float w, float h, float s, const glm::vec3& gold, const glm::vec3& bronze);

private:
    float time_ = 0.0f;
};

#pragma once

#include <array>
#include <string>
#include <glm/glm.hpp>

#include "Assets.h"
#include "Horse.h"
#include "Renderer.h"

// ===========================================================================
// Polished Vector HUD & Minimap (v0.4 Free-For-All)
//
// Features:
// - Free-For-All 4-Participant Scoreboard (Player + 3 Bots) with carrier tags
// - Match Countdown Timer (MM:SS)
// - Dynamic Possession & Steal / Score Event Notifications
// - Match Completion & Winner Announcement Modal
// - Real-time 4-Player Radar Minimap with directional indicators & carrier halo
// - Dynamic Speedometer & Gait Gauge for player
// - F1 full controls manual, F2 performance diagnostics, graphics status badges
//
// All text uses the resolution-aware UIFont system (readable at 720p and 1080p).
// ===========================================================================
class Hud
{
public:
    struct Stats
    {
        float fps = 0.0f;
        float avgFps = 0.0f;
        float frameTimeMs = 0.0f;
        int drawCalls = 0;
        bool toonEnabled = true;
        float sunAzimuthDeg = 0.0f;
        float sunElevationDeg = 0.0f;
        bool freeCam = false;
        bool simPaused = false;
        bool showFps = false;
    };

    void init();
    void update(float dt);

    void triggerScoreCelebration() { scoreFlashTimer_ = 1.4f; }
    void toggleHelp() { showHelp_ = !showHelp_; }
    void setHelpVisible(bool visible) { showHelp_ = visible; }
    bool isHelpVisible() const { return showHelp_; }
    void toggleManual() { showManual_ = !showManual_; }
    void setManualVisible(bool visible) { showManual_ = visible; }
    bool isManualVisible() const { return showManual_; }

    void draw(Renderer& renderer, const Assets& assets, int width, int height,
              const std::array<int, 4>& scores, int carrierIndex,
              float matchTime, bool matchOver, int winnerIndex,
              const std::string& bannerMessage, float bannerTimer,
              float speed, float speed01, const char* gaitName,
              const std::array<Horse, 4>& horses,
              const glm::vec3& goatPos, bool goatCarried,
              const glm::vec3& zonePos, float arenaRadius,
              const Stats& stats);

private:
    void drawMinimap(Renderer& r, const Assets& a, float cx, float cy, float radius,
                     const std::array<Horse, 4>& horses, int carrierIndex,
                     const glm::vec3& goatPos, bool goatCarried,
                     const glm::vec3& zonePos, float arenaRadius) const;

    void drawSpeedGauge(Renderer& r, const Assets& a, float x, float y, float w, float h,
                        float speed01, const char* gaitName) const;

    void drawHelpPanel(Renderer& r, const Assets& a, float x, float y, float scale) const;
    void drawHelpOverlay(Renderer& r, const Assets& a, int width, int height, float scale) const;
    void drawDiagnostics(Renderer& r, const Assets& a, float x, float y, float scale,
                         const Stats& stats) const;
    void drawStatusBadges(Renderer& r, const Assets& a, int width, int height, float scale,
                          const Stats& stats) const;

    float time_ = 0.0f;
    float scoreFlashTimer_ = 0.0f;
    bool showHelp_ = false;
    bool showManual_ = false;
};

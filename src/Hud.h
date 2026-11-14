#pragma once

#include <glm/glm.hpp>

#include "Assets.h"
#include "Renderer.h"

// ===========================================================================
// Polished Vector HUD & Minimap (v0.3)
//
// Features:
// - Seven-segment digital score display with golden carry indicator
// - Dynamic speedometer / gait gauge (Walk / Trot / Gallop / Sprint)
// - Real-time circular radar minimap (arena, goal, goat, player arrow)
// - On-screen controls cheat-sheet panel (toggleable with 'H' or 'F1')
// - Celebration score flash effect
// All drawn procedurally with unlit geometry in orthographic projection.
// ===========================================================================
class Hud
{
public:
    void init();
    void update(float dt);

    void triggerScoreCelebration() { scoreFlashTimer_ = 1.4f; }
    void toggleHelp() { showHelp_ = !showHelp_; }
    bool isHelpVisible() const { return showHelp_; }

    void draw(Renderer& renderer, const Assets& assets, int width, int height,
              int score, bool carrying, float speed, float speed01, const char* gaitName,
              const glm::vec3& horsePos, float horseYaw,
              const glm::vec3& goatPos, bool goatCarried,
              const glm::vec3& zonePos, float arenaRadius);

private:
    void drawMinimap(Renderer& r, const Assets& a, float cx, float cy, float radius,
                     const glm::vec3& horsePos, float horseYaw,
                     const glm::vec3& goatPos, bool goatCarried,
                     const glm::vec3& zonePos, float arenaRadius) const;

    void drawSpeedGauge(Renderer& r, const Assets& a, float x, float y, float w, float h,
                        float speed01, const char* gaitName) const;

    void drawHelpPanel(Renderer& r, const Assets& a, float x, float y, float scale) const;

    float time_ = 0.0f;
    float scoreFlashTimer_ = 0.0f;
    bool showHelp_ = true;
};

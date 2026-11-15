#pragma once

#include <string>
#include <glm/glm.hpp>

class Renderer;
class Assets;

// ===========================================================================
// High-Definition Typography System (Resolution-Aware Vector Font)
//
// Features:
// - Sharp, stroke-based vector character rasterization (solid quads, no blurry
//   texture stretching), drawn through the existing HUD batch.
// - Proportional stroke thickness and uniform character spacing.
// - Resolution-aware scaling calibrated for 1280x720, 1600x900 and 1920x1080+.
// - Latin, numeric, punctuation, Russian Cyrillic and Kazakh-specific letters.
// - Word wrapping, right alignment, centred text and optional dark backdrops for
//   maximum contrast against bright steppe / sunlit scenes.
//
// Sizes are expressed in physical pixels (the HUD uses a bottom-left origin).
// ===========================================================================
namespace UIFont
{
    // Resolution-aware UI scaling factor (1.0 at 720p, 1.5 at 1080p).
    float GetScale(int width, int height);

    // Standardized typography scale (in pixels).
    float TitleSize(float scale);        // 42 px at 720p / 63 px at 1080p
    float LargeSize(float scale);        // 30 px at 720p / 45 px at 1080p
    float MenuItemSize(float scale);     // 25 px at 720p / 37 px at 1080p
    float HudSize(float scale);          // 20 px at 720p / 30 px at 1080p
    float DiagnosticsSize(float scale);  // 17 px at 720p / 25 px at 1080p
    float SmallSize(float scale);        // 15 px at 720p / 22 px at 1080p

    // Text measurement (single line unless the string contains newlines).
    float MeasureWidth(const char* text, float fontHeightPx);
    float MeasureWidth(const std::string& text, float fontHeightPx);
    float MeasureHeight(const char* text, float fontHeightPx);
    float MeasureHeight(const std::string& text, float fontHeightPx);

    // Primary text rendering. y is the bottom edge of the (first) line.
    void DrawText(Renderer& r, const Assets& a, const char* text,
                  float x, float y, float fontHeightPx,
                  const glm::vec3& color, float alpha = 1.0f,
                  bool withDarkBacking = false);

    void DrawText(Renderer& r, const Assets& a, const std::string& text,
                  float x, float y, float fontHeightPx,
                  const glm::vec3& color, float alpha = 1.0f,
                  bool withDarkBacking = false);

    // Centered / right-aligned text helpers.
    void DrawTextCentered(Renderer& r, const Assets& a, const char* text,
                          float centerX, float y, float fontHeightPx,
                          const glm::vec3& color, float alpha = 1.0f,
                          bool withDarkBacking = false);

    void DrawTextRight(Renderer& r, const Assets& a, const char* text,
                       float rightX, float y, float fontHeightPx,
                       const glm::vec3& color, float alpha = 1.0f,
                       bool withDarkBacking = false);

    // Word-wrapped text. topY is the TOP edge of the first line; the function
    // returns the total height consumed (in pixels).
    float DrawWrappedText(Renderer& r, const Assets& a, const char* text,
                          float x, float topY, float maxWidth, float fontHeightPx,
                          const glm::vec3& color, float alpha = 1.0f,
                          bool withDarkBacking = false);

    // Badge / Pill with padding.
    void DrawBadge(Renderer& r, const Assets& a, const char* text,
                   float x, float y, float fontHeightPx,
                   const glm::vec3& textColor, const glm::vec3& bgColor,
                   float bgAlpha = 0.85f, float padX = 10.0f, float padY = 5.0f);
}

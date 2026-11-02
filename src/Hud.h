#pragma once

#include "Assets.h"
#include "Renderer.h"

// Text-free HUD: seven-segment score counter plus a "carrying goat" indicator.
// Drawn with the same shader in pixel-space orthographic mode.
class Hud
{
public:
    void draw(Renderer& renderer, const Assets& assets, int width, int height, int score, bool carrying) const;
};

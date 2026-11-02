#include "Hud.h"

#include <algorithm>
#include <string>

#include "Math.h"

namespace
{
// Bits: a=1 (top), b=2 (top-right), c=4 (bottom-right), d=8 (bottom),
//       e=16 (bottom-left), f=32 (top-left), g=64 (middle)
const int kDigitMask[10] = {63, 6, 91, 79, 102, 109, 125, 7, 127, 111};

void Rect(Renderer& r, const Assets& a, float x, float y, float w, float h, const glm::vec3& color, float alpha)
{
    r.drawUnlit(a.box(), Translate(x + w * 0.5f, y + h * 0.5f, 0.0f) * Scale(w, h, 1.0f), color, alpha);
}

void DrawDigit(Renderer& r, const Assets& a, int digit, float x, float y, float L, float t, const glm::vec3& color)
{
    const int mask = kDigitMask[digit];
    if (mask & 1)  Rect(r, a, x + t,         y + 2 * L + 2 * t, L, t, color, 1.0f);   // a
    if (mask & 2)  Rect(r, a, x + L + t,     y + L + 2 * t,     t, L, color, 1.0f);   // b
    if (mask & 4)  Rect(r, a, x + L + t,     y + t,             t, L, color, 1.0f);   // c
    if (mask & 8)  Rect(r, a, x + t,         y,                 L, t, color, 1.0f);   // d
    if (mask & 16) Rect(r, a, x,             y + t,             t, L, color, 1.0f);   // e
    if (mask & 32) Rect(r, a, x,             y + L + 2 * t,     t, L, color, 1.0f);   // f
    if (mask & 64) Rect(r, a, x + t,         y + L + t,         L, t, color, 1.0f);   // g
}
}

void Hud::draw(Renderer& r, const Assets& a, int width, int height, int score, bool carrying) const
{
    r.beginHud(width, height);

    const float s = std::max(1.0f, static_cast<float>(height) / 720.0f);
    const float L = 18.0f * s;
    const float t = 6.0f * s;
    const float gap = 8.0f * s;
    const float pad = 12.0f * s;
    const float digitW = L + 2.0f * t;
    const float digitH = 2.0f * L + 3.0f * t;
    const float indicator = 26.0f * s;

    const std::string text = std::to_string(std::clamp(score, 0, 9999));
    const float n = static_cast<float>(text.size());

    const float panelW = pad * 2.0f + n * digitW + (n - 1.0f) * gap + gap * 2.0f + indicator;
    const float panelH = digitH + pad * 2.0f;
    const float panelX = 16.0f * s;
    const float panelY = static_cast<float>(height) - 16.0f * s - panelH;

    Rect(r, a, panelX, panelY, panelW, panelH, glm::vec3(0.05f, 0.08f, 0.12f), 0.45f);

    float x = panelX + pad;
    const float y = panelY + pad;
    for (char ch : text)
    {
        DrawDigit(r, a, ch - '0', x, y, L, t, glm::vec3(1.0f, 0.95f, 0.75f));
        x += digitW + gap;
    }

    // Indicator square: gold when the goat is being carried, grey otherwise.
    const glm::vec3 indColor = carrying ? glm::vec3(1.0f, 0.80f, 0.15f) : glm::vec3(0.35f, 0.38f, 0.42f);
    Rect(r, a, x + gap, y + (digitH - indicator) * 0.5f, indicator, indicator, indColor, 1.0f);

    r.endHud();
}

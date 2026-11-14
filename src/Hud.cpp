#include "Hud.h"

#include <algorithm>
#include <cmath>
#include <string>

#include "Math.h"

namespace
{
// 7-segment digital display masks
// Bits: a=1 (top), b=2 (top-right), c=4 (bottom-right), d=8 (bottom),
//       e=16 (bottom-left), f=32 (top-left), g=64 (middle)
const int kDigitMask[10] = {63, 6, 91, 79, 102, 109, 125, 7, 127, 111};

void Rect(Renderer& r, const Assets& a, float x, float y, float w, float h, const glm::vec3& color, float alpha = 1.0f)
{
    r.drawUnlit(a.box(), Translate(x + w * 0.5f, y + h * 0.5f, 0.0f) * Scale(w, h, 1.0f), color, alpha);
}

void Disc(Renderer& r, const Assets& a, float cx, float cy, float radius, const glm::vec3& color, float alpha = 1.0f)
{
    r.drawUnlit(a.cylinder(), Translate(cx, cy, 0.0f) * RotX(kHalfPi) * Scale(radius, 0.05f, radius), color, alpha);
}

void DrawDigit(Renderer& r, const Assets& a, int digit, float x, float y, float L, float t, const glm::vec3& color)
{
    const int mask = kDigitMask[std::clamp(digit, 0, 9)];
    if (mask & 1)  Rect(r, a, x + t,         y + 2 * L + 2 * t, L, t, color, 1.0f);   // a
    if (mask & 2)  Rect(r, a, x + L + t,     y + L + 2 * t,     t, L, color, 1.0f);   // b
    if (mask & 4)  Rect(r, a, x + L + t,     y + t,             t, L, color, 1.0f);   // c
    if (mask & 8)  Rect(r, a, x + t,         y,                 L, t, color, 1.0f);   // d
    if (mask & 16) Rect(r, a, x,             y + t,             t, L, color, 1.0f);   // e
    if (mask & 32) Rect(r, a, x,             y + L + 2 * t,     t, L, color, 1.0f);   // f
    if (mask & 64) Rect(r, a, x + t,         y + L + t,         L, t, color, 1.0f);   // g
}

// Compact 3x5 vector glyph table for crisp, dependency-free text
uint16_t GetGlyph3x5(char c)
{
    switch (c)
    {
    case 'A': case 'a': return 0b111101111101101;
    case 'B': case 'b': return 0b110101110101110;
    case 'C': case 'c': return 0b111100100100111;
    case 'D': case 'd': return 0b110101101101110;
    case 'E': case 'e': return 0b111100111100111;
    case 'F': case 'f': return 0b111100111100100;
    case 'G': case 'g': return 0b111100101101111;
    case 'H': case 'h': return 0b101101111101101;
    case 'I': case 'i': return 0b111010010010111;
    case 'J': case 'j': return 0b001001001101010;
    case 'K': case 'k': return 0b101110110101101;
    case 'L': case 'l': return 0b100100100100111;
    case 'M': case 'm': return 0b101111101101101;
    case 'N': case 'n': return 0b111101101101101;
    case 'O': case 'o': return 0b111101101101111;
    case 'P': case 'p': return 0b111101111100100;
    case 'Q': case 'q': return 0b111101101111001;
    case 'R': case 'r': return 0b111101111110101;
    case 'S': case 's': return 0b111100111001111;
    case 'T': case 't': return 0b111010010010010;
    case 'U': case 'u': return 0b101101101101111;
    case 'V': case 'v': return 0b101101101101010;
    case 'W': case 'w': return 0b101101101111101;
    case 'X': case 'x': return 0b101101010101101;
    case 'Y': case 'y': return 0b101101010010010;
    case 'Z': case 'z': return 0b111001010100111;
    case '0':           return 0b111101101101111;
    case '1':           return 0b010110010010111;
    case '2':           return 0b111001111100111;
    case '3':           return 0b111001111001111;
    case '4':           return 0b101101111001001;
    case '5':           return 0b111100111001111;
    case '6':           return 0b111100111101111;
    case '7':           return 0b111001010010010;
    case '8':           return 0b111101111101111;
    case '9':           return 0b111101111001111;
    case '/':           return 0b001001010100100;
    case '-':           return 0b000000111000000;
    case ':':           return 0b000010000010000;
    case '.':           return 0b000000000000010;
    case '(':           return 0b010100100100010;
    case ')':           return 0b010001001001010;
    case '!':           return 0b010010010000010;
    case '>':           return 0b100010001010100;
    default:            return 0b000000000000000;
    }
}

void DrawText3x5(Renderer& r, const Assets& a, const char* str, float x, float y, float px, const glm::vec3& color, float alpha = 1.0f)
{
    float curX = x;
    while (*str)
    {
        const char c = *str++;
        if (c == ' ')
        {
            curX += px * 4.0f;
            continue;
        }
        const uint16_t glyph = GetGlyph3x5(c);
        for (int row = 0; row < 5; ++row)
        {
            const float py = y + static_cast<float>(4 - row) * px;
            const int shift = (4 - row) * 3;
            const int bits = (glyph >> shift) & 0x7;
            for (int col = 0; col < 3; ++col)
            {
                if (bits & (1 << (2 - col)))
                {
                    Rect(r, a, curX + static_cast<float>(col) * px, py, px, px, color, alpha);
                }
            }
        }
        curX += px * 4.0f;
    }
}
}

void Hud::init()
{
    time_ = 0.0f;
    scoreFlashTimer_ = 0.0f;
    showHelp_ = true;
}

void Hud::update(float dt)
{
    time_ += dt;
    scoreFlashTimer_ = std::max(0.0f, scoreFlashTimer_ - dt);
}

void Hud::draw(Renderer& r, const Assets& a, int width, int height,
               int score, bool carrying, float speed, float speed01, const char* gaitName,
               const glm::vec3& horsePos, float horseYaw,
               const glm::vec3& goatPos, bool goatCarried,
               const glm::vec3& zonePos, float arenaRadius)
{
    (void)speed;
    r.beginHud(width, height);

    const float s = std::max(1.0f, static_cast<float>(height) / 720.0f);

    // =======================================================================
    // 1. Seven-Segment Digital Score Panel (Top-Left)
    // =======================================================================
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
    const float panelX = 18.0f * s;
    const float panelY = static_cast<float>(height) - 18.0f * s - panelH;

    // Background panel with score-flash border
    const glm::vec3 bgPanelColor = (scoreFlashTimer_ > 0.0f) ?
                                   glm::vec3(0.25f, 0.20f, 0.08f) :
                                   glm::vec3(0.05f, 0.08f, 0.12f);
    Rect(r, a, panelX, panelY, panelW, panelH, bgPanelColor, 0.65f);

    if (scoreFlashTimer_ > 0.0f)
    {
        // Glowing gold border
        const float b = 2.5f * s;
        Rect(r, a, panelX - b, panelY - b, panelW + b * 2.0f, b, glm::vec3(1.0f, 0.85f, 0.2f), 0.9f);
        Rect(r, a, panelX - b, panelY + panelH, panelW + b * 2.0f, b, glm::vec3(1.0f, 0.85f, 0.2f), 0.9f);
        Rect(r, a, panelX - b, panelY, b, panelH, glm::vec3(1.0f, 0.85f, 0.2f), 0.9f);
        Rect(r, a, panelX + panelW, panelY, b, panelH, glm::vec3(1.0f, 0.85f, 0.2f), 0.9f);
    }

    float curX = panelX + pad;
    const float curY = panelY + pad;
    for (char ch : text)
    {
        DrawDigit(r, a, ch - '0', curX, curY, L, t, glm::vec3(1.0f, 0.95f, 0.75f));
        curX += digitW + gap;
    }

    // Indicator square: gold when the goat is held, dark slate otherwise
    const glm::vec3 indColor = carrying ? glm::vec3(1.0f, 0.82f, 0.15f) : glm::vec3(0.28f, 0.32f, 0.38f);
    Rect(r, a, curX + gap, curY + (digitH - indicator) * 0.5f, indicator, indicator, indColor, 1.0f);

    // =======================================================================
    // 2. Dynamic Speedometer / Gait Gauge (Below Score Panel)
    // =======================================================================
    drawSpeedGauge(r, a, panelX, panelY - 24.0f * s, panelW, 12.0f * s, speed01, gaitName);

    // =======================================================================
    // 3. Circular Radar Minimap (Bottom-Right)
    // =======================================================================
    const float mapRadius = 62.0f * s;
    const float mapCx = static_cast<float>(width) - mapRadius - 20.0f * s;
    const float mapCy = mapRadius + 20.0f * s;
    drawMinimap(r, a, mapCx, mapCy, mapRadius, horsePos, horseYaw, goatPos, goatCarried, zonePos, arenaRadius);

    // =======================================================================
    // 4. On-Screen Controls Help (Top-Right, toggleable)
    // =======================================================================
    if (showHelp_)
    {
        drawHelpPanel(r, a, static_cast<float>(width) - 245.0f * s, static_cast<float>(height) - 268.0f * s, s);
    }

    r.endHud();
}

void Hud::drawSpeedGauge(Renderer& r, const Assets& a, float x, float y, float w, float h,
                        float speed01, const char* gaitName) const
{
    // Dark track background
    Rect(r, a, x, y, w, h, glm::vec3(0.04f, 0.06f, 0.08f), 0.60f);

    // Speed fill bar with color shifting (Cyan/Green -> Gold -> Ruby)
    const float fillW = w * std::clamp(speed01, 0.0f, 1.0f);
    glm::vec3 speedCol;
    if (speed01 < 0.28f) speedCol = glm::vec3(0.20f, 0.85f, 0.65f);      // Walk (mint)
    else if (speed01 < 0.60f) speedCol = glm::vec3(1.0f, 0.85f, 0.20f);  // Trot (gold)
    else speedCol = glm::vec3(0.95f, 0.28f, 0.22f);                      // Gallop/Sprint (ruby)

    if (fillW > 1.0f)
    {
        Rect(r, a, x, y, fillW, h, speedCol, 0.90f);
    }

    // Gait dividers
    Rect(r, a, x + w * 0.28f, y, 1.5f, h, glm::vec3(0.5f), 0.7f);
    Rect(r, a, x + w * 0.60f, y, 1.5f, h, glm::vec3(0.5f), 0.7f);

    // Active Gait Label above gauge
    if (gaitName && gaitName[0] != '\0')
    {
        const float fontPx = std::max(1.0f, h * 0.17f);
        DrawText3x5(r, a, gaitName, x + 2.0f, y + h + 4.0f, fontPx, speedCol, 0.95f);
    }
}

void Hud::drawMinimap(Renderer& r, const Assets& a, float cx, float cy, float radius,
                     const glm::vec3& horsePos, float horseYaw,
                     const glm::vec3& goatPos, bool goatCarried,
                     const glm::vec3& zonePos, float arenaRadius) const
{
    // 1. Radar background disc
    Disc(r, a, cx, cy, radius, glm::vec3(0.04f, 0.07f, 0.10f), 0.68f);

    // 2. Outer arena perimeter boundary ring
    const int ringSteps = 32;
    for (int i = 0; i < ringSteps; ++i)
    {
        const float theta = kTwoPi * static_cast<float>(i) / static_cast<float>(ringSteps);
        const float px = cx + std::cos(theta) * (radius - 3.0f);
        const float py = cy + std::sin(theta) * (radius - 3.0f);
        Rect(r, a, px - 1.0f, py - 1.0f, 2.0f, 2.0f, glm::vec3(0.00f, 0.64f, 0.82f), 0.75f);
    }

    // Scale world coordinate to minimap coordinate
    const float mapScale = (radius - 6.0f) / std::max(arenaRadius, 1.0f);
    const float maxR = radius - 5.0f;

    auto clampMinimapPos = [&](float wx, float wz, float& outMx, float& outMy) {
        float dx = wx * mapScale;
        float dy = wz * mapScale;
        const float d = std::sqrt(dx * dx + dy * dy);
        if (d > maxR && d > 1e-4f)
        {
            dx = (dx / d) * maxR;
            dy = (dy / d) * maxR;
        }
        outMx = cx + dx;
        outMy = cy + dy;
    };

    // 3. Scoring Zone (Kazanlyk) circle
    float zx = 0.0f, zy = 0.0f;
    clampMinimapPos(zonePos.x, zonePos.z, zx, zy);
    Disc(r, a, zx, zy, 7.5f, glm::vec3(0.92f, 0.25f, 0.18f), 0.85f);
    Disc(r, a, zx, zy, 4.0f, glm::vec3(1.0f, 0.85f, 0.20f), 0.95f);

    // 4. Goat marker (pulsing gold diamond when idle on ground)
    if (!goatCarried)
    {
        float gx = 0.0f, gy = 0.0f;
        clampMinimapPos(goatPos.x, goatPos.z, gx, gy);
        const float gPulse = 4.2f + 1.2f * std::sin(time_ * 7.0f);
        Disc(r, a, gx, gy, gPulse, glm::vec3(1.0f, 0.85f, 0.15f), 1.0f);
    }

    // 5. Horse & Player Marker (Directional turquoise arrow + carried glow)
    float hx = 0.0f, hy = 0.0f;
    clampMinimapPos(horsePos.x, horsePos.z, hx, hy);
    if (goatCarried)
    {
        Disc(r, a, hx, hy, 7.0f, glm::vec3(1.0f, 0.82f, 0.15f), 0.85f);
    }
    Disc(r, a, hx, hy, 4.2f, glm::vec3(0.00f, 0.75f, 0.95f), 1.0f);

    // Heading forward pointer
    const float fwdX = std::sin(horseYaw);
    const float fwdY = std::cos(horseYaw);
    const float tipX = hx + fwdX * 9.0f;
    const float tipY = hy + fwdY * 9.0f;
    Rect(r, a, tipX - 1.5f, tipY - 1.5f, 3.0f, 3.0f, glm::vec3(1.0f, 1.0f, 1.0f), 1.0f);
}

void Hud::drawHelpPanel(Renderer& r, const Assets& a, float x, float y, float scale) const
{
    const float w = 232.0f * scale;
    const float h = 252.0f * scale;

    // Dark semi-transparent card
    Rect(r, a, x, y, w, h, glm::vec3(0.04f, 0.06f, 0.10f), 0.75f);

    // Top accent bar (Turquoise)
    Rect(r, a, x, y + h - 3.0f * scale, w, 3.0f * scale, glm::vec3(0.00f, 0.64f, 0.82f), 0.95f);

    // Panel Header
    const float headPx = 1.6f * scale;
    DrawText3x5(r, a, "CONTROLS (H/F1)", x + 10.0f * scale, y + h - 18.0f * scale, headPx, glm::vec3(0.00f, 0.75f, 0.95f));

    struct ControlRow {
        const char* key;
        const char* desc;
        glm::vec3 col;
    };
    const ControlRow rows[] = {
        {"WASD",    "RIDE / STEER",      glm::vec3(0.20f, 0.85f, 0.60f)},
        {"SHIFT",   "SPRINT GALLOP",     glm::vec3(0.95f, 0.35f, 0.25f)},
        {"SPACE",   "QUICK BRAKE",       glm::vec3(0.95f, 0.75f, 0.20f)},
        {"E / LMB", "GRAB / FLING GOAT", glm::vec3(1.00f, 0.82f, 0.18f)},
        {"MOUSE",   "360 LOOK ORBIT",    glm::vec3(0.00f, 0.65f, 0.90f)},
        {"SCROLL",  "CAMERA ZOOM",       glm::vec3(0.40f, 0.65f, 0.85f)},
        {"TAB",     "FREE / LOCK MOUSE", glm::vec3(0.70f, 0.50f, 0.85f)},
        {"Z / V",   "RESET CAMERA",      glm::vec3(0.45f, 0.75f, 0.85f)},
        {"C",       "CYCLE CAM MODE",    glm::vec3(0.55f, 0.65f, 0.75f)},
        {"R",       "RESET GAME",        glm::vec3(0.85f, 0.40f, 0.40f)},
    };

    float curY = y + h - 38.0f * scale;
    const float rowStep = 20.0f * scale;
    const float fontPx = 1.30f * scale;

    for (const auto& row : rows)
    {
        // Key badge background
        const float keyW = 54.0f * scale;
        Rect(r, a, x + 8.0f * scale, curY - 2.0f * scale, keyW, 14.0f * scale, glm::vec3(0.10f, 0.14f, 0.20f), 0.90f);
        // Key text
        DrawText3x5(r, a, row.key, x + 11.0f * scale, curY + 2.0f * scale, fontPx * 0.95f, row.col);
        // Description text
        DrawText3x5(r, a, row.desc, x + 8.0f * scale + keyW + 8.0f * scale, curY + 2.0f * scale, fontPx * 0.85f, glm::vec3(0.82f, 0.86f, 0.92f));
        curY -= rowStep;
    }
}

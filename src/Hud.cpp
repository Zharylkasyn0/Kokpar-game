#include "Hud.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

#include "Math.h"
#include "UIFont.h"

namespace
{
// 7-segment digital display masks
// Bits: a=1 (top), b=2 (top-right), c=4 (bottom-right), d=8 (bottom),
//       e=16 (bottom-left), f=32 (top-left), g=64 (middle)
const int kDigitMask[10] = {63, 6, 91, 79, 102, 109, 125, 7, 127, 111};

void Rect(Renderer& r, const Assets& a, float x, float y, float w, float h,
          const glm::vec3& color, float alpha = 1.0f)
{
    r.drawUnlit(a.box(), Translate(x + w * 0.5f, y + h * 0.5f, 0.0f) * Scale(w, h, 1.0f), color, alpha);
}

void Disc(Renderer& r, const Assets& a, float cx, float cy, float radius,
          const glm::vec3& color, float alpha = 1.0f)
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

struct TextRow
{
    const char* key;
    const char* desc;
    glm::vec3 color;
};
}

void Hud::init()
{
    time_ = 0.0f;
    scoreFlashTimer_ = 0.0f;
    showHelp_ = false;
    showManual_ = false;
}

void Hud::update(float dt)
{
    time_ += dt;
    scoreFlashTimer_ = std::max(0.0f, scoreFlashTimer_ - dt);
}

void Hud::draw(Renderer& r, const Assets& a, int width, int height,
               const std::array<int, 4>& scores, int carrierIndex,
               float matchTime, bool matchOver, int winnerIndex,
               const std::string& bannerMessage, float bannerTimer,
               float speed, float speed01, const char* gaitName,
               const std::array<Horse, 4>& horses,
               const glm::vec3& goatPos, bool goatCarried,
               const glm::vec3& zonePos, float arenaRadius,
               const Stats& stats)
{
    (void)speed;
    r.beginHud(width, height);

    const float s = UIFont::GetScale(width, height);
    const float hudPx = UIFont::HudSize(s);
    const float smallPx = UIFont::SmallSize(s);

    const glm::vec3 panelBg(0.04f, 0.07f, 0.12f);
    const glm::vec3 gold(1.0f, 0.84f, 0.28f);
    const glm::vec3 cyan(0.00f, 0.80f, 0.95f);
    const glm::vec3 textLight(0.92f, 0.94f, 0.96f);

    // =======================================================================
    // 1. FFA 4-Participant Scoreboard (Top-Left)
    // =======================================================================
    const float panelW = 330.0f * s;
    const float panelH = 178.0f * s;
    const float panelX = 16.0f * s;
    const float panelY = static_cast<float>(height) - 16.0f * s - panelH;

    const glm::vec3 bgPanelColor = (scoreFlashTimer_ > 0.0f)
                                 ? glm::vec3(0.25f, 0.20f, 0.08f)
                                 : panelBg;
    Rect(r, a, panelX, panelY, panelW, panelH, bgPanelColor, 0.78f);

    const float b = 2.0f * s;
    const glm::vec3 borderColor = (scoreFlashTimer_ > 0.0f)
                                ? gold
                                : glm::vec3(0.12f, 0.42f, 0.58f);
    Rect(r, a, panelX, panelY + panelH - b, panelW, b, borderColor, 0.95f);
    Rect(r, a, panelX, panelY, panelW, b, borderColor, 0.60f);
    Rect(r, a, panelX, panelY, b, panelH, borderColor, 0.60f);
    Rect(r, a, panelX + panelW - b, panelY, b, panelH, borderColor, 0.60f);

    const float headPx = hudPx * 0.95f;
    UIFont::DrawText(r, a, "FREE-FOR-ALL KOKPAR",
                     panelX + 12.0f * s, panelY + panelH - 14.0f * s - headPx,
                     headPx, cyan);

    const TextRow pRows[4] = {
        {"PLAYER", "", glm::vec3(0.00f, 0.88f, 1.00f)},
        {"BOT 1",  "", glm::vec3(0.45f, 0.72f, 1.00f)},
        {"BOT 2",  "", glm::vec3(1.00f, 0.50f, 0.42f)},
        {"BOT 3",  "", glm::vec3(0.38f, 0.92f, 0.48f)},
    };

    const float rowFont = hudPx * 0.86f;
    const float rowStartY = panelY + panelH - 52.0f * s;
    const float rowSpacing = 28.0f * s;

    for (int i = 0; i < 4; ++i)
    {
        const float ry = rowStartY - static_cast<float>(i) * rowSpacing;
        const bool isCarrier = (carrierIndex == i);

        const glm::vec3 badgeCol = isCarrier ? gold : glm::vec3(0.18f, 0.22f, 0.28f);
        Rect(r, a, panelX + 12.0f * s, ry + 2.0f * s, 11.0f * s, 11.0f * s,
             badgeCol, isCarrier ? 1.0f : 0.65f);

        UIFont::DrawText(r, a, pRows[i].key, panelX + 30.0f * s, ry, rowFont, pRows[i].color, 1.0f);

        char scoreBuf[16];
        std::snprintf(scoreBuf, sizeof(scoreBuf), "%d", std::clamp(scores[i], 0, 9999));
        UIFont::DrawTextRight(r, a, scoreBuf, panelX + panelW - 112.0f * s, ry,
                              rowFont, glm::vec3(1.0f, 0.96f, 0.86f));

        if (isCarrier)
        {
            UIFont::DrawBadge(r, a, "SERKE", panelX + panelW - 104.0f * s, ry - 2.0f * s,
                              smallPx * 0.92f, glm::vec3(0.12f, 0.09f, 0.02f), gold,
                              0.92f, 6.0f * s, 3.0f * s);
        }
    }

    // =======================================================================
    // 2. Dynamic Speedometer / Gait Gauge (Below Score Panel)
    // =======================================================================
    const float gaugeY = panelY - 52.0f * s;
    drawSpeedGauge(r, a, panelX, gaugeY, panelW, 16.0f * s, speed01, gaitName);

    // =======================================================================
    // 3. Match Countdown Timer (Top-Center)
    // =======================================================================
    const float timerW = 200.0f * s;
    const float timerH = 46.0f * s;
    const float timerX = (static_cast<float>(width) - timerW) * 0.5f;
    const float timerY = static_cast<float>(height) - 16.0f * s - timerH;

    Rect(r, a, timerX, timerY, timerW, timerH, panelBg, 0.82f);
    Rect(r, a, timerX, timerY + timerH - 2.0f * s, timerW, 2.0f * s, cyan, 0.95f);

    const int totalSecs = std::max(0, static_cast<int>(matchTime));
    const int mins = totalSecs / 60;
    const int secs = totalSecs % 60;
    char timeStr[32];
    std::snprintf(timeStr, sizeof(timeStr), "TIME %02d:%02d", mins, secs);

    const glm::vec3 timerCol = (matchTime <= 30.0f && std::fmod(time_, 0.6f) < 0.3f)
                             ? glm::vec3(1.0f, 0.36f, 0.26f)
                             : glm::vec3(1.0f, 0.93f, 0.72f);
    const float timerPx = hudPx * 1.05f;
    UIFont::DrawTextCentered(r, a, timeStr, timerX + timerW * 0.5f,
                             timerY + (timerH - timerPx) * 0.5f, timerPx, timerCol);

    // =======================================================================
    // 4. Possession & Event Notification Banner (Below Timer)
    // =======================================================================
    const float bannerW = 680.0f * s;
    const float bannerH = 36.0f * s;
    const float bannerX = (static_cast<float>(width) - bannerW) * 0.5f;
    const float bannerY = timerY - 46.0f * s;
    const float bannerPx = smallPx * 1.02f;

    if (bannerTimer > 0.0f && !bannerMessage.empty())
    {
        Rect(r, a, bannerX, bannerY, bannerW, bannerH, glm::vec3(0.20f, 0.14f, 0.05f), 0.90f);
        Rect(r, a, bannerX, bannerY, bannerW, 2.0f * s, gold, 0.95f);
        Rect(r, a, bannerX, bannerY + bannerH - 2.0f * s, bannerW, 2.0f * s, gold, 0.95f);
        UIFont::DrawTextCentered(r, a, bannerMessage.c_str(), bannerX + bannerW * 0.5f,
                                 bannerY + (bannerH - bannerPx) * 0.5f, bannerPx,
                                 glm::vec3(1.0f, 0.90f, 0.34f));
    }
    else
    {
        Rect(r, a, bannerX, bannerY, bannerW, bannerH, panelBg, 0.62f);

        std::string statusText;
        glm::vec3 statusCol;
        if (carrierIndex < 0)
        {
            statusText = "[SERKE ON GROUND - PICK UP WITH E]";
            statusCol = gold;
        }
        else if (carrierIndex == 0)
        {
            statusText = "CARRIER: PLAYER (YOU HAVE THE GOAT!)";
            statusCol = cyan;
        }
        else
        {
            statusText = std::string("CARRIER: BOT ") + std::to_string(carrierIndex) + " (PRESS E TO STEAL!)";
            statusCol = pRows[carrierIndex].color;
        }
        UIFont::DrawTextCentered(r, a, statusText.c_str(), bannerX + bannerW * 0.5f,
                                 bannerY + (bannerH - bannerPx) * 0.5f, bannerPx, statusCol, 0.98f);
    }

    // =======================================================================
    // 5. Match Over Modal (Center of screen when match finishes)
    // =======================================================================
    if (matchOver)
    {
        const float modalW = 560.0f * s;
        const float modalH = 250.0f * s;
        const float modalX = (static_cast<float>(width) - modalW) * 0.5f;
        const float modalY = (static_cast<float>(height) - modalH) * 0.5f;

        Rect(r, a, modalX, modalY, modalW, modalH, glm::vec3(0.03f, 0.05f, 0.08f), 0.94f);
        const float mb = 3.0f * s;
        Rect(r, a, modalX - mb, modalY - mb, modalW + mb * 2.0f, mb, gold, 1.0f);
        Rect(r, a, modalX - mb, modalY + modalH, modalW + mb * 2.0f, mb, gold, 1.0f);
        Rect(r, a, modalX - mb, modalY, mb, modalH, gold, 1.0f);
        Rect(r, a, modalX + modalW, modalY, mb, modalH, gold, 1.0f);

        UIFont::DrawTextCentered(r, a, "MATCH FINISHED!", modalX + modalW * 0.5f,
                                 modalY + modalH - 46.0f * s, UIFont::LargeSize(s), gold);

        std::string winStr;
        glm::vec3 winCol;
        if (winnerIndex == 0)
        {
            winStr = "WINNER: PLAYER - VICTORY!";
            winCol = cyan;
        }
        else if (winnerIndex > 0 && winnerIndex < 4)
        {
            winStr = std::string("WINNER: BOT ") + std::to_string(winnerIndex) + "!";
            winCol = pRows[winnerIndex].color;
        }
        else
        {
            winStr = "MATCH RESULT: DRAW / TIE!";
            winCol = glm::vec3(0.95f, 0.95f, 0.95f);
        }
        UIFont::DrawTextCentered(r, a, winStr.c_str(), modalX + modalW * 0.5f,
                                 modalY + modalH - 88.0f * s, UIFont::MenuItemSize(s), winCol);

        char scoreSummary[80];
        std::snprintf(scoreSummary, sizeof(scoreSummary), "P:%d   BOT1:%d   BOT2:%d   BOT3:%d",
                      scores[0], scores[1], scores[2], scores[3]);
        UIFont::DrawTextCentered(r, a, scoreSummary, modalX + modalW * 0.5f,
                                 modalY + modalH - 138.0f * s, hudPx,
                                 glm::vec3(0.86f, 0.90f, 0.94f));

        UIFont::DrawTextCentered(r, a, "PRESS 'R' TO PLAY AGAIN", modalX + modalW * 0.5f,
                                 modalY + 32.0f * s, smallPx * 1.05f,
                                 glm::vec3(0.25f, 0.92f, 0.62f));
    }

    // =======================================================================
    // 6. Circular Radar Minimap (Bottom-Right)
    // =======================================================================
    const float mapRadius = 80.0f * s;
    const float mapCx = static_cast<float>(width) - mapRadius - 20.0f * s;
    const float mapCy = mapRadius + 20.0f * s;
    drawMinimap(r, a, mapCx, mapCy, mapRadius, horses, carrierIndex, goatPos, goatCarried, zonePos, arenaRadius);
    UIFont::DrawTextCentered(r, a, "RADAR", mapCx, mapCy + mapRadius + 7.0f * s,
                             smallPx * 0.85f, glm::vec3(0.55f, 0.78f, 0.88f), 0.85f);

    // =======================================================================
    // 7. Graphics status badges (Top-Right)
    // =======================================================================
    drawStatusBadges(r, a, width, height, s, stats);

    // =======================================================================
    // 8. Quick controls card (Top-Right, toggleable with H / F1)
    // =======================================================================
    if (showHelp_)
    {
        const float helpW = 560.0f * s;
        const float helpH = 306.0f * s;
        drawHelpPanel(r, a, static_cast<float>(width) - helpW - 16.0f * s,
                      static_cast<float>(height) - 124.0f * s - helpH, s);
    }

    // =======================================================================
    // 9. F2 performance & diagnostics overlay (Bottom-Left)
    // =======================================================================
    if (stats.showFps)
    {
        drawDiagnostics(r, a, 16.0f * s, 16.0f * s, s, stats);
    }

    // =======================================================================
    // 10. F1 full controls manual
    // =======================================================================
    if (showManual_)
    {
        drawHelpOverlay(r, a, width, height, s);
    }

    r.endHud();
}

void Hud::drawSpeedGauge(Renderer& r, const Assets& a, float x, float y, float w, float h,
                         float speed01, const char* gaitName) const
{
    Rect(r, a, x, y, w, h, glm::vec3(0.04f, 0.06f, 0.08f), 0.70f);

    const float fillW = w * std::clamp(speed01, 0.0f, 1.0f);
    glm::vec3 speedCol;
    if (speed01 < 0.28f) speedCol = glm::vec3(0.20f, 0.85f, 0.65f);      // Walk (mint)
    else if (speed01 < 0.60f) speedCol = glm::vec3(1.0f, 0.85f, 0.20f);  // Trot (gold)
    else speedCol = glm::vec3(0.95f, 0.28f, 0.22f);                      // Gallop (ruby)

    if (fillW > 1.0f)
    {
        Rect(r, a, x, y, fillW, h, speedCol, 0.92f);
    }

    Rect(r, a, x + w * 0.28f, y, 1.5f, h, glm::vec3(0.55f), 0.7f);
    Rect(r, a, x + w * 0.60f, y, 1.5f, h, glm::vec3(0.55f), 0.7f);

    if (gaitName && gaitName[0] != '\0')
    {
        const float fontPx = UIFont::SmallSize(static_cast<float>(h) / 14.0f) * 0.92f;
        UIFont::DrawText(r, a, gaitName, x + 2.0f, y + h + 5.0f, fontPx, speedCol, 0.98f);
    }
}

void Hud::drawMinimap(Renderer& r, const Assets& a, float cx, float cy, float radius,
                      const std::array<Horse, 4>& horses, int carrierIndex,
                      const glm::vec3& goatPos, bool goatCarried,
                      const glm::vec3& zonePos, float arenaRadius) const
{
    const float k = radius / 66.0f; // scale the original 66 px radar artwork

    Disc(r, a, cx, cy, radius, glm::vec3(0.04f, 0.07f, 0.10f), 0.76f);

    const int ringSteps = 40;
    for (int i = 0; i < ringSteps; ++i)
    {
        const float theta = kTwoPi * static_cast<float>(i) / static_cast<float>(ringSteps);
        const float px = cx + std::cos(theta) * (radius - 3.0f * k);
        const float py = cy + std::sin(theta) * (radius - 3.0f * k);
        Rect(r, a, px - 1.0f * k, py - 1.0f * k, 2.0f * k, 2.0f * k,
             glm::vec3(0.00f, 0.64f, 0.82f), 0.80f);
    }

    const float mapScale = (radius - 6.0f * k) / std::max(arenaRadius, 1.0f);
    const float maxR = radius - 5.0f * k;

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

    float zx = 0.0f, zy = 0.0f;
    clampMinimapPos(zonePos.x, zonePos.z, zx, zy);
    Disc(r, a, zx, zy, 7.5f * k, glm::vec3(0.92f, 0.25f, 0.18f), 0.88f);
    Disc(r, a, zx, zy, 4.0f * k, glm::vec3(1.0f, 0.85f, 0.20f), 0.95f);

    if (!goatCarried)
    {
        float gx = 0.0f, gy = 0.0f;
        clampMinimapPos(goatPos.x, goatPos.z, gx, gy);
        const float gPulse = (4.2f + 1.2f * std::sin(time_ * 7.0f)) * k;
        Disc(r, a, gx, gy, gPulse, glm::vec3(1.0f, 0.85f, 0.15f), 1.0f);
    }

    const glm::vec3 pColors[4] = {
        glm::vec3(0.00f, 0.85f, 0.95f),
        glm::vec3(0.40f, 0.70f, 1.00f),
        glm::vec3(1.00f, 0.45f, 0.38f),
        glm::vec3(0.35f, 0.90f, 0.45f),
    };

    for (int i = 0; i < 4; ++i)
    {
        float hx = 0.0f, hy = 0.0f;
        clampMinimapPos(horses[i].position().x, horses[i].position().z, hx, hy);

        if (carrierIndex == i)
        {
            const float pulseHalo = (7.5f + 1.5f * std::sin(time_ * 8.0f)) * k;
            Disc(r, a, hx, hy, pulseHalo, glm::vec3(1.0f, 0.82f, 0.15f), 0.88f);
        }

        const float dotSize = ((i == 0) ? 4.5f : 3.6f) * k;
        Disc(r, a, hx, hy, dotSize, pColors[i], 1.0f);

        const float fwdX = std::sin(horses[i].yaw());
        const float fwdY = std::cos(horses[i].yaw());
        const float tipDist = ((i == 0) ? 8.5f : 7.0f) * k;
        const float tipX = hx + fwdX * tipDist;
        const float tipY = hy + fwdY * tipDist;
        Rect(r, a, tipX - 1.2f * k, tipY - 1.2f * k, 2.4f * k, 2.4f * k,
             glm::vec3(1.0f), 0.95f);
    }
}

void Hud::drawHelpPanel(Renderer& r, const Assets& a, float x, float y, float scale) const
{
    const float w = 560.0f * scale;
    const float h = 306.0f * scale;

    Rect(r, a, x, y, w, h, glm::vec3(0.04f, 0.06f, 0.10f), 0.84f);
    Rect(r, a, x, y + h - 3.0f * scale, w, 3.0f * scale, glm::vec3(0.00f, 0.64f, 0.82f), 0.95f);

    const float headPx = UIFont::HudSize(scale) * 0.95f;
    UIFont::DrawText(r, a, "CONTROLS  -  PRESS F1 FOR FULL MANUAL",
                     x + 12.0f * scale, y + h - 18.0f * scale - headPx, headPx,
                     glm::vec3(0.00f, 0.80f, 0.95f));

    const TextRow leftRows[] = {
        {"W / S",   "RIDE FWD / BACK",   glm::vec3(0.20f, 0.88f, 0.62f)},
        {"A / D",   "STEER LEFT/RIGHT",  glm::vec3(0.20f, 0.88f, 0.62f)},
        {"SHIFT",   "SPRINT GALLOP",     glm::vec3(0.95f, 0.40f, 0.28f)},
        {"SPACE",   "QUICK BRAKE",       glm::vec3(0.95f, 0.78f, 0.22f)},
        {"E / LMB", "GRAB / STEAL",      glm::vec3(1.00f, 0.84f, 0.20f)},
        {"MOUSE",   "ORBIT CAMERA",      glm::vec3(0.00f, 0.68f, 0.92f)},
        {"SCROLL",  "ZOOM CAMERA",       glm::vec3(0.40f, 0.68f, 0.88f)},
        {"TAB",     "FREE / LOCK MOUSE", glm::vec3(0.72f, 0.52f, 0.88f)},
        {"C",       "CYCLE CAM MODE",    glm::vec3(0.58f, 0.68f, 0.78f)},
        {"Z / V",   "RESET CAMERA",      glm::vec3(0.48f, 0.78f, 0.88f)},
        {"R",       "RESTART MATCH",     glm::vec3(0.88f, 0.44f, 0.42f)},
        {"ESC",     "PAUSE MENU",        glm::vec3(0.88f, 0.44f, 0.42f)},
    };

    const TextRow rightRows[] = {
        {"F1 / H", "FULL HELP",          glm::vec3(0.00f, 0.85f, 0.95f)},
        {"F2",     "FPS DIAGNOSTICS",    glm::vec3(0.45f, 0.90f, 0.65f)},
        {"F3",     "TOON SHADING",       glm::vec3(1.00f, 0.84f, 0.20f)},
        {"F4",     "FREE CAMERA",        glm::vec3(0.40f, 0.72f, 1.00f)},
        {"Q / E",  "FREE CAM DOWN/UP",   glm::vec3(0.40f, 0.72f, 1.00f)},
        {"P",      "PAUSE ANIMATION",    glm::vec3(1.00f, 0.62f, 0.25f)},
        {"I / K",  "SUN ELEVATION",      glm::vec3(1.00f, 0.88f, 0.40f)},
        {"J / L",  "SUN AZIMUTH",        glm::vec3(1.00f, 0.88f, 0.40f)},
        {"[ / ]",  "SUN AZIMUTH ALT",    glm::vec3(0.90f, 0.80f, 0.40f)},
        {"- / =",  "SUN ELEV ALT",       glm::vec3(0.90f, 0.80f, 0.40f)},
        {"F6",     "LIGHTING PANEL",     glm::vec3(0.00f, 0.72f, 0.92f)},
        {"1 - 7",  "LIGHT PRESETS",      glm::vec3(0.00f, 0.72f, 0.92f)},
    };

    const float rowFont = UIFont::SmallSize(scale) * 0.95f;
    const float rowStep = 19.0f * scale;
    const float topY = y + h - 62.0f * scale;
    const float keyW = 68.0f * scale;
    const float colW = w * 0.5f;

    auto drawColumn = [&](float colX, const TextRow* rows, int count) {
        float rowY = topY;
        for (int i = 0; i < count; ++i)
        {
            Rect(r, a, colX, rowY - 3.0f * scale, keyW, rowFont + 7.0f * scale,
                 glm::vec3(0.10f, 0.15f, 0.21f), 0.95f);
            UIFont::DrawText(r, a, rows[i].key, colX + 5.0f * scale, rowY, rowFont, rows[i].color);
            UIFont::DrawText(r, a, rows[i].desc, colX + keyW + 9.0f * scale, rowY, rowFont,
                             glm::vec3(0.86f, 0.90f, 0.94f));
            rowY -= rowStep;
        }
    };

    drawColumn(x + 12.0f * scale, leftRows, static_cast<int>(sizeof(leftRows) / sizeof(leftRows[0])));
    drawColumn(x + colW + 6.0f * scale, rightRows, static_cast<int>(sizeof(rightRows) / sizeof(rightRows[0])));
}

void Hud::drawHelpOverlay(Renderer& r, const Assets& a, int width, int height, float scale) const
{
    const float sw = static_cast<float>(width);
    const float sh = static_cast<float>(height);

    Rect(r, a, 0.0f, 0.0f, sw, sh, glm::vec3(0.0f), 0.55f);

    const float panelW = 900.0f * scale;
    const float panelH = 620.0f * scale;
    const float x = (sw - panelW) * 0.5f;
    const float y = (sh - panelH) * 0.5f;

    Rect(r, a, x, y, panelW, panelH, glm::vec3(0.04f, 0.07f, 0.10f), 0.97f);
    Rect(r, a, x, y + panelH - 76.0f * scale, panelW, 76.0f * scale,
         glm::vec3(0.00f, 0.40f, 0.55f), 1.0f);
    Rect(r, a, x, y, panelW, 3.0f * scale, glm::vec3(1.0f, 0.82f, 0.22f), 1.0f);

    const float titlePx = UIFont::MenuItemSize(scale) * 1.12f;
    UIFont::DrawText(r, a, "KOKPAR: WOBBLY STEPPE  -  CONTROLS & MANUAL",
                     x + 20.0f * scale, y + panelH - 22.0f * scale - titlePx, titlePx,
                     glm::vec3(1.0f, 0.94f, 0.80f));

    UIFont::DrawText(r, a, "Astana IT University  |  Computer Graphics Final Project  |  F1 / H to close",
                     x + 20.0f * scale, y + panelH - 60.0f * scale - UIFont::SmallSize(scale) * 0.85f,
                     UIFont::SmallSize(scale) * 0.85f, glm::vec3(0.70f, 0.84f, 0.92f));

    const char* leftBody =
        "GAMEPLAY\n"
        "W / S ............ MOVE FORWARD / BACK\n"
        "A / D ............ STEER LEFT / RIGHT\n"
        "SHIFT ............ SPRINT GALLOP\n"
        "SPACE ............ QUICK BRAKE\n"
        "E or LMB ......... PICK UP / STEAL / THROW\n"
        "MOUSE ............ ORBIT CAMERA\n"
        "SCROLL ........... ZOOM CAMERA\n"
        "TAB .............. FREE / LOCK MOUSE\n"
        "C ................ CYCLE CAMERA MODE\n"
        "Z / V ............ RESET CAMERA\n"
        "R ................ RESTART FFA MATCH\n"
        "ESC .............. PAUSE MENU\n"
        "F6 ............... QUICK LIGHTING\n"
        "1 - 7 ............ LIGHTING PRESETS";

    const char* rightBody =
        "GRAPHICS & DIAGNOSTICS\n"
        "F1 / H ........... TOGGLE THIS HELP\n"
        "F2 ............... FPS DIAGNOSTICS\n"
        "F3 ............... TOON SHADING ON/OFF\n"
        "F4 ............... FREE-FLY CAMERA\n"
        "  + MOUSE ........ LOOK AROUND\n"
        "  + WASD ......... FLY\n"
        "  + Q / E ........ DOWN / UP\n"
        "  + SHIFT ........ SPEED BOOST\n"
        "P ................ PAUSE ANIMATION\n"
        "I / K ............ SUN ELEVATION\n"
        "J / L ............ SUN AZIMUTH\n"
        "[ / ] ............ SUN AZIMUTH ALT\n"
        "- / = ............ SUN ELEVATION ALT\n"
        "RENDERER ......... DEPTH TEST + CULLING";

    const float bodyPx = UIFont::DiagnosticsSize(scale) * 1.03f;
    const float lineStep = bodyPx * 1.38f;
    const float topY = y + panelH - 96.0f * scale;
    const float colX = x + 26.0f * scale;

    auto drawColumn = [&](float cx, const char* body) {
        const char* cursor = body;
        float lineY = topY;
        std::string line;
        while (*cursor)
        {
            if (*cursor == '\n')
            {
                const bool header = (line == "GAMEPLAY" || line == "GRAPHICS & DIAGNOSTICS");
                UIFont::DrawText(r, a, line.c_str(), cx, lineY, bodyPx,
                                 header ? glm::vec3(1.0f, 0.84f, 0.28f)
                                        : glm::vec3(0.90f, 0.93f, 0.96f));
                if (header)
                {
                    Rect(r, a, cx, lineY - 4.0f * scale,
                         UIFont::MeasureWidth(line.c_str(), bodyPx), 1.5f * scale,
                         glm::vec3(1.0f, 0.84f, 0.28f), 0.75f);
                }
                line.clear();
                lineY -= lineStep;
                ++cursor;
                continue;
            }
            line.push_back(*cursor);
            ++cursor;
        }
        if (!line.empty())
        {
            UIFont::DrawText(r, a, line.c_str(), cx, lineY, bodyPx, glm::vec3(0.90f, 0.93f, 0.96f));
        }
    };

    drawColumn(colX, leftBody);
    drawColumn(x + panelW * 0.52f, rightBody);

    UIFont::DrawTextCentered(r, a, "PRESS F1 OR H TO CLOSE THIS MANUAL",
                             x + panelW * 0.5f, y + 20.0f * scale,
                             UIFont::SmallSize(scale), glm::vec3(0.30f, 0.90f, 0.65f));
}

void Hud::drawDiagnostics(Renderer& r, const Assets& a, float x, float y, float scale,
                          const Stats& stats) const
{
    const float w = 370.0f * scale;
    const float h = 268.0f * scale;

    Rect(r, a, x, y, w, h, glm::vec3(0.03f, 0.05f, 0.09f), 0.90f);
    Rect(r, a, x, y + h - 30.0f * scale, w, 30.0f * scale,
         glm::vec3(0.08f, 0.26f, 0.40f), 0.98f);
    Rect(r, a, x, y + h - 3.0f * scale, w, 3.0f * scale,
         glm::vec3(0.00f, 0.75f, 0.92f), 0.95f);

    const float headPx = UIFont::HudSize(scale) * 0.82f;
    UIFont::DrawText(r, a, "PERFORMANCE / DIAGNOSTICS",
                     x + 12.0f * scale, y + h - 10.0f * scale - headPx,
                     headPx, glm::vec3(1.0f, 0.96f, 0.85f));

    const float linePx = UIFont::DiagnosticsSize(scale) * 0.90f;
    const float step = 22.0f * scale;
    float lineY = y + h - 46.0f * scale;

    char buf[128];
    auto line = [&](const char* text, const glm::vec3& col) {
        UIFont::DrawText(r, a, text, x + 14.0f * scale, lineY, linePx, col);
        lineY -= step;
    };

    std::snprintf(buf, sizeof(buf), "FPS: %.1f   AVG: %.1f", static_cast<double>(stats.fps),
                  static_cast<double>(stats.avgFps));
    line(buf, glm::vec3(0.45f, 0.96f, 0.60f));

    std::snprintf(buf, sizeof(buf), "FRAME TIME: %.2f MS", static_cast<double>(stats.frameTimeMs));
    line(buf, glm::vec3(0.80f, 0.92f, 0.84f));

    std::snprintf(buf, sizeof(buf), "DRAW CALLS: %d", stats.drawCalls);
    line(buf, glm::vec3(0.80f, 0.92f, 0.84f));

    std::snprintf(buf, sizeof(buf), "TOON SHADING: %s  (F3)", stats.toonEnabled ? "ON" : "OFF");
    line(buf, stats.toonEnabled ? glm::vec3(1.0f, 0.85f, 0.30f) : glm::vec3(0.92f, 0.55f, 0.45f));

    std::snprintf(buf, sizeof(buf), "CAMERA: %s  (F4)", stats.freeCam ? "FREE FLY" : "FOLLOW");
    line(buf, glm::vec3(0.50f, 0.78f, 1.00f));

    std::snprintf(buf, sizeof(buf), "SIMULATION: %s  (P)", stats.simPaused ? "PAUSED" : "RUNNING");
    line(buf, stats.simPaused ? glm::vec3(1.0f, 0.72f, 0.30f) : glm::vec3(0.55f, 0.92f, 0.70f));

    std::snprintf(buf, sizeof(buf), "SUN: ELEV %.0f  AZIM %.0f",
                  static_cast<double>(stats.sunElevationDeg),
                  static_cast<double>(stats.sunAzimuthDeg));
    line(buf, glm::vec3(1.0f, 0.88f, 0.55f));

    line("OPENGL 3.3 CORE | DEPTH + CULL ON", glm::vec3(0.70f, 0.82f, 0.90f));
    line("F1 = FULL CONTROLS MANUAL", glm::vec3(0.55f, 0.78f, 0.88f));
}

void Hud::drawStatusBadges(Renderer& r, const Assets& a, int width, int height, float scale,
                           const Stats& stats) const
{
    const float badgeH = 26.0f * scale;
    const float fontPx = UIFont::SmallSize(scale) * 0.95f;
    const float y = static_cast<float>(height) - 16.0f * scale - badgeH;
    float rightX = static_cast<float>(width) - 16.0f * scale;

    auto badge = [&](const char* text, const glm::vec3& bg, const glm::vec3& fg) {
        const float textW = UIFont::MeasureWidth(text, fontPx);
        const float w = textW + 18.0f * scale;
        const float bx = rightX - w;
        Rect(r, a, bx, y, w, badgeH, bg, 0.92f);
        Rect(r, a, bx, y, w, 1.5f * scale, fg, 0.85f);
        UIFont::DrawText(r, a, text, bx + 9.0f * scale, y + (badgeH - fontPx) * 0.5f, fontPx, fg);
        rightX = bx - 8.0f * scale;
    };

    char toon[48];
    std::snprintf(toon, sizeof(toon), "TOON %s  (F3)", stats.toonEnabled ? "ON" : "OFF");
    badge(toon,
          stats.toonEnabled ? glm::vec3(0.10f, 0.40f, 0.22f) : glm::vec3(0.32f, 0.18f, 0.18f),
          stats.toonEnabled ? glm::vec3(0.65f, 1.00f, 0.72f) : glm::vec3(1.00f, 0.75f, 0.68f));

    if (stats.freeCam)
    {
        badge("FREE CAM  (F4)", glm::vec3(0.06f, 0.32f, 0.55f), glm::vec3(0.68f, 0.92f, 1.00f));
    }
    if (stats.simPaused)
    {
        badge("PAUSED  (P)", glm::vec3(0.55f, 0.36f, 0.04f), glm::vec3(1.00f, 0.94f, 0.30f));
    }
}

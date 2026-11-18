#include "Menu.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

#include "Math.h"
#include "UIFont.h"

namespace
{
// Traditional Kazakh palette
const glm::vec3 kDarkEmerald(0.035f, 0.105f, 0.075f);
const glm::vec3 kDeepEmerald(0.020f, 0.065f, 0.045f);
const glm::vec3 kGold(1.00f, 0.82f, 0.22f);
const glm::vec3 kGoldDim(0.82f, 0.65f, 0.18f);
const glm::vec3 kGoldBright(1.00f, 0.92f, 0.60f);
const glm::vec3 kBronze(0.68f, 0.44f, 0.18f);
const glm::vec3 kBronzeDark(0.38f, 0.24f, 0.10f);
const glm::vec3 kKazakhBlue(0.00f, 0.68f, 0.85f);
const glm::vec3 kTextLight(0.93f, 0.94f, 0.92f);

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

// Draw a bordered button rectangle (shared visual language for all menus).
void ButtonFrame(Renderer& r, const Assets& a, float x, float y, float w, float h,
                 bool selected, float scale)
{
    const glm::vec3 bg = selected ? glm::vec3(0.09f, 0.26f, 0.17f)
                                  : glm::vec3(0.02f, 0.08f, 0.05f);
    Rect(r, a, x, y, w, h, bg, 0.94f);

    const glm::vec3 border = selected ? kGold : kBronzeDark;
    const float t = selected ? 2.0f * scale : 1.0f * scale;
    Rect(r, a, x, y, w, t, border, 0.95f);
    Rect(r, a, x, y + h - t, w, t, border, 0.95f);
    Rect(r, a, x, y, t, h, border, 0.95f);
    Rect(r, a, x + w - t, y, t, h, border, 0.95f);
}
}

void MenuSystem::init()
{
    time_ = 0.0f;
}

// Traditional Kazakh ram's horn motif (Qoshqar myiz)
void MenuSystem::drawKazakhHorn(Renderer& r, const Assets& a, float cx, float cy, float size, const glm::vec3& col, float alpha)
{
    Rect(r, a, cx - size * 0.35f, cy - size * 0.35f, size * 0.70f, size * 0.70f, col, alpha);

    Rect(r, a, cx - size * 0.85f, cy + size * 0.15f, size * 0.50f, size * 0.22f, col, alpha);
    Rect(r, a, cx - size * 0.95f, cy - size * 0.25f, size * 0.22f, size * 0.45f, col, alpha);
    Rect(r, a, cx - size * 0.75f, cy - size * 0.35f, size * 0.35f, size * 0.18f, col, alpha);

    Rect(r, a, cx + size * 0.35f, cy + size * 0.15f, size * 0.50f, size * 0.22f, col, alpha);
    Rect(r, a, cx + size * 0.73f, cy - size * 0.25f, size * 0.22f, size * 0.45f, col, alpha);
    Rect(r, a, cx + size * 0.40f, cy - size * 0.35f, size * 0.35f, size * 0.18f, col, alpha);
}

void MenuSystem::drawKazakhBorder(Renderer& r, const Assets& a, float x, float y, float w, float h,
                                  float s, const glm::vec3& gold, const glm::vec3& bronze)
{
    const float b1 = 2.5f * s;
    const float b2 = 1.0f * s;

    Rect(r, a, x, y, w, b1, bronze, 0.85f);
    Rect(r, a, x, y + h - b1, w, b1, bronze, 0.85f);
    Rect(r, a, x, y, b1, h, bronze, 0.85f);
    Rect(r, a, x + w - b1, y, b1, h, bronze, 0.85f);

    const float pad = 4.0f * s;
    Rect(r, a, x + pad, y + pad, w - pad * 2.0f, b2, gold, 0.70f);
    Rect(r, a, x + pad, y + h - pad - b2, w - pad * 2.0f, b2, gold, 0.70f);
    Rect(r, a, x + pad, y + pad, b2, h - pad * 2.0f, gold, 0.70f);
    Rect(r, a, x + w - pad - b2, y + pad, b2, h - pad * 2.0f, gold, 0.70f);

    const float dSize = 7.0f * s;
    Rect(r, a, x + pad - dSize * 0.5f, y + pad - dSize * 0.5f, dSize, dSize, gold, 0.90f);
    Rect(r, a, x + w - pad - dSize * 0.5f, y + pad - dSize * 0.5f, dSize, dSize, gold, 0.90f);
    Rect(r, a, x + pad - dSize * 0.5f, y + h - pad - dSize * 0.5f, dSize, dSize, gold, 0.90f);
    Rect(r, a, x + w - pad - dSize * 0.5f, y + h - pad - dSize * 0.5f, dSize, dSize, gold, 0.90f);
}

// ===========================================================================
// 1. MAIN MENU
// ===========================================================================
void MenuSystem::drawMainMenu(Renderer& r, const Assets& a, int width, int height,
                              int selectedIndex, double mouseX, double mouseY,
                              const LocationInfo& curLoc, const LightingManager& lighting)
{
    (void)curLoc;
    (void)lighting;
    r.beginHud(width, height);

    const float s = UIFont::GetScale(width, height);
    const float sw = static_cast<float>(width);
    const float sh = static_cast<float>(height);

    Rect(r, a, 0.0f, 0.0f, sw, sh, glm::vec3(0.01f, 0.03f, 0.02f), 0.46f);

    const float cardW = 560.0f * s;
    const float cardH = 560.0f * s;
    const float cardX = (sw - cardW) * 0.5f;
    const float cardY = (sh - cardH) * 0.5f - 10.0f * s;

    Rect(r, a, cardX, cardY, cardW, cardH, kDarkEmerald, 0.92f);
    drawKazakhBorder(r, a, cardX, cardY, cardW, cardH, s, kGold, kBronze);

    const float titlePx = UIFont::TitleSize(s);
    const float largePx = UIFont::LargeSize(s);
    const float smallPx = UIFont::SmallSize(s);
    const float centerX = cardX + cardW * 0.5f;

    drawKazakhHorn(r, a, cardX + 50.0f * s, cardY + cardH - 62.0f * s, 19.0f * s, kGold);
    drawKazakhHorn(r, a, cardX + cardW - 50.0f * s, cardY + cardH - 62.0f * s, 19.0f * s, kGold);

    UIFont::DrawTextCentered(r, a, "KÖKPAR", centerX,
                             cardY + cardH - 46.0f * s - titlePx, titlePx, kGold);
    UIFont::DrawTextCentered(r, a, "WOBBLY STEPPE", centerX,
                             cardY + cardH - 100.0f * s - largePx * 0.72f,
                             largePx * 0.72f, kKazakhBlue);
    UIFont::DrawTextCentered(r, a, "KAZAKHSTAN TRADITIONAL MULTIPLAYER", centerX,
                             cardY + cardH - 128.0f * s - smallPx * 0.9f,
                             smallPx * 0.9f, kGoldDim);

	// Menu items
    struct Btn
    {
        const char* en;
        const char* kz;
    };
    const Btn items[5] = {
        {"1. PLAY MATCH",       "ОЙНАУ"},
        {"2. SELECT LOCATION",  "ОРЫН ТАҢДАУ"},
        {"3. TIME & LIGHTING",  "УАҚЫТ ЖӘНЕ ЖАРЫҚ"},
        {"4. SETTINGS",         "БАПТАУЛАР"},
        {"5. EXIT TO DESKTOP",  "ШЫҒУ"}
    };

    const float btnW = cardW - 70.0f * s;
    const float btnH = 50.0f * s;
    const float btnX = cardX + 35.0f * s;
    const float startY = cardY + cardH - 210.0f * s;
    const float spacingY = 58.0f * s;

    const float my = sh - static_cast<float>(mouseY);
    const float mx = static_cast<float>(mouseX);

    const float enPx = UIFont::MenuItemSize(s) * 0.94f;
    const float kzPx = UIFont::SmallSize(s) * 0.90f;

    for (int i = 0; i < 5; ++i)
    {
        const float by = startY - static_cast<float>(i) * spacingY;
        const bool hovered = (mx >= btnX && mx <= btnX + btnW && my >= by && my <= by + btnH);
        const bool selected = (selectedIndex == i) || hovered;

        ButtonFrame(r, a, btnX, by, btnW, btnH, selected, s);

        if (selected)
        {
            drawKazakhHorn(r, a, btnX + 18.0f * s, by + btnH * 0.5f, 10.0f * s, kGold);
        }

        const glm::vec3 textCol = selected ? kGoldBright : kTextLight;
        UIFont::DrawText(r, a, items[i].en, btnX + 38.0f * s,
                         by + 23.0f * s, enPx, textCol);
        UIFont::DrawText(r, a, items[i].kz, btnX + 40.0f * s,
                         by + 7.0f * s, kzPx, kKazakhBlue);
    }

    UIFont::DrawTextCentered(r, a, "USE MOUSE OR ARROW KEYS + ENTER", centerX,
                             cardY + 16.0f * s, smallPx * 0.88f, kGoldDim);

    r.endHud();
}

// ===========================================================================
// 2. LOCATION SELECTION SCREEN
// ===========================================================================
void MenuSystem::drawLocationSelect(Renderer& r, const Assets& a, int width, int height,
                                    const LocationInfo& loc, int locIndex, int totalLocs,
                                    int selectedBtn, double mouseX, double mouseY)
{
    (void)locIndex;
    (void)totalLocs;
    r.beginHud(width, height);

    const float s = UIFont::GetScale(width, height);
    const float sw = static_cast<float>(width);
    const float sh = static_cast<float>(height);

    const float cardW = 760.0f * s;
    const float cardH = 640.0f * s;
    const float cardX = (sw - cardW) * 0.5f;
    const float cardY = (sh - cardH) * 0.5f;

    Rect(r, a, cardX, cardY, cardW, cardH, kDarkEmerald, 0.93f);
    drawKazakhBorder(r, a, cardX, cardY, cardW, cardH, s, kGold, kBronze);

    drawKazakhHorn(r, a, cardX + 50.0f * s, cardY + cardH - 38.0f * s, 16.0f * s, kGold);
    drawKazakhHorn(r, a, cardX + cardW - 50.0f * s, cardY + cardH - 38.0f * s, 16.0f * s, kGold);
    UIFont::DrawTextCentered(r, a, "SELECT KAZAKHSTAN LOCATION", cardX + cardW * 0.5f,
                             cardY + cardH - 22.0f * s - UIFont::LargeSize(s),
                             UIFont::LargeSize(s), kGold);

    char countBuf[48];
    std::snprintf(countBuf, sizeof(countBuf), "LOCATION %d OF 8", static_cast<int>(loc.id) + 1);
    UIFont::DrawTextCentered(r, a, countBuf, cardX + cardW * 0.5f,
                             cardY + cardH - 70.0f * s - UIFont::SmallSize(s),
                             UIFont::SmallSize(s), kKazakhBlue);

    // Location information card
    const float innerW = cardW - 60.0f * s;
    const float innerH = 390.0f * s;
    const float innerX = cardX + 30.0f * s;
    const float innerY = cardY + 160.0f * s;

    Rect(r, a, innerX, innerY, innerW, innerH, kDeepEmerald, 0.95f);
    drawKazakhBorder(r, a, innerX, innerY, innerW, innerH, s * 0.8f, kBronze, kGoldDim);

    // Palette swatches
    Rect(r, a, innerX + 14.0f * s, innerY + innerH - 20.0f * s, innerW - 28.0f * s, 9.0f * s,
         loc.groundColor, 1.0f);
    Rect(r, a, innerX + 14.0f * s, innerY + innerH - 32.0f * s, (innerW - 28.0f * s) * 0.5f, 7.0f * s,
         loc.arenaColor, 1.0f);
    Rect(r, a, innerX + 14.0f * s + (innerW - 28.0f * s) * 0.5f, innerY + innerH - 32.0f * s,
         (innerW - 28.0f * s) * 0.5f, 7.0f * s, loc.boundaryColor, 1.0f);

    const float largePx = UIFont::LargeSize(s);
    const float menuPx = UIFont::MenuItemSize(s);
    const float smallPx = UIFont::SmallSize(s);
    const float hudPx = UIFont::HudSize(s);

    UIFont::DrawText(r, a, loc.nameEn, innerX + 22.0f * s,
                     innerY + innerH - 58.0f * s - largePx, largePx, kGoldBright);
    UIFont::DrawText(r, a, loc.nameKz, innerX + 24.0f * s,
                     innerY + innerH - 104.0f * s - menuPx * 0.85f, menuPx * 0.85f, kKazakhBlue);
    UIFont::DrawText(r, a, loc.regionEn, innerX + 24.0f * s,
                     innerY + innerH - 140.0f * s - smallPx, smallPx, kGoldDim);

    const float wrapW = innerW - 48.0f * s;
    UIFont::DrawWrappedText(r, a, loc.descriptionEn, innerX + 24.0f * s,
                            innerY + innerH - 164.0f * s, wrapW, hudPx * 0.82f,
                            glm::vec3(0.92f, 0.93f, 0.92f));
    UIFont::DrawWrappedText(r, a, loc.descriptionKz, innerX + 24.0f * s,
                            innerY + 108.0f * s, wrapW, hudPx * 0.76f,
                            glm::vec3(0.78f, 0.86f, 0.94f));

    auto drawNavBtn = [&](float bx, float by, float bw, float bh, const char* label,
                          bool selected, float fontPx) {
        ButtonFrame(r, a, bx, by, bw, bh, selected, s);
        UIFont::DrawTextCentered(r, a, label, bx + bw * 0.5f,
                                 by + (bh - fontPx) * 0.5f, fontPx,
                                 selected ? kGoldBright : kTextLight);
    };

    const float my = sh - static_cast<float>(mouseY);
    const float mx = static_cast<float>(mouseX);

    const float navW = 150.0f * s;
    const float navH = 44.0f * s;
    const float navY = cardY + 92.0f * s;
    const float prevX = cardX + 30.0f * s;
    const float nextX = cardX + cardW - 30.0f * s - navW;
    const float selW = 220.0f * s;
    const float selH = 44.0f * s;
    const float selX = cardX + (cardW - selW) * 0.5f;
    const float selY = navY;

    const float navPx = UIFont::SmallSize(s) * 1.05f;
    const float btnPx = UIFont::SmallSize(s) * 1.10f;

    const bool prevHover = (mx >= prevX && mx <= prevX + navW && my >= navY && my <= navY + navH);
    const bool nextHover = (mx >= nextX && mx <= nextX + navW && my >= navY && my <= navY + navH);
    const bool selHover = (mx >= selX && mx <= selX + selW && my >= selY && my <= selY + selH);

    drawNavBtn(prevX, navY, navW, navH, "< PREV", (selectedBtn == 0) || prevHover, navPx);
    drawNavBtn(nextX, navY, navW, navH, "NEXT >", (selectedBtn == 1) || nextHover, navPx);
    drawNavBtn(selX, selY, selW, selH, "SELECT MAP", (selectedBtn == 2) || selHover, btnPx);

    const float backW = 180.0f * s;
    const float backH = 40.0f * s;
    const float backX = cardX + (cardW - backW) * 0.5f;
    const float backY = cardY + 30.0f * s;
    const bool backHover = (mx >= backX && mx <= backX + backW && my >= backY && my <= backY + backH);
    drawNavBtn(backX, backY, backW, backH, "BACK (ESC)", (selectedBtn == 3) || backHover, navPx);

    r.endHud();
}

// ===========================================================================
// 3. TIME & LIGHTING SCREEN
// ===========================================================================
void MenuSystem::drawTimeLightingMenu(Renderer& r, const Assets& a, int width, int height,
                                      const LightingManager& lighting, const LocationInfo& loc,
                                      int selectedBtn, double mouseX, double mouseY)
{
    (void)loc;
    r.beginHud(width, height);

    const float s = UIFont::GetScale(width, height);
    const float sw = static_cast<float>(width);
    const float sh = static_cast<float>(height);

    const float cardW = 760.0f * s;
    const float cardH = 620.0f * s;
    const float cardX = (sw - cardW) * 0.5f;
    const float cardY = (sh - cardH) * 0.5f;

    Rect(r, a, cardX, cardY, cardW, cardH, kDarkEmerald, 0.93f);
    drawKazakhBorder(r, a, cardX, cardY, cardW, cardH, s, kGold, kBronze);

    drawKazakhHorn(r, a, cardX + 48.0f * s, cardY + cardH - 38.0f * s, 16.0f * s, kGold);
    drawKazakhHorn(r, a, cardX + cardW - 48.0f * s, cardY + cardH - 38.0f * s, 16.0f * s, kGold);
    UIFont::DrawTextCentered(r, a, "TIME & CELESTIAL LIGHTING", cardX + cardW * 0.5f,
                             cardY + cardH - 22.0f * s - UIFont::LargeSize(s),
                             UIFont::LargeSize(s), kGold);

    const float smallPx = UIFont::SmallSize(s);
    const float hudPx = UIFont::HudSize(s);
    const float largePx = UIFont::LargeSize(s);

    // Active time display
    const float timeW = cardW - 60.0f * s;
    const float timeH = 64.0f * s;
    const float timeX = cardX + 30.0f * s;
    const float timeY = cardY + cardH - 120.0f * s;
    Rect(r, a, timeX, timeY, timeW, timeH, kDeepEmerald, 0.94f);
    drawKazakhBorder(r, a, timeX, timeY, timeW, timeH, s * 0.7f, kBronze, kGoldDim);

    std::string timeStr = "TIME: " + lighting.formattedTime();
    UIFont::DrawText(r, a, timeStr.c_str(), timeX + 22.0f * s,
                     timeY + (timeH - largePx) * 0.5f, largePx, kGoldBright);

    char angleBuf[64];
    std::snprintf(angleBuf, sizeof(angleBuf), "ELEV: %+.1f   AZIM: %.0f",
                  static_cast<double>(lighting.sunElevation()),
                  static_cast<double>(lighting.sunAzimuth()));
    UIFont::DrawText(r, a, angleBuf, timeX + timeW * 0.52f,
                     timeY + (timeH - hudPx) * 0.5f, hudPx, kKazakhBlue);

    // Preset grid
    UIFont::DrawText(r, a, "LIGHTING PRESETS:", cardX + 32.0f * s,
                     cardY + cardH - 144.0f * s, smallPx, kGold);

    const LightingPresetId presets[7] = {
        LightingPresetId::Dawn, LightingPresetId::Morning, LightingPresetId::Noon,
        LightingPresetId::Afternoon, LightingPresetId::Sunset, LightingPresetId::Night,
        LightingPresetId::Midnight
    };

    const float my = sh - static_cast<float>(mouseY);
    const float mx = static_cast<float>(mouseX);

    const float pBtnW = 210.0f * s;
    const float pBtnH = 52.0f * s;
    const float pStartX = cardX + 30.0f * s;
    const float pStartY = cardY + cardH - 210.0f * s;
    const float colStep = 225.0f * s;
    const float rowStep = 62.0f * s;

    for (int i = 0; i < 7; ++i)
    {
        const int col = i % 3;
        const int row = i / 3;
        const float bx = pStartX + static_cast<float>(col) * colStep;
        const float by = pStartY - static_cast<float>(row) * rowStep;

        const bool isCur = (lighting.currentPreset() == presets[i]);
        const bool hovered = (mx >= bx && mx <= bx + pBtnW && my >= by && my <= by + pBtnH);
        const bool selected = (selectedBtn == i) || hovered || isCur;

        ButtonFrame(r, a, bx, by, pBtnW, pBtnH, selected, s);
        if (isCur)
        {
            Rect(r, a, bx, by, pBtnW, 3.0f * s, kGold, 1.0f);
        }

        UIFont::DrawText(r, a, lighting.presetNameEn(presets[i]), bx + 12.0f * s,
                         by + 25.0f * s, smallPx * 1.02f,
                         isCur ? kGoldBright : glm::vec3(0.90f, 0.92f, 0.92f));
        UIFont::DrawText(r, a, lighting.presetNameKz(presets[i]), bx + 12.0f * s,
                         by + 9.0f * s, smallPx * 0.88f, kKazakhBlue);
    }

    // Auto cycle summary + controls
    const float autoY = cardY + 245.0f * s;
    UIFont::DrawText(r, a, "DAY / NIGHT AUTO-CYCLE:", cardX + 32.0f * s, autoY, smallPx, kGold);

    std::string cycleStr = "SPEED: " + std::string(lighting.cycleDurationName(lighting.cycleDuration()));
    UIFont::DrawText(r, a, cycleStr.c_str(), cardX + 290.0f * s, autoY, smallPx, kKazakhBlue);

    UIFont::DrawText(r, a, "LEFT / RIGHT = TIME   |   UP / DOWN = ELEVATION",
                     cardX + 32.0f * s, autoY - 26.0f * s, smallPx * 0.92f,
                     glm::vec3(0.84f, 0.88f, 0.90f));
    UIFont::DrawText(r, a, "1 - 7 = INSTANT PRESET   |   ENTER = APPLY   |   ESC = CLOSE",
                     cardX + 32.0f * s, autoY - 48.0f * s, smallPx * 0.92f,
                     glm::vec3(0.84f, 0.88f, 0.90f));

    // Bottom action buttons
    auto drawActionBtn = [&](float bx, float by, float bw, float bh, const char* label, bool active) {
        ButtonFrame(r, a, bx, by, bw, bh, active, s);
        UIFont::DrawTextCentered(r, a, label, bx + bw * 0.5f,
                                 by + (bh - smallPx * 1.08f) * 0.5f,
                                 smallPx * 1.08f, active ? kGoldBright : kTextLight);
    };

    const float btnW = 150.0f * s;
    const float btnH = 44.0f * s;
    const float btnY = cardY + 35.0f * s;

    const float applyX = cardX + 60.0f * s;
    const float resetX = cardX + (cardW - btnW) * 0.5f;
    const float backX = cardX + cardW - 60.0f * s - btnW;
    const bool applyHov = (mx >= applyX && mx <= applyX + btnW && my >= btnY && my <= btnY + btnH);
    const bool resetHov = (mx >= resetX && mx <= resetX + btnW && my >= btnY && my <= btnY + btnH);
    const bool backHov = (mx >= backX && mx <= backX + btnW && my >= btnY && my <= btnY + btnH);

    drawActionBtn(applyX, btnY, btnW, btnH, "APPLY", applyHov);
    drawActionBtn(resetX, btnY, btnW, btnH, "RESET (NOON)", resetHov);
    drawActionBtn(backX, btnY, btnW, btnH, "BACK", backHov);

    r.endHud();
}

// ===========================================================================
// 4. SETTINGS MENU
// ===========================================================================
void MenuSystem::drawSettingsMenu(Renderer& r, const Assets& a, int width, int height,
                                  const SettingsManager& settings,
                                  int selectedRow, double mouseX, double mouseY)
{
    (void)selectedRow;
    (void)mouseX;
    (void)mouseY;
    r.beginHud(width, height);

    const float s = UIFont::GetScale(width, height);
    const float sw = static_cast<float>(width);
    const float sh = static_cast<float>(height);

    const float cardW = 680.0f * s;
    const float cardH = 560.0f * s;
    const float cardX = (sw - cardW) * 0.5f;
    const float cardY = (sh - cardH) * 0.5f;

    Rect(r, a, cardX, cardY, cardW, cardH, kDarkEmerald, 0.93f);
    drawKazakhBorder(r, a, cardX, cardY, cardW, cardH, s, kGold, kBronze);

    drawKazakhHorn(r, a, cardX + 48.0f * s, cardY + cardH - 38.0f * s, 16.0f * s, kGold);
    drawKazakhHorn(r, a, cardX + cardW - 48.0f * s, cardY + cardH - 38.0f * s, 16.0f * s, kGold);
    UIFont::DrawTextCentered(r, a, "GAME SETTINGS", cardX + cardW * 0.5f,
                             cardY + cardH - 24.0f * s - UIFont::LargeSize(s),
                             UIFont::LargeSize(s), kGold);

    struct RowInfo
    {
        const char* label;
        std::string value;
    };

    char sensBuf[32];
    std::snprintf(sensBuf, sizeof(sensBuf), "%.1fX", static_cast<double>(settings.settings().mouseSensitivity));

    const RowInfo rows[] = {
        {"GRAPHICS PRESET",   settings.qualityName()},
        {"RESOLUTION",        settings.resolutionName()},
        {"DISPLAY MODE",      settings.settings().fullscreen ? "FULLSCREEN" : "WINDOWED"},
        {"VERTICAL SYNC",     settings.settings().vsync ? "ENABLED (60 FPS)" : "DISABLED"},
        {"CAMERA SENSITIVITY", sensBuf},
        {"DEFAULT TIME",      "NOON (12:00)"},
    };

    const float labelPx = UIFont::SmallSize(s) * 1.02f;
    const float valuePx = UIFont::HudSize(s) * 0.84f;
    const float startY = cardY + cardH - 112.0f * s;
    const float rowH = 58.0f * s;

    for (size_t i = 0; i < 6; ++i)
    {
        const float ry = startY - static_cast<float>(i) * rowH;
        Rect(r, a, cardX + 30.0f * s, ry - 8.0f * s, cardW - 60.0f * s, rowH - 10.0f * s,
             kDeepEmerald, 0.78f);
        Rect(r, a, cardX + 30.0f * s, ry - 8.0f * s, 3.0f * s, rowH - 10.0f * s,
             kBronze, 0.85f);

        UIFont::DrawText(r, a, rows[i].label, cardX + 44.0f * s,
                         ry + 8.0f * s, labelPx, kGold);
        UIFont::DrawTextRight(r, a, rows[i].value.c_str(), cardX + cardW - 44.0f * s,
                              ry + 6.0f * s, valuePx, glm::vec3(1.0f));
    }

    UIFont::DrawTextCentered(r, a, "SETTINGS AUTOMATICALLY SAVED TO 'settings.cfg'",
                             cardX + cardW * 0.5f, cardY + 92.0f * s,
                             UIFont::SmallSize(s) * 0.90f, kKazakhBlue);

    const float backW = 180.0f * s;
    const float backH = 44.0f * s;
    const float backX = cardX + (cardW - backW) * 0.5f;
    const float backY = cardY + 30.0f * s;
    ButtonFrame(r, a, backX, backY, backW, backH, true, s);
    UIFont::DrawTextCentered(r, a, "BACK (ESC)", backX + backW * 0.5f,
                             backY + (backH - UIFont::SmallSize(s) * 1.08f) * 0.5f,
                             UIFont::SmallSize(s) * 1.08f, kGoldBright);

    r.endHud();
}

// ===========================================================================
// 5. PAUSE MENU
// ===========================================================================
void MenuSystem::drawPauseMenu(Renderer& r, const Assets& a, int width, int height,
                               int selectedIndex, double mouseX, double mouseY)
{
    r.beginHud(width, height);

    const float s = UIFont::GetScale(width, height);
    const float sw = static_cast<float>(width);
    const float sh = static_cast<float>(height);

    Rect(r, a, 0.0f, 0.0f, sw, sh, glm::vec3(0.01f, 0.02f, 0.03f), 0.68f);

    const float cardW = 520.0f * s;
    const float cardH = 540.0f * s;
    const float cardX = (sw - cardW) * 0.5f;
    const float cardY = (sh - cardH) * 0.5f;

    Rect(r, a, cardX, cardY, cardW, cardH, kDarkEmerald, 0.95f);
    drawKazakhBorder(r, a, cardX, cardY, cardW, cardH, s, kGold, kBronze);

    drawKazakhHorn(r, a, cardX + 42.0f * s, cardY + cardH - 34.0f * s, 15.0f * s, kGold);
    drawKazakhHorn(r, a, cardX + cardW - 42.0f * s, cardY + cardH - 34.0f * s, 15.0f * s, kGold);
    UIFont::DrawTextCentered(r, a, "GAME PAUSED", cardX + cardW * 0.5f,
                             cardY + cardH - 24.0f * s - UIFont::LargeSize(s),
                             UIFont::LargeSize(s), kGold);

    const char* pItems[6] = {
        "1. RESUME MATCH",
        "2. RESTART MATCH",
        "3. CHANGE LOCATION",
        "4. TIME & LIGHTING",
        "5. SETTINGS",
        "6. EXIT TO MAIN MENU"
    };

    const float btnW = cardW - 70.0f * s;
    const float btnH = 54.0f * s;
    const float btnX = cardX + 35.0f * s;
    const float startY = cardY + cardH - 105.0f * s;
    const float spacingY = 62.0f * s;

    const float my = sh - static_cast<float>(mouseY);
    const float mx = static_cast<float>(mouseX);
    const float itemPx = UIFont::MenuItemSize(s) * 0.90f;

    for (int i = 0; i < 6; ++i)
    {
        const float by = startY - static_cast<float>(i) * spacingY;
        const bool hovered = (mx >= btnX && mx <= btnX + btnW && my >= by && my <= by + btnH);
        const bool selected = (selectedIndex == i) || hovered;

        ButtonFrame(r, a, btnX, by, btnW, btnH, selected, s);
        UIFont::DrawText(r, a, pItems[i], btnX + 26.0f * s,
                         by + (btnH - itemPx) * 0.5f, itemPx,
                         selected ? kGoldBright : kTextLight);
    }

    UIFont::DrawTextCentered(r, a, "ESC RESUMES  |  F1 OPENS HELP",
                             cardX + cardW * 0.5f, cardY + 18.0f * s,
                             UIFont::SmallSize(s) * 0.88f, kGoldDim);

    r.endHud();
}

// ===========================================================================
// 6. IN-GAME LIGHTING OVERLAY (F6)
// ===========================================================================
void MenuSystem::drawInGameLightingPanel(Renderer& r, const Assets& a, int width, int height,
                                         const LightingManager& lighting)
{
    r.beginHud(width, height);

    const float s = UIFont::GetScale(width, height);
    const float sw = static_cast<float>(width);
    const float sh = static_cast<float>(height);

    const float panelW = 660.0f * s;
    const float panelH = 96.0f * s;
    const float panelX = (sw - panelW) * 0.5f;
    const float panelY = sh - 128.0f * s;

    Rect(r, a, panelX, panelY, panelW, panelH, kDarkEmerald, 0.90f);
    drawKazakhBorder(r, a, panelX, panelY, panelW, panelH, s * 0.6f, kGold, kBronze);

    UIFont::DrawText(r, a, "QUICK LIGHTING CONTROLS (F6)", panelX + 16.0f * s,
                     panelY + panelH - 16.0f * s - UIFont::SmallSize(s),
                     UIFont::SmallSize(s), kGold);

    std::string timeStr = "TIME: " + lighting.formattedTime();
    UIFont::DrawText(r, a, timeStr.c_str(), panelX + 16.0f * s,
                     panelY + 33.0f * s, UIFont::HudSize(s), kGoldBright);

    char angleBuf[64];
    std::snprintf(angleBuf, sizeof(angleBuf), "ELEV: %+.0f   AZIM: %.0f",
                  static_cast<double>(lighting.sunElevation()),
                  static_cast<double>(lighting.sunAzimuth()));
    UIFont::DrawText(r, a, angleBuf, panelX + 250.0f * s,
                     panelY + 35.0f * s, UIFont::HudSize(s) * 0.85f, kKazakhBlue);

    UIFont::DrawText(r, a, "LEFT/RIGHT = TIME  |  UP/DOWN = ELEV  |  1-7 = PRESET  |  F6/ESC = CLOSE",
                     panelX + 16.0f * s, panelY + 12.0f * s,
                     UIFont::SmallSize(s) * 0.86f, glm::vec3(0.88f, 0.90f, 0.92f));

    r.endHud();
}

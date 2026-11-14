#include "Menu.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

#include "Math.h"

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
const glm::vec3 kRuby(0.85f, 0.22f, 0.18f);

void Rect(Renderer& r, const Assets& a, float x, float y, float w, float h, const glm::vec3& color, float alpha = 1.0f)
{
    r.drawUnlit(a.box(), Translate(x + w * 0.5f, y + h * 0.5f, 0.0f) * Scale(w, h, 1.0f), color, alpha);
}

void Disc(Renderer& r, const Assets& a, float cx, float cy, float radius, const glm::vec3& color, float alpha = 1.0f)
{
    r.drawUnlit(a.cylinder(), Translate(cx, cy, 0.0f) * RotX(kHalfPi) * Scale(radius, 0.05f, radius), color, alpha);
}

// 3x5 vector glyph bitmask for standard characters
uint16_t GetAsciiGlyph(char c)
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
    case ',':           return 0b000000000010100;
    case '(':           return 0b010100100100010;
    case ')':           return 0b010001001001010;
    case '[':           return 0b111100100100111;
    case ']':           return 0b111001001001111;
    case '<':           return 0b001010100010001;
    case '>':           return 0b100010001010100;
    case '+':           return 0b000010111010000;
    case '\'':          return 0b010010000000000;
    default:            return 0b000000000000000;
    }
}

// Cyrillic and Kazakh 3x5 glyph mapping (codepoints 0x0400 .. 0x04FF)
uint16_t GetCyrillicGlyph(uint32_t cp)
{
    // Normalize lowercase to uppercase
    if (cp >= 0x0430 && cp <= 0x044F) cp -= 0x0020;
    // Kazakh specific letters
    if (cp == 0x0493) cp = 0x0492; // ғ -> Ғ
    if (cp == 0x049B) cp = 0x049A; // қ -> Қ
    if (cp == 0x04A3) cp = 0x04A2; // ң -> Ң
    if (cp == 0x04AF) cp = 0x04AE; // ү -> Ү
    if (cp == 0x04B1) cp = 0x04B0; // ұ -> Ұ
    if (cp == 0x04BB) cp = 0x04BA; // һ -> Һ
    if (cp == 0x04D9) cp = 0x04D8; // ә -> Ә
    if (cp == 0x04E9) cp = 0x04E8; // ө -> Ө
    if (cp == 0x0456) cp = 0x0406; // і -> І

    switch (cp)
    {
    case 0x0410: return 0b111101111101101; // А
    case 0x0411: return 0b111100111101111; // Б
    case 0x0412: return 0b110101110101110; // В
    case 0x0413: return 0b111100100100100; // Г
    case 0x0492: return 0b111110100100100; // Ғ (Kazakh)
    case 0x0414: return 0b010101101111101; // Д
    case 0x0415: return 0b111100111100111; // Е
    case 0x0416: return 0b101101010101101; // Ж
    case 0x0417: return 0b111001011001111; // З
    case 0x0418: return 0b101101111101101; // И
    case 0x0419: return 0b010101111101101; // Й
    case 0x041A: return 0b101110110101101; // К
    case 0x049A: return 0b101111110101101; // Қ (Kazakh)
    case 0x041B: return 0b011101101101101; // Л
    case 0x041C: return 0b101111101101101; // М
    case 0x041D: return 0b101101111101101; // Н
    case 0x04A2: return 0b101101111101111; // Ң (Kazakh)
    case 0x041E: return 0b111101101101111; // О
    case 0x04E8: return 0b111101111101111; // Ө (Kazakh)
    case 0x041F: return 0b111101101101101; // П
    case 0x0420: return 0b111101111100100; // Р
    case 0x0421: return 0b111100100100111; // С
    case 0x0422: return 0b111010010010010; // Т
    case 0x0423: return 0b101101011001110; // У
    case 0x04B0: return 0b101111011001110; // Ұ (Kazakh)
    case 0x04AE: return 0b101101010010010; // Ү (Kazakh)
    case 0x0424: return 0b010111111010010; // Ф
    case 0x0425: return 0b101101010101101; // Х
    case 0x04BA: return 0b100100111101101; // Һ (Kazakh)
    case 0x0426: return 0b101101101111001; // Ц
    case 0x0427: return 0b101101111001001; // Ч
    case 0x0428: return 0b101101101101111; // Ш
    case 0x0429: return 0b101101101111001; // Щ
    case 0x042A: return 0b110010010010010; // Ъ
    case 0x042B: return 0b101101111101111; // Ы
    case 0x0406: return 0b111010010010111; // І (Kazakh)
    case 0x042C: return 0b100100111101111; // Ь
    case 0x042D: return 0b111001111001111; // Э
    case 0x042E: return 0b111111111101111; // Ю
    case 0x042F: return 0b111101111011101; // Я
    case 0x04D8: return 0b111101111101111; // Ә (Kazakh)
    default:     return 0b111111111111111;
    }
}

// Decode single UTF-8 codepoint
uint32_t DecodeUtf8(const char*& ptr)
{
    const unsigned char c = static_cast<unsigned char>(*ptr++);
    if (c < 0x80) return c;
    if ((c & 0xE0) == 0xC0)
    {
        const unsigned char c2 = static_cast<unsigned char>(*ptr++);
        return ((c & 0x1F) << 6) | (c2 & 0x3F);
    }
    if ((c & 0xF0) == 0xE0)
    {
        const unsigned char c2 = static_cast<unsigned char>(*ptr++);
        const unsigned char c3 = static_cast<unsigned char>(*ptr++);
        return ((c & 0x0F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
    }
    return '?';
}

void DrawTextUTF8(Renderer& r, const Assets& a, const char* str, float x, float y, float px, const glm::vec3& color, float alpha = 1.0f)
{
    float curX = x;
    while (*str)
    {
        const uint32_t cp = DecodeUtf8(str);
        if (cp == ' ')
        {
            curX += px * 4.0f;
            continue;
        }

        uint16_t glyph = 0;
        if (cp < 0x80)
        {
            glyph = GetAsciiGlyph(static_cast<char>(cp));
        }
        else if (cp >= 0x0400 && cp <= 0x04FF)
        {
            glyph = GetCyrillicGlyph(cp);
        }

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

// Calculate text width in pixels
float MeasureTextUTF8(const char* str, float px)
{
    float w = 0.0f;
    while (*str)
    {
        DecodeUtf8(str);
        w += px * 4.0f;
    }
    return w;
}
}

void MenuSystem::init()
{
    time_ = 0.0f;
}

// Traditional Kazakh ram's horn motif (Қошқар мүйіз)
void MenuSystem::drawKazakhHorn(Renderer& r, const Assets& a, float cx, float cy, float size, const glm::vec3& col, float alpha)
{
    // Central diamond
    Rect(r, a, cx - size * 0.35f, cy - size * 0.35f, size * 0.70f, size * 0.70f, col, alpha);

    // Left curved horn
    Rect(r, a, cx - size * 0.85f, cy + size * 0.15f, size * 0.50f, size * 0.22f, col, alpha);
    Rect(r, a, cx - size * 0.95f, cy - size * 0.25f, size * 0.22f, size * 0.45f, col, alpha);
    Rect(r, a, cx - size * 0.75f, cy - size * 0.35f, size * 0.35f, size * 0.18f, col, alpha);

    // Right curved horn
    Rect(r, a, cx + size * 0.35f, cy + size * 0.15f, size * 0.50f, size * 0.22f, col, alpha);
    Rect(r, a, cx + size * 0.73f, cy - size * 0.25f, size * 0.22f, size * 0.45f, col, alpha);
    Rect(r, a, cx + size * 0.40f, cy - size * 0.35f, size * 0.35f, size * 0.18f, col, alpha);
}

void MenuSystem::drawKazakhBorder(Renderer& r, const Assets& a, float x, float y, float w, float h, float s, const glm::vec3& gold, const glm::vec3& bronze)
{
    const float b1 = 2.5f * s;
    const float b2 = 1.0f * s;

    // Outer bronze border
    Rect(r, a, x, y, w, b1, bronze, 0.85f);
    Rect(r, a, x, y + h - b1, w, b1, bronze, 0.85f);
    Rect(r, a, x, y, b1, h, bronze, 0.85f);
    Rect(r, a, x + w - b1, y, b1, h, bronze, 0.85f);

    // Inner gold pinstripe
    const float pad = 4.0f * s;
    Rect(r, a, x + pad, y + pad, w - pad * 2.0f, b2, gold, 0.70f);
    Rect(r, a, x + pad, y + h - pad - b2, w - pad * 2.0f, b2, gold, 0.70f);
    Rect(r, a, x + pad, y + pad, b2, h - pad * 2.0f, gold, 0.70f);
    Rect(r, a, x + w - pad - b2, y + pad, b2, h - pad * 2.0f, gold, 0.70f);

    // Corner Kazakh diamond ornaments
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

    const float s = std::max(1.0f, static_cast<float>(height) / 720.0f);
    const float sw = static_cast<float>(width);
    const float sh = static_cast<float>(height);

    // Subtle dark gradient vignette
    Rect(r, a, 0.0f, 0.0f, sw, sh, glm::vec3(0.01f, 0.03f, 0.02f), 0.40f);

    // Main central card
    const float cardW = 390.0f * s;
    const float cardH = 460.0f * s;
    const float cardX = (sw - cardW) * 0.5f;
    const float cardY = (sh - cardH) * 0.5f - 15.0f * s;

    Rect(r, a, cardX, cardY, cardW, cardH, kDarkEmerald, 0.88f);
    drawKazakhBorder(r, a, cardX, cardY, cardW, cardH, s, kGold, kBronze);

    // Title banner: KÖKPAR: WOBBLY STEPPE
    const float titleY = cardY + cardH - 52.0f * s;
    drawKazakhHorn(r, a, cardX + 38.0f * s, titleY + 12.0f * s, 18.0f * s, kGold);
    drawKazakhHorn(r, a, cardX + cardW - 38.0f * s, titleY + 12.0f * s, 18.0f * s, kGold);

    DrawTextUTF8(r, a, "KOKPAR", cardX + 115.0f * s, titleY + 10.0f * s, 3.2f * s, kGold);
    DrawTextUTF8(r, a, "WOBBLY STEPPE", cardX + 90.0f * s, titleY - 14.0f * s, 1.8f * s, kKazakhBlue);
    DrawTextUTF8(r, a, "KAZAKHSTAN TRADITIONAL MULTIPLAYER", cardX + 45.0f * s, titleY - 30.0f * s, 1.0f * s, kGoldDim);

    // Menu buttons
    struct Btn {
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

    const float btnW = cardW - 60.0f * s;
    const float btnH = 42.0f * s;
    const float btnX = cardX + 30.0f * s;
    const float startY = cardY + cardH - 140.0f * s;
    const float spacingY = 52.0f * s;

    // Inverted mouse Y for bottom-left ortho
    const float my = sh - static_cast<float>(mouseY);
    const float mx = static_cast<float>(mouseX);

    for (int i = 0; i < 5; ++i)
    {
        const float by = startY - static_cast<float>(i) * spacingY;
        const bool hovered = (mx >= btnX && mx <= btnX + btnW && my >= by && my <= by + btnH);
        const bool selected = (selectedIndex == i) || hovered;

        const glm::vec3 bgCol = selected ? glm::vec3(0.08f, 0.24f, 0.16f) : glm::vec3(0.02f, 0.08f, 0.05f);
        Rect(r, a, btnX, by, btnW, btnH, bgCol, 0.90f);

        const glm::vec3 borderCol = selected ? kGold : kBronzeDark;
        const float bThick = selected ? 2.0f * s : 1.0f * s;
        Rect(r, a, btnX, by, btnW, bThick, borderCol, 0.9f);
        Rect(r, a, btnX, by + btnH - bThick, btnW, bThick, borderCol, 0.9f);
        Rect(r, a, btnX, by, bThick, btnH, borderCol, 0.9f);
        Rect(r, a, btnX + btnW - bThick, by, bThick, btnH, borderCol, 0.9f);

        if (selected)
        {
            // Indicator gold ornament
            drawKazakhHorn(r, a, btnX + 16.0f * s, by + btnH * 0.5f, 10.0f * s, kGold);
        }

        const glm::vec3 textCol = selected ? kGoldBright : glm::vec3(0.92f, 0.92f, 0.90f);
        DrawTextUTF8(r, a, items[i].en, btnX + 32.0f * s, by + btnH * 0.55f, 1.45f * s, textCol);
        DrawTextUTF8(r, a, items[i].kz, btnX + 34.0f * s, by + btnH * 0.20f, 1.05f * s, kKazakhBlue);
    }

    // Footer credits & prompt
    DrawTextUTF8(r, a, "USE MOUSE OR ARROW KEYS + ENTER", cardX + 68.0f * s, cardY + 16.0f * s, 1.05f * s, kGoldDim);

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

    const float s = std::max(1.0f, static_cast<float>(height) / 720.0f);
    const float sw = static_cast<float>(width);
    const float sh = static_cast<float>(height);

    const float cardW = 580.0f * s;
    const float cardH = 480.0f * s;
    const float cardX = (sw - cardW) * 0.5f;
    const float cardY = (sh - cardH) * 0.5f;

    Rect(r, a, cardX, cardY, cardW, cardH, kDarkEmerald, 0.90f);
    drawKazakhBorder(r, a, cardX, cardY, cardW, cardH, s, kGold, kBronze);

    // Header
    drawKazakhHorn(r, a, cardX + 45.0f * s, cardY + cardH - 32.0f * s, 16.0f * s, kGold);
    drawKazakhHorn(r, a, cardX + cardW - 45.0f * s, cardY + cardH - 32.0f * s, 16.0f * s, kGold);
    DrawTextUTF8(r, a, "SELECT KAZAKHSTAN LOCATION", cardX + 110.0f * s, cardY + cardH - 38.0f * s, 2.0f * s, kGold);

    // Location Counter: e.g. "LOCATION 1 OF 8"
    char countBuf[32];
    std::snprintf(countBuf, sizeof(countBuf), "LOCATION %d OF 8", static_cast<int>(loc.id) + 1);
    DrawTextUTF8(r, a, countBuf, cardX + cardW * 0.5f - 55.0f * s, cardY + cardH - 62.0f * s, 1.15f * s, kKazakhBlue);

    // Location Names Card
    const float innerW = cardW - 50.0f * s;
    const float innerH = 195.0f * s;
    const float innerX = cardX + 25.0f * s;
    const float innerY = cardY + cardH - 268.0f * s;

    Rect(r, a, innerX, innerY, innerW, innerH, kDeepEmerald, 0.92f);
    drawKazakhBorder(r, a, innerX, innerY, innerW, innerH, s * 0.8f, kBronze, kGoldDim);

    // Visual Palette Swatch / Preview Banner inside card
    Rect(r, a, innerX + 10.0f * s, innerY + innerH - 18.0f * s, innerW - 20.0f * s, 8.0f * s, loc.groundColor, 1.0f);
    Rect(r, a, innerX + 10.0f * s, innerY + innerH - 28.0f * s, (innerW - 20.0f * s) * 0.5f, 6.0f * s, loc.arenaColor, 1.0f);
    Rect(r, a, innerX + 10.0f * s + (innerW - 20.0f * s) * 0.5f, innerY + innerH - 28.0f * s, (innerW - 20.0f * s) * 0.5f, 6.0f * s, loc.boundaryColor, 1.0f);

    // English & Kazakh Title
    DrawTextUTF8(r, a, loc.nameEn, innerX + 20.0f * s, innerY + innerH - 58.0f * s, 2.3f * s, kGoldBright);
    DrawTextUTF8(r, a, loc.nameKz, innerX + 22.0f * s, innerY + innerH - 85.0f * s, 1.75f * s, kKazakhBlue);
    DrawTextUTF8(r, a, loc.regionEn, innerX + 22.0f * s, innerY + innerH - 105.0f * s, 1.15f * s, kGoldDim);

    // Description text
    DrawTextUTF8(r, a, loc.descriptionEn, innerX + 20.0f * s, innerY + 45.0f * s, 1.12f * s, glm::vec3(0.92f));
    DrawTextUTF8(r, a, loc.descriptionKz, innerX + 20.0f * s, innerY + 16.0f * s, 1.08f * s, glm::vec3(0.85f, 0.90f, 0.95f));

    // Carousel Navigation Buttons: [< PREV], [NEXT >]
    const float my = sh - static_cast<float>(mouseY);
    const float mx = static_cast<float>(mouseX);

    auto drawNavBtn = [&](float bx, float by, float bw, float bh, const char* label, bool selected) {
        const glm::vec3 bg = selected ? glm::vec3(0.12f, 0.32f, 0.20f) : glm::vec3(0.04f, 0.12f, 0.08f);
        Rect(r, a, bx, by, bw, bh, bg, 0.95f);
        Rect(r, a, bx, by, bw, 2.0f * s, selected ? kGold : kBronzeDark, 1.0f);
        Rect(r, a, bx, by + bh - 2.0f * s, bw, 2.0f * s, selected ? kGold : kBronzeDark, 1.0f);
        Rect(r, a, bx, by, 2.0f * s, bh, selected ? kGold : kBronzeDark, 1.0f);
        Rect(r, a, bx + bw - 2.0f * s, by, 2.0f * s, bh, selected ? kGold : kBronzeDark, 1.0f);
        const float tw = MeasureTextUTF8(label, 1.35f * s);
        DrawTextUTF8(r, a, label, bx + (bw - tw) * 0.5f, by + 12.0f * s, 1.35f * s, selected ? kGoldBright : glm::vec3(0.9f));
    };

    // [< PREVIOUS] button
    const float prevX = cardX + 30.0f * s;
    const float prevY = cardY + 95.0f * s;
    const float navW = 140.0f * s;
    const float navH = 38.0f * s;
    const bool prevHover = (mx >= prevX && mx <= prevX + navW && my >= prevY && my <= prevY + navH);
    drawNavBtn(prevX, prevY, navW, navH, "< PREV", (selectedBtn == 0) || prevHover);

    // [NEXT >] button
    const float nextX = cardX + cardW - 30.0f * s - navW;
    const float nextY = prevY;
    const bool nextHover = (mx >= nextX && mx <= nextX + navW && my >= nextY && my <= nextY + navH);
    drawNavBtn(nextX, nextY, navW, navH, "NEXT >", (selectedBtn == 1) || nextHover);

    // [SELECT LOCATION] button (Primary action)
    const float selW = 190.0f * s;
    const float selH = 42.0f * s;
    const float selX = cardX + (cardW - selW) * 0.5f;
    const float selY = cardY + 93.0f * s;
    const bool selHover = (mx >= selX && mx <= selX + selW && my >= selY && my <= selY + selH);
    drawNavBtn(selX, selY, selW, selH, "SELECT MAP", (selectedBtn == 2) || selHover);

    // [BACK TO MENU] button
    const float backW = 160.0f * s;
    const float backH = 34.0f * s;
    const float backX = cardX + (cardW - backW) * 0.5f;
    const float backY = cardY + 30.0f * s;
    const bool backHover = (mx >= backX && mx <= backX + backW && my >= backY && my <= backY + backH);
    drawNavBtn(backX, backY, backW, backH, "BACK (ESC)", (selectedBtn == 3) || backHover);

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

    const float s = std::max(1.0f, static_cast<float>(height) / 720.0f);
    const float sw = static_cast<float>(width);
    const float sh = static_cast<float>(height);

    const float cardW = 540.0f * s;
    const float cardH = 490.0f * s;
    const float cardX = (sw - cardW) * 0.5f;
    const float cardY = (sh - cardH) * 0.5f;

    Rect(r, a, cardX, cardY, cardW, cardH, kDarkEmerald, 0.90f);
    drawKazakhBorder(r, a, cardX, cardY, cardW, cardH, s, kGold, kBronze);

    drawKazakhHorn(r, a, cardX + 40.0f * s, cardY + cardH - 30.0f * s, 16.0f * s, kGold);
    drawKazakhHorn(r, a, cardX + cardW - 40.0f * s, cardY + cardH - 30.0f * s, 16.0f * s, kGold);
    DrawTextUTF8(r, a, "TIME & CELESTIAL LIGHTING", cardX + 110.0f * s, cardY + cardH - 36.0f * s, 1.9f * s, kGold);

    // Active Time Display Banner
    const float timeW = cardW - 60.0f * s;
    const float timeH = 50.0f * s;
    const float timeX = cardX + 30.0f * s;
    const float timeY = cardY + cardH - 105.0f * s;
    Rect(r, a, timeX, timeY, timeW, timeH, kDeepEmerald, 0.90f);
    drawKazakhBorder(r, a, timeX, timeY, timeW, timeH, s * 0.7f, kBronze, kGoldDim);

    std::string timeStr = "TIME: " + lighting.formattedTime();
    DrawTextUTF8(r, a, timeStr.c_str(), timeX + 20.0f * s, timeY + 16.0f * s, 2.2f * s, kGoldBright);

    char angleBuf[64];
    std::snprintf(angleBuf, sizeof(angleBuf), "ELEV: %+.1f  AZIM: %.0f",
                  static_cast<double>(lighting.sunElevation()),
                  static_cast<double>(lighting.sunAzimuth()));
    DrawTextUTF8(r, a, angleBuf, timeX + 240.0f * s, timeY + 18.0f * s, 1.25f * s, kKazakhBlue);

    // 7 Lighting Presets Grid
    DrawTextUTF8(r, a, "LIGHTING PRESETS:", cardX + 32.0f * s, cardY + cardH - 128.0f * s, 1.25f * s, kGold);

    const LightingPresetId presets[7] = {
        LightingPresetId::Dawn, LightingPresetId::Morning, LightingPresetId::Noon,
        LightingPresetId::Afternoon, LightingPresetId::Sunset, LightingPresetId::Night, LightingPresetId::Midnight
    };

    const float my = sh - static_cast<float>(mouseY);
    const float mx = static_cast<float>(mouseX);

    const float pBtnW = 105.0f * s;
    const float pBtnH = 34.0f * s;
    const float pStartX = cardX + 32.0f * s;
    const float pStartY = cardY + cardH - 180.0f * s;

    for (int i = 0; i < 7; ++i)
    {
        const int col = i % 4;
        const int row = i / 4;
        const float bx = pStartX + static_cast<float>(col) * (pBtnW + 16.0f * s);
        const float by = pStartY - static_cast<float>(row) * (pBtnH + 12.0f * s);

        const bool isCur = (lighting.currentPreset() == presets[i]);
        const bool hovered = (mx >= bx && mx <= bx + pBtnW && my >= by && my <= by + pBtnH);
        const bool selected = (selectedBtn == i) || hovered || isCur;

        const glm::vec3 bg = isCur ? glm::vec3(0.15f, 0.40f, 0.25f) : (selected ? glm::vec3(0.10f, 0.24f, 0.16f) : glm::vec3(0.03f, 0.08f, 0.05f));
        Rect(r, a, bx, by, pBtnW, pBtnH, bg, 0.90f);
        Rect(r, a, bx, by, pBtnW, 1.5f * s, isCur ? kGold : (selected ? kGoldDim : kBronzeDark), 1.0f);
        Rect(r, a, bx, by + pBtnH - 1.5f * s, pBtnW, 1.5f * s, isCur ? kGold : (selected ? kGoldDim : kBronzeDark), 1.0f);
        Rect(r, a, bx, by, 1.5f * s, pBtnH, isCur ? kGold : (selected ? kGoldDim : kBronzeDark), 1.0f);
        Rect(r, a, bx + pBtnW - 1.5f * s, by, 1.5f * s, pBtnH, isCur ? kGold : (selected ? kGoldDim : kBronzeDark), 1.0f);

        DrawTextUTF8(r, a, lighting.presetNameEn(presets[i]), bx + 12.0f * s, by + 18.0f * s, 1.15f * s, isCur ? kGoldBright : glm::vec3(0.9f));
        DrawTextUTF8(r, a, lighting.presetNameKz(presets[i]), bx + 12.0f * s, by + 5.0f * s, 0.95f * s, kKazakhBlue);
    }

    // Auto Cycle & Manual Controls Summary
    const float autoY = cardY + 170.0f * s;
    DrawTextUTF8(r, a, "DAY/NIGHT AUTO-CYCLE:", cardX + 32.0f * s, autoY + 22.0f * s, 1.25f * s, kGold);

    std::string cycleStr = "SPEED: " + std::string(lighting.cycleDurationName(lighting.cycleDuration()));
    DrawTextUTF8(r, a, cycleStr.c_str(), cardX + 220.0f * s, autoY + 22.0f * s, 1.25f * s, kKazakhBlue);

    DrawTextUTF8(r, a, "CONTROLS: LEFT/RIGHT = ADJUST TIME | UP/DOWN = ELEVATION", cardX + 32.0f * s, autoY - 10.0f * s, 1.05f * s, glm::vec3(0.85f));
    DrawTextUTF8(r, a, "1-7 KEYS = INSTANT PRESET | ENTER = APPLY | ESC = CLOSE", cardX + 32.0f * s, autoY - 28.0f * s, 1.05f * s, glm::vec3(0.85f));

    // Bottom Action Buttons
    auto drawActionBtn = [&](float bx, float by, float bw, float bh, const char* label, bool active) {
        Rect(r, a, bx, by, bw, bh, active ? glm::vec3(0.12f, 0.32f, 0.20f) : glm::vec3(0.04f, 0.10f, 0.06f), 0.95f);
        Rect(r, a, bx, by, bw, 2.0f * s, active ? kGold : kBronzeDark, 1.0f);
        Rect(r, a, bx, by + bh - 2.0f * s, bw, 2.0f * s, active ? kGold : kBronzeDark, 1.0f);
        Rect(r, a, bx, by, 2.0f * s, bh, active ? kGold : kBronzeDark, 1.0f);
        Rect(r, a, bx + bw - 2.0f * s, by, 2.0f * s, bh, active ? kGold : kBronzeDark, 1.0f);
        const float tw = MeasureTextUTF8(label, 1.25f * s);
        DrawTextUTF8(r, a, label, bx + (bw - tw) * 0.5f, by + 12.0f * s, 1.25f * s, active ? kGoldBright : glm::vec3(0.9f));
    };

    const float btnW = 120.0f * s;
    const float btnH = 36.0f * s;
    const float btnY = cardY + 40.0f * s;

    // Apply Button
    const float applyX = cardX + 50.0f * s;
    const bool applyHov = (mx >= applyX && mx <= applyX + btnW && my >= btnY && my <= btnY + btnH);
    drawActionBtn(applyX, btnY, btnW, btnH, "APPLY", (selectedBtn == 10) || applyHov);

    // Reset Button
    const float resetX = cardX + (cardW - btnW) * 0.5f;
    const bool resetHov = (mx >= resetX && mx <= resetX + btnW && my >= btnY && my <= btnY + btnH);
    drawActionBtn(resetX, btnY, btnW, btnH, "RESET (NOON)", (selectedBtn == 11) || resetHov);

    // Back Button
    const float backX = cardX + cardW - 50.0f * s - btnW;
    const bool backHov = (mx >= backX && mx <= backX + btnW && my >= btnY && my <= btnY + btnH);
    drawActionBtn(backX, btnY, btnW, btnH, "BACK", (selectedBtn == 12) || backHov);

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

    const float s = std::max(1.0f, static_cast<float>(height) / 720.0f);
    const float sw = static_cast<float>(width);
    const float sh = static_cast<float>(height);

    const float cardW = 520.0f * s;
    const float cardH = 460.0f * s;
    const float cardX = (sw - cardW) * 0.5f;
    const float cardY = (sh - cardH) * 0.5f;

    Rect(r, a, cardX, cardY, cardW, cardH, kDarkEmerald, 0.90f);
    drawKazakhBorder(r, a, cardX, cardY, cardW, cardH, s, kGold, kBronze);

    drawKazakhHorn(r, a, cardX + 40.0f * s, cardY + cardH - 30.0f * s, 16.0f * s, kGold);
    drawKazakhHorn(r, a, cardX + cardW - 40.0f * s, cardY + cardH - 30.0f * s, 16.0f * s, kGold);
    DrawTextUTF8(r, a, "GAME SETTINGS", cardX + 175.0f * s, cardY + cardH - 36.0f * s, 2.0f * s, kGold);

    // Settings Rows
    struct RowInfo {
        const char* label;
        std::string value;
    };

    char sensBuf[32];
    std::snprintf(sensBuf, sizeof(sensBuf), "%.1fx", static_cast<double>(settings.settings().mouseSensitivity));

    const RowInfo rows[] = {
        {"GRAPHICS PRESET:",  settings.qualityName()},
        {"RESOLUTION:",       settings.resolutionName()},
        {"DISPLAY MODE:",     settings.settings().fullscreen ? "FULLSCREEN" : "WINDOWED"},
        {"VERTICAL SYNC:",    settings.settings().vsync ? "ENABLED (60 FPS)" : "DISABLED"},
        {"CAMERA SENSITIVITY:", sensBuf},
        {"DEFAULT TIME PRESET:", "NOON (12:00)"},
    };

    const float startY = cardY + cardH - 95.0f * s;
    const float rowH = 42.0f * s;

    for (size_t i = 0; i < 6; ++i)
    {
        const float ry = startY - static_cast<float>(i) * rowH;
        Rect(r, a, cardX + 30.0f * s, ry, cardW - 60.0f * s, rowH - 6.0f * s, kDeepEmerald, 0.70f);

        DrawTextUTF8(r, a, rows[i].label, cardX + 45.0f * s, ry + 12.0f * s, 1.20f * s, kGold);
        DrawTextUTF8(r, a, rows[i].value.c_str(), cardX + 260.0f * s, ry + 12.0f * s, 1.20f * s, glm::vec3(1.0f));
    }

    DrawTextUTF8(r, a, "SETTINGS AUTOMATICALLY SAVED TO 'settings.cfg'", cardX + 65.0f * s, cardY + 75.0f * s, 1.05f * s, kKazakhBlue);

    // Back Button
    const float backW = 150.0f * s;
    const float backH = 36.0f * s;
    const float backX = cardX + (cardW - backW) * 0.5f;
    const float backY = cardY + 25.0f * s;
    Rect(r, a, backX, backY, backW, backH, glm::vec3(0.08f, 0.22f, 0.14f), 0.95f);
    Rect(r, a, backX, backY, backW, 2.0f * s, kGold, 1.0f);
    Rect(r, a, backX, backY + backH - 2.0f * s, backW, 2.0f * s, kGold, 1.0f);
    Rect(r, a, backX, backY, 2.0f * s, backH, kGold, 1.0f);
    Rect(r, a, backX + backW - 2.0f * s, backY, 2.0f * s, backH, kGold, 1.0f);
    DrawTextUTF8(r, a, "BACK (ESC)", backX + 32.0f * s, backY + 12.0f * s, 1.30f * s, kGoldBright);

    r.endHud();
}

// ===========================================================================
// 5. PAUSE MENU
// ===========================================================================
void MenuSystem::drawPauseMenu(Renderer& r, const Assets& a, int width, int height,
                               int selectedIndex, double mouseX, double mouseY)
{
    r.beginHud(width, height);

    const float s = std::max(1.0f, static_cast<float>(height) / 720.0f);
    const float sw = static_cast<float>(width);
    const float sh = static_cast<float>(height);

    // Dark semi-transparent modal backdrop
    Rect(r, a, 0.0f, 0.0f, sw, sh, glm::vec3(0.01f, 0.02f, 0.03f), 0.65f);

    const float cardW = 380.0f * s;
    const float cardH = 430.0f * s;
    const float cardX = (sw - cardW) * 0.5f;
    const float cardY = (sh - cardH) * 0.5f;

    Rect(r, a, cardX, cardY, cardW, cardH, kDarkEmerald, 0.92f);
    drawKazakhBorder(r, a, cardX, cardY, cardW, cardH, s, kGold, kBronze);

    drawKazakhHorn(r, a, cardX + 35.0f * s, cardY + cardH - 28.0f * s, 15.0f * s, kGold);
    drawKazakhHorn(r, a, cardX + cardW - 35.0f * s, cardY + cardH - 28.0f * s, 15.0f * s, kGold);
    DrawTextUTF8(r, a, "GAME PAUSED", cardX + 120.0f * s, cardY + cardH - 34.0f * s, 2.2f * s, kGold);

    const char* pItems[6] = {
        "1. RESUME MATCH",
        "2. RESTART MATCH",
        "3. CHANGE LOCATION",
        "4. TIME & LIGHTING",
        "5. SETTINGS",
        "6. EXIT TO MAIN MENU"
    };

    const float btnW = cardW - 60.0f * s;
    const float btnH = 38.0f * s;
    const float btnX = cardX + 30.0f * s;
    const float startY = cardY + cardH - 95.0f * s;
    const float spacingY = 48.0f * s;

    const float my = sh - static_cast<float>(mouseY);
    const float mx = static_cast<float>(mouseX);

    for (int i = 0; i < 6; ++i)
    {
        const float by = startY - static_cast<float>(i) * spacingY;
        const bool hovered = (mx >= btnX && mx <= btnX + btnW && my >= by && my <= by + btnH);
        const bool selected = (selectedIndex == i) || hovered;

        const glm::vec3 bgCol = selected ? glm::vec3(0.10f, 0.28f, 0.18f) : glm::vec3(0.02f, 0.08f, 0.05f);
        Rect(r, a, btnX, by, btnW, btnH, bgCol, 0.90f);

        const glm::vec3 borderCol = selected ? kGold : kBronzeDark;
        const float bThick = selected ? 2.0f * s : 1.0f * s;
        Rect(r, a, btnX, by, btnW, bThick, borderCol, 0.9f);
        Rect(r, a, btnX, by + btnH - bThick, btnW, bThick, borderCol, 0.9f);
        Rect(r, a, btnX, by, bThick, btnH, borderCol, 0.9f);
        Rect(r, a, btnX + btnW - bThick, by, bThick, btnH, borderCol, 0.9f);

        DrawTextUTF8(r, a, pItems[i], btnX + 24.0f * s, by + 12.0f * s, 1.35f * s, selected ? kGoldBright : glm::vec3(0.92f));
    }

    r.endHud();
}

// ===========================================================================
// 6. IN-GAME LIGHTING OVERLAY (F6)
// ===========================================================================
void MenuSystem::drawInGameLightingPanel(Renderer& r, const Assets& a, int width, int height,
                                         const LightingManager& lighting)
{
    r.beginHud(width, height);

    const float s = std::max(1.0f, static_cast<float>(height) / 720.0f);
    const float sw = static_cast<float>(width);

    // Floating compact HUD card at top
    const float panelW = 460.0f * s;
    const float panelH = 75.0f * s;
    const float panelX = (sw - panelW) * 0.5f;
    const float panelY = static_cast<float>(height) - 105.0f * s;

    Rect(r, a, panelX, panelY, panelW, panelH, kDarkEmerald, 0.88f);
    drawKazakhBorder(r, a, panelX, panelY, panelW, panelH, s * 0.6f, kGold, kBronze);

    DrawTextUTF8(r, a, "QUICK LIGHTING CONTROLS (F6)", panelX + 15.0f * s, panelY + panelH - 18.0f * s, 1.25f * s, kGold);

    std::string timeStr = "TIME: " + lighting.formattedTime();
    DrawTextUTF8(r, a, timeStr.c_str(), panelX + 15.0f * s, panelY + 32.0f * s, 1.40f * s, kGoldBright);

    char angleBuf[48];
    std::snprintf(angleBuf, sizeof(angleBuf), "ELEV: %+.0f  AZIM: %.0f",
                  static_cast<double>(lighting.sunElevation()),
                  static_cast<double>(lighting.sunAzimuth()));
    DrawTextUTF8(r, a, angleBuf, panelX + 180.0f * s, panelY + 33.0f * s, 1.20f * s, kKazakhBlue);

    DrawTextUTF8(r, a, "LEFT/RIGHT = TIME | UP/DOWN = ELEV | 1-7 = PRESET | ESC = CLOSE",
                 panelX + 15.0f * s, panelY + 12.0f * s, 0.95f * s, glm::vec3(0.9f));

    r.endHud();
}

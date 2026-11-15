#include "UIFont.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

#include "Assets.h"
#include "Math.h"
#include "Renderer.h"

namespace
{
void Rect(Renderer& r, const Assets& a, float x, float y, float w, float h, const glm::vec3& color, float alpha)
{
    r.drawUnlit(a.box(), Translate(x + w * 0.5f, y + h * 0.5f, 0.0f) * Scale(w, h, 1.0f), color, alpha);
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

// 7-row column bitmask struct (up to 7 columns per glyph)
// Bit 0 = Row 0 (top), Bit 6 = Row 6 (bottom).
struct GlyphData
{
    uint8_t cols[7];
    uint8_t colCount;
};

GlyphData GetGlyph(uint32_t cp)
{
    // Normalize Latin lowercase to uppercase
    if (cp >= 'a' && cp <= 'z') cp = cp - 'a' + 'A';
    // Latin-1 accented capitals used by the game title (KOKPAR with O-umlaut)
    if (cp == 0x00D6 || cp == 0x00F8) cp = 'O'; // O-umlaut / o-slash
    if (cp == 0x00DC) cp = 'U';                 // U-umlaut
    if (cp == 0x00C4 || cp == 0x00C5) cp = 'A'; // A-umlaut / A-ring
    if (cp == 0x00DF) cp = 'S';                 // sharp s
    // Normalize Cyrillic lowercase to uppercase
    if (cp >= 0x0430 && cp <= 0x044F) cp -= 0x0020;
    if (cp == 0x0451) cp = 0x0401; // ё -> Ё
    if (cp == 0x04D9) cp = 0x04D8; // ә -> Ә
    if (cp == 0x0493) cp = 0x0492; // ғ -> Ғ
    if (cp == 0x049B) cp = 0x049A; // қ -> Қ
    if (cp == 0x04A3) cp = 0x04A2; // ң -> Ң
    if (cp == 0x04E9) cp = 0x04E8; // ө -> Ө
    if (cp == 0x04B1) cp = 0x04B0; // ұ -> Ұ
    if (cp == 0x04AF) cp = 0x04AE; // ү -> Ү
    if (cp == 0x04BB) cp = 0x04BA; // һ -> Һ
    if (cp == 0x0456) cp = 0x0406; // і -> І

    // Standard ASCII mappings
    switch (cp)
    {
    case ' ':  return {{0, 0, 0}, 3};
    case '!':  return {{0x5F}, 1};
    case '"':  return {{0x07, 0x00, 0x07}, 3};
    case '#':  return {{0x24, 0x7E, 0x24, 0x7E, 0x24}, 5};
    case '$':  return {{0x24, 0x2A, 0x7F, 0x2A, 0x12}, 5};
    case '%':  return {{0x23, 0x13, 0x08, 0x64, 0x62}, 5};
    case '&':  return {{0x36, 0x49, 0x55, 0x22, 0x50}, 5};
    case '\'': return {{0x07}, 1};
    case '(':  return {{0x1C, 0x22, 0x41}, 3};
    case ')':  return {{0x41, 0x22, 0x1C}, 3};
    case '*':  return {{0x14, 0x08, 0x3E, 0x08, 0x14}, 5};
    case '+':  return {{0x08, 0x08, 0x3E, 0x08, 0x08}, 5};
    case ',':  return {{0x50, 0x30}, 2};
    case '-':  return {{0x08, 0x08, 0x08, 0x08, 0x08}, 5};
    case '.':  return {{0x60, 0x60}, 2};
    case '/':  return {{0x60, 0x1C, 0x07}, 3};
    case '0':  return {{0x3E, 0x51, 0x49, 0x45, 0x3E}, 5};
    case '1':  return {{0x42, 0x7F, 0x40}, 3};
    case '2':  return {{0x42, 0x61, 0x51, 0x49, 0x46}, 5};
    case '3':  return {{0x22, 0x41, 0x49, 0x49, 0x36}, 5};
    case '4':  return {{0x18, 0x14, 0x12, 0x7F, 0x10}, 5};
    case '5':  return {{0x27, 0x45, 0x45, 0x45, 0x39}, 5};
    case '6':  return {{0x3E, 0x49, 0x49, 0x49, 0x32}, 5};
    case '7':  return {{0x01, 0x71, 0x09, 0x05, 0x03}, 5};
    case '8':  return {{0x36, 0x49, 0x49, 0x49, 0x36}, 5};
    case '9':  return {{0x26, 0x49, 0x49, 0x49, 0x3E}, 5};
    case ':':  return {{0x36, 0x36}, 2};
    case ';':  return {{0x56, 0x36}, 2};
    case '<':  return {{0x08, 0x14, 0x22, 0x41}, 4};
    case '=':  return {{0x14, 0x14, 0x14, 0x14, 0x14}, 5};
    case '>':  return {{0x41, 0x22, 0x14, 0x08}, 4};
    case '?':  return {{0x02, 0x01, 0x51, 0x09, 0x06}, 5};
    case '@':  return {{0x3E, 0x41, 0x5D, 0x55, 0x1E}, 5};

    case 'A':  return {{0x7E, 0x11, 0x11, 0x11, 0x7E}, 5};
    case 'B':  return {{0x7F, 0x49, 0x49, 0x49, 0x36}, 5};
    case 'C':  return {{0x3E, 0x41, 0x41, 0x41, 0x22}, 5};
    case 'D':  return {{0x7F, 0x41, 0x41, 0x22, 0x1C}, 5};
    case 'E':  return {{0x7F, 0x49, 0x49, 0x49, 0x41}, 5};
    case 'F':  return {{0x7F, 0x09, 0x09, 0x09, 0x01}, 5};
    case 'G':  return {{0x3E, 0x41, 0x49, 0x49, 0x7A}, 5};
    case 'H':  return {{0x7F, 0x08, 0x08, 0x08, 0x7F}, 5};
    case 'I':  return {{0x41, 0x7F, 0x41}, 3};
    case 'J':  return {{0x20, 0x40, 0x41, 0x3F, 0x01}, 5};
    case 'K':  return {{0x7F, 0x08, 0x14, 0x22, 0x41}, 5};
    case 'L':  return {{0x7F, 0x40, 0x40, 0x40, 0x40}, 5};
    case 'M':  return {{0x7F, 0x02, 0x0C, 0x02, 0x7F}, 5};
    case 'N':  return {{0x7F, 0x04, 0x08, 0x10, 0x7F}, 5};
    case 'O':  return {{0x3E, 0x41, 0x41, 0x41, 0x3E}, 5};
    case 'P':  return {{0x7F, 0x09, 0x09, 0x09, 0x06}, 5};
    case 'Q':  return {{0x3E, 0x41, 0x51, 0x21, 0x5E}, 5};
    case 'R':  return {{0x7F, 0x09, 0x19, 0x29, 0x46}, 5};
    case 'S':  return {{0x26, 0x49, 0x49, 0x49, 0x32}, 5};
    case 'T':  return {{0x01, 0x01, 0x7F, 0x01, 0x01}, 5};
    case 'U':  return {{0x3F, 0x40, 0x40, 0x40, 0x3F}, 5};
    case 'V':  return {{0x1F, 0x20, 0x40, 0x20, 0x1F}, 5};
    case 'W':  return {{0x7F, 0x20, 0x18, 0x20, 0x7F}, 5};
    case 'X':  return {{0x63, 0x14, 0x08, 0x14, 0x63}, 5};
    case 'Y':  return {{0x07, 0x08, 0x70, 0x08, 0x07}, 5};
    case 'Z':  return {{0x61, 0x51, 0x49, 0x45, 0x43}, 5};
    case '[':  return {{0x7F, 0x41, 0x41}, 3};
    case ']':  return {{0x41, 0x41, 0x7F}, 3};
    case '|':  return {{0x7F}, 1};
    case '_':  return {{0x40, 0x40, 0x40, 0x40, 0x40}, 5};

    // Cyrillic & Kazakh alphabet mappings
    case 0x0410: return {{0x7E, 0x11, 0x11, 0x11, 0x7E}, 5}; // А
    case 0x0411: return {{0x7F, 0x49, 0x49, 0x49, 0x31}, 5}; // Б
    case 0x0412: return {{0x7F, 0x49, 0x49, 0x49, 0x36}, 5}; // В
    case 0x0413: return {{0x7F, 0x01, 0x01, 0x01, 0x01}, 5}; // Г
    case 0x0492: return {{0x7F, 0x09, 0x09, 0x01, 0x01}, 5}; // Ғ (Kazakh)
    case 0x0414: return {{0x60, 0x3F, 0x21, 0x21, 0x3F, 0x60}, 6}; // Д
    case 0x0415: return {{0x7F, 0x49, 0x49, 0x49, 0x41}, 5}; // Е
    case 0x0401: return {{0x7D, 0x49, 0x49, 0x49, 0x4D}, 5}; // Ё
    case 0x0416: return {{0x77, 0x08, 0x7F, 0x08, 0x77}, 5}; // Ж
    case 0x0417: return {{0x22, 0x41, 0x49, 0x49, 0x36}, 5}; // З
    case 0x0418: return {{0x7F, 0x10, 0x08, 0x04, 0x7F}, 5}; // И
    case 0x0419: return {{0x7D, 0x11, 0x09, 0x05, 0x7D}, 5}; // Й
    case 0x041A: return {{0x7F, 0x08, 0x14, 0x22, 0x41}, 5}; // К
    case 0x049A: return {{0x7F, 0x08, 0x14, 0x22, 0x71}, 5}; // Қ (Kazakh)
    case 0x041B: return {{0x40, 0x3E, 0x01, 0x01, 0x7F}, 5}; // Л
    case 0x041C: return {{0x7F, 0x02, 0x0C, 0x02, 0x7F}, 5}; // М
    case 0x041D: return {{0x7F, 0x08, 0x08, 0x08, 0x7F}, 5}; // Н
    case 0x04A2: return {{0x7F, 0x08, 0x08, 0x08, 0x7F, 0x40}, 6}; // Ң (Kazakh)
    case 0x041E: return {{0x3E, 0x41, 0x41, 0x41, 0x3E}, 5}; // О
    case 0x04E8: return {{0x3E, 0x49, 0x49, 0x49, 0x3E}, 5}; // Ө (Kazakh)
    case 0x041F: return {{0x7F, 0x01, 0x01, 0x01, 0x7F}, 5}; // П
    case 0x0420: return {{0x7F, 0x09, 0x09, 0x09, 0x06}, 5}; // Р
    case 0x0421: return {{0x3E, 0x41, 0x41, 0x41, 0x22}, 5}; // С
    case 0x0422: return {{0x01, 0x01, 0x7F, 0x01, 0x01}, 5}; // Т
    case 0x0423: return {{0x07, 0x08, 0x70, 0x08, 0x07}, 5}; // У
    case 0x04B0: return {{0x07, 0x08, 0x78, 0x08, 0x07}, 5}; // Ұ (Kazakh)
    case 0x04AE: return {{0x03, 0x04, 0x78, 0x04, 0x03}, 5}; // Ү (Kazakh)
    case 0x0424: return {{0x1C, 0x22, 0x7F, 0x22, 0x1C}, 5}; // Ф
    case 0x0425: return {{0x63, 0x14, 0x08, 0x14, 0x63}, 5}; // Х
    case 0x04BA: return {{0x7F, 0x08, 0x08, 0x08, 0x70}, 5}; // Һ (Kazakh)
    case 0x0426: return {{0x7F, 0x40, 0x40, 0x7F, 0x60}, 5}; // Ц
    case 0x0427: return {{0x07, 0x08, 0x08, 0x08, 0x7F}, 5}; // Ч
    case 0x0428: return {{0x7F, 0x40, 0x7F, 0x40, 0x7F}, 5}; // Ш
    case 0x0429: return {{0x7F, 0x40, 0x7F, 0x40, 0x7F, 0x60}, 6}; // Щ
    case 0x042A: return {{0x01, 0x7F, 0x48, 0x48, 0x30}, 5}; // Ъ
    case 0x042B: return {{0x7F, 0x48, 0x48, 0x30, 0x00, 0x7F}, 6}; // Ы
    case 0x0406: return {{0x41, 0x7F, 0x41}, 3}; // І (Kazakh)
    case 0x042C: return {{0x7F, 0x48, 0x48, 0x48, 0x30}, 5}; // Ь
    case 0x042D: return {{0x22, 0x41, 0x49, 0x41, 0x3E}, 5}; // Э
    case 0x042E: return {{0x7F, 0x08, 0x3E, 0x41, 0x3E}, 5}; // Ю
    case 0x042F: return {{0x46, 0x29, 0x19, 0x09, 0x7F}, 5}; // Я
    case 0x04D8: return {{0x3C, 0x4A, 0x49, 0x49, 0x7A}, 5}; // Ә (Kazakh)

    default:   return {{0x7F, 0x41, 0x41, 0x41, 0x7F}, 5}; // Box placeholder
    }
}
}

namespace UIFont
{
float GetScale(int /*width*/, int height)
{
    // 720p is the minimum comfortable reference (scale 1.0); 1080p = 1.5.
    // Clamped so 4K screens do not produce absurdly large HUD text.
    const float s = static_cast<float>(height) / 720.0f;
    return std::clamp(s, 1.0f, 2.0f);
}

// Height in pixels of a full glyph cell (7 rows). These are deliberately larger
// than the old 1.25 px HUD cells: 20 px HUD text is legible at 1280x720.
float TitleSize(float scale)       { return 42.0f * scale; }
float LargeSize(float scale)       { return 30.0f * scale; }
float MenuItemSize(float scale)    { return 25.0f * scale; }
float HudSize(float scale)         { return 20.0f * scale; }
float DiagnosticsSize(float scale) { return 17.0f * scale; }
float SmallSize(float scale)       { return 15.0f * scale; }

float MeasureWidth(const char* text, float fontHeightPx)
{
    if (!text || !*text) return 0.0f;
    const float cellW = (fontHeightPx / 7.0f) * 0.88f;
    const float charGap = cellW * 0.95f;
    float totalW = 0.0f;
    float lineW = 0.0f;

    const char* ptr = text;
    while (*ptr)
    {
        if (*ptr == '\n')
        {
            // Drop the trailing inter-character gap before comparing line widths.
            lineW = std::max(0.0f, lineW - charGap);
            if (lineW > totalW) totalW = lineW;
            lineW = 0.0f;
            ptr++;
            continue;
        }

        const uint32_t cp = DecodeUtf8(ptr);
        const GlyphData g = GetGlyph(cp);
        lineW += static_cast<float>(g.colCount) * cellW + charGap;
    }

    lineW = std::max(0.0f, lineW - charGap);
    if (lineW > totalW) totalW = lineW;
    return totalW;
}

float MeasureWidth(const std::string& text, float fontHeightPx)
{
    return MeasureWidth(text.c_str(), fontHeightPx);
}

float MeasureHeight(const char* text, float fontHeightPx)
{
    if (!text || !*text) return 0.0f;
    int lines = 1;
    for (const char* p = text; *p; ++p)
    {
        if (*p == '\n') ++lines;
    }
    // Glyph height plus the gap between consecutive lines.
    return fontHeightPx + static_cast<float>(lines - 1) * fontHeightPx * 1.35f;
}

float MeasureHeight(const std::string& text, float fontHeightPx)
{
    return MeasureHeight(text.c_str(), fontHeightPx);
}

void DrawText(Renderer& r, const Assets& a, const char* text,
              float x, float y, float fontHeightPx,
              const glm::vec3& color, float alpha,
              bool withDarkBacking)
{
    if (!text || !*text) return;

    const float H = fontHeightPx;
    const float cellH = H / 7.0f;
    const float cellW = cellH * 0.88f;
    const float charGap = cellW * 0.95f;
    const float lineSpacing = H * 1.35f;

    // Optional dark backdrop for ultra-sharp legibility over bright landscapes
    if (withDarkBacking)
    {
        const float textW = MeasureWidth(text, fontHeightPx);
        // Count lines
        int lineCount = 1;
        for (const char* p = text; *p; ++p) if (*p == '\n') lineCount++;
        const float blockH = MeasureHeight(text, fontHeightPx);
        const float padX = cellW * 2.0f;
        const float padY = cellH * 1.5f;

        // y is the bottom of the last (lowest) line; the block grows upwards.
        Rect(r, a, x - padX, y - padY,
             textW + padX * 2.0f, blockH + padY * 2.0f,
             glm::vec3(0.03f, 0.05f, 0.08f), 0.78f * alpha);
        (void)lineCount;
    }

    float curX = x;
    float curY = y;
    const char* ptr = text;

    while (*ptr)
    {
        if (*ptr == '\n')
        {
            curX = x;
            curY -= lineSpacing;
            ptr++;
            continue;
        }

        const uint32_t cp = DecodeUtf8(ptr);
        const GlyphData g = GetGlyph(cp);

        // Render solid vertical stroke bars by merging contiguous set bits
        for (int c = 0; c < g.colCount; ++c)
        {
            const uint8_t bits = g.cols[c];
            if (bits == 0) continue;

            const float colX = curX + static_cast<float>(c) * cellW;
            int rStart = -1;

            for (int row = 0; row < 7; ++row)
            {
                const bool set = (bits & (1 << row)) != 0;
                if (set && rStart < 0)
                {
                    rStart = row;
                }
                else if (!set && rStart >= 0)
                {
                    const float barH = static_cast<float>(row - rStart) * cellH;
                    const float barY = curY + static_cast<float>(6 - (row - 1)) * cellH;
                    Rect(r, a, colX, barY, cellW, barH, color, alpha);
                    rStart = -1;
                }
            }
            if (rStart >= 0)
            {
                const float barH = static_cast<float>(7 - rStart) * cellH;
                const float barY = curY;
                Rect(r, a, colX, barY, cellW, barH, color, alpha);
            }
        }

        curX += static_cast<float>(g.colCount) * cellW + charGap;
    }
}

void DrawText(Renderer& r, const Assets& a, const std::string& text,
              float x, float y, float fontHeightPx,
              const glm::vec3& color, float alpha,
              bool withDarkBacking)
{
    DrawText(r, a, text.c_str(), x, y, fontHeightPx, color, alpha, withDarkBacking);
}

void DrawTextCentered(Renderer& r, const Assets& a, const char* text,
                      float centerX, float y, float fontHeightPx,
                      const glm::vec3& color, float alpha,
                      bool withDarkBacking)
{
    const float w = MeasureWidth(text, fontHeightPx);
    DrawText(r, a, text, centerX - w * 0.5f, y, fontHeightPx, color, alpha, withDarkBacking);
}

void DrawTextRight(Renderer& r, const Assets& a, const char* text,
                   float rightX, float y, float fontHeightPx,
                   const glm::vec3& color, float alpha,
                   bool withDarkBacking)
{
    const float w = MeasureWidth(text, fontHeightPx);
    DrawText(r, a, text, rightX - w, y, fontHeightPx, color, alpha, withDarkBacking);
}

namespace
{
// Extract the next word; stops before spaces / newlines.
bool NextWord(const char*& cursor, std::string& outWord)
{
    while (*cursor == ' ') ++cursor;
    if (*cursor == '\0' || *cursor == '\n') return false;

    const char* start = cursor;
    while (*cursor != '\0' && *cursor != ' ' && *cursor != '\n') ++cursor;
    outWord.assign(start, static_cast<size_t>(cursor - start));
    return !outWord.empty();
}
}

float DrawWrappedText(Renderer& r, const Assets& a, const char* text,
                      float x, float topY, float maxWidth, float fontHeightPx,
                      const glm::vec3& color, float alpha,
                      bool withDarkBacking)
{
    if (!text || !*text) return 0.0f;

    const float lineAdvance = fontHeightPx * 1.38f;
    std::string line;
    float lineY = topY - fontHeightPx;   // bottom of the first line
    float usedHeight = fontHeightPx;

    const char* cursor = text;
    std::string word;

    auto emitLine = [&]() {
        if (line.empty()) return;
        DrawText(r, a, line.c_str(), x, lineY, fontHeightPx, color, alpha, withDarkBacking);
        lineY -= lineAdvance;
        usedHeight += lineAdvance;
        line.clear();
    };

    while (*cursor)
    {
        if (*cursor == '\n')
        {
            emitLine();
            ++cursor;
            continue;
        }
        if (*cursor == ' ')
        {
            ++cursor;
            continue;
        }
        if (!NextWord(cursor, word)) break;

        const std::string candidate = line.empty() ? word : (line + " " + word);
        if (MeasureWidth(candidate, fontHeightPx) <= maxWidth || line.empty())
        {
            line = candidate;
        }
        else
        {
            emitLine();
            line = word;
        }
    }
    emitLine();

    return usedHeight;
}

void DrawBadge(Renderer& r, const Assets& a, const char* text,
               float x, float y, float fontHeightPx,
               const glm::vec3& textColor, const glm::vec3& bgColor,
               float bgAlpha, float padX, float padY)
{
    const float textW = MeasureWidth(text, fontHeightPx);
    const float badgeW = textW + padX * 2.0f;
    const float badgeH = fontHeightPx + padY * 2.0f;

    // Background pill
    Rect(r, a, x, y, badgeW, badgeH, bgColor, bgAlpha);
    // Sharp border outline
    Rect(r, a, x, y + badgeH - 1.5f, badgeW, 1.5f, glm::mix(bgColor, glm::vec3(1.0f), 0.35f), bgAlpha);

    // Text inside badge
    DrawText(r, a, text, x + padX, y + padY, fontHeightPx, textColor, 1.0f, false);
}
}

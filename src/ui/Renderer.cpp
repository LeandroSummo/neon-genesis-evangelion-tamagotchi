// ============================================================
// RENDERER.CPP
// ============================================================
#include "ui/Renderer.h"

void Renderer::begin() {
    M5.Display.setRotation(1);                      // landscape 240×135
    M5.Display.setBrightness(
        BatteryManager::get().getRecommendedBrightness());

    _canvas.setColorDepth(16);
    _canvas.createSprite(Display::W, Display::H);   // 240×135 canvas
    _canvas.setTextWrap(false);
    _initTime = millis();
}

void Renderer::flush() {
    _canvas.pushSprite(&M5.Display, 0, 0);
}

void Renderer::clearCanvas(uint32_t color) {
    _canvas.fillScreen(color);
}

// ─── HEADER ─────────────────────────────────────────────
void Renderer::drawHeader(const char* title, const EvaStats& stats) {
    uint8_t unit  = stats.evaUnit;
    uint32_t col  = EVA_PERSONALITIES[unit].primaryColor;

    // Background bar
    _canvas.fillRect(0, 0, Display::W, Display::HEADER_H, 0x0A1020);
    _canvas.drawFastHLine(0, Display::HEADER_H, Display::W, col);

    // Title
    _canvas.setTextSize(1);
    _canvas.setTextColor(col);
    _canvas.setCursor(4, 3);
    _canvas.print("NERV//");
    _canvas.setTextColor(Colors::WHITE);
    _canvas.print(title);

    // Battery indicator (right side)
    char batBuf[8];
    uint8_t bat = BatteryManager::get().getLevel();
    snprintf(batBuf, sizeof(batBuf), "%3d%%", bat);
    uint32_t batColor = bat > 20 ? Colors::NERV_GREEN : Colors::NERV_RED;
    if (BatteryManager::get().isCharging()) batColor = Colors::NERV_CYAN;
    _canvas.setTextColor(batColor);
    _canvas.setCursor(Display::W - 32, 3);
    _canvas.print(batBuf);

    // Sync rate (center)
    char syncBuf[8];
    snprintf(syncBuf, sizeof(syncBuf), "S%02d%%", stats.syncRate);
    _canvas.setTextColor(Colors::NERV_GREEN_DIM);
    _canvas.setCursor(Display::W / 2 - 12, 3);
    _canvas.print(syncBuf);
}

// ─── FOOTER / MENU ──────────────────────────────────────
// selected < count  → menu mode (highlight selected slot)
// selected == 255   → action-guide mode (colour-code by button)
//   slot 0 = [A] action  → NERV_CYAN
//   slot 1 = [B] action  → NERV_GREEN
//   slot 2 = [B+] action → NERV_YELLOW
void Renderer::drawFooter(const char* labels[], uint8_t count,
                           uint8_t selected) {
    const int16_t y   = Display::H - Display::FOOTER_H;
    const int16_t midY = y + (Display::FOOTER_H - 8) / 2;

    _canvas.fillRect(0, y, Display::W, Display::FOOTER_H, 0x060810);
    _canvas.drawFastHLine(0, y, Display::W, Colors::NERV_GREEN_DIM);

    if (count == 0) return;

    bool isMenu = (selected < count);
    int16_t slotW = Display::W / count;

    // Colour palette for action-guide slots
    static const uint32_t ACTION_FG[3] = {
        Colors::NERV_CYAN,    // A
        Colors::NERV_GREEN,   // B
        Colors::NERV_YELLOW,  // B+
    };

    for (uint8_t i = 0; i < count; i++) {
        int16_t x   = i * slotW;
        bool    sel = isMenu && (i == selected);

        uint32_t fg;
        if (sel) {
            fg = Colors::BG_DARK;
            _canvas.fillRect(x + 1, y + 1, slotW - 2, Display::FOOTER_H - 2,
                             Colors::NERV_GREEN_DIM);
            _canvas.drawRect(x, y, slotW, Display::FOOTER_H, Colors::NERV_GREEN);
        } else {
            fg = isMenu ? Colors::TEXT_DIM : ACTION_FG[i < 3 ? i : 2];
            _canvas.drawRect(x, y, slotW, Display::FOOTER_H, 0x1A1A2A);
        }

        _canvas.setTextColor(fg);
        _canvas.setTextSize(1);
        uint16_t tw = strlen(labels[i]) * 6;
        _canvas.setCursor(x + (slotW - tw) / 2, midY);
        _canvas.print(labels[i]);
    }
}

// ─── STAT BAR ───────────────────────────────────────────
void Renderer::drawStatBar(int16_t x, int16_t y, int16_t w, int16_t h,
                             int16_t value, uint32_t color,
                             const char* label) {
    // Background
    _canvas.fillRect(x, y, w, h, 0x111122);
    _canvas.drawRect(x, y, w, h, Colors::NERV_GREEN_DIM);

    // Fill
    int16_t fillW = (int16_t)(w * value / 100);
    if (fillW > 0) {
        _canvas.fillRect(x, y, fillW, h, color);
    }

    // Label
    _canvas.setTextColor(Colors::WHITE);
    _canvas.setTextSize(1);
    _canvas.setCursor(x + 2, y + (h - 7) / 2);
    _canvas.print(label);

    // Value (right-aligned)
    char buf[5];
    snprintf(buf, sizeof(buf), "%3d", value);
    _canvas.setCursor(x + w - 19, y + (h - 7) / 2);
    _canvas.print(buf);
}

// ─── EVA SPRITE ─────────────────────────────────────────
void Renderer::drawEVASprite(const EvaStats& stats) {
    AnimationSystem& anim = AnimationSystem::get();
    uint8_t unit          = stats.evaUnit;

    AnimState animState = (AnimState)0;
    if (stats.berserk)        animState = AnimState::BERSERK;
    else if (stats.sleeping)  animState = AnimState::SLEEPING;
    else if (stats.angelMode) animState = AnimState::CORRUPTED;
    else if (stats.happiness > 70) animState = AnimState::HAPPY;
    else if (stats.rage > 60) animState = AnimState::ANGRY;
    else if (stats.stability < 30) animState = AnimState::SAD;
    else                      animState = AnimState::IDLE;

    const uint8_t* frame = anim.getCurrentFrame(unit, animState);

    // Sprite area: x=0, y=HEADER_H, w=SPRITE_W, h=CONTENT_H
    // 16x16 at scale 4 = 64x64 centered in 80x101 area
    int16_t sx = (Display::SPRITE_W - 64) / 2;
    int16_t sy = Display::HEADER_H + (Display::CONTENT_H - 64) / 2;

    // Background for sprite pane
    _canvas.fillRect(0, Display::HEADER_H, Display::SPRITE_W,
                     Display::CONTENT_H, 0x060610);

    // Subtle glow behind sprite
    uint32_t glowColor = EVA_PERSONALITIES[unit].primaryColor;
    // Draw dim glow rectangles
    _canvas.drawRect(sx - 4, sy - 4, 72, 72, glowColor & 0x1F1F1F);
    _canvas.drawRect(sx - 2, sy - 2, 68, 68, glowColor & 0x3F3F3F);

    anim.drawSprite(_canvas, frame, sx, sy, unit, 4);

    // Corrupted overlay: random colored pixels on angel mode
    if (animState == AnimState::CORRUPTED) {
        GlitchRenderer::get().drawPixelCorruption(_canvas, 5);
    }

    // Vertical separator line
    _canvas.drawFastVLine(Display::SPRITE_W, Display::HEADER_H,
                           Display::CONTENT_H, Colors::NERV_GREEN_DIM);
}

// ─── STATUS PANEL ───────────────────────────────────────
void Renderer::drawStatusPanel(const EvaStats& stats, int16_t x, int16_t y) {
    uint8_t  unit = stats.evaUnit;
    uint32_t col  = EVA_PERSONALITIES[unit].primaryColor;

    int16_t barW = Display::STATS_W - 8;
    int16_t barH = 10;
    int16_t gap  = 13;

    // SYNC
    uint32_t syncColor = stats.syncRate >= 70 ? Colors::NERV_GREEN :
                         stats.syncRate >= 40 ? Colors::NERV_ORANGE :
                                                Colors::NERV_RED;
    drawStatBar(x, y,       barW, barH, stats.syncRate,     syncColor, "SYNC");

    // HUNGER (inverted — show "FED" level = 100-hunger)
    uint32_t hungerColor = stats.hunger < 40 ? Colors::NERV_GREEN :
                           stats.hunger < 70 ? Colors::NERV_ORANGE :
                                               Colors::NERV_RED;
    drawStatBar(x, y+gap,   barW, barH, 100-stats.hunger,   hungerColor, "FEED");

    // ENERGY
    uint32_t energyColor = stats.energy > 50 ? col : Colors::NERV_ORANGE;
    drawStatBar(x, y+gap*2, barW, barH, stats.energy,       energyColor, "ENRG");

    // STABILITY
    uint32_t stabColor = stats.stability > 50 ? Colors::NERV_CYAN :
                         stats.stability > 25 ? Colors::NERV_ORANGE :
                                                Colors::NERV_RED;
    drawStatBar(x, y+gap*3, barW, barH, stats.stability,    stabColor,  "STAB");

    // RAGE
    uint32_t rageColor = stats.rage < 40 ? Colors::NERV_GREEN_DIM :
                         stats.rage < 70 ? Colors::NERV_ORANGE :
                                           Colors::NERV_RED;
    drawStatBar(x, y+gap*4, barW, barH, stats.rage,         rageColor,  "RAGE");

    // contamination stat hidden in Design v2 (internal only)
}

// ─── NERV HUD FRAME ─────────────────────────────────────
void Renderer::drawNERVFrame(uint32_t accentColor) {
    _canvas.drawRect(0, 0, Display::W, Display::H, accentColor);
    drawCornerMarkers(accentColor);
}

void Renderer::drawCornerMarkers(uint32_t color) {
    // Corner L-brackets
    int16_t s = 8;
    _canvas.drawFastHLine(0, 0, s, color);
    _canvas.drawFastVLine(0, 0, s, color);

    _canvas.drawFastHLine(Display::W - s, 0, s, color);
    _canvas.drawFastVLine(Display::W - 1, 0, s, color);

    _canvas.drawFastHLine(0, Display::H - 1, s, color);
    _canvas.drawFastVLine(0, Display::H - s, s, color);

    _canvas.drawFastHLine(Display::W - s, Display::H - 1, s, color);
    _canvas.drawFastVLine(Display::W - 1, Display::H - s, s, color);
}

// ─── SYNC DISPLAY ───────────────────────────────────────
void Renderer::drawSyncDisplay(int16_t syncRate, int16_t x, int16_t y) {
    char buf[16];
    snprintf(buf, sizeof(buf), "SYNC: %02d%%", syncRate);
    _canvas.setTextColor(Colors::NERV_GREEN);
    _canvas.setTextSize(1);
    _canvas.setCursor(x, y);
    _canvas.print(buf);
}

// ─── TEXT HELPERS ───────────────────────────────────────
void Renderer::drawText(const char* str, int16_t x, int16_t y,
                         uint32_t color, uint8_t fontSize) {
    _canvas.setTextColor(color);
    _canvas.setTextSize(fontSize);
    _canvas.setCursor(x, y);
    _canvas.print(str);
}

void Renderer::drawTextCentered(const char* str, int16_t y,
                                 uint32_t color, uint8_t fontSize) {
    _canvas.setTextColor(color);
    _canvas.setTextSize(fontSize);
    uint16_t tw = strlen(str) * 6 * fontSize;
    _canvas.setCursor((Display::W - tw) / 2, y);
    _canvas.print(str);
}

bool Renderer::isBlinkOn() const {
    return ((millis() / 500) % 2) == 0;
}

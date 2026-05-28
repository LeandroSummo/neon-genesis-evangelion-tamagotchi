#pragma once
// ============================================================
// RENDERER.H — Main rendering engine with offscreen canvas
// ============================================================
#include <M5Unified.h>
#include <M5GFX.h>
#include "core/Types.h"
#include "entities/EvaPet.h"
#include "ui/GlitchRenderer.h"
#include "ui/AnimationSystem.h"
#include "sensors/BatteryManager.h"

class Renderer {
public:
    static Renderer& get() {
        static Renderer instance;
        return instance;
    }

    void begin();
    void flush();  // push canvas to display

    LGFX_Sprite& canvas() { return _canvas; }

    // ─── Common UI elements ───────────────────────────────
    void clearCanvas(uint32_t color = Colors::BG_DARK);

    void drawHeader(const char* title, const EvaStats& stats);
    void drawFooter(const char* labels[], uint8_t count,
                    uint8_t selected = 0);

    void drawStatBar(int16_t x, int16_t y, int16_t w, int16_t h,
                     int16_t value, uint32_t color, const char* label);

    void drawEVASprite(const EvaStats& stats);
    void drawStatusPanel(const EvaStats& stats, int16_t x, int16_t y);

    // ─── NERV HUD elements ────────────────────────────────
    void drawNERVFrame(uint32_t accentColor);
    void drawCornerMarkers(uint32_t color);
    void drawSyncDisplay(int16_t syncRate, int16_t x, int16_t y);

    // ─── Text helpers ─────────────────────────────────────
    void drawText(const char* str, int16_t x, int16_t y,
                  uint32_t color, uint8_t fontSize = 1);
    void drawTextCentered(const char* str, int16_t y,
                          uint32_t color, uint8_t fontSize = 1);

    // Blink helper: returns true every other half-second
    bool isBlinkOn() const;

private:
    Renderer() = default;
    LGFX_Sprite _canvas;
    uint32_t    _initTime = 0;
};

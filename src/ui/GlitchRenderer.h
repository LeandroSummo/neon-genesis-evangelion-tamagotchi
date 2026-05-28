#pragma once
// ============================================================
// GLITCHRENDERER.H — CRT scanlines, pixel corruption, flicker
// ============================================================
#include <M5Unified.h>
#include <M5GFX.h>
#include "core/Types.h"

class GlitchRenderer {
public:
    static GlitchRenderer& get() {
        static GlitchRenderer instance;
        return instance;
    }

    void begin();
    void update();  // call every frame — manages glitch timers

    // Draw effects onto a sprite canvas
    void drawScanlines(LGFX_Sprite& canvas, uint8_t alpha = 60);
    void drawHorizontalGlitch(LGFX_Sprite& canvas, uint8_t intensity = 30);
    void drawPixelCorruption(LGFX_Sprite& canvas, uint8_t density = 10);
    void drawVerticalOffset(LGFX_Sprite& canvas, uint8_t maxShift = 8);
    void drawRedFlash(LGFX_Sprite& canvas, uint8_t intensity = 80);
    void drawVHSNoise(LGFX_Sprite& canvas, uint8_t lines = 3);

    // Corrupt a text string (replace random chars with garbage)
    void corruptString(char* buf, size_t len, float probability = 0.15f);

    // Intensity control (0.0 = off, 1.0 = full chaos)
    void setIntensity(float i) { _intensity = constrain(i, 0.0f, 1.0f); }
    float getIntensity()       const { return _intensity; }

    bool isTriggerFrame()      const { return _glitchFrame; }

private:
    GlitchRenderer() = default;

    float    _intensity   = 0.0f;
    uint32_t _lastGlitch  = 0;
    uint32_t _glitchInterval = 4000;
    bool     _glitchFrame = false;

    // Glitch corruption characters
    static constexpr const char GLITCH_CHARS[] = "▓░█▒■□▪▫◆◇△▽◈";
};

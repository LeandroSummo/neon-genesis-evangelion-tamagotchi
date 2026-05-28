// ============================================================
// GLITCHRENDERER.CPP
// ============================================================
#include "ui/GlitchRenderer.h"

static const char GLITCH_CHARS[] = "#@!%^&*~<>|\\{}[]?/";

void GlitchRenderer::begin() {
    _lastGlitch = millis();
    randomSeed(esp_random());
}

void GlitchRenderer::update() {
    uint32_t now = millis();

    // Vary glitch interval based on intensity
    _glitchInterval = (uint32_t)(4000.0f - _intensity * 3000.0f);
    _glitchInterval = max((uint32_t)300, _glitchInterval);

    if (now - _lastGlitch > _glitchInterval) {
        _glitchFrame = true;
        _lastGlitch  = now;
    } else {
        _glitchFrame = false;
    }
}

// Horizontal scanlines — every other pixel row dimmed
void GlitchRenderer::drawScanlines(LGFX_Sprite& canvas, uint8_t alpha) {
    if (_intensity < 0.01f) return;
    int16_t w = canvas.width();
    int16_t h = canvas.height();
    // Draw semi-transparent black lines on every other row
    for (int16_t y = 0; y < h; y += 2) {
        for (int16_t x = 0; x < w; x++) {
            uint32_t px = canvas.readPixel(x, y);
            // Dim the pixel by ~25%
            uint8_t r = ((px >> 16) & 0xFF) * 3 / 4;
            uint8_t g = ((px >> 8)  & 0xFF) * 3 / 4;
            uint8_t b = ((px)       & 0xFF) * 3 / 4;
            canvas.drawPixel(x, y, canvas.color888(r, g, b));
        }
    }
}

// Horizontal band shift — creates VHS tape error feel
void GlitchRenderer::drawHorizontalGlitch(LGFX_Sprite& canvas,
                                           uint8_t intensity) {
    if (_intensity < 0.1f || !_glitchFrame) return;

    int16_t w = canvas.width();
    int16_t h = canvas.height();
    uint8_t numBands = 1 + (uint8_t)(_intensity * 3);

    for (uint8_t i = 0; i < numBands; i++) {
        int16_t y      = random(0, h - 10);
        int16_t height = random(2, 8);
        int16_t shift  = random(-(int)intensity, (int)intensity);
        if (shift == 0) continue;

        // Read band, shift, write
        for (int16_t row = y; row < y + height && row < h; row++) {
            for (int16_t x = 0; x < w; x++) {
                int16_t srcX = x - shift;
                if (srcX >= 0 && srcX < w) {
                    uint32_t px = canvas.readPixel(srcX, row);
                    canvas.drawPixel(x, row, px);
                }
            }
        }
    }
}

// Random pixel corruption
void GlitchRenderer::drawPixelCorruption(LGFX_Sprite& canvas,
                                          uint8_t density) {
    if (_intensity < 0.05f) return;

    int16_t w = canvas.width();
    int16_t h = canvas.height();
    uint16_t count = (uint16_t)(_intensity * density * 20);

    for (uint16_t i = 0; i < count; i++) {
        int16_t x = random(0, w);
        int16_t y = random(0, h);
        // Random bright pixel (glitch color)
        uint32_t col;
        switch (random(0, 4)) {
            case 0: col = Colors::NERV_RED;    break;
            case 1: col = Colors::NERV_ORANGE;  break;
            case 2: col = Colors::NERV_GREEN;  break;
            default: col = Colors::WHITE;      break;
        }
        canvas.drawPixel(x, y, col);
    }
}

// Vertical offset of a horizontal slice
void GlitchRenderer::drawVerticalOffset(LGFX_Sprite& canvas,
                                         uint8_t maxShift) {
    if (_intensity < 0.2f || !_glitchFrame) return;

    int16_t w     = canvas.width();
    int16_t h     = canvas.height();
    int16_t y     = random(0, h - 20);
    int16_t height = random(5, 15);
    int16_t shift  = random(1, maxShift) * (random(0, 2) ? 1 : -1);

    for (int16_t row = y; row < y + height && row < h; row++) {
        int16_t dstRow = row + shift;
        if (dstRow < 0 || dstRow >= h) continue;
        for (int16_t x = 0; x < w; x++) {
            uint32_t px = canvas.readPixel(x, row);
            canvas.drawPixel(x, dstRow, px);
        }
    }
}

// Red flash overlay (berserk indicator)
void GlitchRenderer::drawRedFlash(LGFX_Sprite& canvas, uint8_t intensity) {
    if (intensity == 0) return;
    // Draw transparent red over the whole screen
    // Implemented as overlaid red-tinted pixels
    int16_t w = canvas.width();
    int16_t h = canvas.height();
    uint8_t step = (uint8_t)(4 + (255 - intensity) / 10);

    for (int16_t y = 0; y < h; y += step) {
        for (int16_t x = 0; x < w; x += step) {
            uint32_t px = canvas.readPixel(x, y);
            uint8_t r = min(255, (int)(((px >> 16) & 0xFF) + intensity));
            uint8_t g = ((px >> 8) & 0xFF) * (255 - intensity / 2) / 255;
            uint8_t b = ((px)      & 0xFF) * (255 - intensity)     / 255;
            canvas.drawPixel(x, y, canvas.color888(r, g, b));
        }
    }
}

// VHS noise lines
void GlitchRenderer::drawVHSNoise(LGFX_Sprite& canvas, uint8_t lines) {
    if (_intensity < 0.05f) return;
    int16_t w = canvas.width();
    int16_t h = canvas.height();
    uint8_t count = (uint8_t)(_intensity * lines);

    for (uint8_t i = 0; i < count; i++) {
        int16_t y   = random(0, h);
        int16_t len = random(10, w / 2);
        int16_t x   = random(0, w - len);
        canvas.drawFastHLine(x, y, len, Colors::NERV_GREEN_DIM);
    }
}

void GlitchRenderer::corruptString(char* buf, size_t len, float prob) {
    if (_intensity < 0.05f || !_glitchFrame) return;
    for (size_t i = 0; i < len && buf[i] != '\0'; i++) {
        if ((float)random(0, 1000) / 1000.0f < prob * _intensity) {
            buf[i] = GLITCH_CHARS[random(0, sizeof(GLITCH_CHARS) - 1)];
        }
    }
}

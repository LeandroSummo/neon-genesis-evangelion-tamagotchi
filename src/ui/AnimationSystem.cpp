// ============================================================
// ANIMATIONSYSTEM.CPP
// ============================================================
#include "ui/AnimationSystem.h"

void AnimationSystem::begin() {
    _lastFrame  = millis();
    _breathTimer = millis();
}

void AnimationSystem::update() {
    uint32_t now = millis();

    // Advance animation frame
    if (now - _lastFrame >= _frameMs) {
        _lastFrame = now;
        _frameIdx  = (_frameIdx + 1) % _maxFrames;
    }

    // Breathing bob (1px vertical shift every 1.5s)
    if (now - _breathTimer >= 1500) {
        _breathTimer = now;
        _breathBob   = (_breathBob == 0) ? 1 : 0;
    }
}

const uint8_t* AnimationSystem::getCurrentFrame(uint8_t unit,
                                                  AnimState state) const {
    if (unit >= 4) unit = 0;

    switch (state) {
        case AnimState::SLEEPING:
            return (const uint8_t*)pgm_read_ptr(&SPR_TABLE[unit][2]);
        case AnimState::BERSERK:
        case AnimState::ANGRY:
            // Alternate berserk frames with current frameIdx
            return (const uint8_t*)pgm_read_ptr(
                &SPR_TABLE[unit][3 + (_frameIdx & 1)]);
        case AnimState::IDLE:
        case AnimState::HAPPY:
        case AnimState::SAD:
        default:
            // Alternate between IDLE1 and IDLE2
            return (const uint8_t*)pgm_read_ptr(
                &SPR_TABLE[unit][_frameIdx & 1]);
    }
}

void AnimationSystem::drawSprite(LGFX_Sprite& canvas, const uint8_t* data,
                                  int16_t x, int16_t y, uint8_t unit,
                                  uint8_t scale) {
    if (!data || unit >= 4) return;

    // Add breathing bob for non-sleeping states
    y += _breathBob;

    const uint32_t* palette = SPR_PALETTES[unit];

    for (uint8_t py = 0; py < 16; py++) {
        for (uint8_t px = 0; px < 16; px++) {
            uint8_t colorIdx = pgm_read_byte(data + py * 16 + px);
            if (colorIdx == 0) continue;  // transparent

            uint32_t color = palette[colorIdx & 3];

            // Draw scaled pixel block
            for (uint8_t sy = 0; sy < scale; sy++) {
                for (uint8_t sx = 0; sx < scale; sx++) {
                    int16_t dx = x + (int16_t)(px * scale + sx);
                    int16_t dy = y + (int16_t)(py * scale + sy);
                    if (dx >= 0 && dx < canvas.width() &&
                        dy >= 0 && dy < canvas.height()) {
                        canvas.drawPixel(dx, dy, color);
                    }
                }
            }
        }
    }
}

#pragma once
// ============================================================
// ANIMATIONSYSTEM.H — Sprite frame animation controller
// ============================================================
#include <M5GFX.h>
#include "core/Types.h"
#include "data/Sprites.h"

class AnimationSystem {
public:
    static AnimationSystem& get() {
        static AnimationSystem instance;
        return instance;
    }

    void begin();
    void update();   // advance frames

    // Get the current frame's sprite data
    const uint8_t* getCurrentFrame(uint8_t unit, AnimState state) const;

    // Draw sprite at position on a canvas (16x16 scaled)
    void drawSprite(LGFX_Sprite& canvas, const uint8_t* data,
                    int16_t x, int16_t y, uint8_t unit, uint8_t scale = 4);

    uint8_t getCurrentFrameIdx() const { return _frameIdx; }

private:
    AnimationSystem() = default;

    uint8_t  _frameIdx   = 0;
    uint32_t _lastFrame  = 0;
    uint32_t _frameMs    = 500;  // ms per frame
    uint8_t  _maxFrames  = 2;

    // Breathing bob: offset in pixels (0 or 1)
    int8_t   _breathBob  = 0;
    uint32_t _breathTimer = 0;
};

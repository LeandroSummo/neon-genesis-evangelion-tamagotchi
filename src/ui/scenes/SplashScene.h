#pragma once
// ============================================================
// SPLASHSCENE.H + BOOTSCENE.H
//
// Splash v2.2 — pure text, no graphics primitives
//
// TIMING (ms):
//   0    : black
//   0    : "NEON GENESIS" wipes in from left (400ms)
//   400  : "EVANGELION" wipes in (300ms)
//   700  : yellow line + typewriter "NERV MAGI SYSTEM v3.0"
//           21 chars × 60ms = 1260ms
//   1960 : tagline appears all at once
//   2600 : auto-advance → BOOT
//   BTN_A/B: skip at any time
// ============================================================
#include "ui/SceneManager.h"
#include "data/LoreData.h"
#include "data/EvaLogo.h"
#include "audio/AudioManager.h"

// ─── NERV SPLASH ─────────────────────────────────────────
class SplashScene : public BaseScene {
public:
    static SplashScene& get() { static SplashScene s; return s; }

    void onEnter() override {
        _startTime = millis();
        AudioManager::get().playBoot();
    }

    void update() override {
        if (millis() - _startTime >= T_AUTO) requestTransition(GameState::BOOT);
    }

    void render() override {
        Renderer& r = Renderer::get();
        uint32_t  el = millis() - _startTime;

        r.clearCanvas(Colors::BLACK);
        _drawSplash(r, el);
        r.flush();
    }

    GameState handleInput(InputEvent ev) override {
        if (ev == InputEvent::BTN_A_CLICK || ev == InputEvent::BTN_A_HOLD ||
            ev == InputEvent::BTN_B_CLICK || ev == InputEvent::BTN_B_HOLD)
            return GameState::BOOT;
        return GameState::COUNT;
    }

private:
    static constexpr uint32_t T_TYPE_START = 600;
    static constexpr uint32_t T_TYPE_DONE  = 1860;  // 21 chars × 60ms
    static constexpr uint32_t T_AUTO       = 2600;

    static constexpr const char* TYPE_TEXT = "NERV MAGI SYSTEM v3.0";
    static constexpr uint8_t     TYPE_LEN  = 21;

    uint32_t _startTime = 0;

    void _drawSplash(Renderer& r, uint32_t el) {
        LGFX_Sprite& c  = r.canvas();
        const int16_t CX = Display::W / 2;

        c.fillScreen(Colors::BLACK);

        // Logo bitmap — 220×55, centred, y=30
        drawEvaLogo(&c, 30);

        // Yellow separator under logo
        c.fillRect(10, 89, 220, 2, 0xE8C3);

        // Typewriter "NERV MAGI SYSTEM v3.0"
        if (el >= T_TYPE_START) {
            int typed = (int)((el - T_TYPE_START) / 60) + 1;
            if (typed > TYPE_LEN) typed = TYPE_LEN;
            char buf[TYPE_LEN + 1];
            strncpy(buf, TYPE_TEXT, typed);
            buf[typed] = '\0';
            int16_t tw = typed * 6;
            c.setTextColor(0xE8C3);
            c.setTextSize(1);
            c.setCursor(CX - (TYPE_LEN * 6) / 2, 97);
            c.print(buf);
            // Cursor blink while typing
            if (typed < TYPE_LEN && ((el / 200) % 2 == 0)) {
                c.fillRect(CX - (TYPE_LEN * 6) / 2 + tw, 97, 5, 7, 0xE8C3);
            }
        }

        // Motto — always visible
        c.setTextColor(0x4A49);
        c.setTextSize(1);
        c.setCursor(4, 124);
        c.print("God's in his heaven.");
    }
};

// ─── BOOT DIAGNOSTICS ────────────────────────────────────
class BootScene : public BaseScene {
public:
    static BootScene& get() { static BootScene s; return s; }

    void onEnter() override {
        _startTime = millis();
        _lineIdx   = 0;
        _lineTimer = millis();
        _done      = false;
    }

    void update() override {
        uint32_t now = millis();
        if (!_done && now - _lineTimer > 160) {
            _lineTimer = now;
            _lineIdx++;
            if (_lineIdx >= BOOT_LINE_COUNT) {
                _done     = true;
                _doneTime = now;
            }
        }
        if (_done && now - _doneTime > 600) {
            if (EvaPet::get().getStats().unitSelected)
                requestTransition(GameState::MAIN);
            else
                requestTransition(GameState::EVA_SELECT);
        }
    }

    void render() override {
        Renderer& r = Renderer::get();
        r.clearCanvas(Colors::BG_DARK);

        r.drawText("[ NERV  MAGI  SYSTEM ]", 4, 2, Colors::NERV_GREEN, 1);
        r.canvas().drawFastHLine(0, 12, Display::W, Colors::NERV_GREEN_DIM);

        uint8_t visible = min(_lineIdx, (uint8_t)BOOT_LINE_COUNT);
        for (uint8_t i = 0; i < visible; i++) {
            char buf[48];
            strncpy_P(buf, BOOT_LINES[i], sizeof(buf));
            bool isLast = (i == visible - 1);
            uint32_t col = (isLast && !_done) ? Colors::NERV_GREEN : Colors::NERV_GREEN_DIM;
            r.drawText(buf, 4, 16 + i * 10, col, 1);

            if (isLast && !_done) {
                uint8_t dots = ((millis() / 280) % 4);
                char d[5] = "    ";
                for (uint8_t k = 0; k < dots; k++) d[k] = '.';
                d[dots] = '\0';
                r.drawText(d, 4 + strlen(buf) * 6, 16 + i * 10, Colors::NERV_GREEN, 1);
            } else if (isLast && _done) {
                r.drawText(" OK", 4 + strlen(buf) * 6, 16 + i * 10, Colors::NERV_GREEN, 1);
            }
        }

        if (_done)
            r.drawTextCentered(">> SYSTEM READY <<", 120, Colors::NERV_GREEN, 1);

        r.drawCornerMarkers(Colors::NERV_GREEN_DIM);
        r.flush();
    }

private:
    uint32_t _startTime = 0;
    uint32_t _lineTimer = 0;
    uint32_t _doneTime  = 0;
    uint8_t  _lineIdx   = 0;
    bool     _done      = false;
};

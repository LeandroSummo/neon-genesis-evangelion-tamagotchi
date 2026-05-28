#pragma once
// ============================================================
// ShutdownScene — Standby / graceful deep-sleep sequence
//
// Tick  0- 30: "EVANGELION / UNIT STANDING BY"
// Tick 30- 60: "SAVING PILOT DATA..." + progress bar
// Tick 60- 80: "DATA SAVED" + "MAGI SYSTEM OFFLINE"
// Tick 80- 90: brightness fades 80→0
// Tick  90   : esp_deep_sleep_start() on GPIO37 (BTN_A)
// ============================================================
#include "ui/SceneManager.h"
#include "entities/EvaPet.h"
#include "core/SaveManager.h"
#include "ui/Renderer.h"
#include "audio/AudioManager.h"
#include <esp_sleep.h>

class ShutdownScene : public BaseScene {
public:
    static ShutdownScene& get() { static ShutdownScene s; return s; }

    void onEnter() override {
        _ticker    = 0;
        _soundDone = false;
        _saved     = false;
        AudioManager::get().stopSequence();
    }

    void update() override {
        _ticker++;

        if (_ticker == 5 && !_soundDone) {
            _soundDone = true;
            _playShutdownSound();
        }

        // Save at tick 30
        if (_ticker == 30 && !_saved) {
            _saved = true;
            const EvaStats& st = EvaPet::get().getStats();
            SaveManager::get().save(st);
            SaveManager::get().setCleanShutdown(true);
        }

        // Fade brightness
        if (_ticker >= 80 && _ticker <= 90) {
            int brightness = map(_ticker, 80, 90, 80, 0);
            M5.Display.setBrightness((uint8_t)brightness);
        }

        // Deep sleep
        if (_ticker >= 91) {
            M5.Display.setBrightness(0);
            esp_sleep_enable_ext0_wakeup(GPIO_NUM_37, 0);  // BTN_A wakes
            esp_deep_sleep_start();
        }
    }

    void render() override {
        Renderer& r = Renderer::get();
        const EvaStats& st = EvaPet::get().getStats();
        uint32_t col = EVA_PERSONALITIES[st.evaUnit].primaryColor;

        r.clearCanvas(Colors::BLACK);

        if (_ticker < 80) {
            r.drawTextCentered("EVANGELION", 24, col, 2);
            r.drawTextCentered("UNIT STANDING BY", 46, Colors::NERV_YELLOW, 1);

            if (_ticker >= 30) {
                int barW = map(min(_ticker - 30, 30), 0, 30, 0, 200);
                r.canvas().fillRect(20, 68, barW, 10, col);
                r.canvas().drawRect(20, 68, 200, 10, Colors::TEXT_DIM);
                r.canvas().setTextColor(Colors::TEXT_DIM);
                r.canvas().setTextSize(1);
                r.canvas().setCursor(20, 83);
                r.canvas().print("SAVING PILOT DATA...");
            }

            if (_ticker >= 60) {
                r.drawTextCentered("DATA SAVED", 96, Colors::NERV_GREEN, 1);
                r.drawTextCentered("MAGI SYSTEM OFFLINE", 107, Colors::NERV_YELLOW, 1);
            }
        }

        r.flush();
    }

    GameState handleInput(InputEvent) override { return GameState::COUNT; }

private:
    int  _ticker    = 0;
    bool _soundDone = false;
    bool _saved     = false;

    void _playShutdownSound() {
        AudioManager::get().playShutdown();
    }
};

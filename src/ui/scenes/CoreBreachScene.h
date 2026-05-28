#pragma once
// ============================================================
// CoreBreachScene — EVA core breach / death screen
//
// Triggered when syncRate < 5 for 5 continuous minutes.
// Shows pilot status unknown, days survived, high score.
// BTN_B: restart → SPLASH (via resetCoreBreach)
// ============================================================
#include "ui/SceneManager.h"
#include "entities/EvaPet.h"
#include "core/SaveManager.h"
#include "ui/Renderer.h"
#include "audio/AudioManager.h"
#include "data/EvaData.h"

class CoreBreachScene : public BaseScene {
public:
    static CoreBreachScene& get() { static CoreBreachScene s; return s; }

    void onEnter() override {
        _ticker    = 0;
        _confirmed = false;
        // Update high score before display
        EvaStats& s = EvaPet::get().getStatsMut();
        if (s.daysAlive > s.highScoreDays) s.highScoreDays = s.daysAlive;
        _days    = s.daysAlive;
        _hiScore = s.highScoreDays;
        _unit    = s.evaUnit;
        SaveManager::get().save(s);
        AudioManager::get().playAlert();
    }

    void update() override {
        _ticker++;
    }

    void render() override {
        Renderer& R = Renderer::get();
        bool blink = (_ticker / 6) % 2 == 0;

        // Alternating dark red background
        R.canvas().fillScreen(blink ? 0x1A0000 : Colors::BG_DARK);
        R.canvas().drawRect(0, 0, Display::W, Display::H, Colors::NERV_RED);
        R.canvas().drawRect(1, 1, Display::W - 2, Display::H - 2, Colors::NERV_RED_DIM);

        // Unit name header
        R.canvas().setTextColor(Colors::TEXT_DIM);
        R.canvas().setTextSize(1);
        R.canvas().setCursor(4, 4);
        R.canvas().print("EVANGELION UNIT: ");
        R.canvas().setTextColor(EVA_PERSONALITIES[_unit].primaryColor);
        R.canvas().print(EVA_PERSONALITIES[_unit].name);

        // Core breach title
        uint32_t titleCol = blink ? Colors::NERV_RED : Colors::WHITE;
        R.drawTextCentered("CORE BREACH", 20, titleCol, 2);
        R.drawTextCentered("DETECTED", 36, titleCol, 2);

        R.canvas().drawFastHLine(20, 54, Display::W - 40, Colors::NERV_RED_DIM);

        // Status lines
        R.canvas().setTextColor(Colors::TEXT_DIM);
        R.canvas().setTextSize(1);
        R.canvas().setCursor(20, 60);
        R.canvas().print("PILOT STATUS: ");
        R.canvas().setTextColor(blink ? Colors::NERV_RED : Colors::TEXT_DIM);
        R.canvas().print("UNKNOWN");

        char dayBuf[24];
        snprintf(dayBuf, sizeof(dayBuf), "DAYS ACTIVE:  %u", (unsigned)_days);
        R.canvas().setTextColor(Colors::NERV_ORANGE);
        R.canvas().setCursor(20, 72);
        R.canvas().print(dayBuf);

        char hiBuf[24];
        snprintf(hiBuf, sizeof(hiBuf), "HIGH SCORE:   %u", (unsigned)_hiScore);
        R.canvas().setTextColor(Colors::NERV_CYAN);
        R.canvas().setCursor(20, 84);
        R.canvas().print(hiBuf);

        R.canvas().drawFastHLine(20, 96, Display::W - 40, Colors::NERV_RED_DIM);

        // Restart prompt
        if (!_confirmed) {
            R.canvas().setTextColor(Colors::NERV_YELLOW);
            R.canvas().setTextSize(1);
            R.canvas().setCursor(20, 102);
            R.canvas().print("RESTART PROTOCOL:");
            R.canvas().setTextColor(blink ? Colors::WHITE : Colors::NERV_GREEN);
            R.canvas().setCursor(20, 114);
            R.canvas().print("[BTN_B] REINITIALIZE UNIT");
        } else {
            R.drawTextCentered("REINITIALIZING...", 108, blink ? Colors::NERV_GREEN : Colors::TEXT_DIM, 1);
        }

        R.flush();
    }

    GameState handleInput(InputEvent ev) override {
        if (ev == InputEvent::BTN_B_CLICK && !_confirmed) {
            _confirmed = true;
            AudioManager::get().playSelect();
            // Reset pet state, go back to splash → boot → eva_select
            EvaPet::get().resetCoreBreach();
            SaveManager::get().save(EvaPet::get().getStats());
            return GameState::SPLASH;
        }
        return GameState::COUNT;
    }

private:
    int      _ticker    = 0;
    bool     _confirmed = false;
    uint16_t _days      = 0;
    uint16_t _hiScore   = 0;
    uint8_t  _unit      = 0;
};

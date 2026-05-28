#pragma once
// ============================================================
// BerserkScene — BERSERK panic mode
//
// Red screen, sprite berserk. One action: FIGHT.
// BTN_A or BTN_B → rage -30, calmedDown() → MAIN.
// Timeout 120s → sync -20 penalty → MAIN.
// ============================================================
#include "ui/SceneManager.h"
#include "entities/EvaPet.h"
#include "ui/Renderer.h"
#include "audio/AudioManager.h"

class BerserkScene : public BaseScene {
public:
    static BerserkScene& get() { static BerserkScene s; return s; }

    void onEnter() override {
        _ticker      = 0;
        _resolved    = false;
        _enterMs     = millis();
        AudioManager::get().playBerserk();
    }

    void onExit() override {}

    void update() override {
        _ticker++;

        if (_resolved && _ticker > 45) {
            requestTransition(GameState::MAIN);
            return;
        }

        // 2-minute timeout: apply sync penalty
        if (!_resolved && millis() - _enterMs > 120000UL) {
            _resolved = true;
            EvaPet::get().calmedDown();
            EvaPet::get().getStatsMut().syncRate =
                statClamp(EvaPet::get().getStatsMut().syncRate - 20);
            AudioManager::get().playBeep(180, 100);
        }
    }

    void render() override {
        Renderer& R = Renderer::get();
        const EvaStats& st = EvaPet::get().getStats();

        // Alternating red flash background
        bool flash = (_ticker / 5) % 2 == 0;
        R.canvas().fillScreen(flash ? 0x1A0000 : Colors::BG_DARK);
        R.canvas().drawRect(0, 0, Display::W, Display::H, Colors::NERV_RED);

        if (_resolved) {
            _drawCalmed(R);
        } else {
            _drawBerserk(R, st, flash);
        }

        R.flush();
    }

    GameState handleInput(InputEvent ev) override {
        if (_resolved) return GameState::COUNT;
        if (ev == InputEvent::BTN_A_CLICK || ev == InputEvent::BTN_B_CLICK) {
            _resolved = true;
            _ticker   = 0;
            EvaPet::get().calmedDown();
            AudioManager::get().playSelect();
        }
        return GameState::COUNT;
    }

private:
    int      _ticker   = 0;
    bool     _resolved = false;
    uint32_t _enterMs  = 0;

    void _drawBerserk(Renderer& R, const EvaStats& st, bool flash) {
        // BERSERK title
        R.canvas().setTextColor(flash ? Colors::WHITE : Colors::NERV_RED);
        R.canvas().setTextSize(2);
        R.canvas().setCursor((Display::W - 7 * 12) / 2, 6);
        R.canvas().print("BERSERK");

        // Sprite
        R.drawEVASprite(st);

        // Rage bar
        R.canvas().setTextColor(Colors::TEXT_DIM);
        R.canvas().setTextSize(1);
        R.canvas().setCursor(88, 30);
        R.canvas().print("RAGE");
        int rw = map(st.rage, 0, 100, 0, 140);
        R.canvas().fillRect(88, 40, rw, 10, Colors::NERV_RED);
        R.canvas().drawRect(88, 40, 140, 10, Colors::TEXT_DIM);

        // Divider
        R.canvas().drawFastHLine(8, 56, Display::W - 16, Colors::NERV_RED & 0x3F0000);

        // Fight prompt
        R.canvas().setTextColor(Colors::NERV_YELLOW);
        R.canvas().setTextSize(1);
        R.canvas().setCursor(88, 62);
        R.canvas().print("PRESS  BTN_A  or");
        R.canvas().setCursor(88, 74);
        R.canvas().print("BTN_B  to  FIGHT!");

        // Timeout warning
        uint32_t elapsed  = millis() - _enterMs;
        uint32_t remaining = (elapsed < 120000UL) ? (120000UL - elapsed) / 1000 : 0;
        char buf[16];
        snprintf(buf, sizeof(buf), "TIME: %us", (unsigned)remaining);
        R.canvas().setTextColor(Colors::TEXT_DIM);
        R.canvas().setCursor(88, 90);
        R.canvas().print(buf);
    }

    void _drawCalmed(Renderer& R) {
        R.canvas().fillScreen(Colors::BG_DARK);
        R.canvas().drawRect(0, 0, Display::W, Display::H, Colors::NERV_GREEN);

        R.drawTextCentered("BERSERK MODE", 28, Colors::NERV_GREEN, 2);
        R.drawTextCentered("TERMINATED",   46, Colors::NERV_GREEN, 2);
        R.drawTextCentered("Unit restrained.", 68, Colors::NERV_CYAN, 1);

        int p = constrain(_ticker * 2, 0, 100);
        int w = map(p, 0, 100, 0, 220);
        R.canvas().fillRect(10, 90, w, 8, Colors::NERV_GREEN);
        R.canvas().drawRect(10, 90, 220, 8, Colors::TEXT_DIM);
    }
};

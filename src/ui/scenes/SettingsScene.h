#pragma once
// ============================================================
// SettingsScene — reset / new game
// ============================================================
#include "ui/SceneManager.h"
#include "core/Types.h"
#include "core/SaveManager.h"
#include "entities/EvaPet.h"
#include "ui/Renderer.h"
#include "audio/AudioManager.h"

class SettingsScene : public BaseScene {
public:
    static SettingsScene& get() { static SettingsScene s; return s; }

    void onEnter() override {
        _sel     = 0;          // 0 = NEW GAME, 1 = BACK
        _confirm = false;
        _ticker  = 0;
    }

    void onExit() override {}

    void update() override {
        _ticker++;
        if (_confirm && _ticker > 60) {
            // Execute new-game reset
            SaveManager::get().reset();
            // Re-init pet with blank stats
            EvaStats blank{};
            EvaPet::get().begin(blank);
            requestTransition(GameState::EVA_SELECT);
        }
    }

    void render() override {
        auto& R = Renderer::get();
        R.clearCanvas(Colors::BG_DARK);

        // Title
        R.canvas().fillRect(0, 0, Display::W, 14, 0x0A0010);
        R.canvas().drawFastHLine(0, 14, Display::W, Colors::NERV_RED);
        R.canvas().setTextColor(Colors::NERV_RED);
        R.canvas().setTextSize(1);
        R.canvas().setCursor(4, 3);
        R.canvas().print("NERV//  SYSTEM  OPTIONS");

        if (_confirm) {
            _drawConfirm(R);
        } else {
            _drawMenu(R);
        }

        R.flush();
    }

    GameState handleInput(InputEvent ev) override {
        if (_confirm) {
            if (ev == InputEvent::BTN_A_CLICK) {
                // Cancel
                _confirm = false;
                _ticker  = 0;
                AudioManager::get().playBeep(400, 40);
            }
            if (ev == InputEvent::BTN_B_CLICK) {
                // Confirmed — update() will handle the actual reset
                AudioManager::get().playSelect();
            }
            return GameState::COUNT;
        }

        if (ev == InputEvent::BTN_A_CLICK) {
            _sel = (_sel + 1) % 2;
            AudioManager::get().playBeep(500, 30);
        }
        if (ev == InputEvent::BTN_B_CLICK) {
            if (_sel == 0) {
                // NEW GAME — ask for confirmation
                _confirm = true;
                _ticker  = 0;
                AudioManager::get().playBeep(300, 60);
            } else {
                // BACK
                AudioManager::get().playSelect();
                requestTransition(GameState::MAIN);
            }
        }
        if (ev == InputEvent::BTN_B_HOLD) {
            AudioManager::get().playSelect();
            requestTransition(GameState::MAIN);
        }
        return GameState::COUNT;
    }

private:
    int  _sel     = 0;
    bool _confirm = false;
    int  _ticker  = 0;

    void _drawMenu(Renderer& R) {
        // Days alive + current EVA info
        const EvaStats& st = EvaPet::get().getStats();
        char buf[48];

        R.canvas().setTextSize(1);
        R.canvas().setTextColor(Colors::TEXT_DIM);
        R.canvas().setCursor(8, 20);
        snprintf(buf, sizeof(buf), "ACTIVE UNIT : %s",
                 EVA_PERSONALITIES[st.evaUnit].name);
        R.canvas().print(buf);

        R.canvas().setCursor(8, 32);
        snprintf(buf, sizeof(buf), "DAYS ALIVE  : %d", st.daysAlive);
        R.canvas().print(buf);

        R.canvas().drawFastHLine(8, 44, 224, Colors::TEXT_DIM);

        // Options
        const char* opts[] = { "NEW GAME  (reset all)", "BACK" };
        const uint32_t optCols[] = { Colors::NERV_RED, Colors::NERV_GREEN };

        for (int i = 0; i < 2; i++) {
            bool sel = (i == _sel);
            int  oy  = 52 + i * 22;

            if (sel) {
                R.canvas().fillRect(6, oy - 2, Display::W - 12, 18, 0x1A0010);
                R.canvas().drawRect(6, oy - 2, Display::W - 12, 18,
                                    optCols[i]);
            }

            R.canvas().setTextColor(sel ? optCols[i] : Colors::TEXT_DIM);
            R.canvas().setTextSize(sel ? 2 : 1);
            R.canvas().setCursor(14, oy + (sel ? 1 : 4));
            R.canvas().print(opts[i]);
        }

        R.canvas().setTextColor(Colors::TEXT_DIM);
        R.canvas().setTextSize(1);
        R.canvas().setCursor(8, 100);
        R.canvas().print("WARNING: NEW GAME erases all data!");

        const char* labels[] = { "A:CYCLE", "B:SELECT", "B+:BACK" };
        R.drawFooter(labels, 3, 255);
    }

    void _drawConfirm(Renderer& R) {
        bool blink = (_ticker / 15) % 2 == 0;

        R.canvas().fillRect(14, 22, Display::W - 28, 74, 0x100008);
        R.canvas().drawRect(14, 22, Display::W - 28, 74, Colors::NERV_RED);

        R.drawTextCentered("!! WARNING !!", 28, Colors::NERV_RED, 1);
        R.canvas().drawFastHLine(16, 38, Display::W - 32, Colors::NERV_RED);

        R.drawTextCentered("Delete save and", 44, Colors::WHITE, 1);
        R.drawTextCentered("choose a new EVA?", 56, Colors::WHITE, 1);

        if (blink) {
            R.drawTextCentered("THIS CANNOT BE UNDONE", 70, Colors::NERV_RED, 1);
        }

        R.canvas().drawFastHLine(16, 82, Display::W - 32, Colors::TEXT_DIM);
        R.drawTextCentered("[A] CANCEL    [B] CONFIRM", 88, Colors::TEXT_DIM, 1);
    }
};

#pragma once
// ============================================================
// SysScene — System settings terminal
//
// 3 options: UNIT CHANGE / SOUND TOGGLE / SYSTEM RESET
// BTN_A: cycle options
// BTN_B: execute
//
// UNIT sub-state: BTN_A cycle units, BTN_B confirm, BTN_B_HOLD cancel
// RESET sub-state: double BTN_B confirm with warning
// ============================================================
#include "ui/SceneManager.h"
#include "entities/EvaPet.h"
#include "core/SaveManager.h"
#include "ui/Renderer.h"
#include "audio/AudioManager.h"
#include "data/EvaData.h"

class SysScene : public BaseScene {
public:
    static SysScene& get() { static SysScene s; return s; }

    void onEnter() override {
        _state   = State::MENU;
        _sel     = 0;
        _unitSel = EvaPet::get().getStats().evaUnit;
        _ticker  = 0;
        _resetConfirm = false;
    }

    void update() override {
        _ticker++;
    }

    void render() override {
        Renderer& R = Renderer::get();
        const EvaStats& st = EvaPet::get().getStats();
        uint32_t col = EVA_PERSONALITIES[st.evaUnit].primaryColor;

        R.clearCanvas(Colors::BG_DARK);
        _drawHeader(R, col);

        switch (_state) {
            case State::MENU:        _drawMenu(R, st, col);       break;
            case State::UNIT_SELECT: _drawUnitSelect(R, st, col); break;
            case State::RESET_WARN:  _drawResetWarn(R, col);      break;
        }

        _drawFooter(R, col);
        R.flush();
    }

    GameState handleInput(InputEvent ev) override {
        switch (_state) {
            case State::MENU:        return _handleMenu(ev);
            case State::UNIT_SELECT: return _handleUnit(ev);
            case State::RESET_WARN:  return _handleReset(ev);
        }
        return GameState::COUNT;
    }

private:
    enum class State { MENU, UNIT_SELECT, RESET_WARN };
    static constexpr int OPTION_COUNT = 4;
    static constexpr const char* OPTIONS[OPTION_COUNT] = {
        "UNIT CHANGE", "SOUND MODE", "STANDBY", "SYSTEM RESET"
    };

    State    _state        = State::MENU;
    int      _sel          = 0;
    int      _unitSel      = 0;
    int      _ticker       = 0;
    bool     _resetConfirm = false;

    void _drawHeader(Renderer& R, uint32_t col) {
        R.canvas().fillRect(0, 0, Display::W, 13, 0x060810);
        R.canvas().drawFastHLine(0, 13, Display::W, col);
        R.canvas().setTextColor(col);
        R.canvas().setTextSize(1);
        R.canvas().setCursor(4, 3);
        R.canvas().print("NERV  SYS  TERMINAL");
    }

    void _drawFooter(Renderer& R, uint32_t col) {
        R.canvas().fillRect(0, Display::H - 14, Display::W, 14, 0x060810);
        R.canvas().drawFastHLine(0, Display::H - 14, Display::W, col);
        R.canvas().setTextColor(Colors::NERV_CYAN);
        R.canvas().setTextSize(1);
        R.canvas().setCursor(4, Display::H - 10);

        if (_state == State::MENU) {
            R.canvas().print("[A] CYCLE  [B] EXECUTE  [B+] EXIT");
        } else if (_state == State::UNIT_SELECT) {
            R.canvas().print("[A] UNIT   [B] CONFIRM  [B+] CANCEL");
        } else {
            R.canvas().print("[A] CANCEL             [B] ERASE");
        }
    }

    void _drawMenu(Renderer& R, const EvaStats& st, uint32_t col) {
        int y = 20;
        for (int i = 0; i < OPTION_COUNT; i++) {
            bool sel = (i == _sel);
            uint32_t bg = sel ? (col & 0x1F1F1F) : 0x0A0A12;
            uint32_t fc = sel ? col : Colors::TEXT_DIM;

            R.canvas().fillRect(6, y, Display::W - 12, 20, bg);
            if (sel) R.canvas().drawRect(6, y, Display::W - 12, 20, col);

            R.canvas().setTextColor(fc);
            R.canvas().setTextSize(1);
            R.canvas().setCursor(14, y + 6);
            R.canvas().print(OPTIONS[i]);

            if (i == 0) {
                R.canvas().setTextColor(Colors::TEXT_DIM);
                R.canvas().setCursor(14 + strlen(OPTIONS[i]) * 6 + 4, y + 6);
                R.canvas().print(EVA_PERSONALITIES[st.evaUnit].name);
            }
            if (i == 1) {
                const char* modeStr = (st.soundMode == 2) ? "[ALL]" :
                                      (st.soundMode == 1) ? "[ALRT]" : "[OFF]";
                uint32_t modeCol = (st.soundMode == 2) ? Colors::NERV_GREEN :
                                   (st.soundMode == 1) ? Colors::NERV_YELLOW
                                                       : Colors::NERV_RED;
                R.canvas().setTextColor(modeCol);
                R.canvas().setCursor(14 + strlen(OPTIONS[i]) * 6 + 4, y + 6);
                R.canvas().print(modeStr);
            }
            y += 22;
        }
    }

    void _drawUnitSelect(Renderer& R, const EvaStats& st, uint32_t col) {
        R.canvas().setTextColor(Colors::TEXT_DIM);
        R.canvas().setTextSize(1);
        R.canvas().setCursor(6, 18);
        R.canvas().print("SELECT EVANGELION UNIT");

        int16_t cardW = Display::W / 4;
        for (uint8_t i = 0; i < 4; i++) {
            bool locked = (i == 3 && !st.mark06Unlocked);
            bool sel    = (i == _unitSel);
            int16_t cx  = i * cardW;

            uint32_t bdr = sel ? EVA_PERSONALITIES[i].primaryColor : Colors::TEXT_DIM;
            R.canvas().drawRect(cx + 2, 28, cardW - 4, 72, bdr);

            if (locked) {
                R.canvas().setTextColor(Colors::TEXT_DIM);
                R.canvas().setTextSize(1);
                R.canvas().setCursor(cx + 8, 60);
                R.canvas().print("LOCK");
                continue;
            }

            R.canvas().setTextColor(sel ? EVA_PERSONALITIES[i].primaryColor : Colors::TEXT_DIM);
            R.canvas().setTextSize(1);
            R.canvas().setCursor(cx + 4, 48);
            R.canvas().print(EVA_PERSONALITIES[i].name);

            char pilot[6];
            strncpy(pilot, EVA_PERSONALITIES[i].pilot, 5);
            pilot[5] = '\0';
            R.canvas().setTextColor(Colors::TEXT_DIM);
            R.canvas().setCursor(cx + 4, 60);
            R.canvas().print(pilot);
        }
    }

    void _drawResetWarn(Renderer& R, uint32_t col) {
        bool blink = (_ticker / 6) % 2 == 0;
        uint32_t warnCol = blink ? Colors::NERV_RED : Colors::WHITE;

        R.drawTextCentered("!! WARNING !!", 22, warnCol, 2);
        R.drawTextCentered("ERASE ALL SAVE DATA?", 46, Colors::NERV_YELLOW, 1);
        R.drawTextCentered("UNIT STATUS LOST.", 58, Colors::TEXT_DIM, 1);
        R.drawTextCentered("HIGH SCORE RETAINED.", 70, Colors::TEXT_DIM, 1);

        if (!_resetConfirm) {
            R.drawTextCentered("[B] TO CONFIRM ERASE", 88, Colors::NERV_RED, 1);
            R.drawTextCentered("[A] ABORT", 100, Colors::NERV_GREEN, 1);
        } else {
            R.drawTextCentered("ERASING...", 92, blink ? Colors::NERV_RED : Colors::TEXT_DIM, 2);
        }
    }

    GameState _handleMenu(InputEvent ev) {
        if (ev == InputEvent::BTN_A_CLICK) {
            _sel = (_sel + 1) % OPTION_COUNT;
            AudioManager::get().playBeep(500, 35);
        }
        if (ev == InputEvent::BTN_B_CLICK) {
            AudioManager::get().playSelect();
            switch (_sel) {
                case 0:  // UNIT CHANGE
                    _state = State::UNIT_SELECT;
                    _unitSel = EvaPet::get().getStats().evaUnit;
                    break;
                case 1: {  // SOUND MODE cycle: ALL(2)→ALERTS(1)→OFF(0)→ALL(2)
                    EvaStats& s = EvaPet::get().getStatsMut();
                    s.soundMode = (s.soundMode == 0) ? 2 : s.soundMode - 1;
                    AudioManager::get().setSoundMode(s.soundMode);
                    SaveManager::get().save(s);
                    if (s.soundMode > 0) AudioManager::get().playBeep(440, 80);
                    break;
                }
                case 2:  // STANDBY
                    return GameState::SHUTDOWN;
                case 3:  // RESET
                    _state = State::RESET_WARN;
                    _resetConfirm = false;
                    break;
            }
        }
        if (ev == InputEvent::BTN_B_HOLD) {
            return GameState::MAIN;
        }
        return GameState::COUNT;
    }

    GameState _handleUnit(InputEvent ev) {
        if (ev == InputEvent::BTN_A_CLICK) {
            uint8_t maxSel = EvaPet::get().getStats().mark06Unlocked ? 3 : 2;
            _unitSel = (_unitSel + 1) % (maxSel + 1);
            AudioManager::get().playBeep(500, 35);
        }
        if (ev == InputEvent::BTN_B_CLICK) {
            // Change unit only — keep all stats
            EvaStats& s = EvaPet::get().getStatsMut();
            s.evaUnit    = _unitSel;
            s.stability  = (int16_t)EVA_PERSONALITIES[_unitSel].stabilityBase;
            SaveManager::get().save(s);
            AudioManager::get().playSelect();
            _state = State::MENU;
        }
        if (ev == InputEvent::BTN_B_HOLD) {
            _state = State::MENU;
            AudioManager::get().playBeep(300, 50);
        }
        return GameState::COUNT;
    }

    GameState _handleReset(InputEvent ev) {
        if (ev == InputEvent::BTN_A_CLICK) {
            _state = State::MENU;
            AudioManager::get().playBeep(400, 50);
        }
        if (ev == InputEvent::BTN_B_CLICK) {
            if (!_resetConfirm) {
                _resetConfirm = true;  // first press: arm
                AudioManager::get().playGlitch();
            } else {
                // second press: execute
                EvaPet::get().resetCoreBreach();
                SaveManager::get().save(EvaPet::get().getStats());
                AudioManager::get().playBeep(200, 200);
                return GameState::SPLASH;
            }
        }
        return GameState::COUNT;
    }
};

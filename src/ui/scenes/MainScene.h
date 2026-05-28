#pragma once
// ============================================================
// EVASELECTSCENE.H + MAINSCENE.H
// ============================================================
#include "ui/SceneManager.h"
#include "core/SaveManager.h"
#include "data/Sprites.h"

// ─── EVA SELECTION ───────────────────────────────────────
class EvaSelectScene : public BaseScene {
public:
    static EvaSelectScene& get() { static EvaSelectScene s; return s; }

    // When true: only change evaUnit, don't wipe stats (SysScene unit change)
    bool unitChangeOnly = false;

    void onEnter() override {
        _selected     = EvaPet::get().getStats().evaUnit;
        _confirm      = false;
        _confirmTimer = 0;
        AudioManager::get().playSelect();
    }

    void update() override {
        if (!_confirm) return;
        if (millis() - _confirmTimer < 1000) return;

        EvaStats& s = EvaPet::get().getStatsMut();

        if (unitChangeOnly) {
            s.evaUnit      = _selected;
            s.stability    = (int16_t)EVA_PERSONALITIES[_selected].stabilityBase;
            unitChangeOnly = false;
            SaveManager::get().save(s);
            requestTransition(GameState::MAIN);
            return;
        }

        s.evaUnit           = _selected;
        s.unitSelected      = true;
        s.syncRate          = 40 + random(0, 20);
        s.hunger            = 80;
        s.stability         = (int16_t)EVA_PERSONALITIES[_selected].stabilityBase;
        s.energy            = 90;
        s.happiness         = 70;
        s.affection         = 50;
        s.rage              = 0;
        s.contamination     = 0;
        s.daysAlive         = 0;
        s.berserk           = false;
        s.sleeping          = false;
        s.angelMode         = false;
        s.soundMode         = 2;  // ALL
        s.lastSaveTimestamp = millis() / 1000;

        SaveManager::get().save(s);
        EvaPet::get().begin(s);
        AudioManager::get().setSoundMode(2);
        requestTransition(GameState::MAIN);
    }

    void render() override {
        Renderer& r = Renderer::get();
        r.clearCanvas(Colors::BG_DARK);

        r.drawTextCentered("SELECT  EVANGELION  UNIT", 2, Colors::NERV_RED, 1);
        r.canvas().drawFastHLine(0, 12, Display::W, Colors::NERV_RED);

        int16_t cardW = Display::W / 4;

        for (uint8_t i = 0; i < 4; i++) {
            bool locked = (i == 3 && !EvaPet::get().getStats().mark06Unlocked);
            bool sel    = (i == _selected && !locked);
            int16_t cx  = i * cardW;

            uint32_t bg  = sel ? (EVA_PERSONALITIES[i].primaryColor & 0x1F1F1F) : 0x080810;
            uint32_t bdr = sel ? EVA_PERSONALITIES[i].primaryColor : Colors::TEXT_DIM;

            r.canvas().fillRect(cx + 1, 13, cardW - 2, 100, bg);
            r.canvas().drawRect(cx + 1, 13, cardW - 2, 100, bdr);

            if (locked) {
                r.canvas().setTextColor(Colors::TEXT_DIM);
                r.canvas().setTextSize(1);
                r.canvas().setCursor(cx + 8, 58);
                r.canvas().print("????");
                r.canvas().setCursor(cx + 6, 70);
                r.canvas().print("LOCKED");
                continue;
            }

            AnimationSystem& anim = AnimationSystem::get();
            const uint8_t* frame  = anim.getCurrentFrame(i, AnimState::IDLE);
            anim.drawSprite(r.canvas(), frame, cx + (cardW - 32) / 2, 17, i, 2);

            r.canvas().setTextColor(EVA_PERSONALITIES[i].primaryColor);
            r.canvas().setTextSize(1);
            r.canvas().setCursor(cx + 2, 52);
            r.canvas().print(EVA_PERSONALITIES[i].name);

            char pilot[10];
            strncpy(pilot, EVA_PERSONALITIES[i].pilot, 9);
            pilot[9] = '\0';
            for (int c = 0; c < 9; c++) { if (pilot[c] == ' ') { pilot[c] = '\0'; break; } }
            r.canvas().setTextColor(Colors::TEXT_DIM);
            r.canvas().setCursor(cx + 2, 63);
            r.canvas().print(pilot);

            r.canvas().setTextColor(Colors::TEXT_DIM);
            r.canvas().setCursor(cx + 2, 84);
            r.canvas().print(EVA_PERSONALITIES[i].designator);
        }

        if (!_confirm) {
            const int16_t fy = Display::H - 16;
            r.canvas().fillRect(0, fy, Display::W, 16, 0x060810);
            r.canvas().drawFastHLine(0, fy, Display::W, Colors::NERV_RED);
            r.canvas().setTextColor(Colors::NERV_CYAN);
            r.canvas().setTextSize(1);
            r.canvas().setCursor(8, fy + 4);
            r.canvas().print("[A] NEXT UNIT");
            r.canvas().setTextColor(Colors::NERV_GREEN);
            r.canvas().setCursor(148, fy + 4);
            r.canvas().print("[B] CHOOSE");
        } else {
            char buf[32];
            snprintf(buf, sizeof(buf), "STARTING %s...", EVA_PERSONALITIES[_selected].name);
            r.drawTextCentered(buf, 117, Colors::NERV_GREEN, 1);
        }

        r.drawCornerMarkers(Colors::NERV_RED);
        r.flush();
    }

    GameState handleInput(InputEvent ev) override {
        if (_confirm) return GameState::COUNT;
        uint8_t maxSel = EvaPet::get().getStats().mark06Unlocked ? 3 : 2;
        if (ev == InputEvent::BTN_A_CLICK) {
            _selected = (_selected + 1) % (maxSel + 1);
            AudioManager::get().playBeep(500, 50);
        }
        if (ev == InputEvent::BTN_B_CLICK) {
            _confirm      = true;
            _confirmTimer = millis();
            AudioManager::get().playSelect();
        }
        return GameState::COUNT;
    }

private:
    uint8_t  _selected     = 0;
    bool     _confirm      = false;
    uint32_t _confirmTimer = 0;
};

// ─── MAIN SCENE ──────────────────────────────────────────
// Layout (240 × 135):
//   Header 14px : unit name · day · alert icons
//   Content 97px: sprite (80px left) | 4 stat bars (right)
//   Footer 24px : [A] ◄ ACTION ► [B]
//
// 5 actions: FEED · FIGHT · REST · DATA · SYS
// Alert icons blink in header when stats critical.
// BTN_A jumps to priority alert action if one exists.
// ─────────────────────────────────────────────────────────
class MainScene : public BaseScene {
public:
    static MainScene& get() { static MainScene s; return s; }

    void onEnter() override {
        _menuSel          = _priorityMenuSel();
        _wasBerserk       = false;
        _ticker           = 0;
        _angelInterrupt   = false;
        _interruptStart   = 0;
    }

    void update() override {
        _ticker++;
        if (_overlayTicks > 0) _overlayTicks--;

        const EvaStats& st = EvaPet::get().getStats();

        // Angel interrupt: 1s overlay before transitioning to alarm scene
        if (_angelInterrupt) {
            if (millis() - _interruptStart >= 1000) {
                _angelInterrupt = false;
                requestTransition(GameState::ANGEL_ALARM);
            }
            return;
        }

        if (st.berserk && !_wasBerserk) {
            _wasBerserk = true;
            requestTransition(GameState::BERSERK);
            return;
        }
        _wasBerserk = st.berserk;

        if (EvaPet::get().isCoreBreach()) {
            requestTransition(GameState::CORE_BREACH);
            return;
        }
        if (EvaPet::get().isAngelAlert() && !_angelInterrupt) {
            _angelInterrupt = true;
            _interruptStart = millis();
        }
    }

    void render() override {
        Renderer& r       = Renderer::get();
        const EvaStats& st = EvaPet::get().getStats();
        uint8_t  unit     = st.evaUnit;
        uint32_t col      = EVA_PERSONALITIES[unit].primaryColor;
        bool     blink    = (_ticker / 8) % 2 == 0;

        r.clearCanvas(Colors::BG_DARK);
        r.drawHeader(EVA_PERSONALITIES[unit].name, st);
        _drawAlertIcons(r, st, blink);
        r.drawEVASprite(st);
        _drawStats(r, st, blink);
        r.drawNERVFrame(col);
        _drawFooter(r, st);

        // Angel interrupt overlay: red scanlines + flashing border
        if (_angelInterrupt) {
            for (int16_t y = 0; y < Display::H; y += 2) {
                r.canvas().drawFastHLine(0, y, Display::W, 0x1A0000);
            }
            if ((_ticker / 4) % 2 == 0) {
                r.canvas().drawRect(0, 0, Display::W, Display::H, Colors::NERV_RED);
                r.canvas().drawRect(1, 1, Display::W - 2, Display::H - 2,
                                    Colors::NERV_RED_DIM);
            }
        }

        // Execution-blocked overlay (90 ticks)
        if (_overlayTicks > 0 && _overlayMsg) {
            r.canvas().fillRect(60, 58, 120, 18, Colors::BLACK);
            r.canvas().drawRect(60, 58, 120, 18, Colors::NERV_AMBER);
            r.canvas().setTextColor(Colors::NERV_AMBER);
            r.canvas().setTextSize(1);
            int16_t tw = strlen(_overlayMsg) * 6;
            r.canvas().setCursor(60 + (120 - tw) / 2, 63);
            r.canvas().print(_overlayMsg);
        }

        r.flush();
    }

    GameState handleInput(InputEvent ev) override {
        if (_angelInterrupt) return GameState::COUNT;  // block input during alarm
        if (ev == InputEvent::BTN_A_CLICK) {
            _menuSel = (_menuSel + 1) % MENU_COUNT;
            AudioManager::get().playBeep(500, 35);
        }
        if (ev == InputEvent::BTN_B_CLICK) {
            return _executeAction();
        }
        if (ev == InputEvent::BTN_B_HOLD) {
            AudioManager::get().playSelect();
            return GameState::QUOTE;
        }
        return GameState::COUNT;
    }

private:
    static constexpr int MENU_COUNT = 6;
    static constexpr const char* MENU_LABELS[MENU_COUNT] = {
        "FEED", "FIGHT", "MIND", "REST", "DATA", "SYS"
    };

    int         _menuSel        = 0;
    bool        _wasBerserk     = false;
    int         _ticker         = 0;
    bool        _angelInterrupt = false;
    uint32_t    _interruptStart = 0;
    int         _overlayTicks   = 0;
    const char* _overlayMsg     = nullptr;

    int _priorityMenuSel() {
        const EvaStats& st = EvaPet::get().getStats();
        if (st.hunger < 25) return 0;
        if (st.energy < 10) return 3;  // REST at index 3
        if (st.rage   > 70) return 1;
        return 0;
    }

    GameState _executeAction() {
        EvaPet& pet = EvaPet::get();
        switch (_menuSel) {
            case 0:  // FEED
                if (!pet.canFeed()) {
                    _showOverlay("NOT HUNGRY");
                    return GameState::COUNT;
                }
                AudioManager::get().playSelect();
                return GameState::FEED;
            case 1:  // FIGHT
                if (!pet.canFight()) {
                    _showOverlay("PILOT EXHAUSTED");
                    return GameState::COUNT;
                }
                AudioManager::get().playSelect();
                return GameState::FIGHT;
            case 2:  // MIND
                if (!pet.canMind()) {
                    _showOverlay("SYNC AT PEAK");
                    return GameState::COUNT;
                }
                AudioManager::get().playSelect();
                return GameState::MIND;
            case 3:  // REST
                if (!pet.canRest()) {
                    _showOverlay("NOT TIRED");
                    return GameState::COUNT;
                }
                AudioManager::get().playSelect();
                return GameState::SLEEP;
            case 4:  // DATA
                AudioManager::get().playSelect();
                return GameState::DATA;
            case 5:  // SYS
                AudioManager::get().playSelect();
                return GameState::SYS;
        }
        return GameState::COUNT;
    }

    void _showOverlay(const char* msg) {
        _overlayMsg   = msg;
        _overlayTicks = 90;
        AudioManager::get().playBeep(200, 80);
    }

    // ── Alert icons in header ─────────────────────────────
    void _drawAlertIcons(Renderer& r, const EvaStats& st, bool blink) {
        if (!blink) return;
        int16_t ix = Display::W - 2;
        auto _icon = [&](const char* lbl, uint32_t bg) {
            ix -= 9;
            r.canvas().fillRect(ix, 2, 8, 10, bg);
            r.canvas().setTextColor(Colors::BLACK);
            r.canvas().setTextSize(1);
            r.canvas().setCursor(ix + 1, 3);
            r.canvas().print(lbl);
        };
        if (st.syncRate < 20)  _icon("!", Colors::NERV_RED);
        if (st.rage     > 70)  _icon("R", Colors::NERV_RED);
        if (st.energy   < 10)  _icon("E", Colors::NERV_YELLOW);
        if (st.hunger   < 25)  _icon("F", Colors::NERV_ORANGE);
    }

    // ── Stats panel ───────────────────────────────────────
    void _drawStats(Renderer& r, const EvaStats& st, bool blink) {
        const int BX  = Display::SPRITE_W + 4;
        const int BW  = Display::W - BX - 4;
        const int BH  = 9;
        const int GAP = 14;
        const int BY  = Display::HEADER_H + 5;

        uint32_t syncCol = (st.syncRate < 30) ? Colors::NERV_RED
                         : (st.syncRate < 55) ? Colors::NERV_ORANGE
                                              : Colors::NERV_GREEN;
        r.drawStatBar(BX, BY,        BW, BH, st.syncRate, syncCol, "SYNC");

        uint32_t hgrCol  = (st.hunger < 25) ? Colors::NERV_RED
                         : (st.hunger < 50) ? Colors::NERV_ORANGE
                                            : Colors::NERV_CYAN;
        r.drawStatBar(BX, BY+GAP,   BW, BH, st.hunger,   hgrCol,  "FOOD");

        uint32_t enrgCol = (st.energy < 20) ? Colors::NERV_RED
                         : (st.energy < 45) ? Colors::NERV_ORANGE
                                            : Colors::NERV_BLUE;
        r.drawStatBar(BX, BY+GAP*2, BW, BH, st.energy,   enrgCol, "ENRG");

        uint32_t rageCol = (st.rage > 70) ? Colors::NERV_RED
                         : (st.rage > 45) ? Colors::NERV_ORANGE
                                          : Colors::NERV_GREEN_DIM;
        r.drawStatBar(BX, BY+GAP*3, BW, BH, st.rage,     rageCol, "RAGE");

        // Alert text row (below bars)
        int alertY = BY + GAP * 4 + 2;
        if (st.hunger < 25 && blink) {
            r.canvas().setTextColor(Colors::NERV_RED);
            r.canvas().setTextSize(1);
            r.canvas().setCursor(BX, alertY);
            r.canvas().print(">> PILOT: HUNGRY!");
            alertY += 10;
        }
        if (st.energy < 10 && blink) {
            r.canvas().setTextColor(Colors::NERV_YELLOW);
            r.canvas().setTextSize(1);
            r.canvas().setCursor(BX, alertY);
            r.canvas().print(">> UNIT: CRITICAL!");
            alertY += 10;
        }
        if (st.rage > 70 && blink) {
            r.canvas().setTextColor(Colors::NERV_RED);
            r.canvas().setTextSize(1);
            r.canvas().setCursor(BX, alertY);
            r.canvas().print(">> BERSERK RISK!");
        }
    }

    // ── Footer: 5-slot tab bar always visible ────────────
    void _drawFooter(Renderer& r, const EvaStats& st) {
        const int16_t fy  = Display::H - Display::FOOTER_H;
        const int16_t fh  = Display::FOOTER_H;
        uint32_t col      = EVA_PERSONALITIES[st.evaUnit].primaryColor;

        r.canvas().fillRect(0, fy, Display::W, fh, 0x060810);
        r.canvas().drawFastHLine(0, fy, Display::W, col);

        EvaPet& pet = EvaPet::get();

        int16_t slotW = Display::W / MENU_COUNT;   // 40px
        int16_t midY  = fy + (fh - 8) / 2;

        for (int i = 0; i < MENU_COUNT; i++) {
            int16_t sx = i * slotW;
            bool sel = (i == _menuSel);

            bool available = true;
            if (i == 0) available = pet.canFeed();
            if (i == 1) available = pet.canFight();
            if (i == 2) available = pet.canMind();
            if (i == 3) available = pet.canRest();

            if (sel) {
                r.canvas().fillRect(sx + 1, fy + 2, slotW - 2, fh - 4,
                                    col & 0x1A1A1A);
                r.canvas().drawRect(sx, fy, slotW, fh, col);
            } else {
                r.canvas().drawRect(sx, fy, slotW, fh, 0x111122);
            }

            uint32_t fc = sel ? col
                        : (!available ? Colors::NERV_RED_DIM : Colors::TEXT_DIM);
            r.canvas().setTextColor(fc);
            r.canvas().setTextSize(1);
            int16_t tw = strlen(MENU_LABELS[i]) * 6;
            r.canvas().setCursor(sx + (slotW - tw) / 2, midY);
            r.canvas().print(MENU_LABELS[i]);
        }
    }
};

#pragma once
#include "ui/SceneManager.h"
#include "core/Types.h"
#include "entities/EvaPet.h"
#include "ui/Renderer.h"
#include "audio/AudioManager.h"
#include "core/TimeManager.h"
#include "data/EvaData.h"

// ============================================================
// StatusScene — full stat readout, NERV terminal style
// ============================================================
class StatusScene : public BaseScene {
public:
    static StatusScene& get() { static StatusScene s; return s; }

    void onEnter() override {
        _page   = 0;
        _ticker = 0;
    }

    void onExit() override {}

    void update() override { _ticker++; }

    void render() override {
        auto& R  = Renderer::get();
        auto& pet = EvaPet::get();
        const EvaStats& st = pet.getStats();

        R.clearCanvas(Colors::BG_DARK);
        R.drawNERVFrame(EVA_PERSONALITIES[st.evaUnit].primaryColor);

        switch (_page) {
            case 0: _drawStatsPage(R, st);   break;
            case 1: _drawProfilePage(R, st); break;
            case 2: _drawDiagPage(R, st);    break;
        }

        R.flush();
    }

    GameState handleInput(InputEvent ev) override {
        switch (ev) {
            case InputEvent::BTN_A_CLICK:
                _page = (_page + 1) % 3;
                AudioManager::get().playBeep(600, 30);
                break;
            case InputEvent::BTN_B_CLICK:
            case InputEvent::BTN_B_HOLD:
                AudioManager::get().playSelect();
                requestTransition(GameState::MAIN);
                break;
            default: break;
        }
        return GameState::COUNT;
    }

private:
    int _page   = 0;
    int _ticker = 0;

    // ─── PAGE 0: Stats bars ───────────────────────────────
    void _drawStatsPage(Renderer& R, const EvaStats& st) {
        R.drawHeader("[ UNIT STATUS ]", st);
        R.drawEVASprite(st);

        // 6 bars fit cleanly with GAP=13, BH=10, start BY=18
        // Last bar ends at 18 + 5*13 + 10 = 93. Footer at 111. Fine.
        const int BX = 84, BY = 18, BW = 148, BH = 10, GAP = 13;

        struct SE { const char* n; int v; uint32_t c; };
        SE entries[] = {
            { "SYNC",  st.syncRate,   Colors::NERV_CYAN   },
            { "FOOD",  st.hunger,     Colors::NERV_GREEN  },
            { "STAB",  st.stability,  Colors::NERV_CYAN   },
            { "ENRG",  st.energy,     Colors::NERV_BLUE   },
            { "AFTN",  st.affection,  Colors::NERV_CYAN   },
            { "HPNS",  st.happiness,  Colors::NERV_GREEN  },
        };
        for (int i = 0; i < 6; i++) {
            R.drawStatBar(BX, BY + i * GAP, BW, BH,
                          entries[i].v, entries[i].c, entries[i].n);
        }

        // RAGE and CONT as compact text line below bars (fits at y=97)
        char warningBuf[32];
        int wy = BY + 6 * GAP + 4;  // = 18 + 78 + 4 = 100 → well above footer
        uint32_t rageCol = st.rage > 70 ? Colors::NERV_RED : Colors::NERV_GREEN_DIM;
        uint32_t contCol = st.contamination > 60 ? Colors::NERV_RED : Colors::NERV_YELLOW;

        snprintf(warningBuf, sizeof(warningBuf), "RAGE %3d%%", st.rage);
        R.canvas().setTextSize(1);
        R.canvas().setTextColor(rageCol);
        R.canvas().setCursor(BX, wy);
        R.canvas().print(warningBuf);

        snprintf(warningBuf, sizeof(warningBuf), "CONT %3d%%", st.contamination);
        R.canvas().setTextColor(contCol);
        R.canvas().setCursor(BX + 76, wy);
        R.canvas().print(warningBuf);

        const char* labels[] = { "A:PAGE", "B:BACK" };
        R.drawFooter(labels, 2, 255);
    }

    // ─── PAGE 1: Profile ──────────────────────────────────
    void _drawProfilePage(Renderer& R, const EvaStats& st) {
        char buf[48];
        R.drawHeader("[ PILOT PROFILE ]", st);

        const EvaPersonality& p = EVA_PERSONALITIES[static_cast<int>(st.evaUnit)];

        // Unit name — size 2 for readability
        R.canvas().setTextColor(p.accentColor);
        R.canvas().setTextSize(2);
        R.canvas().setCursor(8, 18);
        R.canvas().print(p.name);

        R.canvas().setTextSize(1);
        R.canvas().setTextColor(Colors::TEXT_DIM);
        R.canvas().setCursor(8, 36);
        R.canvas().print(p.pilot);

        R.canvas().drawFastHLine(8, 46, 224, Colors::TEXT_DIM);

        const char* evolNames[] = {
            "PROTOTYPE", "OPERATIONAL", "SYNCHRONIZED",
            "BERSERK",   "ANGEL HYBRID", "PERFECT SYNC"
        };
        snprintf(buf, sizeof(buf), "STAGE  : %s",
                 evolNames[st.evolutionStage < 6 ? st.evolutionStage : 0]);
        R.canvas().setTextColor(Colors::NERV_CYAN);
        R.canvas().setCursor(8, 50);
        R.canvas().print(buf);

        snprintf(buf, sizeof(buf), "DAYS   : %d", st.daysAlive);
        R.canvas().setTextColor(Colors::NERV_GREEN);
        R.canvas().setCursor(8, 62);
        R.canvas().print(buf);

        snprintf(buf, sizeof(buf), "SYNC   : %d%%", st.syncRate);
        R.canvas().setTextColor(Colors::NERV_CYAN);
        R.canvas().setCursor(8, 74);
        R.canvas().print(buf);

        int fy = 88;
        if (st.berserk) {
            R.canvas().setTextColor(Colors::NERV_RED);
            R.canvas().setCursor(8, fy); R.canvas().print(">> BERSERK MODE ACTIVE");
            fy += 12;
        }
        if (st.angelMode) {
            R.canvas().setTextColor(Colors::NERV_YELLOW);
            R.canvas().setCursor(8, fy); R.canvas().print(">> ANGEL CONTAMINATION");
            fy += 12;
        }
        if (st.sleeping) {
            R.canvas().setTextColor(Colors::NERV_BLUE);
            R.canvas().setCursor(8, fy); R.canvas().print(">> STANDBY / SLEEP");
        }

        const char* labels[] = { "A:PAGE", "B:BACK" };
        R.drawFooter(labels, 2, 255);
    }

    // ─── PAGE 2: System diagnostics ──────────────────────
    void _drawDiagPage(Renderer& R, const EvaStats& st) {
        char buf[40];
        R.drawHeader("[ SYS DIAGNOSTICS ]", st);
        R.canvas().setTextSize(1);

        auto row = [&](int y, const char* label, const char* val, uint32_t col) {
            R.canvas().setTextColor(Colors::TEXT_DIM);
            R.canvas().setCursor(8, y);
            R.canvas().print(label);
            R.canvas().setTextColor(col);
            R.canvas().setCursor(110, y);
            R.canvas().print(val);
        };

        snprintf(buf, sizeof(buf), "%d%%", st.syncRate);
        row(18, "SYNC RATIO   :", buf, Colors::NERV_CYAN);

        snprintf(buf, sizeof(buf), "%d%%", st.contamination);
        row(30, "CONTAMINATION:", buf,
            st.contamination > 60 ? Colors::NERV_RED : Colors::NERV_YELLOW);

        snprintf(buf, sizeof(buf), "%d%%", st.rage);
        row(42, "RAGE INDEX   :", buf,
            st.rage > 70 ? Colors::NERV_RED : Colors::NERV_GREEN);

        snprintf(buf, sizeof(buf), "%d%%", st.stability);
        row(54, "STABILITY    :", buf,
            st.stability < 30 ? Colors::NERV_RED : Colors::NERV_CYAN);

        snprintf(buf, sizeof(buf), "%d DAY%s", st.daysAlive, st.daysAlive != 1 ? "S" : "");
        row(66, "UPTIME       :", buf, Colors::NERV_GREEN);

        char clockBuf[8];
        TimeManager::get().getTimeStr(clockBuf, sizeof(clockBuf));
        row(78, "CLOCK        :", clockBuf, Colors::NERV_CYAN);

        bool critical = st.rage > 80 || st.contamination > 80 || st.syncRate < 20;
        bool warning  = st.rage > 60 || st.contamination > 60 || st.syncRate < 40;

        const char* coreStr = critical ? "CRITICAL" : (warning ? "WARNING" : "NOMINAL");
        uint32_t    coreCol = critical ? Colors::NERV_RED
                            : (warning ? Colors::NERV_YELLOW : Colors::NERV_GREEN);

        R.canvas().drawFastHLine(8, 92, 224, Colors::TEXT_DIM);
        row(96, "CORE STATUS  :", coreStr, coreCol);

        if (critical && R.isBlinkOn()) {
            R.canvas().fillRect(108, 94, 124, 10, Colors::NERV_RED);
            R.canvas().setTextColor(Colors::BG_DARK);
            R.canvas().setCursor(110, 96);
            R.canvas().print(coreStr);
        }

        const char* labels[] = { "A:PAGE", "B:BACK" };
        R.drawFooter(labels, 2, 255);
    }
};

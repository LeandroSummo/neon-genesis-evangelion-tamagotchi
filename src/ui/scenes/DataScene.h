#pragma once
// ============================================================
// DataScene — MAGI data terminal, 3 tabs
//
// Tab 0: UNIT STATUS  — 4 stat bars + evolution + days
// Tab 1: PILOT FILE   — designator / pilot / motto + NGE quote
// Tab 2: NERV BROADCAST — typewriter NERV message
//
// BTN_A: cycle tabs
// BTN_B: return to MAIN
// ============================================================
#include "ui/SceneManager.h"
#include "entities/EvaPet.h"
#include "ui/Renderer.h"
#include "audio/AudioManager.h"
#include "data/LoreData.h"
#include "data/EvaData.h"

class DataScene : public BaseScene {
public:
    static DataScene& get() { static DataScene s; return s; }

    void onEnter() override {
        _tab       = 0;
        _ticker    = 0;
        _typedChar = 0;
        _typeTimer = millis();
        _msgIdx    = random(0, NERV_MSG_COUNT);
        _quoteIdx  = random(0, NGE_QUOTE_COUNT);
    }

    void update() override {
        _ticker++;
        // Tab 2 typewriter
        if (_tab == 2) {
            uint32_t now = millis();
            char msgBuf[48];
            strncpy_P(msgBuf, NERV_MESSAGES[_msgIdx], sizeof(msgBuf));
            int msgLen = strlen(msgBuf);
            if (_typedChar < msgLen && now - _typeTimer >= TYPE_SPEED_MS) {
                _typeTimer = now;
                _typedChar++;
            }
            // Advance to next message after hold
            if (_typedChar >= msgLen && now - _typeTimer >= HOLD_MS) {
                _msgIdx    = (_msgIdx + 1) % NERV_MSG_COUNT;
                _typedChar = 0;
                _typeTimer = now;
            }
        }
    }

    void render() override {
        Renderer& R = Renderer::get();
        const EvaStats& st = EvaPet::get().getStats();
        uint32_t col = EVA_PERSONALITIES[st.evaUnit].primaryColor;

        R.clearCanvas(Colors::BG_DARK);

        // Header
        _drawHeader(R, st, col);

        // Tab content
        switch (_tab) {
            case 0: _drawUnitStatus(R, st, col); break;
            case 1: _drawPilotFile(R, st, col);  break;
            case 2: _drawBroadcast(R, col);       break;
        }

        // Footer
        R.canvas().fillRect(0, Display::H - 14, Display::W, 14, 0x060810);
        R.canvas().drawFastHLine(0, Display::H - 14, Display::W, col);
        R.canvas().setTextColor(Colors::NERV_CYAN);
        R.canvas().setTextSize(1);
        R.canvas().setCursor(4, Display::H - 10);
        R.canvas().print("[A] TAB");
        R.canvas().setTextColor(Colors::NERV_GREEN);
        R.canvas().setCursor(Display::W - 70, Display::H - 10);
        R.canvas().print("[B] CLOSE");

        R.flush();
    }

    GameState handleInput(InputEvent ev) override {
        if (ev == InputEvent::BTN_A_CLICK) {
            _tab = (_tab + 1) % 3;
            _typedChar = 0;
            _typeTimer = millis();
            _msgIdx    = random(0, NERV_MSG_COUNT);
            AudioManager::get().playBeep(500, 35);
        }
        if (ev == InputEvent::BTN_B_CLICK || ev == InputEvent::BTN_B_HOLD) {
            AudioManager::get().playSelect();
            return GameState::MAIN;
        }
        return GameState::COUNT;
    }

private:
    static constexpr uint32_t TYPE_SPEED_MS = 55;
    static constexpr uint32_t HOLD_MS       = 2500;

    int      _tab      = 0;
    int      _ticker   = 0;
    int      _msgIdx   = 0;
    int      _quoteIdx = 0;
    int      _typedChar = 0;
    uint32_t _typeTimer = 0;

    void _drawHeader(Renderer& R, const EvaStats& st, uint32_t col) {
        R.canvas().fillRect(0, 0, Display::W, 13, 0x060810);
        R.canvas().drawFastHLine(0, 13, Display::W, col);

        const char* tabs[3] = { "STATUS", "PILOT", "BROADCAST" };
        int16_t tx = 4;
        for (int i = 0; i < 3; i++) {
            bool sel = (i == _tab);
            uint32_t c = sel ? col : Colors::TEXT_DIM;
            if (sel) R.canvas().fillRect(tx - 1, 1, strlen(tabs[i]) * 6 + 2, 10, col & 0x1A1A1A);
            R.canvas().setTextColor(c);
            R.canvas().setTextSize(1);
            R.canvas().setCursor(tx, 3);
            R.canvas().print(tabs[i]);
            tx += strlen(tabs[i]) * 6 + 8;
        }
    }

    void _drawUnitStatus(Renderer& R, const EvaStats& st, uint32_t col) {
        const int BX = 6, BW = 228, BH = 9, GAP = 14, BY = 18;

        uint32_t syncCol = (st.syncRate < 30) ? Colors::NERV_RED
                         : (st.syncRate < 55) ? Colors::NERV_ORANGE
                                              : Colors::NERV_GREEN;
        R.drawStatBar(BX, BY,        BW, BH, st.syncRate, syncCol,          "SYNC");
        uint32_t hCol = (st.hunger < 25) ? Colors::NERV_RED
                      : (st.hunger < 50) ? Colors::NERV_ORANGE : Colors::NERV_CYAN;
        R.drawStatBar(BX, BY+GAP,   BW, BH, st.hunger,   hCol,             "FOOD");
        uint32_t eCol = (st.energy < 20) ? Colors::NERV_RED
                      : (st.energy < 45) ? Colors::NERV_ORANGE : Colors::NERV_BLUE;
        R.drawStatBar(BX, BY+GAP*2, BW, BH, st.energy,   eCol,             "ENRG");
        uint32_t rCol = (st.rage > 70) ? Colors::NERV_RED
                      : (st.rage > 45) ? Colors::NERV_ORANGE : Colors::NERV_GREEN_DIM;
        R.drawStatBar(BX, BY+GAP*3, BW, BH, st.rage,     rCol,             "RAGE");

        // Evolution + days
        char evoBuf[24], dayBuf[24];
        uint8_t evo = st.evolutionStage < 4 ? st.evolutionStage : 0;
        char evoName[16];
        strncpy_P(evoName, EVOLUTION_NAMES[evo], sizeof(evoName));
        snprintf(evoBuf, sizeof(evoBuf), "STAGE: %s", evoName);
        snprintf(dayBuf, sizeof(dayBuf), "DAYS ACTIVE: %u", (unsigned)st.daysAlive);

        R.canvas().setTextColor(col);
        R.canvas().setTextSize(1);
        R.canvas().setCursor(BX, BY + GAP * 4 + 4);
        R.canvas().print(evoBuf);
        R.canvas().setTextColor(Colors::TEXT_DIM);
        R.canvas().setCursor(BX, BY + GAP * 4 + 14);
        R.canvas().print(dayBuf);
    }

    void _drawPilotFile(Renderer& R, const EvaStats& st, uint32_t col) {
        const EvaPersonality& p = EVA_PERSONALITIES[st.evaUnit];
        int y = 18;

        auto _row = [&](const char* label, const char* val, uint32_t vc) {
            R.canvas().setTextColor(Colors::TEXT_DIM);
            R.canvas().setTextSize(1);
            R.canvas().setCursor(6, y);
            R.canvas().print(label);
            R.canvas().setTextColor(vc);
            R.canvas().setCursor(6 + strlen(label) * 6 + 2, y);
            R.canvas().print(val);
            y += 10;
        };

        _row("UNIT:      ", p.name,       col);
        _row("TYPE:      ", p.designator, Colors::TEXT_DIM);
        _row("PILOT:     ", p.pilot,      Colors::WHITE);

        y += 2;
        R.canvas().drawFastHLine(6, y, Display::W - 12, Colors::TEXT_DIM);
        y += 6;

        // Motto
        R.canvas().setTextColor(col);
        R.canvas().setTextSize(1);
        R.canvas().setCursor(6, y);
        R.canvas().print("\"");
        R.canvas().print(p.motto);
        R.canvas().print("\"");
        y += 14;

        // NGE Quote
        R.canvas().drawFastHLine(6, y, Display::W - 12, Colors::TEXT_DIM & 0x101018);
        y += 6;
        R.canvas().setTextColor(Colors::TEXT_DIM);
        R.canvas().setTextSize(1);
        R.canvas().setCursor(6, y);

        // Wrap quote at 36 chars
        const char* qtext = NGE_QUOTES_TEXT[_quoteIdx];
        int qlen = strlen(qtext);
        if (qlen <= 36) {
            R.canvas().print(qtext);
            y += 10;
        } else {
            char line1[37]; strncpy(line1, qtext, 36); line1[36] = '\0';
            for (int i = 35; i > 0; i--) if (line1[i] == ' ') { line1[i] = '\0'; break; }
            R.canvas().print(line1); y += 9;
            R.canvas().setCursor(6, y);
            R.canvas().print(qtext + strlen(line1) + 1); y += 10;
        }
        R.canvas().setTextColor(Colors::TEXT_DIM & 0x505050);
        R.canvas().setCursor(6, y);
        R.canvas().print("— ");
        R.canvas().print(NGE_QUOTES_ATTR[_quoteIdx]);
    }

    void _drawBroadcast(Renderer& R, uint32_t col) {
        // NERV broadcast header
        bool blink = (_ticker / 8) % 2 == 0;
        uint32_t hdrCol = blink ? col : Colors::NERV_RED;
        R.drawTextCentered(">> NERV BROADCAST <<", 18, hdrCol, 1);
        R.canvas().drawFastHLine(6, 28, Display::W - 12, Colors::NERV_GREEN_DIM);

        // Typewriter message
        char msgBuf[48];
        strncpy_P(msgBuf, NERV_MESSAGES[_msgIdx], sizeof(msgBuf));
        msgBuf[_typedChar] = '\0';

        R.canvas().setTextColor(Colors::NERV_GREEN);
        R.canvas().setTextSize(2);

        // Wrap at ~20 chars (20 * 12px = 240px)
        int mlen = _typedChar;
        if (mlen <= 20) {
            int tx = (Display::W - mlen * 12) / 2;
            R.canvas().setCursor(tx, 42);
            R.canvas().print(msgBuf);
        } else {
            // Two lines
            char line1[21]; strncpy(line1, msgBuf, 20); line1[20] = '\0';
            for (int i = 19; i > 0; i--) if (line1[i] == ' ') { line1[i] = '\0'; break; }
            int l1 = strlen(line1);
            int tx1 = (Display::W - l1 * 12) / 2;
            R.canvas().setCursor(tx1, 40);
            R.canvas().print(line1);
            if ((int)strlen(msgBuf) > l1 + 1) {
                const char* line2 = msgBuf + l1 + 1;
                int l2 = strlen(line2);
                int tx2 = (Display::W - l2 * 12) / 2;
                R.canvas().setCursor(tx2, 60);
                R.canvas().print(line2);
            }
        }

        // Cursor blink
        if ((_ticker / 5) % 2 == 0) {
            R.canvas().fillRect(Display::W / 2 + 4, 80, 8, 14, col);
        }

        // Message number dim
        char buf[12];
        snprintf(buf, sizeof(buf), "MSG %02d/%02d", _msgIdx + 1, NERV_MSG_COUNT);
        R.canvas().setTextColor(Colors::TEXT_DIM);
        R.canvas().setTextSize(1);
        R.canvas().setCursor(4, Display::H - 24);
        R.canvas().print(buf);
    }
};

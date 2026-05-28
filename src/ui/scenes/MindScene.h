#pragma once
// ============================================================
// MindScene — Pilot mental conditioning
//
// 3 random modes:
//   A) QUOTE INTERACTIVE — 12 reflective quote pairs
//   B) SYNC BREATH       — 5 breathing cycles (pulse)
//   C) AT FIELD          — typewriter + concentric hex
//
// Effects: sync+15 rage-15 energy-2 happiness+10 (base)
// Not available if syncRate >= 95.
// BTN_B long → MAIN via SceneManager universal escape.
// ============================================================
#include "ui/SceneManager.h"
#include "entities/EvaPet.h"
#include "ui/Renderer.h"
#include "audio/AudioManager.h"
#include "data/LoreData.h"

// ─── Mind quote structure ──────────────────────────────────
struct MindQuote {
    const char* text;
    const char* author;
    const char* episode;
    const char* choiceA;
    const char* choiceB;
    int8_t      syncA,  rageA,  happyA;
    int8_t      syncB,  rageB,  happyB;
    const char* flavourA;
    const char* flavourB;
};

static const MindQuote MIND_QUOTES[] = {
    { "I mustn't run away.", "Shinji Ikari", "EP.01",
      "PRESS ON", "FIND PEACE",
      +10, -8,  0,   +15, -12, 0,
      "Determination: engaged.", "Inner calm restored." },
    { "Nobody understands me.", "Shinji Ikari", "EP.04",
      "I UNDERSTAND", "YOU'RE ALONE",
      +10, -5,  +5,  -5,  +5,  -5,
      "Connection established.", "Unit remains isolated." },
    { "How disgraceful.", "Asuka Langley", "EP.08",
      "PROVE HER WRONG", "SHE IS RIGHT",
      +5,  0,   +5,  -5,  +10,  -5,
      "Competitive drive engaged.", "Self-doubt amplified." },
    { "I am not alone.", "Rei Ayanami", "EP.14",
      "BELIEVE IT", "DOUBT IT",
      +15, -10, 0,   -5,  +5,  0,
      "AT Field softens.", "Isolation deepens." },
    { "Congratulations.", "All Cast", "EoE",
      "THANK YOU", "WHY?",
      +20, 0,   +15, +5,  -5,  0,
      "Third Impact averted.", "Instrumentality reconsidered." },
    { "Pain is something man\nmust endure in his heart.", "Kaworu Nagisa", "EP.24",
      "I ACCEPT IT", "I REFUSE IT",
      +12, -8,  0,   +5,  0,   +5,
      "Strength through acceptance.", "Resistance fuels the soul." },
    { "Don't you know\neveryone's alone?", "Asuka Langley", "EP.22",
      "ALWAYS", "NOT ME",
      -5,  +5,  0,   +15, -10, 0,
      "Walls remain standing.", "Connection is possible." },
    { "I need you.", "Kaworu Nagisa", "EP.24",
      "I'M HERE", "GO AWAY",
      +20, 0,   +10, -5,  +8,  0,
      "Bond strengthened.", "Rejection logged." },
    { "Who are you? I am I.", "Shinji Ikari", "EP.25",
      "I KNOW MYSELF", "I DON'T KNOW",
      +15, -10, 0,   +5,  0,  -5,
      "Identity: confirmed.", "Search continues." },
    { "An angel has no need\nfor weapons.", "Rei Ayanami", "EP.06",
      "AGREED", "DISAGREE",
      +10, -15, 0,   +5,  +5,  0,
      "Peace as strength.", "Combat readiness: high." },
    { "You need to figure out\nwhere you belong.", "Misato Katsuragi", "EP.09",
      "I BELONG HERE", "I DON'T KNOW",
      +15, 0,   +10, +5,  +5,  0,
      "Purpose: found.", "Search continues." },
    { "I hate myself.\nBut maybe I could love myself.", "Shinji Ikari", "EP.26",
      "MAYBE", "NEVER",
      +20, 0,   +15, -5,  +10, 0,
      "Self-acceptance begins.", "The wall remains." },
};
static constexpr uint8_t MIND_QUOTE_COUNT = 12;

// ─────────────────────────────────────────────────────────

class MindScene : public BaseScene {
public:
    static MindScene& get() { static MindScene s; return s; }

    void onEnter() override {
        _mode       = (Mode)(random(0, 3));
        _phase      = Phase::ACTIVE;
        _ticker     = 0;
        _phaseStart = millis();
        _applied    = false;
        _choiceSel  = 0;
        _quoteIdx   = random(0, MIND_QUOTE_COUNT);
        _breathCycles   = 0;
        _atTypeIdx      = 0;
        _atTypeLast     = millis();
        _atAnimStart    = 0;
        AudioManager::get().playQuoteAppear();
        AudioManager::get().playHeartbeat();
    }

    void update() override {
        _ticker++;
        uint32_t now = millis();

        if (_phase == Phase::RESULT) {
            if (now - _phaseStart >= 2000) requestTransition(GameState::MAIN);
            return;
        }

        switch (_mode) {
            case Mode::QUOTE_INTERACTIVE: _updateQuote(now); break;
            case Mode::SYNC_BREATH:       _updateBreath(now); break;
            case Mode::AT_FIELD:          _updateATField(now); break;
        }
    }

    void render() override {
        Renderer& r = Renderer::get();
        r.clearCanvas(Colors::BG_DARK);

        if (_phase == Phase::RESULT) {
            _drawResult(r);
        } else {
            switch (_mode) {
                case Mode::QUOTE_INTERACTIVE: _drawQuote(r);   break;
                case Mode::SYNC_BREATH:       _drawBreath(r);  break;
                case Mode::AT_FIELD:          _drawATField(r); break;
            }
        }
        r.flush();
    }

    GameState handleInput(InputEvent ev) override {
        if (_phase == Phase::RESULT) return GameState::COUNT;

        switch (_mode) {
            case Mode::QUOTE_INTERACTIVE:
                if (_phase == Phase::CHOICE) {
                    if (ev == InputEvent::BTN_A_CLICK) {
                        _choiceSel = 1 - _choiceSel;
                        AudioManager::get().playBeep(500, 35);
                    }
                    if (ev == InputEvent::BTN_B_CLICK) {
                        _applyQuoteChoice();
                        _phase      = Phase::RESULT;
                        _phaseStart = millis();
                    }
                } else if (_phase == Phase::ACTIVE) {
                    if (ev == InputEvent::BTN_B_CLICK) {
                        _phase      = Phase::CHOICE;
                        _phaseStart = millis();
                        AudioManager::get().playBeep(660, 60);
                    }
                }
                break;

            case Mode::SYNC_BREATH:
            case Mode::AT_FIELD:
                if (ev == InputEvent::BTN_A_CLICK || ev == InputEvent::BTN_B_CLICK) {
                    _applyBase(_breathCycles);
                    _phase      = Phase::RESULT;
                    _phaseStart = millis();
                }
                break;
        }
        return GameState::COUNT;
    }

private:
    enum class Mode  { QUOTE_INTERACTIVE, SYNC_BREATH, AT_FIELD };
    enum class Phase { ACTIVE, CHOICE, RESULT };

    static constexpr const char* AT_TEXT =
        "The AT Field is the barrier\n"
        "of the soul. The wall that\n"
        "separates one person from\n"
        "another. Lower yours now.";

    static constexpr const char* AT_ENDINGS[] = {
        "Field lowered. Connection possible.",
        "Absolute Terror Field: dissolved.",
        "You are not alone.",
    };

    Mode     _mode         = Mode::SYNC_BREATH;
    Phase    _phase        = Phase::ACTIVE;
    int      _ticker       = 0;
    uint32_t _phaseStart   = 0;
    bool     _applied      = false;
    int      _choiceSel    = 0;
    uint8_t  _quoteIdx     = 0;

    // Breath mode
    int  _breathCycles  = 0;
    bool _breathExpand  = true;
    uint32_t _breathPhaseStart = 0;

    // AT Field mode
    uint8_t  _atTypeIdx    = 0;
    uint32_t _atTypeLast   = 0;
    uint32_t _atAnimStart  = 0;
    uint8_t  _atEndIdx     = 0;
    bool     _atTypeDone   = false;

    // Result text
    const char* _resultLine1 = nullptr;
    const char* _resultLine2 = nullptr;
    int8_t  _rsync = 0, _rrage = 0, _rhappy = 0;

    // ── Update helpers ────────────────────────────────────

    void _updateQuote(uint32_t now) {
        // Auto-advance from ACTIVE after 3s to show choice
        if (_phase == Phase::ACTIVE && now - _phaseStart >= 3000) {
            _phase      = Phase::CHOICE;
            _phaseStart = now;
        }
    }

    void _updateBreath(uint32_t now) {
        uint32_t phase_ms = now - _breathPhaseStart;
        static constexpr uint32_t HALF_CYCLE = 2000;
        if (phase_ms >= HALF_CYCLE) {
            _breathPhaseStart = now;
            _breathExpand = !_breathExpand;
            if (_breathExpand) _breathCycles++;  // completed one full cycle
        }
        if (_breathCycles >= 5) {
            _applyBase(5);
            _phase      = Phase::RESULT;
            _phaseStart = now;
        }
    }

    void _updateATField(uint32_t now) {
        static constexpr uint8_t AT_LEN = 80;  // strlen of AT_TEXT approx
        if (!_atTypeDone) {
            if (now - _atTypeLast >= 60) {
                _atTypeLast = now;
                _atTypeIdx++;
                uint8_t textLen = strlen(AT_TEXT);
                if (_atTypeIdx >= textLen) {
                    _atTypeDone  = true;
                    _atAnimStart = now;
                    _atEndIdx    = random(0, 3);
                }
            }
        } else {
            // Hex animation: 8 seconds
            if (now - _atAnimStart >= 8000) {
                _applyBase(5);
                _phase      = Phase::RESULT;
                _phaseStart = now;
            }
        }
    }

    // ── Apply stat effects ────────────────────────────────

    void _applyQuoteChoice() {
        if (_applied) return;
        _applied = true;
        const MindQuote& q = MIND_QUOTES[_quoteIdx];
        EvaStats& s = EvaPet::get().getStatsMut();
        if (_choiceSel == 0) {
            s.syncRate  = statClamp(s.syncRate  + q.syncA);
            s.rage      = statClamp(s.rage      + q.rageA);
            s.happiness = statClamp(s.happiness + q.happyA);
            s.energy    = statClamp(s.energy    - 2);
            _resultLine1 = q.flavourA;
            _rsync = q.syncA; _rrage = q.rageA; _rhappy = q.happyA;
        } else {
            s.syncRate  = statClamp(s.syncRate  + q.syncB);
            s.rage      = statClamp(s.rage      + q.rageB);
            s.happiness = statClamp(s.happiness + q.happyB);
            s.energy    = statClamp(s.energy    - 2);
            _resultLine1 = q.flavourB;
            _rsync = q.syncB; _rrage = q.rageB; _rhappy = q.happyB;
        }
        EvaPet::get().markDirty();
    }

    void _applyBase(int cycles) {
        if (_applied) return;
        _applied = true;
        float ratio = (float)min(cycles, 5) / 5.0f;
        EvaStats& s = EvaPet::get().getStatsMut();
        s.syncRate  = statClamp(s.syncRate  + (int8_t)(15 * ratio));
        s.rage      = statClamp(s.rage      - (int8_t)(15 * ratio));
        s.happiness = statClamp(s.happiness + (int8_t)(10 * ratio));
        s.energy    = statClamp(s.energy    - 2);
        _rsync  = (int8_t)(15 * ratio);
        _rrage  = -(int8_t)(15 * ratio);
        _rhappy = (int8_t)(10 * ratio);
        EvaPet::get().markDirty();
    }

    // ── Draw helpers ──────────────────────────────────────

    void _drawHeader(Renderer& r, const char* title, uint32_t col) {
        r.canvas().fillRect(0, 0, Display::W, 13, 0x060810);
        r.canvas().drawFastHLine(0, 13, Display::W, col);
        r.canvas().setTextColor(col);
        r.canvas().setTextSize(1);
        r.canvas().setCursor(4, 3);
        r.canvas().print(title);
    }

    void _drawQuote(Renderer& r) {
        const EvaStats& st = EvaPet::get().getStats();
        uint32_t col = EVA_PERSONALITIES[st.evaUnit].primaryColor;
        _drawHeader(r, "[ REFLECTION ]", col);

        const MindQuote& q = MIND_QUOTES[_quoteIdx];

        r.canvas().drawFastHLine(0, 14, Display::W, Colors::NERV_AMBER);
        r.canvas().drawFastHLine(0, Display::H - 30, Display::W, Colors::NERV_AMBER);

        // Quote text (multi-line, textSize 1 to fit)
        r.canvas().setTextColor(Colors::WHITE);
        r.canvas().setTextSize(1);
        r.canvas().setCursor(8, 22);
        // Simple multi-line print
        const char* p = q.text;
        int16_t lx = 8, ly = 22;
        while (*p) {
            if (*p == '\n') { lx = 8; ly += 10; }
            else {
                r.canvas().setCursor(lx, ly);
                char ch[2] = {*p, 0};
                r.canvas().print(ch);
                lx += 6;
            }
            p++;
        }

        // Author right-aligned
        r.canvas().setTextColor(Colors::NERV_AMBER);
        r.canvas().setTextSize(1);
        int16_t aw = strlen(q.author) * 6;
        r.canvas().setCursor(Display::W - aw - 4, ly + 14);
        r.canvas().print(q.author);

        // Episode bottom-left
        r.canvas().setTextColor(Colors::TEXT_DIM);
        r.canvas().setCursor(4, Display::H - 26);
        r.canvas().print(q.episode);

        if (_phase == Phase::CHOICE) {
            _drawChoices(r, q, col);
        } else {
            // "BTN_B TO REFLECT" hint
            bool blink = (_ticker / 10) % 2 == 0;
            if (blink) {
                r.canvas().setTextColor(Colors::TEXT_DIM);
                r.canvas().setCursor((Display::W - 18*6)/2, Display::H - 10);
                r.canvas().print("BTN_B: CHOOSE RESPONSE");
            }
        }
    }

    void _drawChoices(Renderer& r, const MindQuote& q, uint32_t col) {
        int16_t y0 = Display::H - 28;
        int16_t halfW = Display::W / 2;

        for (int i = 0; i < 2; i++) {
            const char* lbl = (i == 0) ? q.choiceA : q.choiceB;
            bool sel = (i == _choiceSel);
            int16_t bx = i * halfW + 2;
            uint32_t bg = sel ? (col & 0x202020) : 0x0A0A12;
            r.canvas().fillRect(bx, y0, halfW - 4, 24, bg);
            r.canvas().drawRect(bx, y0, halfW - 4, 24, sel ? col : Colors::TEXT_DIM);
            r.canvas().setTextColor(sel ? col : Colors::TEXT_DIM);
            r.canvas().setTextSize(1);
            int16_t tw = strlen(lbl) * 6;
            r.canvas().setCursor(bx + (halfW - 4 - tw) / 2, y0 + 8);
            r.canvas().print(lbl);
        }
    }

    void _drawBreath(Renderer& r) {
        const EvaStats& st = EvaPet::get().getStats();
        uint32_t col = EVA_PERSONALITIES[st.evaUnit].primaryColor;
        _drawHeader(r, "[ SYNC BREATH ]", col);

        uint32_t elapsed = millis() - _breathPhaseStart;
        float ratio = (float)elapsed / 2000.0f;
        if (!_breathExpand) ratio = 1.0f - ratio;
        ratio = max(0.0f, min(1.0f, ratio));

        int16_t radius = (int16_t)(10 + ratio * 30);
        int16_t cx = Display::W / 2, cy = 72;

        for (int t = 0; t < 3; t++)
            r.canvas().drawCircle(cx, cy, radius - t, col);

        // Glow rings
        for (int t = 0; t < 2; t++)
            r.canvas().drawCircle(cx, cy, radius + 4 + t, col & 0x3F3F3F);

        const char* phase = _breathExpand ? "BREATHE IN" : "BREATHE OUT";
        r.canvas().setTextColor(Colors::WHITE);
        r.canvas().setTextSize(1);
        int16_t tw = strlen(phase) * 6;
        r.canvas().setCursor((Display::W - tw) / 2, 28);
        r.canvas().print(phase);

        // Cycle dots
        int dotX = (Display::W - 5 * 14) / 2;
        for (int i = 0; i < 5; i++) {
            uint32_t dc = (i < _breathCycles) ? col : Colors::TEXT_DIM;
            r.canvas().fillCircle(dotX + i * 14 + 6, Display::H - 10, 4, dc);
        }

        r.canvas().setTextColor(Colors::TEXT_DIM);
        r.canvas().setTextSize(1);
        r.canvas().setCursor(4, Display::H - 10);
        r.canvas().print("BTN_A/B: SKIP");
    }

    void _drawATField(Renderer& r) {
        const EvaStats& st = EvaPet::get().getStats();
        uint32_t col = EVA_PERSONALITIES[st.evaUnit].primaryColor;
        _drawHeader(r, "[ AT FIELD ]", col);

        if (!_atTypeDone) {
            // Typewriter text
            r.canvas().setTextColor(Colors::WHITE);
            r.canvas().setTextSize(1);
            int16_t tx = 8, ty = 22;
            for (uint8_t i = 0; i < _atTypeIdx && AT_TEXT[i]; i++) {
                if (AT_TEXT[i] == '\n') { tx = 8; ty += 10; continue; }
                r.canvas().setCursor(tx, ty);
                char ch[2] = {AT_TEXT[i], 0};
                r.canvas().print(ch);
                tx += 6;
            }
            // Cursor blink
            if ((_ticker / 8) % 2 == 0) {
                r.canvas().fillRect(tx, ty, 5, 8, Colors::WHITE);
            }
        } else {
            // Concentric hexagons contracting toward center
            uint32_t elapsed = millis() - _atAnimStart;
            float prog = min(1.0f, (float)elapsed / 8000.0f);
            int16_t cx = Display::W / 2, cy = 72;

            static const uint32_t hexCols[] = {Colors::NERV_RED, Colors::NERV_YELLOW, Colors::NERV_GREEN};
            for (int h = 0; h < 3; h++) {
                int16_t baseR = 45 - h * 12;
                int16_t r_ = (int16_t)(baseR * (1.0f - prog * 0.7f));
                if (r_ < 4) r_ = 4;
                // Draw hexagon as 6 line segments
                for (int seg = 0; seg < 6; seg++) {
                    float a1 = seg * M_PI / 3.0f;
                    float a2 = (seg + 1) * M_PI / 3.0f;
                    int16_t x1 = cx + (int16_t)(r_ * cosf(a1));
                    int16_t y1 = cy + (int16_t)(r_ * sinf(a1));
                    int16_t x2 = cx + (int16_t)(r_ * cosf(a2));
                    int16_t y2 = cy + (int16_t)(r_ * sinf(a2));
                    r.canvas().drawLine(x1, y1, x2, y2, hexCols[h]);
                }
            }

            // End message when almost done
            if (prog > 0.85f) {
                r.canvas().setTextColor(Colors::NERV_GREEN);
                r.canvas().setTextSize(1);
                int16_t tw = strlen(AT_ENDINGS[_atEndIdx]) * 6;
                r.canvas().setCursor((Display::W - tw) / 2, Display::H - 14);
                r.canvas().print(AT_ENDINGS[_atEndIdx]);
            }
        }

        r.canvas().setTextColor(Colors::TEXT_DIM);
        r.canvas().setTextSize(1);
        r.canvas().setCursor(4, Display::H - 10);
        r.canvas().print("BTN_A/B: SKIP");
    }

    void _drawResult(Renderer& r) {
        const EvaStats& st = EvaPet::get().getStats();
        uint32_t col = EVA_PERSONALITIES[st.evaUnit].primaryColor;

        r.canvas().fillScreen(Colors::BG_DARK);
        r.drawTextCentered("CONDITIONING", 20, col, 1);
        r.drawTextCentered("COMPLETE", 32, col, 2);
        r.canvas().drawFastHLine(20, 54, Display::W - 40, col);

        if (_resultLine1) {
            r.canvas().setTextColor(Colors::WHITE);
            r.canvas().setTextSize(1);
            int16_t tw = strlen(_resultLine1) * 6;
            r.canvas().setCursor((Display::W - tw) / 2, 62);
            r.canvas().print(_resultLine1);
        }

        char buf[32];
        if (_rsync != 0) {
            snprintf(buf, sizeof(buf), "SYNC %+d", (int)_rsync);
            r.canvas().setTextColor(_rsync > 0 ? Colors::NERV_GREEN : Colors::NERV_RED);
            r.canvas().setTextSize(1);
            r.canvas().setCursor(30, 80);
            r.canvas().print(buf);
        }
        if (_rrage != 0) {
            snprintf(buf, sizeof(buf), "RAGE %+d", (int)_rrage);
            r.canvas().setTextColor(_rrage < 0 ? Colors::NERV_GREEN : Colors::NERV_RED);
            r.canvas().setTextSize(1);
            r.canvas().setCursor(120, 80);
            r.canvas().print(buf);
        }

        r.canvas().drawRect(0, 0, Display::W, Display::H, col);
    }
};

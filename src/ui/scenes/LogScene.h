#pragma once
#include "ui/SceneManager.h"
#include "core/Types.h"
#include "data/LoreData.h"
#include "entities/EvaPet.h"
#include "ui/Renderer.h"
#include "audio/AudioManager.h"

// ============================================================
// LogScene — scrollable NERV terminal log viewer
// ============================================================
class LogScene : public BaseScene {
public:
    static LogScene& get() { static LogScene s; return s; }

    void onEnter() override {
        _section   = 0;
        _logIndex  = 0;
        _charPos   = 0;
        _ticker    = 0;
        _typeTimer = 0;
        _done      = false;
    }

    void onExit() override {}

    void update() override {
        _ticker++;
        if (!_done) {
            _typeTimer++;
            if (_typeTimer >= 2) {
                _typeTimer = 0;
                _charPos++;
                int maxLen = _currentText().length();
                if (_charPos >= maxLen) {
                    _charPos = maxLen;
                    _done    = true;
                }
            }
        }
    }

    void render() override {
        auto& R   = Renderer::get();
        auto& pet = EvaPet::get();

        R.clearCanvas(Colors::BG_DARK);
        R.drawNERVFrame(Colors::NERV_GREEN);
        R.drawHeader("[ DATA TERMINAL ]", pet.getStats());

        _drawSectionTabs(R);
        _drawLogContent(R);

        const char* labels[] = { "A:NEXT", "B:SECT", "B+:BACK" };
        R.drawFooter(labels, 3, 255);

        R.flush();
    }

    GameState handleInput(InputEvent ev) override {
        int maxIndex = _sectionMax();

        switch (ev) {
            case InputEvent::BTN_A_CLICK:
                if (!_done) {
                    _charPos = _currentText().length();
                    _done    = true;
                } else {
                    _logIndex = (_logIndex + 1) % maxIndex;
                    _resetTypewriter();
                }
                AudioManager::get().playBeep(600, 30);
                break;

            case InputEvent::BTN_A_HOLD:
                _logIndex = (_logIndex - 1 + maxIndex) % maxIndex;
                _resetTypewriter();
                AudioManager::get().playBeep(400, 30);
                break;

            case InputEvent::BTN_B_CLICK:
                _section  = (_section + 1) % 3;
                _logIndex = 0;
                _resetTypewriter();
                AudioManager::get().playBeep(800, 40);
                break;

            case InputEvent::BTN_B_HOLD:
                AudioManager::get().playSelect();
                requestTransition(GameState::MAIN);
                break;

            default: break;
        }
        return GameState::COUNT;
    }

private:
    int  _section   = 0;
    int  _logIndex  = 0;
    int  _charPos   = 0;
    int  _ticker    = 0;
    int  _typeTimer = 0;
    bool _done      = false;

    int _sectionMax() const {
        switch (_section) {
            case 0: return 20;
            case 1: return 12;
            case 2: return 9;
        }
        return 1;
    }

    String _currentText() const {
        switch (_section) {
            case 0: {
                char buf[80];
                strncpy_P(buf, NERV_MESSAGES[_logIndex % 20], sizeof(buf));
                return String(buf);
            }
            case 1: {
                char buf[128];
                strncpy_P(buf, DATA_LOGS[_logIndex % 12], sizeof(buf));
                return String(buf);
            }
            case 2: {
                GameEvent ev;
                memcpy_P(&ev, &RANDOM_EVENTS[_logIndex % 9], sizeof(GameEvent));
                return String(ev.message);
            }
        }
        return "";
    }

    String _currentTitle() const {
        switch (_section) {
            case 0: return "NERV BROADCAST";
            case 1: return "DATA LOG";
            case 2: return "EVENT RECORD";
        }
        return "";
    }

    void _resetTypewriter() {
        _charPos   = 0;
        _typeTimer = 0;
        _done      = false;
    }

    void _drawSectionTabs(Renderer& R) {
        const char* tabs[] = { "NERV", "DATA", "EVNT" };
        const int TAB_W = 52, TAB_Y = 16;
        int startX = (Display::W - 3 * TAB_W) / 2;

        for (int i = 0; i < 3; i++) {
            bool active = (i == _section);
            uint32_t bg  = active ? Colors::NERV_GREEN     : Colors::BG_DARK;
            uint32_t fg  = active ? Colors::BG_DARK        : Colors::TEXT_DIM;
            uint32_t bdr = active ? Colors::NERV_GREEN      : Colors::TEXT_DIM;

            R.canvas().fillRect(startX + i * TAB_W, TAB_Y, TAB_W - 2, 11, bg);
            R.canvas().drawRect(startX + i * TAB_W, TAB_Y, TAB_W - 2, 11, bdr);
            R.canvas().setTextColor(fg);
            R.canvas().setTextSize(1);
            R.canvas().setCursor(startX + i * TAB_W + 8, TAB_Y + 2);
            R.canvas().print(tabs[i]);
        }
    }

    void _drawLogContent(Renderer& R) {
        const int CONTENT_Y = 30;
        const int CONTENT_W = 232;

        // Entry counter
        char ctr[12];
        snprintf(ctr, sizeof(ctr), "%02d/%02d", _logIndex + 1, _sectionMax());
        R.canvas().setTextColor(Colors::TEXT_DIM);
        R.canvas().setTextSize(1);
        R.canvas().setCursor(194, CONTENT_Y);
        R.canvas().print(ctr);

        // Section title
        R.canvas().setTextColor(Colors::NERV_CYAN);
        R.canvas().setCursor(8, CONTENT_Y);
        R.canvas().print(_currentTitle().c_str());

        R.canvas().drawFastHLine(8, CONTENT_Y + 10, CONTENT_W, Colors::TEXT_DIM);

        // Typewriter text — word-wrap to 5 lines × 38 cols
        String fullText = _currentText();
        String visible  = fullText.substring(0, _charPos);

        R.canvas().setTextColor(Colors::NERV_GREEN);
        R.canvas().setTextSize(1);

        int y         = CONTENT_Y + 14;
        int lineStart = 0;
        int lineLen   = 0;
        const int MAX_COLS = 38;
        const int LINE_H   = 10;
        int lineCount = 0;

        for (int i = 0; i <= (int)visible.length() && lineCount < 6; i++) {
            bool eol  = (i == (int)visible.length()) || (visible[i] == '\n');
            bool wrap = (lineLen >= MAX_COLS);

            if (eol || wrap) {
                String line = visible.substring(lineStart, i);
                R.canvas().setCursor(8, y + lineCount * LINE_H);
                R.canvas().print(line);
                lineStart = (wrap && !eol) ? i : i + 1;
                lineLen   = 0;
                lineCount++;
            } else {
                lineLen++;
            }
        }

        // Cursor blink
        if (!_done && R.isBlinkOn()) {
            int cx = 8 + (lineLen % MAX_COLS) * 6;
            int cy = y + (lineCount < 6 ? lineCount : 5) * LINE_H;
            R.canvas().fillRect(cx, cy, 4, 8, Colors::NERV_GREEN);
        }
    }
};

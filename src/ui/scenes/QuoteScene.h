#pragma once
// ============================================================
// QuoteScene — NGE eyecatch quote display
//
// Access : BTN_B_HOLD from MainScene
// Visual : diagonal gray pattern + 2px yellow bars top/bottom
//          wipe-in 400ms, hold 4s, BTN_B exits early
// Quotes : 25 entries, Fisher-Yates shuffle without repeat
// ============================================================
#include "ui/SceneManager.h"
#include "ui/Renderer.h"
#include "audio/AudioManager.h"
#include "data/LoreData.h"

class QuoteScene : public BaseScene {
public:
    static QuoteScene& get() { static QuoteScene s; return s; }

    void onEnter() override {
        _enterTime = millis();
        if (_orderIdx >= QUOTE_COUNT) _shuffle();
        _quoteIdx = _order[_orderIdx++];
        AudioManager::get().playQuoteAppear();
    }

    void update() override {
        if (millis() - _enterTime >= HOLD_MS)
            requestTransition(GameState::MAIN);
    }

    void render() override {
        Renderer& r = Renderer::get();
        uint32_t el = millis() - _enterTime;

        r.clearCanvas(Colors::BG_DARK);
        _drawBackground(r);
        _drawContent(r);

        // Curtain wipe-in: two black panels slide apart (top & bottom)
        if (el < WIPE_MS) {
            float prog = (float)el / WIPE_MS;
            int16_t coverH = (int16_t)(Display::H / 2 * (1.0f - prog));
            if (coverH > 0) {
                r.canvas().fillRect(0, 0,
                    Display::W, coverH, Colors::BLACK);
                r.canvas().fillRect(0, Display::H - coverH,
                    Display::W, coverH, Colors::BLACK);
            }
        }

        r.flush();
    }

    GameState handleInput(InputEvent ev) override {
        if (ev == InputEvent::BTN_B_CLICK || ev == InputEvent::BTN_B_HOLD)
            return GameState::MAIN;
        return GameState::COUNT;
    }

private:
    static constexpr uint32_t WIPE_MS = 400;
    static constexpr uint32_t HOLD_MS = 4400;  // wipe(400) + hold(4000)

    uint32_t _enterTime = 0;
    uint8_t  _quoteIdx  = 0;
    uint8_t  _order[QUOTE_COUNT];
    uint8_t  _orderIdx  = QUOTE_COUNT;  // triggers shuffle on first entry

    void _shuffle() {
        for (uint8_t i = 0; i < QUOTE_COUNT; i++) _order[i] = i;
        for (int i = QUOTE_COUNT - 1; i > 0; i--) {
            int j = random(0, i + 1);
            uint8_t tmp = _order[i]; _order[i] = _order[j]; _order[j] = tmp;
        }
        _orderIdx = 0;
    }

    void _drawBackground(Renderer& r) {
        LGFX_Sprite& c = r.canvas();

        // Diagonal gray stripes every 6px
        for (int16_t x = -Display::H; x < Display::W + Display::H; x += 6) {
            c.drawLine(x, 0, x + Display::H, Display::H,
                       Colors::TEXT_DIM & 0x141420);
        }

        // Top + bottom yellow accent bars (2px each)
        c.fillRect(0, 0,             Display::W, 2, Colors::NERV_AMBER);
        c.fillRect(0, Display::H - 2, Display::W, 2, Colors::NERV_AMBER);
    }

    void _drawContent(Renderer& r) {
        LGFX_Sprite& c = r.canvas();
        const QuoteEntry& q = QUOTE_ENTRIES[_quoteIdx];

        // Header bar
        c.fillRect(0, 2, Display::W, 14, Colors::NERV_AMBER & 0x181400);
        c.setTextColor(Colors::NERV_AMBER);
        c.setTextSize(1);
        c.setCursor(6, 5);
        c.print("NERV // RECORD ACCESS");

        // Quote text — wrap at 36 chars
        const char* txt = q.text;
        int tlen = strlen(txt);
        int16_t textY = 32;

        c.setTextColor(Colors::WHITE);
        c.setTextSize(1);

        if (tlen <= 36) {
            int16_t tx = (Display::W - tlen * 6) / 2;
            c.setCursor(tx, textY);
            c.print(txt);
            textY += 12;
        } else {
            // Split at last space before char 36
            char line1[37];
            strncpy(line1, txt, 36);
            line1[36] = '\0';
            for (int i = 35; i > 0; i--) {
                if (line1[i] == ' ') { line1[i] = '\0'; break; }
            }
            int l1 = strlen(line1);
            c.setCursor((Display::W - l1 * 6) / 2, textY);
            c.print(line1);
            textY += 12;

            const char* rest = txt + l1 + 1;
            int l2 = strlen(rest);
            if (l2 > 0) {
                c.setCursor((Display::W - l2 * 6) / 2, textY);
                c.print(rest);
                textY += 12;
            }
        }

        // Divider
        textY += 6;
        c.drawFastHLine(20, textY, Display::W - 40, Colors::NERV_AMBER);
        textY += 8;

        // Attribution
        c.setTextColor(Colors::NERV_AMBER);
        c.setTextSize(1);
        char attrib[48];
        snprintf(attrib, sizeof(attrib), "— %s", q.author);
        int16_t aw = strlen(attrib) * 6;
        c.setCursor((Display::W - aw) / 2, textY);
        c.print(attrib);
        textY += 11;

        // Episode dim
        c.setTextColor(Colors::TEXT_DIM);
        int16_t ew = strlen(q.episode) * 6;
        c.setCursor((Display::W - ew) / 2, textY);
        c.print(q.episode);

        // Footer hint
        c.setTextColor(Colors::TEXT_DIM);
        c.setCursor(Display::W - 66, Display::H - 12);
        c.print("[B] CLOSE");
    }
};

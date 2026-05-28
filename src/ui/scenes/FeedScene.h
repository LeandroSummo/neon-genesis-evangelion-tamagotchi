#pragma once
// ============================================================
// FeedScene — 6-item NGE food selection + consume animation
//
// Phase SELECTING: BTN_A cycles items, BTN_B confirms.
//   Guard: hunger > 85 → "UNIT: SATIATED"
// Phase CONSUMING: 45-frame animation, then → MAIN
// ============================================================
#include "ui/SceneManager.h"
#include "entities/EvaPet.h"
#include "ui/Renderer.h"
#include "audio/AudioManager.h"
#include "data/EvaData.h"

class FeedScene : public BaseScene {
public:
    static FeedScene& get() { static FeedScene s; return s; }

    void onEnter() override {
        _phase   = Phase::SELECTING;
        _item    = 0;
        _ticker  = 0;
        _denied  = false;
    }

    void onExit() override {}

    void update() override {
        _ticker++;
        if (_phase == Phase::CONSUMING && _ticker >= ANIM_FRAMES) {
            requestTransition(GameState::MAIN);
        }
        // Denied message: show for 60 frames then back to MAIN
        if (_denied && _ticker >= 60) {
            requestTransition(GameState::MAIN);
        }
    }

    void render() override {
        Renderer& R = Renderer::get();
        R.clearCanvas(Colors::BG_DARK);

        const EvaStats& st = EvaPet::get().getStats();
        uint32_t col = EVA_PERSONALITIES[st.evaUnit].primaryColor;
        R.canvas().drawRect(0, 0, Display::W, Display::H, col);

        if (_denied) {
            _drawDenied(R, col);
        } else if (_phase == Phase::SELECTING) {
            _drawSelecting(R, st, col);
        } else {
            _drawConsuming(R, st, col);
        }

        R.flush();
    }

    GameState handleInput(InputEvent ev) override {
        if (_phase != Phase::SELECTING || _denied) return GameState::COUNT;

        if (ev == InputEvent::BTN_A_CLICK) {
            _item = (_item + 1) % 6;
            AudioManager::get().playBeep(500, 35);
        }
        if (ev == InputEvent::BTN_B_CLICK) {
            if (!EvaPet::get().canFeed()) {
                _denied = true;
                _ticker = 0;
                AudioManager::get().playBeep(200, 100);
            } else {
                EvaPet::get().feed((FeedItem)_item);
                AudioManager::get().playFeed();
                _phase  = Phase::CONSUMING;
                _ticker = 0;
            }
        }
        if (ev == InputEvent::BTN_B_HOLD) {
            return GameState::MAIN;
        }
        return GameState::COUNT;
    }

private:
    enum class Phase { SELECTING, CONSUMING };
    static constexpr int ANIM_FRAMES = 45;

    Phase _phase  = Phase::SELECTING;
    int   _item   = 0;
    int   _ticker = 0;
    bool  _denied = false;

    void _drawSelecting(Renderer& R, const EvaStats& st, uint32_t col) {
        const FeedItemData& food = FEED_ITEMS[_item];

        // Header bar
        R.canvas().fillRect(0, 0, Display::W, 13, 0x060810);
        R.canvas().drawFastHLine(0, 13, Display::W, col);
        R.canvas().setTextColor(col);
        R.canvas().setTextSize(1);
        R.canvas().setCursor(4, 3);
        R.canvas().print("RATION SELECT");
        char ibuf[8];
        snprintf(ibuf, sizeof(ibuf), "%d / 6", _item + 1);
        R.canvas().setCursor(Display::W - strlen(ibuf) * 6 - 4, 3);
        R.canvas().print(ibuf);

        // Item name (size 2)
        R.canvas().setTextColor(Colors::WHITE);
        R.canvas().setTextSize(2);
        R.canvas().setCursor(6, 18);
        R.canvas().print(food.name);

        // Flavour text (size 1, 2 lines max, truncate at 36 chars)
        R.canvas().setTextColor(Colors::TEXT_DIM);
        R.canvas().setTextSize(1);
        const char* flav = food.flavour;
        int flen = strlen(flav);
        char line1[37], line2[37];
        if (flen <= 36) {
            strncpy(line1, flav, 36); line1[36] = '\0';
            line2[0] = '\0';
        } else {
            // split at last space before char 36
            strncpy(line1, flav, 36); line1[36] = '\0';
            for (int i = 35; i > 0; i--) {
                if (line1[i] == ' ') { line1[i] = '\0'; break; }
            }
            int l1len = strlen(line1);
            strncpy(line2, flav + l1len + 1, 36); line2[36] = '\0';
        }
        R.canvas().setCursor(6, 40);
        R.canvas().print(line1);
        if (line2[0]) {
            R.canvas().setCursor(6, 50);
            R.canvas().print(line2);
        }

        // Stat modifiers
        _drawMod(R, 6,  66, "FOOD", food.hungerMod, Colors::NERV_CYAN,  false);
        _drawMod(R, 70, 66, "ENRG", food.energyMod, Colors::NERV_BLUE,  false);
        _drawMod(R, 6,  80, "SYNC", food.syncMod,   Colors::NERV_GREEN, false);
        _drawMod(R, 70, 80, "RAGE", food.rageMod,   Colors::NERV_GREEN, true); // inverted: neg=good

        // Current hunger bar
        R.canvas().setTextColor(Colors::TEXT_DIM);
        R.canvas().setTextSize(1);
        R.canvas().setCursor(6, 94);
        R.canvas().print("CURRENT SATIATION:");
        int hw = map(st.hunger, 0, 100, 0, 228);
        R.canvas().fillRect(6, 104, hw, 7, Colors::NERV_CYAN);
        R.canvas().drawRect(6, 104, 228, 7, Colors::TEXT_DIM);

        // Footer
        R.canvas().fillRect(0, Display::H - 14, Display::W, 14, 0x060810);
        R.canvas().drawFastHLine(0, Display::H - 14, Display::W, col);
        R.canvas().setTextColor(Colors::NERV_CYAN);
        R.canvas().setTextSize(1);
        R.canvas().setCursor(4, Display::H - 10);
        R.canvas().print("[A] CYCLE");
        R.canvas().setTextColor(Colors::NERV_GREEN);
        R.canvas().setCursor(Display::W - 82, Display::H - 10);
        R.canvas().print("[B] INGEST");
    }

    void _drawMod(Renderer& R, int x, int y, const char* label,
                  int16_t val, uint32_t goodCol, bool invertColor) {
        char buf[12];
        if (val > 0)      snprintf(buf, sizeof(buf), "%s:+%d", label, val);
        else if (val < 0) snprintf(buf, sizeof(buf), "%s:%d",  label, val);
        else              snprintf(buf, sizeof(buf), "%s:--",  label);
        uint32_t c;
        if (val == 0)       c = Colors::TEXT_DIM;
        else if (invertColor) c = (val < 0) ? goodCol : Colors::NERV_RED;
        else                  c = (val > 0) ? goodCol : Colors::NERV_RED;
        R.canvas().setTextColor(c);
        R.canvas().setTextSize(1);
        R.canvas().setCursor(x, y);
        R.canvas().print(buf);
    }

    void _drawConsuming(Renderer& R, const EvaStats& st, uint32_t col) {
        const FeedItemData& food = FEED_ITEMS[_item];

        int bobY = (_ticker < ANIM_FRAMES / 2)
                   ? (int)(-3.0f * sinf((float)_ticker / (ANIM_FRAMES / 2) * PI))
                   : 0;

        AnimationSystem& anim = AnimationSystem::get();
        const uint8_t* frame  = anim.getCurrentFrame(st.evaUnit, AnimState::HAPPY);
        int sx = (Display::SPRITE_W - 48) / 2;
        anim.drawSprite(R.canvas(), frame, sx, 18 + bobY, st.evaUnit, 3);

        bool flash = (_ticker / 5) % 2 == 0;
        if (flash) R.drawTextCentered("INGESTING", 95, col, 2);

        R.canvas().setTextColor(Colors::TEXT_DIM);
        R.canvas().setTextSize(1);
        R.canvas().setCursor(Display::SPRITE_W + 8, 18);
        R.canvas().print(food.name);

        // Updated hunger bar
        int hw = map(st.hunger, 0, 100, 0, 144);
        R.canvas().setTextColor(Colors::NERV_CYAN);
        R.canvas().setCursor(Display::SPRITE_W + 8, 34);
        R.canvas().print("FOOD");
        R.canvas().fillRect(Display::SPRITE_W + 8, 44, hw, 8, Colors::NERV_CYAN);
        R.canvas().drawRect(Display::SPRITE_W + 8, 44, 144, 8, Colors::TEXT_DIM);

        // Sync bar
        int sw = map(st.syncRate, 0, 100, 0, 144);
        R.canvas().setTextColor(Colors::NERV_GREEN);
        R.canvas().setCursor(Display::SPRITE_W + 8, 58);
        R.canvas().print("SYNC");
        R.canvas().fillRect(Display::SPRITE_W + 8, 68, sw, 8, Colors::NERV_GREEN);
        R.canvas().drawRect(Display::SPRITE_W + 8, 68, 144, 8, Colors::TEXT_DIM);
    }

    void _drawDenied(Renderer& R, uint32_t col) {
        R.drawTextCentered("UNIT: SATIATED", 42, col, 2);
        R.drawTextCentered("PILOT DOES NOT", 66, Colors::TEXT_DIM, 1);
        R.drawTextCentered("REQUIRE FUEL AT", 78, Colors::TEXT_DIM, 1);
        R.drawTextCentered("THIS TIME.", 90, Colors::TEXT_DIM, 1);
    }
};

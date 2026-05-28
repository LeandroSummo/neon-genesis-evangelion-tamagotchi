#pragma once
// ============================================================
// SleepScene — Real sleep with recovery ticks
//
// SLEEPING: 30s ticks → energy+15, sync+3, hunger-2
//   BTN_A / BTN_B: manual wake → WAKING
//   energy > 90: auto-wake → WAKING
// WAKING: 2s "REACTIVATING..." then → MAIN
// ============================================================
#include "ui/SceneManager.h"
#include "entities/EvaPet.h"
#include "ui/Renderer.h"
#include "audio/AudioManager.h"

class SleepScene : public BaseScene {
public:
    static SleepScene& get() { static SleepScene s; return s; }

    void onEnter() override {
        _phase    = Phase::SLEEPING;
        _ticker   = 0;
        _lastTick = millis();
        EvaPet::get().rest();
        AudioManager::get().playHeartbeat();
        M5.Display.setBrightness(30);
    }

    void onExit() override {
        EvaPet::get().wakeUp();
        M5.Display.setBrightness(80);
    }

    void update() override {
        _ticker++;
        uint32_t now = millis();

        if (_phase == Phase::SLEEPING) {
            // 8s recovery tick
            if (now - _lastTick >= TICK_MS) {
                _lastTick = now;
                EvaPet::get().sleepTick();
            }
            // Periodic heartbeat
            if (_ticker % HEARTBEAT_RATE == 0) {
                AudioManager::get().playHeartbeat();
            }
            // Auto-wake at energy > 85
            if (EvaPet::get().getStats().energy > 85) {
                _startWaking();
            }
        } else {  // WAKING
            _wakeProgress += 8;
            if (_wakeProgress >= 100) {
                requestTransition(GameState::MAIN);
            }
        }
    }

    void render() override {
        Renderer& R = Renderer::get();
        const EvaStats& st = EvaPet::get().getStats();
        uint32_t col = EVA_PERSONALITIES[st.evaUnit].primaryColor;

        R.clearCanvas(Colors::BG_DARK);
        R.canvas().drawRect(0, 0, Display::W, Display::H, col & 0x3F3F3F);

        if (_phase == Phase::SLEEPING) {
            _drawSleeping(R, st, col);
        } else {
            _drawWaking(R, col);
        }

        R.flush();
    }

    GameState handleInput(InputEvent ev) override {
        if (_phase == Phase::SLEEPING) {
            if (ev == InputEvent::BTN_A_CLICK || ev == InputEvent::BTN_B_CLICK ||
                ev == InputEvent::BTN_A_HOLD  || ev == InputEvent::BTN_B_HOLD) {
                _startWaking();
            }
        }
        return GameState::COUNT;
    }

private:
    enum class Phase { SLEEPING, WAKING };
    static constexpr uint32_t TICK_MS        = 8000;  // 8s sleep recovery
    static constexpr int      HEARTBEAT_RATE = 60;    // ticks between replays

    Phase    _phase        = Phase::SLEEPING;
    int      _ticker       = 0;
    uint32_t _lastTick     = 0;
    int      _wakeProgress = 0;

    void _startWaking() {
        _phase        = Phase::WAKING;
        _ticker       = 0;
        _wakeProgress = 0;
        AudioManager::get().playBeep(440, 50);
        M5.Display.setBrightness(55);
    }

    void _drawSleeping(Renderer& R, const EvaStats& st, uint32_t col) {
        AnimationSystem& anim = AnimationSystem::get();
        const uint8_t* frame  = anim.getCurrentFrame(st.evaUnit, AnimState::SLEEPING);
        anim.drawSprite(R.canvas(), frame, 8, 18, st.evaUnit, 3);

        R.canvas().setTextColor(Colors::NERV_BLUE);
        R.canvas().setTextSize(2);
        R.canvas().setCursor(Display::SPRITE_W + 8, 18);
        R.canvas().print("STANDBY");

        // ZZZ floating
        float breath = sinf(_ticker * 0.08f);
        int zx = Display::SPRITE_W + 14 + (int)(breath * 3.0f);
        int zy = 44 - (int)(breath * 5.0f);
        R.canvas().setTextColor(Colors::NERV_BLUE);
        R.canvas().setTextSize(2);
        int phase = (_ticker / 18) % 3;
        R.canvas().setCursor(zx, zy);
        if      (phase == 0) R.canvas().print("z");
        else if (phase == 1) R.canvas().print("zz");
        else                 R.canvas().print("zzz");

        // Energy bar
        R.canvas().setTextColor(Colors::NERV_BLUE & 0x7F7FFF);
        R.canvas().setTextSize(1);
        R.canvas().setCursor(Display::SPRITE_W + 8, 74);
        R.canvas().print("ENERGY RECOVERY");
        int ew = map(st.energy, 0, 100, 0, 152);
        R.canvas().fillRect(Display::SPRITE_W + 8, 84, ew, 8, Colors::NERV_BLUE);
        R.canvas().drawRect(Display::SPRITE_W + 8, 84, 152, 8, Colors::TEXT_DIM);

        // "PRESS ANY BTN TO WAKE" hint
        bool blink = (_ticker / 12) % 2 == 0;
        if (blink) {
            R.canvas().setTextColor(Colors::TEXT_DIM);
            R.canvas().setTextSize(1);
            R.canvas().setCursor(4, Display::H - 10);
            R.canvas().print("ANY BTN: WAKE UNIT");
        }
    }

    void _drawWaking(Renderer& R, uint32_t col) {
        int progress = min(_wakeProgress, 100);

        R.drawTextCentered("REACTIVATING", 34, col, 2);
        R.drawTextCentered("EVANGELION UNIT", 54, Colors::TEXT_DIM, 1);

        // Progress bar
        int pw = map(progress, 0, 100, 0, 220);
        R.canvas().fillRect(10, 76, pw, 10, col);
        R.canvas().drawRect(10, 76, 220, 10, Colors::TEXT_DIM);

        char pct[8];
        snprintf(pct, sizeof(pct), "%d%%", progress);
        R.drawTextCentered(pct, 92, col, 1);

        // Dot animation
        bool dot = (_ticker / 5) % 2 == 0;
        if (dot) {
            R.canvas().setTextColor(Colors::TEXT_DIM);
            R.canvas().setTextSize(1);
            R.canvas().setCursor(4, Display::H - 10);
            R.canvas().print("SYNCHRONIZATION RESTORING...");
        }
    }
};

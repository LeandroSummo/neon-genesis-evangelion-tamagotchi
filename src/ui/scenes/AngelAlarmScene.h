#pragma once
// ============================================================
// AngelAlarmScene — Emergency angel-attack alert
//
// Phase RED_ALERT      (2.5s): flashing black/red, "!! ALERT !!"
//                               corner arrows, SFX_ALERT_ANGEL loop
// Phase IDENTIFICATION (2.0s): angel name, threat assessment
// Phase BRIEFING       (2.0s): assigned game instructions (non-skippable)
// Phase DISPATCH       (0.8s): white flash, then → FightScene
//
// No user input accepted during any phase.
// ============================================================
#include "ui/SceneManager.h"
#include "ui/Renderer.h"
#include "entities/EvaPet.h"
#include "audio/AudioManager.h"
#include "data/LoreData.h"
#include "ui/scenes/FightScene.h"

class AngelAlarmScene : public BaseScene {
public:
    static AngelAlarmScene& get() { static AngelAlarmScene s; return s; }

    void onEnter() override {
        _phase          = Phase::RED_ALERT;
        _phaseStart     = millis();
        _ticker         = 0;
        _alertSoundTime = 0;
        _angelIdx       = EvaPet::get().getAngelIdx();
        EvaPet::get().clearAngelAlert();
        AudioManager::get().playAlertAngel();
    }

    void update() override {
        _ticker++;
        uint32_t now     = millis();
        uint32_t elapsed = now - _phaseStart;

        // Loop alert sound during alert/identification phases only
        if (_phase == Phase::RED_ALERT || _phase == Phase::IDENTIFICATION) {
            if (now - _alertSoundTime >= ALERT_LOOP_MS) {
                _alertSoundTime = now;
                AudioManager::get().playAlertAngel();
            }
        }

        if (_phase == Phase::RED_ALERT && elapsed >= RED_ALERT_MS) {
            _phase      = Phase::IDENTIFICATION;
            _phaseStart = now;
        } else if (_phase == Phase::IDENTIFICATION && elapsed >= IDENTIFY_MS) {
            _phase      = Phase::BRIEFING;
            _phaseStart = now;
            AudioManager::get().stopSequence();
        } else if (_phase == Phase::BRIEFING && elapsed >= BRIEFING_MS) {
            _phase      = Phase::DISPATCH;
            _phaseStart = now;
            AudioManager::get().playHitSuccess();  // "go" signal
        } else if (_phase == Phase::DISPATCH && elapsed >= DISPATCH_MS) {
            FightScene::get().setAngel(_angelIdx);
            requestTransition(GameState::FIGHT);
        }
    }

    void render() override {
        Renderer& r = Renderer::get();
        r.clearCanvas(Colors::BLACK);

        switch (_phase) {
            case Phase::RED_ALERT:      _drawRedAlert(r);       break;
            case Phase::IDENTIFICATION: _drawIdentification(r); break;
            case Phase::BRIEFING:       _drawBriefing(r);       break;
            case Phase::DISPATCH:       _drawDispatch(r);       break;
        }

        r.flush();
    }

    GameState handleInput(InputEvent) override { return GameState::COUNT; }

private:
    enum class Phase { RED_ALERT, IDENTIFICATION, BRIEFING, DISPATCH };

    static constexpr uint32_t RED_ALERT_MS  = 2500;
    static constexpr uint32_t IDENTIFY_MS   = 2000;
    static constexpr uint32_t BRIEFING_MS   = 2000;
    static constexpr uint32_t DISPATCH_MS   = 800;
    static constexpr uint32_t ALERT_LOOP_MS = 1800;

    Phase    _phase          = Phase::RED_ALERT;
    uint32_t _phaseStart     = 0;
    uint32_t _alertSoundTime = 0;
    uint8_t  _angelIdx       = 0;
    int      _ticker         = 0;

    // ── Corner arrows pointing inward ────────────────────
    void _drawCornerArrows(LGFX_Sprite& c, uint32_t col, int16_t aw = 18) {
        // Top-left: right-down triangle
        c.fillTriangle(0, 0, aw, 0, 0, aw, col);
        // Top-right
        c.fillTriangle(Display::W, 0, Display::W - aw, 0, Display::W, aw, col);
        // Bottom-left
        c.fillTriangle(0, Display::H, aw, Display::H, 0, Display::H - aw, col);
        // Bottom-right
        c.fillTriangle(Display::W, Display::H, Display::W - aw, Display::H,
                       Display::W, Display::H - aw, col);
    }

    void _drawRedAlert(Renderer& r) {
        LGFX_Sprite& c  = r.canvas();
        bool flashOn    = (_ticker / 4) % 2 == 0;  // 4Hz at 30fps

        uint32_t bgCol = flashOn ? 0x1A0000 : Colors::BLACK;
        c.fillScreen(bgCol);

        // Scanlines for CRT feel
        for (int16_t y = 1; y < Display::H; y += 4) {
            c.drawFastHLine(0, y, Display::W, 0x0A0000);
        }

        // Corner arrows
        _drawCornerArrows(c, flashOn ? Colors::NERV_RED : Colors::NERV_RED_DIM);

        // Border
        c.drawRect(0, 0, Display::W, Display::H, Colors::NERV_RED);

        // "!! ALERT !!" flashing
        uint32_t alertCol = flashOn ? Colors::WHITE : Colors::NERV_RED;
        r.drawTextCentered("!! ALERT !!", 28, alertCol, 2);

        // Sub-text
        r.drawTextCentered("PATTERN BLUE CONFIRMED", 54, Colors::NERV_RED, 1);
        r.drawTextCentered("CATEGORY F THREAT", 66, Colors::NERV_RED_DIM, 1);
    }

    void _drawIdentification(Renderer& r) {
        LGFX_Sprite& c   = r.canvas();
        uint32_t elapsed = millis() - _phaseStart;

        // Solid red background
        c.fillScreen(0x120000);

        // Top bar
        c.fillRect(0, 0, Display::W, 18, Colors::BLACK);
        c.setTextColor(Colors::NERV_YELLOW);
        c.setTextSize(1);
        c.setCursor(4, 5);
        c.print("NERV EMERGENCY BROADCAST");

        c.drawFastHLine(0, 18, Display::W, Colors::NERV_RED);

        // Angel name — large, centered
        const char* name = ANGEL_LIST[_angelIdx].name;
        r.drawTextCentered(name, 30, Colors::WHITE, 3);

        // Angel number
        char numBuf[16];
        snprintf(numBuf, sizeof(numBuf), "ANGEL N.%02d", _angelIdx + 1);
        r.drawTextCentered(numBuf, 58, Colors::NERV_RED_DIM, 1);

        r.drawTextCentered("THREAT ASSESSMENT: EXTREME", 70, Colors::NERV_RED, 1);

        // Bottom bar with animated "SCRAMBLE..."
        c.fillRect(0, Display::H - 18, Display::W, 18, Colors::BLACK);
        c.drawFastHLine(0, Display::H - 18, Display::W, Colors::NERV_RED);

        int dotCount = (elapsed / 500) % 4;
        char scramBuf[20];
        strncpy(scramBuf, "SCRAMBLE", sizeof(scramBuf));
        for (int i = 0; i < dotCount; i++) strncat(scramBuf, ".", sizeof(scramBuf) - strlen(scramBuf) - 1);
        r.drawTextCentered(scramBuf, Display::H - 13, Colors::NERV_YELLOW, 1);

        // Border
        c.drawRect(0, 0, Display::W, Display::H, Colors::NERV_RED);
    }

    void _drawBriefing(Renderer& r) {
        LGFX_Sprite& c = r.canvas();
        c.fillScreen(Colors::BLACK);

        // Red header bar
        c.fillRect(0, 0, Display::W, 16, 0x1A0000);
        c.drawFastHLine(0, 16, Display::W, Colors::NERV_RED);
        c.setTextColor(Colors::NERV_RED);
        c.setTextSize(1);
        c.setCursor(4, 4);
        c.print("COMBAT BRIEFING");

        // Game name
        uint8_t gt = ANGEL_LIST[_angelIdx].gameType;
        c.setTextColor(Colors::NERV_YELLOW);
        c.setTextSize(1);
        int16_t nw = (int16_t)(strlen(GAME_DEFS[gt].name) * 6);
        c.setCursor((Display::W - nw) / 2, 24);
        c.print(GAME_DEFS[gt].name);

        // Stars
        c.setTextColor(Colors::NERV_YELLOW);
        int16_t sw = GAME_DEFS[gt].stars * 14 - 2;
        int16_t sx = (Display::W - sw) / 2;
        for (int i = 0; i < 3; i++) {
            c.setCursor(sx + i * 14, 36);
            c.print(i < GAME_DEFS[gt].stars ? "* " : "- ");
        }

        // Instructions
        c.setTextColor(Colors::WHITE);
        c.setTextSize(1);
        c.setCursor(6, 52);
        c.print(GAME_DEFS[gt].instr1);
        c.setTextColor(Colors::TEXT_DIM);
        c.setCursor(6, 66);
        c.print(GAME_DEFS[gt].instr2);

        // Separator
        c.drawFastHLine(0, Display::H - 16, Display::W, Colors::NERV_YELLOW);

        // Bottom footer
        c.fillRect(0, Display::H - 15, Display::W, 15, 0x0A0800);
        c.setTextColor(Colors::NERV_YELLOW);
        c.setTextSize(1);
        const char* footer = "UNIT  --  SCRAMBLE";
        int16_t fw = (int16_t)(strlen(footer) * 6);
        c.setCursor((Display::W - fw) / 2, Display::H - 11);
        c.print(footer);

        c.drawRect(0, 0, Display::W, Display::H, Colors::NERV_RED);
    }

    void _drawDispatch(Renderer& r) {
        uint32_t elapsed = millis() - _phaseStart;
        float    prog    = (float)elapsed / DISPATCH_MS;

        // White panels grow from left and right
        int16_t coverW = (int16_t)(Display::W / 2 * prog);
        r.canvas().fillScreen(Colors::BLACK);
        r.canvas().fillRect(0, 0, coverW, Display::H, Colors::WHITE);
        r.canvas().fillRect(Display::W - coverW, 0, coverW, Display::H, Colors::WHITE);

        if (prog > 0.5f) r.canvas().fillScreen(Colors::WHITE);
    }
};

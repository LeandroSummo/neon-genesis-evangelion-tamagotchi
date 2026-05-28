#pragma once
// ============================================================
// FightScene — 9 mini-games with selection menu
//
// Phase: GAME_SELECT → INSTRUCTIONS → READY → ACTIVE → RESULT
// Angel fights: skip GAME_SELECT, jump to INSTRUCTIONS.
// setAngel(idx) must be called before requestTransition(FIGHT).
// ============================================================
#include "ui/SceneManager.h"
#include "entities/EvaPet.h"
#include "ui/Renderer.h"
#include "audio/AudioManager.h"
#include "data/LoreData.h"

class FightScene : public BaseScene {
public:
    static FightScene& get() { static FightScene s; return s; }

    void setAngel(uint8_t idx) {
        _isAngelFight = true;
        _angelName    = ANGEL_LIST[idx].name;
        _forcedGame   = ANGEL_LIST[idx].gameType;
    }

    void onEnter() override {
        _phase         = _isAngelFight ? Phase::INSTRUCTIONS : Phase::GAME_SELECT;
        _phaseStart    = millis();
        _gameSel       = 0;
        _round         = 0;
        _hits          = 0;
        _mashCount     = 0;
        _hitThisRound  = false;
        _correct       = random(0, 2);
        _ticker        = 0;
        _resultApplied = false;
        if (_isAngelFight) _game = (MiniGame)_forcedGame;
        _resetGameState();
    }

    void onExit() override {
        _isAngelFight = false;
        _angelName    = nullptr;
        _forcedGame   = 255;
    }

    void update() override {
        _ticker++;
        uint32_t now = millis();

        if (_phase == Phase::GAME_SELECT) {
            // waiting for input
        } else if (_phase == Phase::INSTRUCTIONS) {
            // waiting for BTN_B
        } else if (_phase == Phase::READY) {
            if (now - _phaseStart >= READY_MS) {
                _phase      = Phase::ACTIVE;
                _phaseStart = now;
                _roundStart = now;
            }
        } else if (_phase == Phase::ACTIVE) {
            _updateActive(now);
        } else if (_phase == Phase::RESULT) {
            if (now - _phaseStart >= RESULT_MS) {
                requestTransition(GameState::MAIN);
            }
        }
    }

    void render() override {
        Renderer& r = Renderer::get();
        const EvaStats& st = EvaPet::get().getStats();
        uint32_t col = EVA_PERSONALITIES[st.evaUnit].primaryColor;

        r.clearCanvas(Colors::BG_DARK);

        switch (_phase) {
            case Phase::GAME_SELECT:   _drawGameSelect(r, col);   break;
            case Phase::INSTRUCTIONS:  _drawInstructions(r, col); break;
            case Phase::READY:         _drawReady(r, col);        break;
            case Phase::ACTIVE:        _drawActive(r, col);       break;
            case Phase::RESULT:        _drawResult(r);            break;
        }

        r.flush();
    }

    GameState handleInput(InputEvent ev) override {
        uint32_t now = millis();

        if (_phase == Phase::GAME_SELECT) {
            if (ev == InputEvent::BTN_A_CLICK) {
                _gameSel = (_gameSel + 1) % GAME_COUNT;
                AudioManager::get().playBeep(500, 35);
            }
            if (ev == InputEvent::BTN_B_CLICK) {
                _game       = (MiniGame)_gameSel;
                _phase      = Phase::INSTRUCTIONS;
                _phaseStart = now;
                AudioManager::get().playSelect();
            }
            return GameState::COUNT;
        }

        if (_phase == Phase::INSTRUCTIONS) {
            if (ev == InputEvent::BTN_B_CLICK) {
                _phase      = Phase::READY;
                _phaseStart = now;
                _resetGameState();
                AudioManager::get().playSelect();
            }
            return GameState::COUNT;
        }

        if (_phase != Phase::ACTIVE) return GameState::COUNT;

        switch (_game) {
            case MiniGame::SYNC_PULSE:      _inputSyncPulse(ev, now);   break;
            case MiniGame::INTERCEPT:       _inputIntercept(ev, now);   break;
            case MiniGame::BERSERK_CONTROL: _inputBerserk(ev);          break;
            case MiniGame::LANCE:           _inputLance(ev, now);       break;
            case MiniGame::MAGI_VOTE:       _inputMagi(ev, now);        break;
            case MiniGame::DUMMY_PLUG:      _inputDummy(ev, now);       break;
            case MiniGame::CORE_DEFENSE:    _inputCoreDefense(ev, now); break;
            case MiniGame::CALIBRATION:     _inputCalibration(ev);      break;
            case MiniGame::THIRD_IMPACT:    _inputThirdImpact(ev, now); break;
        }
        return GameState::COUNT;
    }

private:
    // ── Enums / constants ──────────────────────────────────

    enum class MiniGame : uint8_t {
        SYNC_PULSE = 0, INTERCEPT = 1, BERSERK_CONTROL = 2,
        LANCE = 3, MAGI_VOTE = 4, DUMMY_PLUG = 5,
        CORE_DEFENSE = 6, CALIBRATION = 7, THIRD_IMPACT = 8
    };
    enum class Phase { GAME_SELECT, INSTRUCTIONS, READY, ACTIVE, RESULT };

    static constexpr int      MAX_ROUNDS       = 5;
    static constexpr uint32_t READY_MS         = 1500;
    static constexpr uint32_t RESULT_MS        = 2500;
    static constexpr uint32_t PULSE_ROUND_MS   = 2000;
    static constexpr int      PULSE_MAX_R      = 55;
    static constexpr int      PULSE_TARGET_R   = 38;
    static constexpr int      PULSE_TOLERANCE  = 10;
    static constexpr uint32_t INTERCEPT_WIN_MS = 1200;
    static constexpr uint32_t MASH_DUR_MS      = 5000;
    static constexpr uint32_t MAGI_ROUND_MS    = 2500;
    static constexpr uint32_t CALIB_DUR_MS     = 15000;
    static constexpr uint32_t TI_DUR_MS        = 10000;

    // ── State ─────────────────────────────────────────────

    MiniGame    _game          = MiniGame::SYNC_PULSE;
    Phase       _phase         = Phase::GAME_SELECT;
    uint32_t    _phaseStart    = 0;
    uint32_t    _roundStart    = 0;
    int         _round         = 0;
    int         _hits          = 0;
    int         _mashCount     = 0;
    bool        _hitThisRound  = false;
    int         _correct       = 0;
    int         _ticker        = 0;
    int         _gameSel       = 0;

    bool        _isAngelFight  = false;
    const char* _angelName     = nullptr;
    uint8_t     _forcedGame    = 255;
    bool        _resultApplied = false;

    // Lance state
    uint32_t _lanceStart    = 0;
    int      _lanceRound    = 0;

    // Magi state
    uint8_t  _magiVotes[3]  = {0,0,0};
    uint8_t  _magiMajority  = 0;
    bool     _magiAnswered  = false;
    int      _magiScore     = 0;

    // Dummy Plug state
    uint8_t  _dpSeq[6]      = {};
    uint8_t  _dpInput[6]    = {};
    uint8_t  _dpSeqLen      = 4;
    uint8_t  _dpShowIdx     = 0;
    uint8_t  _dpInputIdx    = 0;
    uint8_t  _dpRound       = 0;
    uint8_t  _dpRoundsWon   = 0;
    bool     _dpShowing     = true;
    uint32_t _dpShowStart   = 0;

    // Core Defense state
    int      _cbLives       = 3;
    int      _cbHits        = 0;

    // Calibration state
    float    _calPos        = 0.0f;
    float    _calVel        = 0.0f;
    int      _calInZone     = 0;
    int      _calTotal      = 0;

    // Third Impact state
    float    _tiTimer       = 10.0f;
    uint32_t _tiLast        = 0;
    int      _tiPresses     = 0;

    // ── Reset per-game state ──────────────────────────────

    void _resetGameState() {
        _round        = 0;
        _hits         = 0;
        _mashCount    = 0;
        _hitThisRound = false;
        _correct      = random(0, 2);
        _lanceRound   = 0;
        _magiScore    = 0;
        _magiAnswered = false;
        _dpRound      = 0;
        _dpRoundsWon  = 0;
        _dpShowing    = true;
        _dpSeqLen     = 4;
        _dpShowIdx    = 0;
        _dpInputIdx   = 0;
        _dpShowStart  = millis();
        _cbLives      = 3;
        _cbHits       = 0;
        _calPos       = 0.0f;
        _calVel       = 0.0f;
        _calInZone    = 0;
        _calTotal     = 0;
        _tiTimer      = 10.0f;
        _tiLast       = millis();
        _tiPresses    = 0;
        _generateMagi();
        _generateDummySeq();
    }

    void _generateMagi() {
        _magiMajority = random(0, 2);  // 0=NO, 1=YES
        int count = 0;
        for (int i = 0; i < 3; i++) {
            _magiVotes[i] = (count < 2 && random(0,2) == _magiMajority) ? _magiMajority
                                                                          : (1 - _magiMajority);
        }
        // Ensure majority (2 of 3) vote correctly
        int yes = 0;
        for (int i = 0; i < 3; i++) yes += (_magiVotes[i] == 1) ? 1 : 0;
        if (_magiMajority == 1 && yes < 2) { _magiVotes[0] = 1; _magiVotes[1] = 1; }
        if (_magiMajority == 0 && yes > 1) { _magiVotes[0] = 0; _magiVotes[1] = 0; }
        _magiAnswered = false;
    }

    void _generateDummySeq() {
        for (int i = 0; i < _dpSeqLen; i++)
            _dpSeq[i] = random(0, 2);
        _dpShowIdx   = 0;
        _dpInputIdx  = 0;
        _dpShowing   = true;
        _dpShowStart = millis();
    }

    // ── Update helpers ────────────────────────────────────

    void _updateActive(uint32_t now) {
        switch (_game) {
            case MiniGame::BERSERK_CONTROL:
                if (now - _phaseStart >= MASH_DUR_MS) {
                    if (!_isAngelFight && _mashCount >= 8) {
                        EvaPet::get().getStatsMut().syncRate =
                            statClamp(EvaPet::get().getStatsMut().syncRate + 10);
                        EvaPet::get().markDirty();
                    }
                    _hits = _mashCount;
                    _goResult(now);
                }
                break;

            case MiniGame::SYNC_PULSE:
            case MiniGame::INTERCEPT:
            case MiniGame::LANCE:
            case MiniGame::CORE_DEFENSE:
            {
                uint32_t windowMs = ((_game == MiniGame::SYNC_PULSE) ? PULSE_ROUND_MS :
                                     (_game == MiniGame::LANCE)       ? _lanceRoundMs() :
                                     (_game == MiniGame::INTERCEPT || _game == MiniGame::CORE_DEFENSE) ? INTERCEPT_WIN_MS : 0);
                if (now - _roundStart >= windowMs) {
                    if (_game == MiniGame::INTERCEPT || _game == MiniGame::CORE_DEFENSE) {
                        EvaPet::get().getStatsMut().rage =
                            statClamp(EvaPet::get().getStatsMut().rage + 5);
                        EvaPet::get().markDirty();
                        if (_game == MiniGame::CORE_DEFENSE) { _cbHits++; }
                    }
                    _advanceRound(now);
                }
                // Core defense: 3 lives check
                if (_game == MiniGame::CORE_DEFENSE && _cbHits >= 3) {
                    _goResult(now);
                }
                break;
            }

            case MiniGame::MAGI_VOTE:
                if (now - _roundStart >= MAGI_ROUND_MS && !_magiAnswered) {
                    // Timeout = wrong answer
                    _round++;
                    if (_round >= 8) { _hits = _magiScore; _goResult(now); }
                    else { _roundStart = now; _generateMagi(); }
                }
                break;

            case MiniGame::DUMMY_PLUG:
                _updateDummy(now);
                break;

            case MiniGame::CALIBRATION:
                _updateCalibration(now);
                break;

            case MiniGame::THIRD_IMPACT:
                _updateThirdImpact(now);
                break;
        }
    }

    uint32_t _lanceRoundMs() {
        // Speed increases: 2000ms, 1600ms, 1300ms, 1100ms, 950ms
        static const uint32_t speeds[] = {2000,1600,1300,1100,950};
        return speeds[min(_lanceRound, 4)];
    }

    void _updateDummy(uint32_t now) {
        if (_dpShowing) {
            uint32_t showDur = now - _dpShowStart;
            if (showDur >= (uint32_t)(_dpShowIdx * 700 + 700)) {
                _dpShowIdx++;
                if (_dpShowIdx >= _dpSeqLen) {
                    _dpShowing  = false;
                    _dpInputIdx = 0;
                }
            }
        }
        // If round timed out without full input (5s window)
        if (!_dpShowing && now - _roundStart >= 5000) {
            _dpRound++;
            if (_dpRound >= 3) { _hits = _dpRoundsWon; _goResult(now); }
            else { _dpSeqLen = 4 + _dpRound; _roundStart = now; _generateDummySeq(); }
        }
    }

    void _updateCalibration(uint32_t now) {
        float dt = 0.033f;  // ~30fps
        float push = 0.0f;
        if (M5.BtnA.isPressed()) push = 0.12f;
        if (M5.BtnB.isPressed()) push = -0.12f;
        _calVel  = _calVel * 0.92f + push + (random(-5,6) * 0.004f);
        _calPos  = max(-1.0f, min(1.0f, _calPos + _calVel * dt));
        _calTotal++;
        if (fabsf(_calPos) < 0.2f) _calInZone++;

        if (now - _phaseStart >= CALIB_DUR_MS) {
            _hits = _calTotal > 0 ? (_calInZone * 100 / _calTotal) : 0;
            _goResult(now);
        }
    }

    void _updateThirdImpact(uint32_t now) {
        float elapsed = (now - _tiLast) / 1000.0f;
        _tiLast = now;
        _tiTimer -= elapsed;
        if (_tiTimer <= 0.0f) {
            _tiTimer = 0.0f;
            _hits = _tiPresses;
            _goResult(now);
        }
    }

    void _advanceRound(uint32_t now) {
        _round++;
        int maxRounds = (_game == MiniGame::CORE_DEFENSE) ? 12 : MAX_ROUNDS;
        if (_round >= maxRounds) {
            _goResult(now);
        } else {
            _roundStart   = now;
            _hitThisRound = false;
            _correct      = random(0, 2);
            if (_game == MiniGame::LANCE) _lanceRound++;
        }
    }

    void _goResult(uint32_t now) {
        _phase      = Phase::RESULT;
        _phaseStart = now;
        _applyResult();
    }

    // ── Input handlers ────────────────────────────────────

    void _inputSyncPulse(InputEvent ev, uint32_t now) {
        if (ev != InputEvent::BTN_A_CLICK) return;
        uint32_t elapsed = now - _roundStart;
        int ring  = (int)map(elapsed, 0, PULSE_ROUND_MS, 0, PULSE_MAX_R);
        int delta = abs(ring - PULSE_TARGET_R);
        if (!_hitThisRound && delta <= PULSE_TOLERANCE) {
            _registerHit();
        } else {
            EvaPet::get().getStatsMut().syncRate =
                statClamp(EvaPet::get().getStatsMut().syncRate - 2);
            EvaPet::get().markDirty();
            AudioManager::get().playHitMiss();
        }
        _advanceRound(now);
    }

    void _inputIntercept(InputEvent ev, uint32_t now) {
        bool pA = (ev == InputEvent::BTN_A_CLICK);
        bool pB = (ev == InputEvent::BTN_B_CLICK);
        if (!pA && !pB) return;
        uint32_t elapsed = now - _roundStart;
        bool inWin  = elapsed < INTERCEPT_WIN_MS;
        bool ok     = (pA && _correct == 0) || (pB && _correct == 1);
        if (inWin && ok) { _registerHit(); }
        else {
            EvaPet::get().getStatsMut().rage =
                statClamp(EvaPet::get().getStatsMut().rage + 5);
            EvaPet::get().markDirty();
            AudioManager::get().playHitMiss();
        }
        _advanceRound(now);
    }

    void _inputBerserk(InputEvent ev) {
        if (ev != InputEvent::BTN_A_CLICK) return;
        _mashCount++;
        EvaPet::get().getStatsMut().rage =
            statClamp(EvaPet::get().getStatsMut().rage - 2);
        EvaPet::get().getStatsMut().energy =
            statClamp(EvaPet::get().getStatsMut().energy - 1);
        EvaPet::get().markDirty();
        AudioManager::get().playBeep(300 + _mashCount * 15, 20);
    }

    void _inputLance(InputEvent ev, uint32_t now) {
        if (ev != InputEvent::BTN_A_CLICK) return;
        uint32_t elapsed = now - _roundStart;
        uint32_t ms = _lanceRoundMs();
        int16_t  lx = (int16_t)map(elapsed, 0, ms, 0, Display::W);
        int16_t  cx = Display::W / 2;
        if (abs(lx - cx) <= 10) { _registerHit(); }
        else {
            EvaPet::get().getStatsMut().syncRate =
                statClamp(EvaPet::get().getStatsMut().syncRate - 2);
            EvaPet::get().markDirty();
            AudioManager::get().playHitMiss();
        }
        _advanceRound(now);
        _lanceStart = now;
    }

    void _inputMagi(InputEvent ev, uint32_t now) {
        if (_magiAnswered) return;
        bool pA = (ev == InputEvent::BTN_A_CLICK);
        bool pB = (ev == InputEvent::BTN_B_CLICK);
        if (!pA && !pB) return;
        _magiAnswered = true;
        uint8_t vote = pA ? 1 : 0;  // BTN_A=YES=1, BTN_B=NO=0
        if (vote == _magiMajority) {
            _magiScore++;
            AudioManager::get().playHitSuccess();
        } else {
            AudioManager::get().playHitMiss();
        }
        _round++;
        if (_round >= 8) { _hits = _magiScore; _goResult(now); }
        else { _roundStart = now; _generateMagi(); }
    }

    void _inputDummy(InputEvent ev, uint32_t now) {
        if (_dpShowing) return;  // wait until sequence shown
        uint8_t pressed = (ev == InputEvent::BTN_A_CLICK) ? 0 :
                          (ev == InputEvent::BTN_B_CLICK) ? 1 : 255;
        if (pressed == 255) return;

        _dpInput[_dpInputIdx] = pressed;
        if (pressed == _dpSeq[_dpInputIdx]) {
            AudioManager::get().playBeep(500 + _dpInputIdx * 50, 30);
            _dpInputIdx++;
            if (_dpInputIdx >= _dpSeqLen) {
                _dpRoundsWon++;
                _dpRound++;
                if (_dpRound >= 3) { _hits = _dpRoundsWon; _goResult(now); }
                else { _dpSeqLen = 4 + _dpRound; _roundStart = now; _generateDummySeq(); }
            }
        } else {
            AudioManager::get().playHitMiss();
            _dpRound++;
            if (_dpRound >= 3) { _hits = _dpRoundsWon; _goResult(now); }
            else { _dpSeqLen = 4 + _dpRound; _roundStart = now; _generateDummySeq(); }
        }
    }

    void _inputCoreDefense(InputEvent ev, uint32_t now) {
        bool pA = (ev == InputEvent::BTN_A_CLICK);
        bool pB = (ev == InputEvent::BTN_B_CLICK);
        if (!pA && !pB) return;
        uint32_t elapsed = now - _roundStart;
        bool inWin = elapsed < INTERCEPT_WIN_MS;
        bool ok    = (pA && _correct == 0) || (pB && _correct == 1);
        if (inWin && ok) { _registerHit(); }
        else {
            _cbHits++;
            EvaPet::get().getStatsMut().syncRate =
                statClamp(EvaPet::get().getStatsMut().syncRate - 3);
            EvaPet::get().markDirty();
            AudioManager::get().playHitMiss();
            if (_cbHits >= 3) { _goResult(now); return; }
        }
        _advanceRound(now);
    }

    void _inputCalibration(InputEvent ev) {
        // Handled in _updateCalibration via isPressed()
    }

    void _inputThirdImpact(InputEvent ev, uint32_t now) {
        if (ev != InputEvent::BTN_A_CLICK) return;
        _tiPresses++;
        _tiTimer -= 0.3f;
        if (_tiTimer < 0.0f) _tiTimer = 0.0f;
        AudioManager::get().playBeep(400 + _tiPresses * 10, 15);
        if (_tiTimer <= 0.0f) { _hits = _tiPresses; _goResult(now); }
    }

    // ── Common helpers ────────────────────────────────────

    void _registerHit() {
        _hits++;
        _hitThisRound = true;
        EvaPet::get().applyFightHit();
        AudioManager::get().playHitSuccess();
    }

    void _applyResult() {
        if (_resultApplied) return;
        _resultApplied = true;
        if (!_isAngelFight) return;

        bool victory;
        switch (_game) {
            case MiniGame::BERSERK_CONTROL: victory = (_mashCount >= 8);        break;
            case MiniGame::MAGI_VOTE:       victory = (_magiScore >= 5);        break;
            case MiniGame::DUMMY_PLUG:      victory = (_dpRoundsWon >= 2);      break;
            case MiniGame::CALIBRATION:     victory = (_hits >= 50);            break;  // 50% in zone
            case MiniGame::THIRD_IMPACT:    victory = (_tiPresses >= 30);       break;
            default:                        victory = (_hits >= 3);             break;
        }

        EvaStats& s = EvaPet::get().getStatsMut();
        if (victory) {
            s.syncRate = statClamp(s.syncRate + 15);
            AudioManager::get().playAngelDefeated();
        } else {
            s.syncRate = statClamp(s.syncRate - 15);
            s.energy   = statClamp(s.energy   - 20);
            AudioManager::get().playUnitDamaged();
        }
        EvaPet::get().markDirty();
    }

    // ── Draw: GAME_SELECT ─────────────────────────────────

    void _drawGameSelect(Renderer& r, uint32_t col) {
        r.canvas().fillRect(0, 0, Display::W, 13, 0x060810);
        r.canvas().drawFastHLine(0, 13, Display::W, col);
        r.canvas().setTextColor(col);
        r.canvas().setTextSize(1);
        r.canvas().setCursor(4, 3);
        r.canvas().print("SELECT COMBAT MODE");

        int16_t y = 17;
        int16_t rowH = 12;
        for (int i = 0; i < GAME_COUNT; i++) {
            bool sel = (i == _gameSel);
            if (sel) {
                r.canvas().fillRect(2, y - 1, Display::W - 4, rowH, col & 0x1E1E1E);
                r.canvas().drawRect(2, y - 1, Display::W - 4, rowH, col);
            }
            r.canvas().setTextColor(sel ? col : Colors::TEXT_DIM);
            r.canvas().setTextSize(1);
            r.canvas().setCursor(8, y + 1);
            r.canvas().print(GAME_DEFS[i].name);

            // Stars
            r.canvas().setTextColor(sel ? Colors::NERV_YELLOW : Colors::TEXT_DIM);
            for (int s = 0; s < 3; s++) {
                r.canvas().setCursor(Display::W - 24 + s * 7, y + 1);
                r.canvas().print(s < GAME_DEFS[i].stars ? "*" : "-");
            }
            y += rowH;
        }

        r.canvas().setTextColor(Colors::TEXT_DIM);
        r.canvas().setTextSize(1);
        r.canvas().setCursor(4, Display::H - 10);
        r.canvas().print("[A] CYCLE  [B] SELECT");
    }

    // ── Draw: INSTRUCTIONS ────────────────────────────────

    void _drawInstructions(Renderer& r, uint32_t col) {
        r.canvas().fillRect(0, 0, Display::W, 13, 0x060810);
        r.canvas().drawFastHLine(0, 13, Display::W, col);
        r.canvas().setTextColor(col);
        r.canvas().setTextSize(1);
        r.canvas().setCursor(4, 3);
        r.canvas().print(GAME_DEFS[(int)_game].name);

        if (_isAngelFight && _angelName) {
            r.canvas().setTextColor(Colors::NERV_RED);
            r.canvas().setTextSize(1);
            char buf[20];
            snprintf(buf, sizeof(buf), "VS  %s", _angelName);
            int16_t tw = strlen(buf) * 6;
            r.canvas().setCursor(Display::W - tw - 4, 3);
            r.canvas().print(buf);
        }

        r.canvas().drawFastHLine(0, 13, Display::W, col);

        r.canvas().setTextColor(Colors::WHITE);
        r.canvas().setTextSize(1);
        r.canvas().setCursor(8, 26);
        r.canvas().print(GAME_DEFS[(int)_game].instr1);
        r.canvas().setCursor(8, 40);
        r.canvas().print(GAME_DEFS[(int)_game].instr2);

        // Stars
        r.canvas().setCursor(8, 58);
        r.canvas().setTextColor(Colors::NERV_YELLOW);
        for (int s = 0; s < 3; s++) {
            r.canvas().print(s < GAME_DEFS[(int)_game].stars ? "* " : "- ");
        }

        bool blink = (_ticker / 8) % 2 == 0;
        if (blink) {
            r.canvas().setTextColor(Colors::NERV_AMBER);
            r.canvas().setTextSize(1);
            int16_t tw = 14 * 6;
            r.canvas().setCursor((Display::W - tw) / 2, Display::H - 14);
            r.canvas().print("[BTN_B] TO START");
        }
    }

    // ── Draw: READY ───────────────────────────────────────

    void _drawReady(Renderer& r, uint32_t col) {
        r.canvas().drawRect(0, 0, Display::W, Display::H, col);
        r.drawTextCentered(GAME_DEFS[(int)_game].name, 18, col, 1);

        if (_isAngelFight && _angelName) {
            char buf[24];
            snprintf(buf, sizeof(buf), "VS  %s", _angelName);
            r.drawTextCentered(buf, 30, Colors::NERV_RED, 1);
        }

        uint32_t elapsed = millis() - _phaseStart;
        int count = 3 - (int)(elapsed / 500);
        if (count > 0 && count <= 3) {
            char buf[4];
            snprintf(buf, sizeof(buf), "%d", count);
            uint32_t fc = (_ticker / 4) % 2 == 0 ? Colors::WHITE : col;
            r.drawTextCentered(buf, 52, fc, 3);
        }
    }

    // ── Draw: ACTIVE ──────────────────────────────────────

    void _drawActive(Renderer& r, uint32_t col) {
        uint32_t now = millis();
        r.canvas().drawRect(0, 0, Display::W, Display::H, col);

        switch (_game) {
            case MiniGame::SYNC_PULSE:      _drawSyncPulse(r, now, col);    break;
            case MiniGame::INTERCEPT:       _drawIntercept(r, now, col);    break;
            case MiniGame::BERSERK_CONTROL: _drawBerserk(r, now, col);      break;
            case MiniGame::LANCE:           _drawLance(r, now, col);        break;
            case MiniGame::MAGI_VOTE:       _drawMagi(r, now, col);         break;
            case MiniGame::DUMMY_PLUG:      _drawDummy(r, now, col);        break;
            case MiniGame::CORE_DEFENSE:    _drawCoreDefense(r, now, col);  break;
            case MiniGame::CALIBRATION:     _drawCalibration(r, now, col);  break;
            case MiniGame::THIRD_IMPACT:    _drawThirdImpact(r, now, col);  break;
        }
    }

    void _drawSyncPulse(Renderer& r, uint32_t now, uint32_t col) {
        uint32_t elapsed = now - _roundStart;
        int16_t cx = Display::W / 2, cy = 60;
        int curR = (int)map(elapsed, 0, PULSE_ROUND_MS, 0, PULSE_MAX_R);

        for (int t = 0; t < 3; t++)
            r.canvas().drawCircle(cx, cy, PULSE_TARGET_R - t, Colors::NERV_YELLOW);

        uint32_t rc = abs(curR - PULSE_TARGET_R) <= PULSE_TOLERANCE ? Colors::NERV_GREEN : col;
        for (int t = 0; t < 4; t++)
            if (curR - t > 0) r.canvas().drawCircle(cx, cy, curR - t, rc);

        _drawRoundDots(r, MAX_ROUNDS);
        r.canvas().setTextColor(Colors::TEXT_DIM);
        r.canvas().setTextSize(1);
        r.canvas().setCursor(4, 4);
        r.canvas().print("ROUND "); r.canvas().print(_round + 1);
    }

    void _drawIntercept(Renderer& r, uint32_t now, uint32_t col) {
        uint32_t elapsed = now - _roundStart;
        bool inWin = elapsed < INTERCEPT_WIN_MS;
        const char* prompt = (_correct == 0) ? "[ A ]" : "[ B ]";
        r.drawTextCentered(prompt, 42, inWin ? col : Colors::TEXT_DIM, 3);
        int bw = inWin ? (int)map(elapsed, 0, INTERCEPT_WIN_MS, 200, 0) : 0;
        r.canvas().fillRect(20, 90, bw, 10, col);
        r.canvas().drawRect(20, 90, 200, 10, Colors::TEXT_DIM);
        _drawRoundDots(r, MAX_ROUNDS);
    }

    void _drawBerserk(Renderer& r, uint32_t now, uint32_t col) {
        uint32_t rem = (now - _phaseStart < MASH_DUR_MS) ? MASH_DUR_MS - (now - _phaseStart) : 0;
        r.drawTextCentered("MASH BTN_A!", 26, Colors::NERV_RED, 2);
        char buf[16];
        snprintf(buf, sizeof(buf), "PRESSES: %d", _mashCount);
        r.drawTextCentered(buf, 56, col, 2);
        int tw = (int)map(rem, 0, MASH_DUR_MS, 0, 220);
        r.canvas().fillRect(10, 82, tw, 12, Colors::NERV_RED);
        r.canvas().drawRect(10, 82, 220, 12, Colors::TEXT_DIM);
    }

    void _drawLance(Renderer& r, uint32_t now, uint32_t col) {
        uint32_t elapsed = now - _roundStart;
        uint32_t ms = _lanceRoundMs();
        int16_t lx = (int16_t)map(min(elapsed, ms), 0, ms, 0, Display::W);
        int16_t cy = Display::H / 2;
        int16_t cx = Display::W / 2;

        // Target circle
        r.canvas().drawCircle(cx, cy, 8, Colors::NERV_YELLOW);

        // Lance (diagonal segment)
        bool inZone = abs(lx - cx) <= 10;
        uint32_t lc = inZone ? Colors::NERV_GREEN : col;
        r.canvas().drawLine(lx - 15, cy - 15, lx + 15, cy + 15, lc);
        r.canvas().drawLine(lx - 14, cy - 15, lx + 16, cy + 15, lc);
        // Tip dot
        r.canvas().fillCircle(lx + 15, cy + 15, 3, lc);

        _drawRoundDots(r, MAX_ROUNDS);
        r.canvas().setTextColor(Colors::TEXT_DIM);
        r.canvas().setTextSize(1);
        r.canvas().setCursor(4, 4);
        r.canvas().print("THROW "); r.canvas().print(_lanceRound + 1);
        r.canvas().print("/5  [A] WHEN ALIGNED");
    }

    void _drawMagi(Renderer& r, uint32_t now, uint32_t col) {
        uint32_t elapsed = now - _roundStart;
        int16_t panW = Display::W / 3;
        static const char* magiNames[] = {"MELCHIOR", "BALTHASAR", "CASPAR"};
        static const char* voteStr[]   = {"NO", "YES"};

        for (int i = 0; i < 3; i++) {
            int16_t px = i * panW + 2;
            r.canvas().drawRect(px, 20, panW - 4, 70, col);
            r.canvas().setTextColor(Colors::TEXT_DIM);
            r.canvas().setTextSize(1);
            int16_t tw = strlen(magiNames[i]) * 6;
            r.canvas().setCursor(px + (panW - 4 - tw) / 2, 28);
            r.canvas().print(magiNames[i]);

            uint32_t vc = _magiVotes[i] ? Colors::NERV_GREEN : Colors::NERV_RED;
            r.canvas().setTextColor(vc);
            r.canvas().setTextSize(2);
            int16_t vw = strlen(voteStr[_magiVotes[i]]) * 12;
            r.canvas().setCursor(px + (panW - 4 - vw) / 2, 46);
            r.canvas().print(voteStr[_magiVotes[i]]);
        }

        // Countdown bar
        bool inWin = elapsed < MAGI_ROUND_MS;
        int bw = inWin ? (int)map(elapsed, 0, MAGI_ROUND_MS, 220, 0) : 0;
        r.canvas().fillRect(10, 100, bw, 8, col);
        r.canvas().drawRect(10, 100, 220, 8, Colors::TEXT_DIM);

        r.canvas().setTextColor(Colors::TEXT_DIM);
        r.canvas().setTextSize(1);
        r.canvas().setCursor(4, Display::H - 10);
        r.canvas().print("[A]=YES  [B]=NO");

        char buf[16];
        snprintf(buf, sizeof(buf), "ROUND %d/8", _round + 1);
        r.canvas().setTextColor(col);
        r.canvas().setCursor(Display::W - strlen(buf)*6 - 4, Display::H - 10);
        r.canvas().print(buf);
    }

    void _drawDummy(Renderer& r, uint32_t now, uint32_t col) {
        r.canvas().setTextColor(col);
        r.canvas().setTextSize(1);
        char buf[20];
        snprintf(buf, sizeof(buf), "ROUND %d/3  [%d]", _dpRound + 1, _dpSeqLen);
        r.canvas().setCursor(4, 4);
        r.canvas().print(buf);

        if (_dpShowing) {
            r.drawTextCentered("MEMORIZE:", 24, Colors::TEXT_DIM, 1);
            // Show sequence symbol by symbol
            int16_t sx = (Display::W - _dpSeqLen * 28) / 2;
            for (int i = 0; i < _dpSeqLen; i++) {
                bool shown = (i <= (int)_dpShowIdx - 1);
                bool cur   = (i == (int)_dpShowIdx);
                uint32_t bc = shown ? col : (cur ? Colors::WHITE : 0x111122);
                r.canvas().fillRect(sx + i * 28, 38, 24, 24, bc);
                r.canvas().drawRect(sx + i * 28, 38, 24, 24, col);
                if (shown || cur) {
                    r.canvas().setTextColor(shown ? Colors::BLACK : Colors::BLACK);
                    r.canvas().setTextSize(2);
                    r.canvas().setCursor(sx + i * 28 + 6, 42);
                    r.canvas().print(_dpSeq[i] == 0 ? "A" : "B");
                }
            }
        } else {
            r.drawTextCentered("REPEAT:", 24, Colors::NERV_YELLOW, 1);
            int16_t sx = (Display::W - _dpSeqLen * 28) / 2;
            for (int i = 0; i < _dpSeqLen; i++) {
                bool done = (i < (int)_dpInputIdx);
                uint32_t bc = done ? (col & 0x404040) : 0x111122;
                r.canvas().fillRect(sx + i * 28, 38, 24, 24, bc);
                r.canvas().drawRect(sx + i * 28, 38, 24, 24,
                                    (i == (int)_dpInputIdx) ? Colors::WHITE : col);
            }
            r.canvas().setTextColor(Colors::TEXT_DIM);
            r.canvas().setTextSize(1);
            r.canvas().setCursor(4, Display::H - 10);
            r.canvas().print("[A]=left  [B]=right");
        }

        r.canvas().setTextColor(col);
        r.canvas().setTextSize(1);
        for (int i = 0; i < 3; i++) {
            r.canvas().fillCircle(Display::W - 20 + i * 0, 76 + i * 14,
                                  5, i < _dpRoundsWon ? col : Colors::TEXT_DIM);
        }
    }

    void _drawCoreDefense(Renderer& r, uint32_t now, uint32_t col) {
        uint32_t elapsed = now - _roundStart;
        bool inWin = elapsed < INTERCEPT_WIN_MS;
        const char* prompt = (_correct == 0) ? "< A  DODGE" : "DODGE  B >";
        r.drawTextCentered(prompt, 38, inWin ? col : Colors::TEXT_DIM, 2);

        int bw = inWin ? (int)map(elapsed, 0, INTERCEPT_WIN_MS, 200, 0) : 0;
        r.canvas().fillRect(20, 82, bw, 8, col);
        r.canvas().drawRect(20, 82, 200, 8, Colors::TEXT_DIM);

        // Core circle (blinks red when hit)
        bool coreFlash = (_cbHits > 0 && (_ticker / 4) % 2 == 0);
        uint32_t cc = coreFlash ? Colors::NERV_RED : Colors::NERV_BLUE;
        r.canvas().fillCircle(Display::W / 2, 105, 10, cc);

        // Lives
        r.canvas().setTextColor(Colors::TEXT_DIM);
        r.canvas().setTextSize(1);
        r.canvas().setCursor(4, 4);
        r.canvas().print("LIVES:");
        for (int i = 0; i < 3; i++) {
            uint32_t lc = (i < 3 - _cbHits) ? Colors::NERV_RED : Colors::TEXT_DIM;
            r.canvas().fillCircle(52 + i * 12, 8, 4, lc);
        }

        char buf[14];
        snprintf(buf, sizeof(buf), "WAVE %d/12", _round + 1);
        r.canvas().setTextColor(col);
        r.canvas().setCursor(Display::W - strlen(buf)*6 - 4, 4);
        r.canvas().print(buf);
    }

    void _drawCalibration(Renderer& r, uint32_t now, uint32_t col) {
        uint32_t elapsed = now - _phaseStart;
        uint32_t rem = elapsed < CALIB_DUR_MS ? CALIB_DUR_MS - elapsed : 0;

        r.drawTextCentered("KEEP IN GREEN ZONE", 16, col, 1);

        int16_t bx = 10, by = 58, bw = 220, bh = 20;
        r.canvas().drawRect(bx, by, bw, bh, col);

        // Green zone (center 20%)
        int16_t zoneW = bw * 20 / 100;
        int16_t zoneX = bx + (bw - zoneW) / 2;
        r.canvas().fillRect(zoneX, by + 1, zoneW, bh - 2, Colors::NERV_GREEN & 0x1A1A1A);
        r.canvas().drawRect(zoneX, by, zoneW, bh, Colors::NERV_GREEN);

        // Indicator bar position
        int16_t indX = bx + (int16_t)(((_calPos + 1.0f) / 2.0f) * bw) - 3;
        indX = (int16_t)max((int)bx, min((int)(bx + bw - 6), (int)indX));
        r.canvas().fillRect(indX, by + 1, 6, bh - 2, col);

        // Timer
        char buf[16];
        snprintf(buf, sizeof(buf), "%d.%ds", (int)(rem/1000), (int)(rem%1000/100));
        r.canvas().setTextColor(Colors::TEXT_DIM);
        r.canvas().setTextSize(1);
        r.canvas().setCursor((Display::W - strlen(buf)*6)/2, 84);
        r.canvas().print(buf);

        // Score %
        int pct = _calTotal > 0 ? (_calInZone * 100 / _calTotal) : 0;
        snprintf(buf, sizeof(buf), "IN ZONE: %d%%", pct);
        r.canvas().setTextColor(pct >= 50 ? Colors::NERV_GREEN : Colors::NERV_RED);
        r.canvas().setCursor((Display::W - strlen(buf)*6)/2, 96);
        r.canvas().print(buf);

        r.canvas().setTextColor(Colors::TEXT_DIM);
        r.canvas().setCursor(4, Display::H - 10);
        r.canvas().print("[A]=push right  [B]=push left");
    }

    void _drawThirdImpact(Renderer& r, uint32_t now, uint32_t col) {
        bool flash = (_ticker / 3) % 2 == 0;
        if (flash) r.drawTextCentered("THIRD IMPACT!", 14, Colors::NERV_RED, 1);

        // Timer display
        char buf[16];
        snprintf(buf, sizeof(buf), "%.1f", max(0.0f, _tiTimer));
        r.drawTextCentered(buf, 38, _tiTimer > 3 ? Colors::WHITE : Colors::NERV_RED, 3);

        // Press counter
        snprintf(buf, sizeof(buf), "PRESSES: %d/30", _tiPresses);
        r.canvas().setTextColor(col);
        r.canvas().setTextSize(1);
        int16_t tw = strlen(buf) * 6;
        r.canvas().setCursor((Display::W - tw) / 2, 80);
        r.canvas().print(buf);

        // Progress bar
        int bw = map(min(_tiPresses, 30), 0, 30, 0, 220);
        r.canvas().fillRect(10, 92, bw, 10, col);
        r.canvas().drawRect(10, 92, 220, 10, Colors::TEXT_DIM);

        r.canvas().setTextColor(Colors::TEXT_DIM);
        r.canvas().setTextSize(1);
        r.canvas().setCursor((Display::W - 15*6)/2, Display::H - 10);
        r.canvas().print("MASH BTN_A NOW!");
    }

    void _drawRoundDots(Renderer& r, int maxR) {
        int dotX = (Display::W - maxR * 14) / 2;
        for (int i = 0; i < maxR; i++) {
            uint32_t dc = (i < _round) ? Colors::NERV_GREEN
                        : (i == _round) ? Colors::NERV_YELLOW
                                        : Colors::TEXT_DIM;
            r.canvas().fillCircle(dotX + i * 14 + 6, Display::H - 12, 5, dc);
        }
    }

    // ── Draw: RESULT ──────────────────────────────────────

    void _drawResult(Renderer& r) {
        if (_isAngelFight) { _drawAngelResult(r); return; }

        uint32_t elapsed = millis() - _phaseStart;
        bool flash = (elapsed / 250) % 2 == 0;

        switch (_game) {
            case MiniGame::BERSERK_CONTROL: {
                const char* v = (_mashCount >= 15) ? "BERSERK: SUPPRESSED" :
                                (_mashCount >= 8)  ? "PARTIAL CONTROL" : "UNIT UNRESPONSIVE";
                if (flash) { char buf[24]; snprintf(buf,sizeof(buf),"%d PRESSES",_mashCount);
                             r.drawTextCentered(buf, 28, Colors::NERV_RED, 1); }
                r.drawTextCentered(v, 46, Colors::NERV_ORANGE, 2);
                break;
            }
            case MiniGame::MAGI_VOTE: {
                char buf[24];
                snprintf(buf, sizeof(buf), "%d / 8  CORRECT", _magiScore);
                if (flash) r.drawTextCentered(buf, 28, col_for_score(_magiScore*100/8), 1);
                r.drawTextCentered(_magiScore>=5 ? "MAGI: CONSENSUS" : "VOTE FAILED", 46, Colors::TEXT_DIM, 2);
                break;
            }
            case MiniGame::DUMMY_PLUG: {
                if (flash) {
                    char buf[24];
                    snprintf(buf, sizeof(buf), "%d / 3  ROUNDS", _dpRoundsWon);
                    r.drawTextCentered(buf, 28, col_for_score(_dpRoundsWon*33), 1);
                }
                r.drawTextCentered(_dpRoundsWon==3 ? "PLUG: SYNC OK" :
                                   _dpRoundsWon>=1 ? "PARTIAL SYNC" : "SEQUENCE FAILED", 46, Colors::TEXT_DIM, 2);
                break;
            }
            case MiniGame::CALIBRATION: {
                char buf[20];
                snprintf(buf, sizeof(buf), "%d%% IN ZONE", _hits);
                if (flash) r.drawTextCentered(buf, 28, col_for_score(_hits), 1);
                r.drawTextCentered(_hits>=80 ? "CALIBRATION OK" :
                                   _hits>=50 ? "PARTIAL STABLE" : "DRIFT DETECTED", 46, Colors::TEXT_DIM, 2);
                break;
            }
            case MiniGame::THIRD_IMPACT: {
                if (flash) {
                    char buf[20]; snprintf(buf,sizeof(buf),"%d PRESSES", _tiPresses);
                    r.drawTextCentered(buf, 28, _tiPresses>=30 ? Colors::NERV_GREEN : Colors::NERV_RED, 1);
                }
                r.drawTextCentered(_tiPresses>=30 ? "IMPACT AVERTED" : "INSTRUMENTALITY", 46, Colors::TEXT_DIM, 2);
                break;
            }
            default: {
                char buf[24];
                snprintf(buf, sizeof(buf), "%d / %d  HITS", _hits, MAX_ROUNDS);
                if (flash) r.drawTextCentered(buf, 28, col_for_score(_hits*20), 2);
                const char* v = (_hits==MAX_ROUNDS) ? "PERFECT SYNC" :
                                (_hits>=3)           ? "SYNC STABLE" : "DRIFT DETECTED";
                r.drawTextCentered(v, 56, Colors::TEXT_DIM, 1);
                break;
            }
        }
        _drawStatSummary(r, 84);
    }

    void _drawAngelResult(Renderer& r) {
        bool victory;
        switch (_game) {
            case MiniGame::BERSERK_CONTROL: victory = (_mashCount >= 8); break;
            case MiniGame::MAGI_VOTE:       victory = (_magiScore >= 5); break;
            case MiniGame::DUMMY_PLUG:      victory = (_dpRoundsWon >= 2); break;
            case MiniGame::CALIBRATION:     victory = (_hits >= 50); break;
            case MiniGame::THIRD_IMPACT:    victory = (_tiPresses >= 30); break;
            default:                        victory = (_hits >= 3); break;
        }
        uint32_t elapsed = millis() - _phaseStart;
        bool flash = (elapsed / 250) % 2 == 0;

        if (victory) {
            r.canvas().fillScreen(0x021002);
            if (_angelName) r.drawTextCentered(_angelName, 14, Colors::NERV_GREEN_DIM, 1);
            r.drawTextCentered("ANGEL NEUTRALIZED", 36,
                flash ? Colors::NERV_GREEN : Colors::WHITE, 2);
            r.drawTextCentered("SYNC +15", 62, Colors::NERV_GREEN, 1);
            r.canvas().drawRect(0, 0, Display::W, Display::H, Colors::NERV_GREEN);
        } else {
            r.canvas().fillScreen(Colors::BLACK);
            if (_angelName) r.drawTextCentered(_angelName, 14, Colors::NERV_RED_DIM, 1);
            r.drawTextCentered("UNIT DISABLED", 36,
                flash ? Colors::NERV_RED : Colors::WHITE, 2);
            r.drawTextCentered("SYNC -15  ENRG -20", 62, Colors::NERV_RED, 1);
            r.canvas().drawRect(0, 0, Display::W, Display::H, Colors::NERV_RED);
        }
        _drawStatSummary(r, 84);
    }

    void _drawStatSummary(Renderer& r, int16_t y) {
        const EvaStats& st = EvaPet::get().getStats();
        char buf[36];
        snprintf(buf, sizeof(buf), "SYNC:%d  RAGE:%d  ENRG:%d",
                 st.syncRate, st.rage, st.energy);
        r.canvas().setTextColor(Colors::TEXT_DIM);
        r.canvas().setTextSize(1);
        int16_t tx = (Display::W - (int16_t)(strlen(buf) * 6)) / 2;
        r.canvas().setCursor(tx, y);
        r.canvas().print(buf);
    }

    uint32_t col_for_score(int pct) {
        if (pct >= 80) return Colors::NERV_GREEN;
        if (pct >= 50) return Colors::NERV_YELLOW;
        return Colors::NERV_RED;
    }
};

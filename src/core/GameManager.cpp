#include "core/GameManager.h"

// ============================================================
// GameManager::begin()
// ============================================================
void GameManager::begin() {
    _initHardware();
    _initSave();
    _initScenes();
    _lastAutosave = millis();
    _lastFrameMs  = millis();
}

// ============================================================
// GameManager::loop()
// ============================================================
void GameManager::loop() {
    uint32_t now = millis();

    if (now - _lastFrameMs < FRAME_MS) return;
    _lastFrameMs = now;
    _frameCount++;

    M5.update();

    _pollInput();

    IMUManager::get().update();
    AnimationSystem::get().update();
    EvaPet::get().update();
    AudioManager::get().update();

    SceneManager::get().update();
    SceneManager::get().render();

    _checkAutosave();
    _checkBattery();
    _updateGlitchAmbient();
}

// ============================================================
// _initHardware()
// ============================================================
void GameManager::_initHardware() {
    auto cfg = M5.config();
    cfg.clear_display   = true;
    cfg.internal_imu    = true;
    cfg.internal_spk    = true;
    cfg.output_power    = true;
    M5.begin(cfg);

    M5.Display.setRotation(1);
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setBrightness(80);

    randomSeed(analogRead(0) ^ (uint32_t)esp_timer_get_time());

    Serial.begin(115200);

    BatteryManager::get().begin();
    Renderer::get().begin();
    GlitchRenderer::get().begin();
    AnimationSystem::get().begin();
    AudioManager::get().begin();
    IMUManager::get().begin();
    TimeManager::get().begin();

    Serial.println(F("[NGEva] Hardware init OK"));
}

// ============================================================
// _initSave()
// ============================================================
void GameManager::_initSave() {
    auto& save = SaveManager::get();
    auto& time = TimeManager::get();
    auto& pet  = EvaPet::get();

    EvaStats stats;

    if (save.hasSave()) {
        Serial.println(F("[NGEva] Save found — loading"));
        save.load(stats);

        bool cleanShutdown = save.getCleanShutdown();
        if (cleanShutdown) {
            // Voluntary standby: restore stats exactly, no offline decay
            Serial.println(F("[NGEva] Clean shutdown — stats restored as-is"));
        } else {
            // Unexpected power-off: simulate offline decay
            uint32_t savedTs = save.getTimestamp();
            float hours = time.simulateOffline(stats, savedTs);
            Serial.printf("[NGEva] Simulated %.2f hours offline\r\n", hours);
        }

        // Apply stored sound preference (clamp invalid values from old saves)
        if (stats.soundMode > 2) stats.soundMode = 2;
        AudioManager::get().setSoundMode(stats.soundMode);
    } else {
        Serial.println(F("[NGEva] No save — fresh start"));
    }

    pet.begin(stats);
}

// ============================================================
// _initScenes()
// ============================================================
void GameManager::_initScenes() {
    // Array MUST match GameState enum order:
    // SPLASH=0 BOOT=1 EVA_SELECT=2 MAIN=3 FEED=4 FIGHT=5
    // SLEEP=6  BERSERK=7 DATA=8 SYS=9 CORE_BREACH=10
    // QUOTE=11 ANGEL_ALARM=12 MIND=13 SHUTDOWN=14
    static BaseScene* scenes[static_cast<int>(GameState::COUNT)] = {
        &SplashScene::get(),      // 0  SPLASH
        &BootScene::get(),        // 1  BOOT
        &EvaSelectScene::get(),   // 2  EVA_SELECT
        &MainScene::get(),        // 3  MAIN
        &FeedScene::get(),        // 4  FEED
        &FightScene::get(),       // 5  FIGHT
        &SleepScene::get(),       // 6  SLEEP
        &BerserkScene::get(),     // 7  BERSERK
        &DataScene::get(),        // 8  DATA
        &SysScene::get(),         // 9  SYS
        &CoreBreachScene::get(),  // 10 CORE_BREACH
        &QuoteScene::get(),       // 11 QUOTE
        &AngelAlarmScene::get(),  // 12 ANGEL_ALARM
        &MindScene::get(),        // 13 MIND
        &ShutdownScene::get(),    // 14 SHUTDOWN
    };

    SceneManager::get().begin(scenes);
    SceneManager::get().transitionTo(GameState::SPLASH);

    Serial.println(F("[NGEva] Scenes registered"));
}

// ============================================================
// _pollInput()
// ============================================================
void GameManager::_pollInput() {
    uint32_t now = millis();

    if (M5.BtnA.isPressed()) {
        if (!_btnAWasPressed) {
            _btnAWasPressed = true;
            _btnADownMs     = now;
        }
    } else {
        if (_btnAWasPressed) {
            uint32_t held = now - _btnADownMs;
            if (held >= LONG_PRESS_MS) {
                SceneManager::get().handleInput(InputEvent::BTN_A_HOLD);
            } else {
                SceneManager::get().handleInput(InputEvent::BTN_A_CLICK);
            }
            _btnAWasPressed = false;
        }
    }

    if (M5.BtnB.isPressed()) {
        if (!_btnBWasPressed) {
            _btnBWasPressed = true;
            _btnBDownMs     = now;
        }
    } else {
        if (_btnBWasPressed) {
            uint32_t held = now - _btnBDownMs;
            if (held >= LONG_PRESS_MS) {
                SceneManager::get().handleInput(InputEvent::BTN_B_HOLD);
            } else {
                SceneManager::get().handleInput(InputEvent::BTN_B_CLICK);
            }
            _btnBWasPressed = false;
        }
    }

    if (M5.BtnPWR.wasHold()) {
        SaveManager::get().save(EvaPet::get().getStats());
        BatteryManager::get().goDeepSleep(0);
    }
}

// ============================================================
// _checkAutosave()
// ============================================================
void GameManager::_checkAutosave() {
    uint32_t now = millis();
    if (now - _lastAutosave < AUTOSAVE_MS) return;
    _lastAutosave = now;

    auto& pet = EvaPet::get();
    if (pet.isDirty()) {
        SaveManager::get().save(pet.getStats());
        pet.clearDirty();
        Serial.println(F("[NGEva] Autosave OK"));
    }
}

// ============================================================
// _checkBattery()
// ============================================================
void GameManager::_checkBattery() {
    auto& batt = BatteryManager::get();
    batt.update();

    int pct = batt.getLevel();
    if (pct <= (int)LOW_BAT_PCT && pct >= 0) {
        Serial.println(F("[NGEva] Low battery — saving + deep sleep"));
        SaveManager::get().save(EvaPet::get().getStats());
        AudioManager::get().playWarning();
        delay(500);
        batt.goDeepSleep(0);
    }
}

// ============================================================
// _updateGlitchAmbient()
// ============================================================
void GameManager::_updateGlitchAmbient() {
    GlitchRenderer::get().setIntensity(0.0f);
}

#pragma once
#include <M5Unified.h>
#include "core/Types.h"
#include "core/SaveManager.h"
#include "core/TimeManager.h"
#include "entities/EvaPet.h"
#include "ui/Renderer.h"
#include "ui/GlitchRenderer.h"
#include "ui/SceneManager.h"
#include "ui/AnimationSystem.h"
#include "ui/scenes/SplashScene.h"
#include "ui/scenes/MainScene.h"
#include "ui/scenes/FeedScene.h"
#include "ui/scenes/FightScene.h"
#include "ui/scenes/SleepScene.h"
#include "ui/scenes/BerserkScene.h"
#include "ui/scenes/DataScene.h"
#include "ui/scenes/SysScene.h"
#include "ui/scenes/CoreBreachScene.h"
#include "ui/scenes/QuoteScene.h"
#include "ui/scenes/AngelAlarmScene.h"
#include "ui/scenes/MindScene.h"
#include "ui/scenes/ShutdownScene.h"
#include "audio/AudioManager.h"
#include "sensors/IMUManager.h"
#include "sensors/BatteryManager.h"

// ============================================================
// GameManager — top-level orchestrator
// ============================================================
class GameManager {
public:
    static GameManager& get() {
        static GameManager gm;
        return gm;
    }

    void begin();
    void loop();

private:
    GameManager() = default;

    void _initHardware();
    void _initSave();
    void _initScenes();

    void _pollInput();
    void _checkAutosave();
    void _checkBattery();
    void _updateGlitchAmbient();

    bool _btnAWasPressed  = false;
    bool _btnBWasPressed  = false;
    uint32_t _btnADownMs  = 0;
    uint32_t _btnBDownMs  = 0;

    static constexpr uint32_t LONG_PRESS_MS  = 800;
    static constexpr uint32_t AUTOSAVE_MS    = 60000;
    static constexpr uint32_t LOW_BAT_PCT    = 5;
    static constexpr int      FPS_TARGET     = 30;
    static constexpr uint32_t FRAME_MS       = 1000 / FPS_TARGET;

    uint32_t _lastAutosave  = 0;
    uint32_t _lastFrameMs   = 0;
    uint32_t _frameCount    = 0;
};

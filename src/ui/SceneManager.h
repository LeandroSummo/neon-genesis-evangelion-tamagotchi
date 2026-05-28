#pragma once
// ============================================================
// SCENEMANAGER.H — Scene FSM + BaseScene interface
// ============================================================
#include "core/Types.h"
#include "ui/Renderer.h"
#include "audio/AudioManager.h"
#include "sensors/IMUManager.h"
#include "entities/EvaPet.h"

// ─── Base Scene ──────────────────────────────────────────
class BaseScene {
public:
    virtual ~BaseScene() = default;
    virtual void onEnter()  {}
    virtual void onExit()   {}
    virtual void update()   = 0;
    virtual void render()   = 0;

    // Called with resolved input; return next state or NONE to stay
    virtual GameState handleInput(InputEvent ev) {
        (void)ev;
        return GameState::COUNT;  // COUNT = no transition
    }

    GameState getRequestedTransition() const { return _nextState; }
    void      clearTransition()               { _nextState = GameState::COUNT; }

protected:
    void requestTransition(GameState s) { _nextState = s; }

private:
    GameState _nextState = GameState::COUNT;
};

// ─── Scene Manager ───────────────────────────────────────
class SceneManager {
public:
    static SceneManager& get() {
        static SceneManager instance;
        return instance;
    }

    // Register all scenes; call once at startup
    void begin(BaseScene* scenes[(int)GameState::COUNT]);

    void update();
    void render();
    void handleInput(InputEvent ev);

    void transitionTo(GameState next);
    GameState currentState() const { return _current; }

private:
    SceneManager() = default;
    BaseScene* _scenes[(int)GameState::COUNT] = {};
    GameState  _current = GameState::SPLASH;
    bool       _transitioning = false;
};

// ──────────────────────────────────────────────────────────
// Inline implementations
// ──────────────────────────────────────────────────────────

inline void SceneManager::begin(BaseScene* scenes[(int)GameState::COUNT]) {
    for (int i = 0; i < (int)GameState::COUNT; i++) {
        _scenes[i] = scenes[i];
    }
    if (_scenes[(int)_current]) {
        _scenes[(int)_current]->onEnter();
    }
}

inline void SceneManager::update() {
    BaseScene* s = _scenes[(int)_current];
    if (!s) return;
    s->update();

    // Check if scene requested a transition
    GameState next = s->getRequestedTransition();
    if (next != GameState::COUNT) {
        s->clearTransition();
        transitionTo(next);
    }
}

inline void SceneManager::render() {
    BaseScene* s = _scenes[(int)_current];
    if (s) s->render();
}

inline void SceneManager::handleInput(InputEvent ev) {
    // Universal escape: long-press returns to MAIN from any gameplay scene.
    // Excluded: MAIN (keeps BTN_B_HOLD→QUOTE), SPLASH/BOOT/EVA_SELECT (bootstrap).
    if (ev == InputEvent::BTN_B_HOLD &&
        _current != GameState::MAIN       &&
        _current != GameState::SPLASH     &&
        _current != GameState::BOOT       &&
        _current != GameState::EVA_SELECT) {
        transitionTo(GameState::MAIN);
        return;
    }
    BaseScene* s = _scenes[(int)_current];
    if (!s) return;
    GameState next = s->handleInput(ev);
    if (next != GameState::COUNT) {
        transitionTo(next);
    }
}

inline void SceneManager::transitionTo(GameState next) {
    if (next == _current || (int)next >= (int)GameState::COUNT) return;
    BaseScene* cur = _scenes[(int)_current];
    if (cur) cur->onExit();
    _current = next;
    BaseScene* nxt = _scenes[(int)_current];
    if (nxt) nxt->onEnter();
}

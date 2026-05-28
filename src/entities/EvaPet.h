#pragma once
// ============================================================
// EVAPET.H — The core digital organism
// ============================================================
#include "core/Types.h"
#include "core/TimeManager.h"
#include "data/EvaData.h"

class EvaPet {
public:
    static EvaPet& get() {
        static EvaPet instance;
        return instance;
    }

    void begin(EvaStats& initialStats);
    void update();

    // Actions (called from scenes)
    void feed(FeedItem item);   // apply food item modifiers
    void rest();                // enter sleep mode
    void sleepTick();           // called by SleepScene every 30s
    void wakeUp();              // called by SleepScene on manual/auto wake
    void pet();                 // sync+10, rage-5 (legacy)
    void applyFightHit();       // per successful fight hit
    void calmedDown();          // berserk recovery

    // Core breach (syncRate < 5 for 5 min → death)
    bool isCoreBreach()  const { return _coreBreach; }
    void resetCoreBreach();     // restart: keep unit/sound/highScore

    // Angel alarm (random trigger, cleared by AngelAlarmScene)
    bool    isAngelAlert() const { return _angelAlert; }
    uint8_t getAngelIdx()  const { return _angelIdx; }
    void    clearAngelAlert()    { _angelAlert = false; }

    // Getters
    const EvaStats&       getStats()       const { return _stats; }
    EvaStats&             getStatsMut()          { return _stats; }
    const EvaPersonality& getPersonality() const;
    AnimState             getAnimState()   const { return _animState; }
    EvolutionStage        getEvolution()   const;
    bool                  justEvolved()    const { return _justEvolved; }
    void                  clearJustEvolved()     { _justEvolved = false; }

    bool isDirty()         const { return _dirty; }
    void clearDirty()            { _dirty = false; }
    void markDirty()             { _dirty = true; }

    bool     isBerserk()   const { return _stats.berserk; }
    bool     isSleeping()  const { return _stats.sleeping; }
    uint8_t  getUnit()     const { return _stats.evaUnit; }
    bool     canFeed()     const { return _stats.hunger < 85; }
    bool     canRest()     const { return !_stats.sleeping && _stats.energy < 88; }
    bool     canFight()    const { return _stats.energy >= 8 && !_stats.berserk; }
    bool     canMind()     const { return _stats.syncRate < 95; }
    uint32_t lastFightMs() const { return _lastFightMs; }

private:
    EvaPet() = default;

    void _updateEvolution();
    void _updateAnimState();
    void _checkDecay(uint32_t now);
    void _checkCoreBreach(uint32_t now);
    void _checkAngelAlert(uint32_t now);

    EvaStats  _stats;
    AnimState _animState       = AnimState::IDLE;
    uint32_t  _lastDecayCheck  = 0;
    uint32_t  _lastFightMs     = 0;
    uint32_t  _syncLowStart    = 0;
    bool      _coreBreach      = false;
    bool      _dirty           = false;
    bool      _justEvolved     = false;
    uint8_t   _prevEvolution   = 0;

    bool     _angelAlert   = false;
    uint8_t  _angelIdx     = 0;
    uint32_t _lastAngelMs  = 0;

    static constexpr uint32_t DECAY_INTERVAL_MS    = 60000;
    static constexpr uint32_t CORE_BREACH_DELAY_MS = 300000;  // 5 min
};

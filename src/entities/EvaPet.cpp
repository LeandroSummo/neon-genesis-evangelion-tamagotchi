// ============================================================
// EVAPET.CPP
// ============================================================
#include "entities/EvaPet.h"
#include <string.h>

void EvaPet::begin(EvaStats& initialStats) {
    _stats          = initialStats;
    _lastDecayCheck = millis();
    _lastFightMs    = millis();
    _syncLowStart   = 0;
    _coreBreach     = false;
    _prevEvolution  = _stats.evolutionStage;
    _updateAnimState();
    _dirty = false;
}

void EvaPet::update() {
    uint32_t now = millis();

    if (now - _lastDecayCheck >= DECAY_INTERVAL_MS) {
        _lastDecayCheck = now;
        _checkDecay(now);
    }

    _prevEvolution = _stats.evolutionStage;
    _updateEvolution();
    if (_stats.evolutionStage != _prevEvolution) _justEvolved = true;

    _updateAnimState();

    // Berserk failsafe: auto-calm after 3 minutes
    if (_stats.berserk && millis() - _lastFightMs > 180000UL) {
        calmedDown();
    }

    _checkCoreBreach(now);
}

// ── Actions ──────────────────────────────────────────────

void EvaPet::feed(FeedItem item) {
    if (_stats.hunger > 85) return;
    const FeedItemData& food = FEED_ITEMS[(uint8_t)item];
    _stats.hunger   = statClamp(_stats.hunger   + food.hungerMod);
    _stats.energy   = statClamp(_stats.energy   + food.energyMod);
    _stats.syncRate = statClamp(_stats.syncRate  + food.syncMod);
    _stats.rage     = statClamp(_stats.rage      + food.rageMod);
    _dirty = true;
}

void EvaPet::rest() {
    _stats.sleeping = true;
    _dirty = true;
}

void EvaPet::sleepTick() {
    if (!_stats.sleeping) return;
    _stats.energy   = statClamp(_stats.energy   + 20);
    _stats.syncRate = statClamp(_stats.syncRate  + 3);
    _stats.hunger   = statClamp(_stats.hunger    - 2);
    _dirty = true;
}

void EvaPet::wakeUp() {
    _stats.sleeping = false;
    _dirty = true;
}

void EvaPet::pet() {
    if (_stats.rage > 70) {
        _stats.syncRate = statClamp(_stats.syncRate + 5);
        _stats.rage     = statClamp(_stats.rage     - 2);
    } else {
        _stats.syncRate = statClamp(_stats.syncRate + 10);
        _stats.rage     = statClamp(_stats.rage     - 5);
    }
    _dirty = true;
}

void EvaPet::applyFightHit() {
    _stats.syncRate = statClamp(_stats.syncRate + 8);
    _stats.rage     = statClamp(_stats.rage     - 10);
    _stats.energy   = statClamp(_stats.energy   - 5);
    _lastFightMs    = millis();
    _dirty = true;
}

void EvaPet::calmedDown() {
    _stats.berserk      = false;
    _stats.rage         = statClamp(_stats.rage - 30);
    _stats.berserkTimer = 0;
    _lastFightMs        = millis();
    _dirty = true;
}

void EvaPet::resetCoreBreach() {
    if (_stats.daysAlive > _stats.highScoreDays)
        _stats.highScoreDays = _stats.daysAlive;

    uint8_t  unit    = _stats.evaUnit;
    bool     mark06  = _stats.mark06Unlocked;
    uint8_t  sndMode = _stats.soundMode;
    uint16_t hiScore = _stats.highScoreDays;

    memset(&_stats, 0, sizeof(_stats));
    _stats.evaUnit        = unit;
    _stats.mark06Unlocked = mark06;
    _stats.soundMode      = sndMode;
    _stats.highScoreDays  = hiScore;
    _stats.unitSelected   = false;

    _coreBreach   = false;
    _syncLowStart = 0;
    _dirty = true;
}

// ── Internal ─────────────────────────────────────────────

void EvaPet::_checkDecay(uint32_t now) {
    if (_stats.sleeping) return;  // SleepScene manages recovery

    _stats.hunger = statClamp(_stats.hunger - 5);
    _stats.energy = statClamp(_stats.energy - 1);

    int16_t syncDrop = (_stats.hunger < 25) ? 8 : 4;
    _stats.syncRate = statClamp(_stats.syncRate - syncDrop);

    int16_t rageRise = (now - _lastFightMs > 300000UL) ? 6 : 3;
    _stats.rage = statClamp(_stats.rage + rageRise);

    if (_stats.rage >= 85) _stats.berserk = true;
    _dirty = true;

    _checkAngelAlert(now);
}

void EvaPet::_checkAngelAlert(uint32_t now) {
    if (_angelAlert || _stats.sleeping || _stats.berserk) return;
    if (now - _lastAngelMs < 180000UL) return;  // 3-min minimum interval

    // Probability grows with age: 20% base + 2% per day, capped at 60%
    float prob = min(0.60f, 0.20f + _stats.daysAlive * 0.02f);
    if ((float)random(0, 1000) / 1000.0f < prob) {
        _angelAlert  = true;
        _angelIdx    = random(0, 12);
        _lastAngelMs = now;
    }
}

void EvaPet::_checkCoreBreach(uint32_t now) {
    if (_coreBreach || _stats.berserk) return;
    if (_stats.syncRate < 5) {
        if (_syncLowStart == 0) _syncLowStart = now;
        else if (now - _syncLowStart >= CORE_BREACH_DELAY_MS) _coreBreach = true;
    } else {
        _syncLowStart = 0;
    }
}

void EvaPet::_updateEvolution() {
    if (_stats.syncRate >= 90 && _stats.hunger > 50 && _stats.energy > 60) {
        _stats.evolutionStage = (uint8_t)EvolutionStage::PERFECT_SYNC;
    } else if (_stats.berserk) {
        _stats.evolutionStage = (uint8_t)EvolutionStage::BERSERK_EVA;
    } else if (_stats.daysAlive <= 3) {
        _stats.evolutionStage = (uint8_t)EvolutionStage::NASCENT;
    } else {
        _stats.evolutionStage = (uint8_t)EvolutionStage::STABLE;
    }
}

void EvaPet::_updateAnimState() {
    if (_stats.berserk) {
        _animState = AnimState::BERSERK;
    } else if (_stats.sleeping) {
        _animState = AnimState::SLEEPING;
    } else if (_stats.syncRate > 75) {
        _animState = AnimState::HAPPY;
    } else if (_stats.rage > 60) {
        _animState = AnimState::ANGRY;
    } else if (_stats.hunger < 25) {
        _animState = AnimState::SAD;
    } else {
        _animState = AnimState::IDLE;
    }
}

const EvaPersonality& EvaPet::getPersonality() const {
    uint8_t u = (_stats.evaUnit < 4) ? _stats.evaUnit : 0;
    return EVA_PERSONALITIES[u];
}

EvolutionStage EvaPet::getEvolution() const {
    return (EvolutionStage)_stats.evolutionStage;
}

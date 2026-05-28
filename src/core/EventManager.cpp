// ============================================================
// EVENTMANAGER.CPP
// ============================================================
#include "core/EventManager.h"

void EventManager::begin() {
    _lastEventTime = millis();
    _hasEvent      = false;
}

void EventManager::update(EvaStats& stats) {
    if (_hasEvent) return;  // Wait for current event to be dismissed

    uint32_t now = millis();
    if (now - _lastEventTime < _eventCheckInterval) return;

    _lastEventTime = now;

    // Roll for event
    float roll = (float)random(0, 1000) / 1000.0f;
    if (roll >= _eventChance) return;

    _rollRandomEvent(stats);
}

void EventManager::resolveEvent(EvaStats& stats, bool fight) {
    if (!_hasEvent) return;
    const GameEvent& ev = _activeEvent;

    if (fight) {
        // Player engaged — apply full modifiers but pay energy cost
        stats.syncRate      = statClamp(stats.syncRate      + ev.syncMod);
        stats.rage          = statClamp(stats.rage          + ev.rageMod / 2);
        stats.contamination = statClamp(stats.contamination + ev.contaminationMod / 2);
        stats.stability     = statClamp(stats.stability     + ev.stabilityMod);
        stats.energy        = statClamp(stats.energy        - 15);  // fight costs energy
    } else {
        // Player fled — avoid the worst effects, minor sync penalty
        stats.syncRate      = statClamp(stats.syncRate - 5);
        // rage and contamination still drift slightly
        stats.rage          = statClamp(stats.rage          + ev.rageMod / 4);
        stats.contamination = statClamp(stats.contamination + ev.contaminationMod / 4);
    }

    _hasEvent = false;
}

void EventManager::_rollRandomEvent(EvaStats& stats) {
    // Weight events based on stats
    // Emergency events more likely when stressed
    uint8_t maxIdx = RANDOM_EVENT_COUNT;

    // Pick random event
    uint8_t idx = random(0, maxIdx);

    // Bias toward emergencies if stats are bad
    bool stressful = stats.rage > 60 || stats.hunger > 70 ||
                     stats.stability < 30 || stats.contamination > 50;
    if (stressful && random(0, 2) == 0) {
        // Prefer emergency events (indices 0, 2, 4, 6, 8)
        static const uint8_t emergency_indices[] = {0, 2, 4, 6, 8};
        idx = emergency_indices[random(0, 5)];
    }

    if (idx >= RANDOM_EVENT_COUNT) idx = RANDOM_EVENT_COUNT - 1;

    // Copy from PROGMEM (GameEvent is trivially copyable since
    // its char* members point to PROGMEM strings — fine on ESP32)
    GameEvent ev;
    memcpy_P(&ev, &RANDOM_EVENTS[idx], sizeof(GameEvent));

    _triggerEvent(ev);
}

void EventManager::_triggerEvent(const GameEvent& ev) {
    _activeEvent = ev;
    _hasEvent    = true;
    // Stats are NOT applied here — player must resolve via resolveEvent()
    // or dismissEvent() (for non-emergency events).
}

void EventManager::dismissEvent() {
    _hasEvent = false;
}

#pragma once
// ============================================================
// EVENTMANAGER.H — Random event system
// ============================================================
#include "core/Types.h"
#include "data/LoreData.h"

class EventManager {
public:
    static EventManager& get() {
        static EventManager instance;
        return instance;
    }

    void begin();
    void update(EvaStats& stats);          // call every loop tick

    bool        hasActiveEvent() const { return _hasEvent; }
    const GameEvent& getActiveEvent() const { return _activeEvent; }
    void        dismissEvent();

    // For emergency events: player chooses to FIGHT [A] or FLEE [B].
    // For non-emergency events, call dismissEvent() directly.
    // Stats are applied here, not at trigger time.
    void        resolveEvent(EvaStats& stats, bool fight);

    void setEventChance(float chance) { _eventChance = chance; }

private:
    EventManager() = default;

    void  _triggerEvent(const GameEvent& ev);
    void  _rollRandomEvent(EvaStats& stats);

    uint32_t  _lastEventTime = 0;
    uint32_t  _eventCheckInterval = 300000; // check every 5 min
    bool      _hasEvent = false;
    GameEvent _activeEvent;
    float     _eventChance = 0.20f;  // 20% chance per interval
};

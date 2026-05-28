#pragma once
// ============================================================
// TIMEMANAGER.H — Real-time clock + offline decay simulation
// ============================================================
#include "core/Types.h"
#include "data/EvaData.h"

class TimeManager {
public:
    static TimeManager& get() {
        static TimeManager instance;
        return instance;
    }

    void begin();
    void update();

    // Simulate offline time since last save
    // Modifies stats in-place; returns hours elapsed
    float simulateOffline(EvaStats& stats, uint32_t savedTimestamp);

    // Tick-based decay (called every N seconds in-game)
    void  applyDecay(EvaStats& stats, float deltaHours,
                     const EvaPersonality& personality);

    // Check if a new day has passed
    bool  isNewDay();

    // Returns seconds since device boot (millis / 1000)
    uint32_t now() const { return millis() / 1000; }

    // Formatted time string "HH:MM"
    void getTimeStr(char* buf, size_t len);

    // Hour of day (0-23) — estimated from millis
    uint8_t getHour() const;

    uint32_t getSessionStart() const { return _sessionStart; }

private:
    TimeManager() = default;
    uint32_t _sessionStart   = 0;
    uint32_t _lastDayCheck   = 0;
    uint32_t _lastDecayTick  = 0;
    bool     _newDay         = false;

    // Runtime clock offset (seconds from boot used as relative time)
    uint32_t _bootEpochOffset = 0; // set from saved timestamp
};

#pragma once
// ============================================================
// BATTERYMANAGER.H — Power management + deep sleep
// ============================================================
#include <M5Unified.h>

class BatteryManager {
public:
    static BatteryManager& get() {
        static BatteryManager instance;
        return instance;
    }

    void begin();
    void update();

    uint8_t getLevel()      const { return _level; }
    bool    isLow()         const { return _level < 20; }
    bool    isCritical()    const { return _level < 5; }
    bool    isCharging()    const { return _charging; }

    // Deep sleep for N seconds, wake via button or timer
    void    goDeepSleep(uint32_t sleepSecs = 3600);

    // Adaptive brightness based on battery
    uint8_t getRecommendedBrightness() const;

private:
    BatteryManager() = default;
    uint8_t  _level    = 100;
    bool     _charging = false;
    uint32_t _lastPoll = 0;
    static constexpr uint32_t POLL_INTERVAL_MS = 30000; // every 30s
};

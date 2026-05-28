// ============================================================
// BATTERYMANAGER.CPP
// ============================================================
#include "sensors/BatteryManager.h"
#include <esp_sleep.h>

void BatteryManager::begin() {
    _lastPoll  = millis();
    _level     = M5.Power.getBatteryLevel();
    _charging  = M5.Power.isCharging();
}

void BatteryManager::update() {
    uint32_t now = millis();
    if (now - _lastPoll < POLL_INTERVAL_MS) return;
    _lastPoll = now;

    _level    = M5.Power.getBatteryLevel();
    _charging = M5.Power.isCharging();
}

void BatteryManager::goDeepSleep(uint32_t sleepSecs) {
    // Save must happen before this is called (GameManager responsibility)
    M5.Display.setBrightness(0);
    M5.Display.sleep();

    // Configure wake sources
    // Wake on GPIO37 (BtnA) — active low
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_37, 0);
    // Wake after timer
    esp_sleep_enable_timer_wakeup((uint64_t)sleepSecs * 1000000ULL);

    esp_deep_sleep_start();
    // Never returns
}

uint8_t BatteryManager::getRecommendedBrightness() const {
    if (_charging) return 150;
    if (_level > 50) return 120;
    if (_level > 20) return 80;
    return 40; // dim on low battery
}

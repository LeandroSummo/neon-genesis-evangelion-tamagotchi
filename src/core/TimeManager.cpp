// ============================================================
// TIMEMANAGER.CPP
// ============================================================
#include "core/TimeManager.h"

void TimeManager::begin() {
    _sessionStart  = millis() / 1000;
    _lastDayCheck  = _sessionStart;
    _lastDecayTick = _sessionStart;
}

void TimeManager::update() {
    uint32_t t = now();

    // New day check (every 86400 seconds = 1 real day)
    if (t - _lastDayCheck >= 86400) {
        _newDay = true;
        _lastDayCheck = t;
    } else {
        _newDay = false;
    }
}

float TimeManager::simulateOffline(EvaStats& stats,
                                    uint32_t savedTimestamp) {
    uint32_t nowTs = now() + _bootEpochOffset;
    if (savedTimestamp == 0 || nowTs <= savedTimestamp) return 0.0f;

    uint32_t diffSecs = nowTs - savedTimestamp;
    float    hours    = diffSecs / 3600.0f;

    // Cap at 48h of offline time (reasonable limit)
    if (hours > 48.0f) hours = 48.0f;

    const EvaPersonality& pers = EVA_PERSONALITIES[stats.evaUnit];
    applyDecay(stats, hours, pers);

    // Sleep bonus: if unit was sleeping when saved, energy recovers
    if (stats.sleeping) {
        float sleepHours = min(hours, 8.0f);
        stats.energy   = statClamp(stats.energy + (int16_t)(sleepHours * 5));
        stats.hunger   = statClamp(stats.hunger + (int16_t)(sleepHours * 1.5f));
        stats.stability = statClamp(stats.stability + (int16_t)(sleepHours * 3));
        stats.sleeping  = false;  // wake up
    }

    // Update days alive
    uint32_t daysElapsed = diffSecs / 86400;
    stats.daysAlive += (uint16_t)daysElapsed;

    return hours;
}

void TimeManager::applyDecay(EvaStats& stats, float deltaHours,
                              const EvaPersonality& personality) {
    // Hunger rises over time
    float hungerRise = BASE_DECAY.hunger * personality.hungerRate * deltaHours;
    stats.hunger = statClamp(stats.hunger + (int16_t)hungerRise);

    // Energy drops
    float energyDrop = BASE_DECAY.energy * deltaHours;
    if (stats.sleeping) energyDrop *= -2.0f; // recover during sleep
    stats.energy = statClamp(stats.energy - (int16_t)energyDrop);

    // Happiness drifts down without care
    float happDrop = BASE_DECAY.happiness * deltaHours;
    stats.happiness = statClamp(stats.happiness - (int16_t)happDrop);

    // Sync rate drifts toward 50 without training
    int16_t syncTarget = 50;
    float syncDrift = BASE_DECAY.syncRate * deltaHours;
    if (stats.syncRate > syncTarget)
        stats.syncRate = statClamp(stats.syncRate - (int16_t)syncDrift);
    else if (stats.syncRate < syncTarget)
        stats.syncRate = statClamp(stats.syncRate + (int16_t)syncDrift);

    // Stability drops (especially if hungry or tired)
    float stabDrop = BASE_DECAY.stability * deltaHours;
    if (stats.hunger > 60) stabDrop *= 2.0f;
    if (stats.energy < 20) stabDrop *= 1.5f;
    stats.stability = statClamp(stats.stability - (int16_t)stabDrop);

    // Rage rises if hungry or unstable
    float rageRise = BASE_DECAY.rage * personality.rageRate * deltaHours;
    if (stats.hunger > 70) rageRise *= 3.0f;
    if (stats.stability < 30) rageRise *= 2.0f;
    stats.rage = statClamp(stats.rage + (int16_t)rageRise);

    // Contamination passive drift (slow)
    float contRise = 0.2f * personality.contaminationRes * deltaHours;
    if (stats.rage > 70) contRise *= 2.5f;
    stats.contamination = statClamp(stats.contamination + (int16_t)contRise);

    // Affection fades without interaction
    float affDrop = BASE_DECAY.affection * deltaHours;
    stats.affection = statClamp(stats.affection - (int16_t)affDrop);

    // Berserk trigger check
    if (stats.rage >= 80 || (stats.hunger >= 95 && stats.stability < 15)) {
        stats.berserk = true;
    }

    // Angel mode trigger
    if (stats.contamination >= 70) {
        stats.angelMode = true;
    }
}

bool TimeManager::isNewDay() {
    return _newDay;
}

void TimeManager::getTimeStr(char* buf, size_t len) {
    uint32_t secs = now();
    uint32_t h    = (secs / 3600) % 24;
    uint32_t m    = (secs / 60) % 60;
    snprintf(buf, len, "%02u:%02u", h, m);
}

uint8_t TimeManager::getHour() const {
    return (now() / 3600) % 24;
}

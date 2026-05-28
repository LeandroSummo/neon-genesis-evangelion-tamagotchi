// ============================================================
// SAVEMANAGER.CPP
// ============================================================
#include "core/SaveManager.h"

void SaveManager::begin() {
    _prefs.begin(NS, false);
}

void SaveManager::save(const EvaStats& s) {
    _prefs.begin(NS, false);
    _prefs.putShort("sync",    s.syncRate);
    _prefs.putShort("hunger",  s.hunger);
    _prefs.putShort("stab",    s.stability);
    _prefs.putShort("rage",    s.rage);
    _prefs.putShort("cont",    s.contamination);
    _prefs.putShort("happy",   s.happiness);
    _prefs.putShort("energy",  s.energy);
    _prefs.putShort("affec",   s.affection);
    _prefs.putUChar("evo",     s.evolutionStage);
    _prefs.putUShort("days",   s.daysAlive);
    _prefs.putBool("berserk",  s.berserk);
    _prefs.putBool("sleep",    s.sleeping);
    _prefs.putBool("angel",    s.angelMode);
    _prefs.putUInt("ts",       s.lastSaveTimestamp);
    _prefs.putUChar("unit",    s.evaUnit);
    _prefs.putBool("selected", s.unitSelected);
    _prefs.putBool("mk6",      s.mark06Unlocked);
    _prefs.putBool("saved",    true);   // save flag
    _prefs.end();
}

bool SaveManager::load(EvaStats& s) {
    _prefs.begin(NS, true);   // read-only
    bool exists = _prefs.getBool("saved", false);
    if (!exists) {
        _prefs.end();
        return false;
    }

    s.syncRate          = _prefs.getShort("sync",    50);
    s.hunger            = _prefs.getShort("hunger",  0);
    s.stability         = _prefs.getShort("stab",    80);
    s.rage              = _prefs.getShort("rage",    0);
    s.contamination     = _prefs.getShort("cont",    0);
    s.happiness         = _prefs.getShort("happy",   70);
    s.energy            = _prefs.getShort("energy",  80);
    s.affection         = _prefs.getShort("affec",   50);
    s.evolutionStage    = _prefs.getUChar("evo",     0);
    s.daysAlive         = _prefs.getUShort("days",   0);
    s.berserk           = _prefs.getBool("berserk",  false);
    s.sleeping          = _prefs.getBool("sleep",    false);
    s.angelMode         = _prefs.getBool("angel",    false);
    s.lastSaveTimestamp = _prefs.getUInt("ts",       0);
    s.evaUnit           = _prefs.getUChar("unit",    0);
    s.unitSelected      = _prefs.getBool("selected", false);
    s.mark06Unlocked    = _prefs.getBool("mk6",      false);

    // Runtime fields reset on load
    s.lastFed     = millis();
    s.lastSlept   = millis();
    s.berserkTimer = 0;

    _prefs.end();
    return true;
}

void SaveManager::reset() {
    _prefs.begin(NS, false);
    _prefs.clear();
    _prefs.end();
}

bool SaveManager::hasSave() {
    _prefs.begin(NS, true);
    bool has = _prefs.getBool("saved", false);
    _prefs.end();
    return has;
}

void SaveManager::setTimestamp(uint32_t t) {
    _prefs.begin(NS, false);
    _prefs.putUInt("ts", t);
    _prefs.end();
}

uint32_t SaveManager::getTimestamp() {
    _prefs.begin(NS, true);
    uint32_t t = _prefs.getUInt("ts", 0);
    _prefs.end();
    return t;
}

void SaveManager::setCleanShutdown(bool val) {
    _prefs.begin(NS, false);
    _prefs.putBool("csd", val);
    _prefs.end();
}

bool SaveManager::getCleanShutdown() {
    _prefs.begin(NS, false);
    bool val = _prefs.getBool("csd", false);
    _prefs.putBool("csd", false);  // clear on read
    _prefs.end();
    return val;
}

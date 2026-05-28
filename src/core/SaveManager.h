#pragma once
// ============================================================
// SAVEMANAGER.H — NVS persistent save/load via Preferences
// ============================================================
#include "core/Types.h"
#include <Preferences.h>

class SaveManager {
public:
    static SaveManager& get() {
        static SaveManager instance;
        return instance;
    }

    void begin();
    void save(const EvaStats& stats);
    bool load(EvaStats& stats);    // returns false if no save exists
    void reset();                  // factory reset
    bool hasSave();

    // Timestamp helpers
    void    setTimestamp(uint32_t t);
    uint32_t getTimestamp();

    // Standby clean-shutdown flag
    void setCleanShutdown(bool val);
    bool getCleanShutdown();        // also clears the flag

private:
    SaveManager() = default;
    Preferences _prefs;
    static constexpr const char* NS = "ngeva";   // NVS namespace
};

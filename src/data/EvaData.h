#pragma once
// ============================================================
// EVADATA.H — EVA personalities + feed item stats
// ============================================================
#include "core/Types.h"

// ─── EVA PERSONALITIES ───────────────────────────────────
static const EvaPersonality EVA_PERSONALITIES[4] = {
    { "EVA-00", "PROTO-TYPE",     "REI AYANAMI",   1.2f, 3.0f, 1.5f, 70.0f, 0.7f,
      Colors::EVA00_PRIMARY, Colors::EVA00_DARK, Colors::EVA00_ACCENT,
      "I AM NOT AFRAID TO DIE" },
    { "EVA-01", "TEST-TYPE",      "SHINJI IKARI",  0.9f, 4.0f, 3.0f, 50.0f, 1.4f,
      Colors::EVA01_PRIMARY, Colors::EVA01_DARK, Colors::EVA01_ACCENT,
      "GET IN THE ROBOT" },
    { "EVA-02", "PRODUCTION-TYPE","ASUKA LANGLEY",  1.0f, 3.5f, 2.5f, 60.0f, 1.0f,
      Colors::EVA02_PRIMARY, Colors::EVA02_DARK, Colors::EVA02_ACCENT,
      "I'M THE BEST THERE IS" },
    { "MARK.06","NEMESIS SERIES", "KAWORU NAGISA",  1.5f, 2.0f, 0.5f, 80.0f, 0.3f,
      Colors::MARK06_PRIMARY, Colors::MARK06_DARK, Colors::MARK06_ACCENT,
      "I WAS BORN FOR THIS MOMENT" },
};

// ─── FEED ITEMS (6 NGE-themed) ───────────────────────────
// hungerMod: positive = fills hunger (fullness model)
static const FeedItemData FEED_ITEMS[6] = {
    {
        "LCL RATION",
        "Standard pilot nutritional pack. Smells like blood.",
        25,  // hunger
        0,   // energy
        5,   // sync
        0    // rage
    },
    {
        "INSTANT RAMEN",
        "Misato's emergency supply. Questionable expiry.",
        35,   // hunger
        5,    // energy
        0,    // sync
        5     // rage (sugar crash incoming)
    },
    {
        "S2 PROTEIN",
        "Derived from Angel core matter. Highly energetic.",
        20,   // hunger
        20,   // energy
        0,    // sync
        0     // rage
    },
    {
        "VITAMIN TANG",
        "Rei's favorite. She eats three per day. Silently.",
        15,   // hunger
        0,    // energy
        15,   // sync
        0     // rage
    },
    {
        "EMERGENCY BAR",
        "NERV field ration. Tastes like existential dread.",
        40,   // hunger
        0,    // energy
        0,    // sync
        10    // rage
    },
    {
        "SOUL FRUIT",
        "Origin unknown. Yui would have approved.",
        20,   // hunger
        0,    // energy
        25,   // sync
        -10   // rage
    },
};

// ─── STAT DECAY RATES (kept for offline simulation only) ─
struct DecayRates {
    float hunger;
    float energy;
    float happiness;
    float syncRate;
    float stability;
    float rage;
    float affection;
};

static const DecayRates BASE_DECAY = {
    5.0f,   // hunger  (per hour — online decay uses fixed ticks)
    3.0f,   // energy
    4.0f,   // syncRate
    1.0f,   // syncRate drift
    1.5f,   // stability
    3.0f,   // rage
    1.0f    // affection
};

// ─── EVOLUTION THRESHOLDS ────────────────────────────────
struct EvolutionCheck {
    static bool isPerfectSync(const EvaStats& s) {
        return s.syncRate >= 90 && s.hunger > 50 && s.energy > 60 && s.rage < 20;
    }
    static bool isBerserk(const EvaStats& s) {
        return s.rage >= 85;
    }
};

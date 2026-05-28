#pragma once
// ============================================================
// TYPES.H — All enums, constants, and data structures
// NEON GENESIS TAMAGOTCHI
// ============================================================
#include <Arduino.h>

// ─── EVA UNITS ────────────────────────────────────────────
enum class EvaUnit : uint8_t {
    EVA_00 = 0,
    EVA_01,
    EVA_02,
    MARK_06,
    NONE,
    COUNT = 4
};

// ─── GAME STATE MACHINE ──────────────────────────────────
enum class GameState : uint8_t {
    SPLASH      = 0,
    BOOT        = 1,
    EVA_SELECT  = 2,
    MAIN        = 3,
    FEED        = 4,
    FIGHT       = 5,
    SLEEP       = 6,
    BERSERK     = 7,
    DATA        = 8,
    SYS         = 9,
    CORE_BREACH = 10,
    QUOTE       = 11,
    ANGEL_ALARM = 12,
    MIND        = 13,
    SHUTDOWN    = 14,
    COUNT
};

// ─── EVOLUTION STAGES ────────────────────────────────────
enum class EvolutionStage : uint8_t {
    NASCENT      = 0,
    STABLE       = 1,
    BERSERK_EVA  = 2,
    PERFECT_SYNC = 3,
    COUNT
};

// ─── FEED ITEMS ──────────────────────────────────────────
enum class FeedItem : uint8_t {
    LCL_RATION    = 0,
    INSTANT_RAMEN = 1,
    S2_PROTEIN    = 2,
    VITAMIN_TANG  = 3,
    EMERGENCY_BAR = 4,
    SOUL_FRUIT    = 5,
    COUNT
};

// ─── ANIMATION STATES ────────────────────────────────────
enum class AnimState : uint8_t {
    IDLE      = 0,
    BLINK     = 1,
    SLEEPING  = 2,
    EATING    = 3,
    BERSERK   = 4,
    CORRUPTED = 5,
    HAPPY     = 6,
    SAD       = 7,
    ANGRY     = 8,
    COUNT
};

// ─── INPUT EVENTS ────────────────────────────────────────
enum class InputEvent : uint8_t {
    NONE        = 0,
    BTN_A_CLICK,
    BTN_A_HOLD,
    BTN_B_CLICK,
    BTN_B_HOLD,
    SHAKE,
    TILT_LEFT,
    TILT_RIGHT,
    TILT_FORWARD,
    GENTLE_MOVE
};

// ─── RGB COLOUR CONSTANTS (24-bit, M5GFX converts) ───────
namespace Colors {
    constexpr uint32_t BLACK          = 0x000000;
    constexpr uint32_t BG_DARK        = 0x080810;
    constexpr uint32_t NERV_GREEN     = 0x00FF41;
    constexpr uint32_t NERV_GREEN_DIM = 0x007A1F;
    constexpr uint32_t NERV_RED       = 0xFF2020;
    constexpr uint32_t NERV_RED_DIM   = 0x7A0000;
    constexpr uint32_t NERV_ORANGE    = 0xFF8800;
    constexpr uint32_t NERV_BLUE      = 0x0088FF;
    constexpr uint32_t NERV_CYAN      = 0x00FFEE;
    constexpr uint32_t NERV_YELLOW    = 0xFFFF00;

    constexpr uint32_t EVA00_DARK     = 0x00145C;
    constexpr uint32_t EVA00_PRIMARY  = 0x0044FF;
    constexpr uint32_t EVA00_ACCENT   = 0xCCCCFF;

    constexpr uint32_t EVA01_PRIMARY  = 0x6600CC;
    constexpr uint32_t EVA01_DARK     = 0x3D007A;
    constexpr uint32_t EVA01_ACCENT   = 0x00FF41;

    constexpr uint32_t EVA02_PRIMARY  = 0xFF2200;
    constexpr uint32_t EVA02_DARK     = 0x7A0000;
    constexpr uint32_t EVA02_ACCENT   = 0xFF8800;

    constexpr uint32_t MARK06_PRIMARY = 0xEEEEFF;
    constexpr uint32_t MARK06_DARK    = 0x222244;
    constexpr uint32_t MARK06_ACCENT  = 0x8888FF;

    constexpr uint32_t NERV_AMBER     = 0xE8C840;
    constexpr uint32_t WARNING_YELLOW = 0xFFFF00;
    constexpr uint32_t TEXT_DIM       = 0x404055;
    constexpr uint32_t WHITE          = 0xFFFFFF;
    constexpr uint32_t GREY           = 0x888888;
}

// ─── DISPLAY LAYOUT ──────────────────────────────────────
namespace Display {
    constexpr int16_t W          = 240;
    constexpr int16_t H          = 135;
    constexpr int16_t HEADER_H   = 14;
    constexpr int16_t FOOTER_H   = 24;
    constexpr int16_t CONTENT_H  = H - HEADER_H - FOOTER_H;  // 97px
    constexpr int16_t SPRITE_W   = 80;
    constexpr int16_t STATS_X    = SPRITE_W + 4;
    constexpr int16_t STATS_W    = W - SPRITE_W - 4;
}

// ─── CORE STATS STRUCTURE ────────────────────────────────
struct EvaStats {
    int16_t  syncRate;        // 0-100 (high = good synchronization)
    int16_t  hunger;          // 0-100 (high = full, low = starving)
    int16_t  stability;       // internal only
    int16_t  rage;            // 0-100 (high = berserk risk)
    int16_t  contamination;   // internal only
    int16_t  happiness;       // internal only
    int16_t  energy;          // 0-100 (high = good)
    int16_t  affection;       // internal only

    uint8_t  evolutionStage;
    uint16_t daysAlive;

    bool     berserk;
    bool     sleeping;
    bool     angelMode;

    uint32_t lastSaveTimestamp;
    uint8_t  evaUnit;
    bool     unitSelected;
    bool     mark06Unlocked;

    // ── New persistent fields ──────────────────────────
    uint8_t  soundMode;       // 0=OFF 1=ALERTS_ONLY 2=ALL
    uint16_t highScoreDays;   // best survival record

    // Runtime (not saved meaningfully)
    uint32_t lastFed;
    uint32_t lastSlept;
    uint32_t berserkTimer;
};

// ─── EVA PERSONALITY ─────────────────────────────────────
struct EvaPersonality {
    const char* name;
    const char* designator;
    const char* pilot;
    float  syncGainMod;
    float  hungerRate;
    float  rageRate;
    float  stabilityBase;
    float  contaminationRes;
    uint32_t primaryColor;
    uint32_t secondaryColor;
    uint32_t accentColor;
    const char* motto;
};

// ─── FEED ITEM DEFINITION ────────────────────────────────
struct FeedItemData {
    const char* name;        // display name
    const char* flavour;     // NERV terminal flavour text
    int16_t hungerMod;       // positive = fills hunger
    int16_t energyMod;
    int16_t syncMod;
    int16_t rageMod;
};

// ─── RANDOM EVENT (kept for EventManager compilation) ────
enum class EventType : uint8_t {
    NONE=0, ANGEL_ATTACK=1, SYNC_TEST=2, SYSTEM_FAULT=3,
    NERV_MESSAGE=4, CONTAMINATION_SPIKE=5, DREAM_SEQUENCE=6,
    THIRD_IMPACT_WARNING=7, COMMANDER_MESSAGE=8, POWER_SURGE=9, COUNT
};

struct GameEvent {
    EventType   type;
    const char* title;
    const char* message;
    int16_t     syncMod;
    int16_t     rageMod;
    int16_t     contaminationMod;
    int16_t     stabilityMod;
    bool        isEmergency;
};

// ─── QUOTE ENTRY ─────────────────────────────────────────
struct QuoteEntry {
    const char* text;
    const char* author;
    const char* episode;
};

// ─── ANGEL DATA ──────────────────────────────────────────
struct AngelData {
    const char* name;
    uint8_t     gameType;  // 0=SYNC_PULSE 1=INTERCEPT 2=BERSERK 3=random
};

// ─── UTILITY: Clamp ──────────────────────────────────────
inline int16_t statClamp(int16_t v, int16_t lo = 0, int16_t hi = 100) {
    return v < lo ? lo : (v > hi ? hi : v);
}

#pragma once
// ============================================================
// LOREDATA.H — NERV messages, quotes, boot lines
// ============================================================
#include <Arduino.h>
#include "core/Types.h"

// ─── NGE QUOTES (for DATA → Pilot File tab) ──────────────
static const char* const NGE_QUOTES_TEXT[] = {
    "I mustn't run away.",
    "How disgraceful.",
    "...",
    "The fate of destruction is the joy of rebirth.",
    "I am not afraid to die.",
    "I'm the best pilot. Don't forget that.",
    "I hate myself. But I can change.",
    "We are all afraid.",
    "Without you, I can do nothing.",
    "I was born for this moment.",
    "Don't worry. Everything will be alright.",
    "Mankind's greatest fear is mankind itself.",
    "I'm not your doll.",
    "People cannot erase what they have done.",
    "There is no salvation in this place.",
    "Fear is freedom. Constraint is liberation.",
    "We have to protect everyone.",
    "Nobody should be alone.",
    "Eva is not a weapon. It is a prayer.",
    "Congratulations.",
};
static const char* const NGE_QUOTES_ATTR[] = {
    "Shinji Ikari",
    "Asuka Langley",
    "Rei Ayanami",
    "SEELE transcript",
    "Rei Ayanami",
    "Asuka Langley",
    "Shinji Ikari",
    "Kaworu Nagisa",
    "Shinji Ikari",
    "Kaworu Nagisa",
    "Misato Katsuragi",
    "Gendo Ikari",
    "Rei Ayanami",
    "Gendo Ikari",
    "Ritsuko Akagi",
    "NERV doctrine",
    "Misato Katsuragi",
    "Yui Ikari",
    "Yui Ikari",
    "All cast",
};
constexpr uint8_t NGE_QUOTE_COUNT = 20;

// ─── NERV BROADCAST MESSAGES ─────────────────────────────
static const char* const NERV_MESSAGES[] PROGMEM = {
    "ALL PERSONNEL TO BATTLE STATIONS",
    "MAGI SYSTEM NOMINAL",
    "SYNCHRONIZATION RATIO WITHIN LIMITS",
    "PATTERN BLUE — ANGEL DETECTED",
    "LCL PRESSURE NOMINAL",
    "EVANGELION UNIT OPERATIONAL",
    "COMMANDER IKARI: PROCEED AS PLANNED",
    "UMBILICAL CABLE CONNECTED",
    "A.T. FIELD ENGAGED",
    "ENTRY PLUG SEALED",
    "CENTRAL DOGMA SECURE",
    "CLASSIFIED: EYES ONLY",
    "DO NOT OPEN DEAD INSIDE",
    "HUMAN INSTRUMENTALITY PROJECT: ONGOING",
    "ABSOLUTE TERROR FIELD: ACTIVE",
    "SEELE MONITORING... BE ADVISED",
    "DUMMY SYSTEM STANDBY",
    "THIRD IMPACT PREVENTION PRIORITY: ALPHA",
    "SOUL CONTAINMENT NOMINAL",
    "UNIT MENTAL CONTAMINATION: MONITOR",
};
constexpr uint8_t NERV_MSG_COUNT = 20;

// ─── DATA LOG ENTRIES ────────────────────────────────────
static const char* const DATA_LOGS[] PROGMEM = {
    "> LOG 0001\n  FIRST ACTIVATION.\n  IT MOVED.",
    "> LOG 0012\n  SYNC RATE RISING.\n  ANOMALOUS.",
    "> LOG 0047\n  PILOT TRAUMA NOTED.\n  PROCEED.",
    "> LOG 0088\n  BERSERK INCIDENT.\n  CONTAINED.",
    "> LOG 0112\n  SOUL RESONANCE.\n  CLASSIFIED.",
    "> LOG 0156\n  ANGEL ABSORBED.\n  CONTAMINATION\n  SPIKE.",
    "> LOG 0201\n  MATERNAL UNIT\n  RESPONDING.",
    "> LOG 0244\n  SCENARIO DELTA.\n  UNFORESEEN.",
    "> LOG 0288\n  AT FIELD INVERSION.\n  THEORIZED.",
    "> LOG 0333\n  PROJECT: ONGOING.\n  NO DEVIATION.",
    "> LOG 0401\n  PILOT CONDITION:\n  ACCEPTABLE.",
    "> LOG 0444\n  ZERO POINT BREACH.\n  MONITOR.",
};
constexpr uint8_t DATA_LOG_COUNT = 12;

// ─── RANDOM EVENTS ───────────────────────────────────────
static const GameEvent RANDOM_EVENTS[] PROGMEM = {
    // ANGEL_ATTACK
    { EventType::ANGEL_ATTACK,
      "ANGEL ATTACK",
      "PATTERN BLUE\nDETECTED.\nDEFEND NOW.",
      -10, +25, +10, -15, true },

    // SYNC_TEST
    { EventType::SYNC_TEST,
      "SYNC TEST",
      "MAGI INITIATED\nDIAGNOSTICS.\nFOCUS.",
      +15, -5, 0, +5, false },

    // SYSTEM_FAULT
    { EventType::SYSTEM_FAULT,
      "SYSTEM FAULT",
      "CRITICAL ERROR\nIN SUBSYSTEM 7.\nREBOOT?",
      -5, +10, +5, -20, true },

    // NERV_MESSAGE
    { EventType::NERV_MESSAGE,
      "NERV DISPATCH",
      "COMMANDER:\nALL IS PROCEEDING\nAS PLANNED.",
      0, -10, 0, +5, false },

    // CONTAMINATION_SPIKE
    { EventType::CONTAMINATION_SPIKE,
      "CONTAMINATION",
      "ANGEL DATA\nDETECTED IN\nCORE MATRIX.",
      0, +5, +20, -10, true },

    // DREAM_SEQUENCE
    { EventType::DREAM_SEQUENCE,
      "DREAM.EXE",
      "SYNCHRONIZATION\nTOO DEEP.\nWHERE AM I?",
      +10, 0, +5, -5, false },

    // THIRD_IMPACT_WARNING
    { EventType::THIRD_IMPACT_WARNING,
      "!! ALERT !!",
      "LANCE DETECTED.\nTHIRD IMPACT\nIMMINENT?",
      -15, +30, +15, -25, true },

    // COMMANDER_MESSAGE
    { EventType::COMMANDER_MESSAGE,
      "GENDO IKARI",
      "YOU HAVE\nPURPOSE.\nDO NOT FAIL.",
      +5, -5, 0, +10, false },

    // POWER_SURGE
    { EventType::POWER_SURGE,
      "POWER SURGE",
      "UMBILICAL\nOVERLOAD.\nSWITCH TO\nINTERNAL.",
      0, +15, +8, -15, true },
};
constexpr uint8_t RANDOM_EVENT_COUNT = 9;

// ─── EVOLUTION STAGE DESCRIPTIONS ────────────────────────
static const char* const EVOLUTION_NAMES[6] PROGMEM = {
    "NASCENT",
    "STABLE",
    "BERSERK",
    "ANGEL-HYB",
    "BRKN CORE",
    "PERF SYNC"
};

static const char* const EVOLUTION_DESC[6] PROGMEM = {
    "NEWLY AWAKENED.\nSYNAPTIC PATHS\nFORMING.",
    "OPERATIONAL.\nPILOT BOND\nSTABLE.",
    "CONTROL LOST.\nBERSERK MODE\nACTIVE.",
    "ANGEL DATA\nCOMPROMISING\nCORE.",
    "CORE FRACTURE\nIMMINENT.\nCRITICAL.",
    "ABSOLUTE SYNC.\nBOUNDARY\nDISSOLVED."
};

// ─── BOOT SEQUENCE LINES ─────────────────────────────────
static const char* const BOOT_LINES[] PROGMEM = {
    "NERV MAGI SYSTEM v3.14.2",
    "EVANGELION OS KERNEL LOADING...",
    "CHECKING CORE INTEGRITY......OK",
    "SYNCHRONIZATION MATRIX.......OK",
    "LCL FLUID SYSTEM.............OK",
    "A.T. FIELD EMITTER...........OK",
    "IMU SENSOR CALIBRATING.......",
    "LOADING PILOT PROFILE........",
    "RESTORING UNIT STATE.........",
    "INITIATING EVANGELION........",
};
constexpr uint8_t BOOT_LINE_COUNT = 10;

// ─── QUOTE DATABASE (25 entries, shuffled in QuoteScene) ─
constexpr uint8_t QUOTE_COUNT = 25;
static const QuoteEntry QUOTE_ENTRIES[QUOTE_COUNT] = {
    {"I mustn't run away.",                        "Shinji Ikari",      "EP:01"},
    {"How disgraceful.",                           "Asuka Langley",     "EP:09"},
    {"...",                                        "Rei Ayanami",       "Various"},
    {"The fate of destruction is the joy of rebirth.",
                                                   "SEELE",             "Various"},
    {"I am not afraid to die.",                    "Rei Ayanami",       "EP:19"},
    {"I'm the best pilot. Don't forget that.",     "Asuka Langley",     "EP:02"},
    {"I hate myself. But I can change.",           "Shinji Ikari",      "EoE"},
    {"We are all afraid.",                         "Kaworu Nagisa",     "EP:24"},
    {"Without you, I can do nothing.",             "Shinji Ikari",      "EP:24"},
    {"I was born for this moment.",                "Kaworu Nagisa",     "EP:24"},
    {"Don't worry. Everything will be alright.",   "Misato Katsuragi",  "EP:24"},
    {"Mankind's greatest fear is mankind itself.", "Gendo Ikari",       "EoE"},
    {"I'm not your doll.",                         "Rei Ayanami",       "EP:23"},
    {"People cannot erase what they have done.",   "Gendo Ikari",       "Various"},
    {"There is no salvation in this place.",       "Ritsuko Akagi",     "EoE"},
    {"Fear is freedom. Constraint is liberation.", "NERV Doctrine",     "Various"},
    {"We have to protect everyone.",               "Misato Katsuragi",  "Various"},
    {"Nobody should be alone.",                    "Yui Ikari",         "Various"},
    {"Eva is not a weapon. It is a prayer.",       "Yui Ikari",         "Various"},
    {"Congratulations.",                           "All Cast",          "EoE"},
    {"I need you.",                                "Shinji Ikari",      "EP:24"},
    {"The AT Field is the light of the soul.",     "Rei Ayanami",       "Various"},
    {"This is... goodbye.",                        "Kaworu Nagisa",     "EP:24"},
    {"I feel... good.",                            "Shinji Ikari",      "EoE"},
    {"Komm, susser Tod.",                          "NERV Archive",      "EoE"},
};

// ─── GAME DEFINITIONS (shared by FightScene + AngelAlarmScene) ──
struct GameDef {
    const char* name;
    const char* instr1;
    const char* instr2;
    uint8_t     stars;  // 1-3
};
constexpr uint8_t GAME_COUNT = 9;
static const GameDef GAME_DEFS[GAME_COUNT] = {
    { "SYNC PULSE",     "Press A when ring hits target",   "Time your press precisely!",       2 },
    { "INTERCEPT",      "Press matching button A or B",    "React before window closes.",       1 },
    { "BERSERK CTRL",   "Mash BTN_A to suppress rage",     "More presses = better control.",    2 },
    { "LANCE",          "Press A when lance aligns",       "5 throws — aim for the target.",    2 },
    { "MAGI VOTE",      "A=YES  B=NO — vote the majority", "3 MAGI vote — match 2 of 3.",       1 },
    { "DUMMY PLUG",     "A=left  B=right — watch first",   "Repeat the sequence from memory.",  3 },
    { "CORE DEFENSE",   "A=dodge left   B=dodge right",    "Survive 12 waves. 3 lives only.",   3 },
    { "CALIBRATION",    "Keep bar inside the green zone",  "A=push right   B=push left",        2 },
    { "THIRD IMPACT",   "Third Impact is imminent!",       "Mash BTN_A as fast as you can!",    3 },
};

// ─── ANGEL DATABASE (12 entries, gameType = GAME_DEFS index) ─
constexpr uint8_t ANGEL_COUNT = 12;
static const AngelData ANGEL_LIST[ANGEL_COUNT] = {
    {"SACHIEL",    3},   // LANCE
    {"SHAMSHEL",   7},   // CALIBRATION
    {"RAMIEL",     0},   // SYNC_PULSE
    {"GAGHIEL",    1},   // INTERCEPT
    {"ISRAFEL",    5},   // DUMMY_PLUG
    {"SANDALPHON", 8},   // THIRD_IMPACT
    {"MATARAEL",   6},   // CORE_DEFENSE
    {"SAHAQUIEL",  2},   // BERSERK_CTRL
    {"IRUEL",      4},   // MAGI_VOTE
    {"LELIEL",     1},   // INTERCEPT
    {"BARDIEL",    5},   // DUMMY_PLUG
    {"ZERUEL",     8},   // THIRD_IMPACT
};

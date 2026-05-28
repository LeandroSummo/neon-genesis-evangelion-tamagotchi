// ============================================================
// AUDIOMANAGER.CPP
// ============================================================
#include "audio/AudioManager.h"

// ─── Note sequences ──────────────────────────────────────
static const AudioManager::Note SND_BOOT[] = {
    {220, 80}, {330, 80}, {440, 80}, {660, 120}, {0, 60},
    {880, 200}, {660, 80}, {880, 300}
};
static const AudioManager::Note SND_SELECT[] = {
    {440, 60}, {660, 60}, {880, 120}
};
static const AudioManager::Note SND_FEED[] = {
    {523, 80}, {659, 80}, {784, 120}
};
static const AudioManager::Note SND_ALERT[] = {
    {880, 100}, {0, 50}, {880, 100}, {0, 50}, {880, 200}
};
static const AudioManager::Note SND_BERSERK[] = {
    {100, 80}, {150, 60}, {80,  80}, {200, 60},
    {60,  100},{250, 80}, {40,  120}
};
static const AudioManager::Note SND_HEARTBEAT[] = {
    {80, 100}, {0, 80}, {80, 100}, {0, 500}
};
static const AudioManager::Note SND_GLITCH[] = {
    {1200, 30}, {400, 20}, {900, 25}, {200, 30}, {1500, 20}
};
static const AudioManager::Note SND_EVOLUTION[] = {
    {261, 60},{329, 60},{392, 60},{523, 100},{0,80},
    {392, 80},{523, 80},{659, 100},{784, 200}
};
static const AudioManager::Note SND_WARNING[] = {
    {660, 150},{0, 100},{660, 150},{0, 100},{880, 300}
};

// ─── v2.1 sequences ──────────────────────────────────────
static const AudioManager::Note SND_ALERT_ANGEL[] = {
    {880, 150},{0, 80},{880, 150},{0, 80},
    {1047,200},{0,100},{1047,200},{0,100},
    {1319,300},{0, 50},{1319,300}
};
static const AudioManager::Note SND_HIT_SUCCESS[] = {
    {523, 60},{659, 60},{784, 80}
};
static const AudioManager::Note SND_HIT_MISS[] = {
    {300, 80},{220, 120}
};
static const AudioManager::Note SND_ANGEL_DEFEATED[] = {
    {523, 80},{659, 80},{784, 80},{1047,150},
    {0,  100},{1047, 80},{1319,200}
};
static const AudioManager::Note SND_UNIT_DAMAGED[] = {
    {150, 200},{0, 50},{100, 300},{0, 50},{80, 400}
};
static const AudioManager::Note SND_QUOTE_APPEAR[] = {
    {880, 30},{0, 20},{1047, 30},{0, 20},{1319, 60}
};
static const AudioManager::Note SND_BERSERK_LOOP[] = {
    {200, 40},{300, 40},{150, 40},{400, 40}
};

// ─────────────────────────────────────────────────────────

void AudioManager::begin() {
    M5.Speaker.setVolume(120);
    M5.Speaker.begin();
}

void AudioManager::update() {
    if (!_seqActive) return;
    if (_soundMode == 0) { _seqActive = false; M5.Speaker.stop(); return; }
    if (_soundMode == 1 && !_seqIsAlert) { _seqActive = false; M5.Speaker.stop(); return; }

    uint32_t now     = millis();
    uint32_t elapsed = now - _seqStart;

    uint32_t acc = 0;
    for (uint8_t i = 0; i < _seqLen; i++) {
        acc += _seq[i].dur;
        if (elapsed < acc) {
            if (i != _seqIdx) {
                _seqIdx = i;
                uint16_t f = _seq[i].freq;
                if (f > 0) M5.Speaker.tone(f, _seq[i].dur);
                else       M5.Speaker.stop();
            }
            return;
        }
    }
    // Sequence finished
    if (_seqLoop) {
        _seqStart = now;
        _seqIdx   = 0xFF;
        if (_seq[0].freq > 0) M5.Speaker.tone(_seq[0].freq, _seq[0].dur);
    } else {
        M5.Speaker.stop();
        _seqActive = false;
    }
}

void AudioManager::_playSequence(const Note* notes, uint8_t len,
                                  bool isAlert, bool loop) {
    if (_soundMode == 0) return;
    if (_soundMode == 1 && !isAlert) return;
    _seq        = notes;
    _seqLen     = len;
    _seqIdx     = 0xFF;
    _seqStart   = millis();
    _seqActive  = true;
    _seqLoop    = loop;
    _seqIsAlert = isAlert;
    if (len > 0 && notes[0].freq > 0) M5.Speaker.tone(notes[0].freq, notes[0].dur);
}

void AudioManager::stopSequence() {
    _seqActive = false;
    _seqLoop   = false;
    M5.Speaker.stop();
}

// ─── Existing SFX ────────────────────────────────────────
void AudioManager::playBoot()      { _playSequence(SND_BOOT,      sizeof(SND_BOOT)      / sizeof(Note)); }
void AudioManager::playSelect()    { _playSequence(SND_SELECT,    sizeof(SND_SELECT)    / sizeof(Note)); }
void AudioManager::playFeed()      { _playSequence(SND_FEED,      sizeof(SND_FEED)      / sizeof(Note)); }
void AudioManager::playAlert()     { _playSequence(SND_ALERT,     sizeof(SND_ALERT)     / sizeof(Note), true); }
void AudioManager::playBerserk()   { _playSequence(SND_BERSERK,   sizeof(SND_BERSERK)   / sizeof(Note), true); }
void AudioManager::playHeartbeat() { _playSequence(SND_HEARTBEAT, sizeof(SND_HEARTBEAT) / sizeof(Note), true); }
void AudioManager::playGlitch()    { _playSequence(SND_GLITCH,    sizeof(SND_GLITCH)    / sizeof(Note)); }
void AudioManager::playEvolution() { _playSequence(SND_EVOLUTION, sizeof(SND_EVOLUTION) / sizeof(Note)); }
void AudioManager::playWarning()   { _playSequence(SND_WARNING,   sizeof(SND_WARNING)   / sizeof(Note), true); }

void AudioManager::playBeep(uint16_t freq, uint16_t durMs) {
    if (_soundMode == 0) return;
    M5.Speaker.tone(freq, durMs);
}

// ─── v2.1 SFX ────────────────────────────────────────────
void AudioManager::playAlertAngel()   { _playSequence(SND_ALERT_ANGEL,   sizeof(SND_ALERT_ANGEL)   / sizeof(Note), true); }
void AudioManager::playHitSuccess()   { _playSequence(SND_HIT_SUCCESS,   sizeof(SND_HIT_SUCCESS)   / sizeof(Note)); }
void AudioManager::playHitMiss()      { _playSequence(SND_HIT_MISS,      sizeof(SND_HIT_MISS)      / sizeof(Note)); }
void AudioManager::playAngelDefeated(){ _playSequence(SND_ANGEL_DEFEATED, sizeof(SND_ANGEL_DEFEATED)/ sizeof(Note), true); }
void AudioManager::playUnitDamaged()  { _playSequence(SND_UNIT_DAMAGED,  sizeof(SND_UNIT_DAMAGED)  / sizeof(Note), true); }
void AudioManager::playQuoteAppear()  { _playSequence(SND_QUOTE_APPEAR,  sizeof(SND_QUOTE_APPEAR)  / sizeof(Note)); }
void AudioManager::playBerserkLoop()  { _playSequence(SND_BERSERK_LOOP,  sizeof(SND_BERSERK_LOOP)  / sizeof(Note), true, true); }

static const AudioManager::Note SND_SHUTDOWN[] = {
    {880, 200},{0, 100},{659, 200},{0, 100},
    {523, 200},{0, 100},{392, 300},{0, 200},{262, 500}
};
void AudioManager::playShutdown() { _playSequence(SND_SHUTDOWN, sizeof(SND_SHUTDOWN) / sizeof(Note)); }

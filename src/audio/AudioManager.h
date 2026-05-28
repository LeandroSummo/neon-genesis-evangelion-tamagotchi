#pragma once
// ============================================================
// AUDIOMANAGER.H — Low-bit speaker sound effects
// soundMode: 0=OFF  1=ALERTS_ONLY  2=ALL
// ============================================================
#include <M5Unified.h>

class AudioManager {
public:
    static AudioManager& get() {
        static AudioManager instance;
        return instance;
    }

    void begin();
    void update();   // call each loop; handles non-blocking sequences

    // Existing SFX
    void playBoot();
    void playSelect();
    void playFeed();
    void playAlert();        // alert
    void playBerserk();      // alert
    void playHeartbeat();    // alert
    void playGlitch();
    void playEvolution();
    void playWarning();      // alert
    void playBeep(uint16_t freq, uint16_t durMs);

    // v2.1 SFX
    void playAlertAngel();    // alert
    void playHitSuccess();
    void playHitMiss();
    void playAngelDefeated(); // alert
    void playUnitDamaged();   // alert
    void playQuoteAppear();
    void playBerserkLoop();   // alert, loops until stopSequence()
    void playShutdown();
    void stopSequence();

    // Sound mode
    void    setSoundMode(uint8_t m) { _soundMode = (m <= 2) ? m : 2; }
    uint8_t getSoundMode()  const   { return _soundMode; }
    bool    isMuted()       const   { return _soundMode == 0; }

    struct Note { uint16_t freq; uint16_t dur; };

private:
    AudioManager() = default;

    void _playSequence(const Note* notes, uint8_t len,
                       bool isAlert = false, bool loop = false);

    uint8_t      _soundMode  = 2;     // ALL by default
    uint32_t     _seqStart   = 0;
    const Note*  _seq        = nullptr;
    uint8_t      _seqLen     = 0;
    uint8_t      _seqIdx     = 0;
    bool         _seqActive  = false;
    bool         _seqLoop    = false;
    bool         _seqIsAlert = false;
};

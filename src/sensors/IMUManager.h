#pragma once
// ============================================================
// IMUMANAGER.H — MPU6886 gesture detection
// ============================================================
#include <M5Unified.h>
#include "core/Types.h"

class IMUManager {
public:
    static IMUManager& get() {
        static IMUManager instance;
        return instance;
    }

    void begin();
    void update();          // call every frame

    InputEvent getEvent();  // returns and consumes pending event
    bool       hasEvent()   const { return _event != InputEvent::NONE; }

    // Raw values for training minigame
    float getAccX() const { return _ax; }
    float getAccY() const { return _ay; }
    float getAccZ() const { return _az; }

    // Tilt direction (-1 left/back, 0 neutral, +1 right/forward)
    int8_t getTiltX() const;
    int8_t getTiltY() const;

    bool shakeDetected()      const { return _shaking; }
    bool gentleMoveDetected() const { return _wasGentle; }

private:
    IMUManager() = default;

    float      _ax = 0, _ay = 0, _az = 0;
    InputEvent _event     = InputEvent::NONE;
    uint32_t   _lastUpdate = 0;
    uint32_t   _lastShake  = 0;

    // Shake detection
    float      _shakeAccum = 0;
    uint32_t   _shakeTimer = 0;
    bool       _shaking    = false;

    // Gentle motion detection (for petting)
    float _prevMag        = 0;
    uint32_t _gentleTimer = 0;
    bool     _wasGentle   = false;

    static constexpr float SHAKE_THRESHOLD  = 1.8f;  // g
    static constexpr float TILT_THRESHOLD   = 0.4f;  // g
    static constexpr float GENTLE_THRESHOLD = 0.15f; // g above 1g
    static constexpr uint32_t UPDATE_MS     = 50;    // 20Hz polling
};

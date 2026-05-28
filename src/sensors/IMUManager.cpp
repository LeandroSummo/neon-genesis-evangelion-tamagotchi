// ============================================================
// IMUMANAGER.CPP
// ============================================================
#include "sensors/IMUManager.h"
#include <math.h>

void IMUManager::begin() {
    // M5Unified initialises IMU automatically in M5.begin()
    _lastUpdate = millis();
    _prevMag    = 1.0f; // ~1g at rest
}

void IMUManager::update() {
    uint32_t now = millis();
    if (now - _lastUpdate < UPDATE_MS) return;
    _lastUpdate = now;

    // Read accelerometer
    M5.Imu.getAccel(&_ax, &_ay, &_az);

    // ─── Magnitude for shake detection ───
    float mag = sqrtf(_ax * _ax + _ay * _ay + _az * _az);

    // Shake: rapid change in acceleration > threshold
    float delta = fabsf(mag - _prevMag);
    _prevMag = mag;

    if (delta > SHAKE_THRESHOLD) {
        if (now - _lastShake > 800) {  // debounce
            _event    = InputEvent::SHAKE;
            _lastShake = now;
            return;
        }
    }

    // ─── Gentle motion (petting) ───
    bool gentle = (delta > GENTLE_THRESHOLD && delta < 0.5f);
    if (gentle) {
        if (!_wasGentle) {
            _gentleTimer = now;
            _wasGentle   = true;
        } else if (now - _gentleTimer > 600) {
            // Sustained gentle motion = petting
            _event = InputEvent::GENTLE_MOVE;
            _wasGentle = false;
            return;
        }
    } else {
        _wasGentle = false;
    }

    // ─── Tilt detection ───
    // _ay: tilt left/right on landscape M5StickC
    // _ax: tilt forward/backward
    if (_ay > TILT_THRESHOLD) {
        _event = InputEvent::TILT_RIGHT;
    } else if (_ay < -TILT_THRESHOLD) {
        _event = InputEvent::TILT_LEFT;
    } else if (_ax > TILT_THRESHOLD) {
        _event = InputEvent::TILT_FORWARD;
    }
}

InputEvent IMUManager::getEvent() {
    InputEvent ev = _event;
    _event = InputEvent::NONE;
    return ev;
}

int8_t IMUManager::getTiltX() const {
    if (_ay > TILT_THRESHOLD)  return 1;
    if (_ay < -TILT_THRESHOLD) return -1;
    return 0;
}

int8_t IMUManager::getTiltY() const {
    if (_ax > TILT_THRESHOLD)  return 1;
    if (_ax < -TILT_THRESHOLD) return -1;
    return 0;
}

#ifndef LAD_H
#define LAD_H

#include <Arduino.h>

//
// Таймер TON (включение с задержкой)
//
struct TimerTON {
    bool IN_input = false;                  // Входной сигнал
    bool Q_output = false;                  // Выходной результат
    unsigned long PT_presetTime = 1000;     // Время задержки (мс)
    unsigned long startTime_ms = 0;         // Время начала

    void update(bool input, unsigned long now_ms) {
        // Входной сигнал сменился с false на true → стартуем таймер
        if (input && !IN_input) {
            startTime_ms = now_ms;
        }

        IN_input = input;

        if (!IN_input) {
            Q_output = false;
        } else {
            Q_output = (now_ms - startTime_ms) >= PT_presetTime;
        }
    }

    void reset() {
        IN_input = false;
        Q_output = false;
        startTime_ms = 0;
    }

    bool done() const { return Q_output; }
};

//
// Таймер TOF (выключение с задержкой)
//
struct TimerTOF {
    bool IN_input = false;
    bool Q_output = false;
    unsigned long PT_presetTime = 1000;
    unsigned long stopTime_ms = 0;
    bool timing = false;

    void update(bool input, unsigned long now_ms) {
        if (!input && IN_input) {
            stopTime_ms = now_ms;
            timing = true;
        }

        IN_input = input;

        if (IN_input) {
            Q_output = true;
            timing = false;
        } else if (timing && (now_ms - stopTime_ms >= PT_presetTime)) {
            Q_output = false;
            timing = false;
        }
    }

    void reset() {
        IN_input = false;
        Q_output = false;
        timing = false;
        stopTime_ms = 0;
    }

    bool done() const { return Q_output; }
};

#endif // LAD_H

#include <Arduino.h>
#include <EEPROM.h>

#include "config.h"
#include "scheduler.h"
#include "webserver_me.h"
#include "ntp_time.h"
#include "eeprom_store.h"


unsigned long lastFillPumpOffTime = 0;
bool fillPumpState = false;

void setupPins() {
    pinMode(PIN_FILL_PUMP, OUTPUT);
    pinMode(PIN_IRR_PUMP, OUTPUT);
    pinMode(PIN_VALVE_1, OUTPUT);
    pinMode(PIN_VALVE_2, OUTPUT);
    pinMode(PIN_VALVE_3, OUTPUT);
    pinMode(PIN_FLOAT_SENSOR, INPUT);
}

void controlFillPump() {
    bool floatLow = digitalRead(PIN_FLOAT_SENSOR) == LOW;
    bool anyValveActive = isValveScheduled(1) || isValveScheduled(2) || isValveScheduled(3);
    bool canStart = (millis() - lastFillPumpOffTime) > FILL_PUMP_MIN_OFF_TIME * 1000;

    if (floatLow && !anyValveActive && canStart) {
        digitalWrite(PIN_FILL_PUMP, HIGH);
        fillPumpState = true;
    } else {
        if (fillPumpState) {
            fillPumpState = false;
            lastFillPumpOffTime = millis();
            saveLastFillPumpOffTime(lastFillPumpOffTime);
        }
        digitalWrite(PIN_FILL_PUMP, LOW);
    }
}

void controlValvesAndIrrigationPump() {
    bool pumpNeeded = false;

    for (uint8_t i = 1; i <= 3; i++) {
        bool valveOn = getScheduleState(i);
        digitalWrite(PIN_VALVE_1 + i - 1, valveOn ? HIGH : LOW);
        if (valveOn) pumpNeeded = true;
    }

    // Учитываем ручной режим
    if (isManualIrrigationActive()) pumpNeeded = true;

    digitalWrite(PIN_IRR_PUMP, pumpNeeded ? HIGH : LOW);
}

void setup() {
    Serial.begin(115200);
    setupPins();
    EEPROM.begin(EEPROM_SIZE);

    initScheduler();
    setupWiFi();
    setupWebServer();
    initTime();

    lastFillPumpOffTime = getLastFillPumpOffTime();
}

void loop() {
    uint16_t nowMinutes = getMinutesSinceMidnight();

    // Релейный цикл
    checkSchedules(nowMinutes);
    controlValvesAndIrrigationPump();
    controlFillPump();
    handleWebRequests();

    delay(1000); // Основной цикл раз в 1 сек
}

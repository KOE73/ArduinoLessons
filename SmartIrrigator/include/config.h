#ifndef CONFIG_H
#define CONFIG_H

// Пины устройств
#define PIN_FILL_PUMP 5 // Насос для заполнения емкости
#define PIN_IRR_PUMP 6  // Насос для полива
#define PIN_VALVE_1 7
#define PIN_VALVE_2 8
#define PIN_VALVE_3 9

#define PIN_FLOAT_SENSOR 3 // Поплавковый датчик уровня

#define PIN_SDA 20
#define PIN_SCL 21

// Время
#define FILL_PUMP_MAX_RUNTIME_MS 15 * 60 * 1000UL  // 15 минут Максимальное время работы насоса
#define FILL_PUMP_MIN_OFF_TIME_MS 30 * 60 * 1000UL // 30 минут Минимальная пауза между включениями насоса
#define FULL_STABLE_TIME_MS 10 * 1000UL         // 10 секунд устойчивого сигнала

// Расписание
#define MAX_SCHEDULES 8
#define VALVE_COUNT 3

// WiFi
#define AP_SSID "IrrigationSystem"
#define AP_PASSWORD "12345678"
#define WIFI_CONNECT_TIMEOUT 5000 // мс

// Веб
#define SERVER_PORT 80

// EEPROM
#define EEPROM_SIZE 512

// LOG
#define BOOL_STR(b) ((b) ? "✅" : "❌")

#endif

#ifndef CONFIG_H
#define CONFIG_H

// Пины устройств
#define PIN_FILL_PUMP     2    // Насос для заполнения емкости
#define PIN_IRR_PUMP      3    // Насос для полива
#define PIN_VALVE_1       4
#define PIN_VALVE_2       5
#define PIN_VALVE_3       6
#define PIN_FLOAT_SENSOR  7    // Поплавковый датчик уровня

// Время
#define FILL_PUMP_MIN_OFF_TIME 1800  // 30 минут (в секундах)

// Расписание
#define MAX_SCHEDULES 8

// WiFi
#define AP_SSID "IrrigationSystem"
#define AP_PASSWORD "12345678"
#define WIFI_CONNECT_TIMEOUT 15000  // мс

// Веб
#define SERVER_PORT 80

// EEPROM
#define EEPROM_SIZE 512

#endif

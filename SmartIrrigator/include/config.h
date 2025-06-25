#ifndef CONFIG_H
#define CONFIG_H

// Пины устройств
#define PIN_FILL_PUMP     5    // Насос для заполнения емкости
#define PIN_IRR_PUMP      6    // Насос для полива
#define PIN_VALVE_1       7
#define PIN_VALVE_2       8
#define PIN_VALVE_3       9

#define PIN_FLOAT_SENSOR  4    // Поплавковый датчик уровня

#define PIN_SDA 20
#define PIN_SCL 21

// Время
#define FILL_PUMP_MIN_OFF_TIME 1800  // 30 минут (в секундах)

// Расписание
#define MAX_SCHEDULES 8
#define VALVE_COUNT 3

// WiFi
#define AP_SSID "IrrigationSystem"
#define AP_PASSWORD "12345678"
#define WIFI_CONNECT_TIMEOUT 5000  // мс

// Веб
#define SERVER_PORT 80

// EEPROM
#define EEPROM_SIZE 512

#endif

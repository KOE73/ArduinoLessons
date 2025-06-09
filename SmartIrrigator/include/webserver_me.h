#ifndef WEBSERVER_H
#define WEBSERVER_H

void setupWiFi();          // Подключение к Wi-Fi или поднятие точки доступа
void setupWebServer();     // Инициализация веб-сервера
void handleWebRequests();  // Обработка HTTP-запросов (вызов в loop)

#endif

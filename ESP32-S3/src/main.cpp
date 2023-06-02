#include <Arduino.h>
#include "Arduino_GFX_Library.h"
#include "pin_config.h"

Arduino_DataBus *bus = new Arduino_ESP32LCD8(7 /* DC */, 6 /* CS */, 8 /* WR */, 9 /* RD */, 39 /* D0 */, 40 /* D1 */, 41 /* D2 */, 42 /* D3 */,
                                             45 /* D4 */, 46 /* D5 */, 47 /* D6 */, 48 /* D7 */);
Arduino_GFX *gfx = new Arduino_ST7789(bus, 5 /* RST */, 0 /* rotation */, true /* IPS */, 170 /* width */, 320 /* height */, 35 /* col offset 1 */,
                                      0 /* row offset 1 */, 35 /* col offset 2 */, 0 /* row offset 2 */);

void setup()
{
  Serial.begin(115200);
  Serial.println("Start!");

  ledcSetup(0, 2000, 8);
  ledcAttachPin(PIN_LCD_BL, 0);
  ledcWrite(0, 255); /* Screen brightness can be modified by adjusting this parameter. (0-255) */

  gfx->begin();
  gfx->setRotation(1);

  gfx->setTextSize(3);
  gfx->setTextColor(gfx->color565(0xff, 0x00, 0x00));
  gfx->print(F("RED "));
  gfx->setTextColor(gfx->color565(0x00, 0xff, 0x00));
  gfx->print(F("GREEN "));
  gfx->setTextColor(gfx->color565(0x00, 0x00, 0xff));
  gfx->println(F("BLUE"));

  Serial.println("Work!");
}

const char *typeName(esp_partition_type_t t) 
{
  switch (t)
  {
  case ESP_PARTITION_TYPE_APP:
    return "APP";
  case ESP_PARTITION_TYPE_DATA:
    return "DATA";
  case ESP_PARTITION_TYPE_ANY:
    return "ANY";
  }
  return "???";
}

const char *subTypeName(esp_partition_type_t t, esp_partition_subtype_t st)
{
  switch (t)
  {
  case ESP_PARTITION_TYPE_APP:
    switch (st)
    {
    case ESP_PARTITION_SUBTYPE_APP_FACTORY:
      return "FACTORY";
    case ESP_PARTITION_SUBTYPE_APP_OTA_0:
      return "OTA_0";
    case ESP_PARTITION_SUBTYPE_APP_OTA_1:
      return "OTA_1";
    case ESP_PARTITION_SUBTYPE_APP_OTA_2:
      return "OTA_2";
    case ESP_PARTITION_SUBTYPE_APP_OTA_3:
      return "OTA_3";
    case ESP_PARTITION_SUBTYPE_APP_OTA_4:
      return "OTA_4";
    case ESP_PARTITION_SUBTYPE_APP_OTA_5:
      return "OTA_5";
    case ESP_PARTITION_SUBTYPE_APP_OTA_6:
      return "OTA_6";
    case ESP_PARTITION_SUBTYPE_APP_OTA_7:
      return "OTA_7";
    case ESP_PARTITION_SUBTYPE_APP_OTA_8:
      return "OTA_8";
    case ESP_PARTITION_SUBTYPE_APP_OTA_9:
      return "OTA_9";
    case ESP_PARTITION_SUBTYPE_APP_OTA_10:
      return "OTA_10";
    case ESP_PARTITION_SUBTYPE_APP_OTA_11:
      return "OTA_11";
    case ESP_PARTITION_SUBTYPE_APP_OTA_12:
      return "OTA_12";
    case ESP_PARTITION_SUBTYPE_APP_OTA_13:
      return "OTA_13";
    case ESP_PARTITION_SUBTYPE_APP_OTA_14:
      return "OTA_14";
    case ESP_PARTITION_SUBTYPE_APP_OTA_15:
      return "OTA_15";
    case ESP_PARTITION_SUBTYPE_APP_TEST:
      return "TEST";
    }
    return "APP ?";
  case ESP_PARTITION_TYPE_DATA:
    switch (st)
    {
    case ESP_PARTITION_SUBTYPE_DATA_OTA:
      return "OTA";
    case ESP_PARTITION_SUBTYPE_DATA_PHY:
      return "PHY";
    case ESP_PARTITION_SUBTYPE_DATA_NVS:
      return "NVS";
    case ESP_PARTITION_SUBTYPE_DATA_COREDUMP:
      return "COREDUMP";
    case ESP_PARTITION_SUBTYPE_DATA_NVS_KEYS:
      return "NVS_KEYS";
    case ESP_PARTITION_SUBTYPE_DATA_EFUSE_EM:
      return "EFUSE_EM";
    case ESP_PARTITION_SUBTYPE_DATA_UNDEFINED:
      return "UNDEFINED";
    case ESP_PARTITION_SUBTYPE_DATA_ESPHTTPD:
      return "ESPHTTPD";
    case ESP_PARTITION_SUBTYPE_DATA_FAT:
      return "FAT";
    case ESP_PARTITION_SUBTYPE_DATA_SPIFFS:
      return "SPIFFS";
    }
    return "DATA ?";
  }

  return "???";
}

int iii = 0;
void loop()
{
  int n = 0;
  esp_partition_iterator_t i = esp_partition_find(ESP_PARTITION_TYPE_ANY, ESP_PARTITION_SUBTYPE_ANY, NULL);
  do
  {
    const esp_partition_t *p = esp_partition_get(i);
    Serial.printf("%i.\t%-12s%-12s%-17s0x%06lX\t0x%06lX  %5liKb %8li \n", n, typeName(p->type), subTypeName(p->type, p->subtype), p->label, p->address, p->size, p->size / 1024, p->size);
    n++;
  } while (i = esp_partition_next(i));

  esp_partition_iterator_release(i);

  Serial.printf("N %i\n", iii++);
  delay(10000);
}
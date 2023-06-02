#pragma once

#include "Arduino_GFX_Library.h"
#include "pin_config.h"

#define ScreenWidth 170
#define ScreenHeight 320

// Из-за setRotation(1) поворачиваем координаты для ризования.
#define ScreenW ScreenHeight
#define ScreenH ScreenWidth

Arduino_DataBus *bus = new Arduino_ESP32LCD8(
    7 /* DC */,
    6 /* CS */,
    8 /* WR */,
    9 /* RD */,
    39 /* D0 */,
    40 /* D1 */,
    41 /* D2 */,
    42 /* D3 */,
    45 /* D4 */,
    46 /* D5 */,
    47 /* D6 */,
    48 /* D7 */);

Arduino_GFX *gfx = new Arduino_ST7789(
    bus,
    5 /* RST */,
    0 /* rotation */,
    true /* IPS */,
    ScreenWidth /* width */,
    ScreenHeight /* height */,
    35 /* col offset 1 */,
    0 /* row offset 1 */,
    35 /* col offset 2 */,
    0 /* row offset 2 */);

void ScreenInit()
{
    ledcSetup(0, 2000, 8);
    ledcAttachPin(PIN_LCD_BL, 0);
    ledcWrite(0, 255); /* Screen brightness can be modified by adjusting this parameter. (0-255) */

    gfx->begin();
    gfx->setRotation(1);

    gfx->fillScreen(gfx->color565(0x00, 0x30, 0x00));

    gfx->setTextSize(4);
    gfx->print("AAAAAA");

    //gfx->setTextSize(3);
    //gfx->setTextColor(gfx->color565(0xff, 0x00, 0x00));
    //gfx->print(F("RED "));
    //gfx->setTextColor(gfx->color565(0x00, 0xff, 0x00));
    //gfx->print(F("GREEN "));
    //gfx->setTextColor(gfx->color565(0x00, 0x00, 0xff));
    //gfx->println(F("BLUE"));
}
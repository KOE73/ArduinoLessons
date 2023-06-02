#include <Arduino.h>

#include "arduinoFFT.h"
#include "Arduino_GFX_Library.h"
#include "AudioTools.h" // Для микрофона

#include "pin_config.h"
#include "gfx_config.h"
// #include "m.h"

arduinoFFT FFT;
/*
These values can be changed in order to evaluate the functions
*/
const uint16_t samples = 64; // This value MUST ALWAYS be a power of 2
const double signalFrequency = 1000;
const double samplingFrequency = 5000;
const uint8_t amplitude = 100;
/*
These are the input and output vectors
Input vectors receive computed results from FFT
*/
double vReal[samples];
double vImag[samples];

#define SCL_INDEX1 0x00
#define SCL_INDEX2 0x01
#define SCL_TIME1 0x02
#define SCL_TIME2 0x03
#define SCL_FREQUENCY 0x04
#define SCL_PLOT 0x05

I2SStream in;
CallbackStream out;
DynamicMemoryStream out2(true);
StreamCopy copier(out, in);     // copies sound into i2s
StreamCopy copier2(out2, in); // copies sound into i2s

uint16_t sample_rate = 44100;
uint16_t channels = 1;         // 2
uint16_t bits_per_sample = 24; // 16; // or try with 24 or 32

void PrintVector(double *vData, uint16_t bufferSize, uint8_t scaleType);
void DrawVector(double *vData, uint16_t bufferSize, uint8_t scaleType);
void DrawVector(uint8_t *vData, uint16_t bufferSize);

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println("Ready");

  ScreenInit();

  // msetup();
  AudioLogger::instance().begin(Serial, AudioLogger::Info);

  Serial.println("starting I2S...");
  auto config_in = in.defaultConfig(RX_MODE);
  config_in.sample_rate = sample_rate;
  config_in.bits_per_sample = bits_per_sample;
  config_in.i2s_format = I2S_MSB_FORMAT; // I2S_STD_FORMAT;
  config_in.auto_clear = true;
  config_in.buffer_size = 1024;
  config_in.is_master = true;
  config_in.port_no = 0;
  config_in.pin_bck = 44;  // 14;
  config_in.pin_ws = 43;   // 15;
  config_in.pin_data = 21; // 22;
  // config_in.fixed_mclk = sample_rate * 256
  // config_in.pin_mck = 2
  in.begin(config_in);

  

  // auto config_btout= btout.defaultConfig(TX_MODE);
  // config_btout.bufferSize=1024;
  ////config_btout. startLogic = ;
  // btout.begin(config_btout);
}

size_t reader(uint8_t *data, size_t len)
{
  Serial.print("callb len");
  Serial.println(len);
  return len;
}

uint8_t buf[1024];

void loop()
{
  DrawVector(buf, 100);

  Serial.print("Avail ");
  Serial.println(in.available());

  out.setReadCallback(reader);
  int d = 0;
  while (1)
  {
    copier2.copy();

    //Serial.print(  out2.readString());

    //uint8_t size = out2.readBytes(buf, 100);
    //Serial.println(size);
    //if (size < 100)
    //  continue;

    

    //DrawVector(buf, 100);

    // in.flush();
    // copier.copy();
    vTaskDelay(10 / portTICK_PERIOD_MS);
    // d++;
    // while (d == 10)
    //   ;
  }

  /* Build raw data */
  double cycles = (((samples - 1) * signalFrequency) / samplingFrequency); // Number of signal cycles that the sampling will read
  double cycles2 = (((samples - 1) * 1953) / samplingFrequency);           // Number of signal cycles that the sampling will read
  for (uint16_t i = 0; i < samples; i++)
  {
    vReal[i] = int8_t((amplitude * (sin((i * (twoPi * cycles)) / samples))) / 2.0);       /* Build data with positive and negative values*/
    vReal[i] += int8_t((amplitude / 2 * (sin((i * (twoPi * cycles2)) / samples))) / 2.0); /* Build data with positive and negative values*/
    // vReal[i] = uint8_t((amplitude * (sin((i * (twoPi * cycles)) / samples) + 1.0)) / 2.0);/* Build data displaced on the Y axis to include only positive values*/
    vImag[i] = 0.0; // Imaginary part must be zeroed in case of looping to avoid wrong calculations and overflows
  }

  FFT = arduinoFFT(vReal, vImag, samples, samplingFrequency); /* Create FFT object */
  /* Print the results of the simulated sampling according to time */
  Serial.println("Data:");
  PrintVector(vReal, samples, SCL_TIME1);

  FFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD); /* Weigh data */
  Serial.println("Weighed data:");
  PrintVector(vReal, samples, SCL_TIME2);

  FFT.Compute(FFT_FORWARD); /* Compute FFT */
  Serial.println("Computed Real values:");
  PrintVector(vReal, samples, SCL_INDEX1);
  Serial.println("Computed Imaginary values:");
  PrintVector(vImag, samples, SCL_INDEX2);

  FFT.ComplexToMagnitude(); /* Compute magnitudes */
  Serial.println("Computed magnitudes:");
  PrintVector(vReal, (samples >> 1), SCL_FREQUENCY);

  double x = FFT.MajorPeak();
  Serial.println(x, 6);
  while (1)
    ; /* Run Once */
  // delay(2000); /* Repeat after delay */
}

void PrintVector(double *vData, uint16_t bufferSize, uint8_t scaleType)
{
  DrawVector(vData, bufferSize, scaleType);

  for (uint16_t i = 0; i < bufferSize; i++)
  {
    double abscissa;
    /* Print abscissa value */
    switch (scaleType)
    {
    case SCL_INDEX1:
    case SCL_INDEX2:
      abscissa = (i * 1.0);
      break;
    case SCL_TIME1:
    case SCL_TIME2:
      abscissa = ((i * 1.0) / samplingFrequency);
      break;
    case SCL_FREQUENCY:
      abscissa = ((i * 1.0 * samplingFrequency) / samples);
      break;
    }
    Serial.print(abscissa, 6);
    if (scaleType == SCL_FREQUENCY)
      Serial.print("Hz");
    Serial.print(" ");
    Serial.println(vData[i], 4);
  }
  Serial.println();
}

template <class X, class M, class N, class O, class Q>
X map_Generic(X x, M in_min, N in_max, O out_min, Q out_max)
{
  O deltaIn = in_max - in_min;
  if (deltaIn == 0)
    return x + out_min;

  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void DrawVector(double *vData, uint16_t bufferSize, uint8_t scaleType)
{
  uint16_t color = gfx->color565(255, 255, 0);
  uint16_t colorX = gfx->color565(120, 120, 120);

  int yMin, yMax;

  switch (scaleType)
  {
  case SCL_INDEX1:
    color = gfx->color565(255, 128, 128);
    yMax = ScreenH - 30;
    yMin = ScreenH - 0;
    break;
  case SCL_INDEX2:
    color = gfx->color565(255, 128, 128);
    yMax = ScreenH - 60;
    yMin = ScreenH - 30;
    break;
  case SCL_TIME1:
    color = gfx->color565(128, 255, 128);
    yMax = ScreenH - 90;
    yMin = ScreenH - 60;
    break;
  case SCL_TIME2:
    color = gfx->color565(128, 255, 128);
    yMax = ScreenH - 120;
    yMin = ScreenH - 90;
    break;
  case SCL_FREQUENCY:
    color = gfx->color565(0, 255, 255);
    yMax = ScreenH - 160;
    yMin = ScreenH - 120;
    break;
  }

  double minVal = 0, maxVal = 0;

  // std::ranges::min_element(v)

  for (uint16_t i = 0; i < bufferSize; i++)
  {
    minVal = _min(minVal, vData[i]);
    maxVal = _max(maxVal, vData[i]);
  }

  Serial.print("Count-Min-Max");
  Serial.print(bufferSize);
  Serial.print(" ");
  Serial.print(minVal, 6);
  Serial.print(" ");
  Serial.println(maxVal, 6);
  Serial.println(" ");

  int x0 = -1, y0 = -1;

  for (uint16_t i = 0; i < bufferSize; i++)
  {
    double abscissa;

    int x = map_Generic(i, 0, bufferSize, 10, ScreenW - 10);
    int y = map_Generic(vData[i], minVal, maxVal, yMin, yMax);

    // Serial.print("x v=");
    // Serial.print(x);
    // Serial.print(" ");
    // Serial.print(y);
    // Serial.print(" ");
    // Serial.println(vData[i], 4);

    gfx->drawLine(x, yMin, x, yMax, colorX);

    if (x0 >= 0)
      gfx->drawLine(x0, y0, x, y, color);

    x0 = x;
    y0 = y;
  }
}

void DrawVector(uint8_t *vData, uint16_t bufferSize)
{
  gfx->fillScreen(gfx->color565(0x00, 0x30, 0x00));

  uint16_t color = gfx->color565(255, 255, 0);
  int yMin = 0, yMax = 100;
  yMax = ScreenH - 150;
  yMin = ScreenH - 30;

  uint8_t minVal = 0, maxVal = 0;
  for (uint16_t i = 0; i < bufferSize; i++)
  {
    minVal = _min(minVal, vData[i]);
    maxVal = _max(maxVal, vData[i]);
  }

  int x0 = -1, y0 = -1;

  for (uint16_t i = 0; i < bufferSize; i++)
  {
    double abscissa;

    int x = map_Generic(i, 0, bufferSize, 10, ScreenW - 10);
    int y = map_Generic(vData[i], minVal, maxVal, yMin, yMax);

    //Serial.print("x v=");
    //Serial.print(x);
    //Serial.print(" ");
    //Serial.print(y);
    //Serial.print(" ");
    //Serial.println(vData[i], 4);

    gfx->drawLine(x, yMin, x, yMax, color);

    if (x0 >= 0)
      gfx->drawLine(x0, y0, x, y, color);

    x0 = x;
    y0 = y;
  }
}
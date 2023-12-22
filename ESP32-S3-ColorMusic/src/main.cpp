#include <Arduino.h>

#include "Arduino_GFX_Library.h"
#include "AudioTools.h"             // Для микрофона
#include "AudioToolsSPH0645.h"      // Для микрофона
#include "AudioLibs/AudioRealFFT.h" // or any other supported inplementation

#include "pin_config.h"
#include "gfx_config.h"
#include "graphlib.h"
// #include "m.h"

// #include "arduinoFFT.h"
// arduinoFFT FFT;

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

uint16_t sample_rate = 44100;   // 44100/4; 62500
uint16_t channels = 1;          // 2
uint16_t bits_per_sample = 32;  // 16; // or try with 24 or 32
uint16_t buffer_size = 512 * 2; // 16; // or try with 24 or 32

AudioInfo info(sample_rate, channels, bits_per_sample);
SineWaveGenerator<int16_t> sineWave(32000);    // subclass of SoundGenerator with max amplitude of 32000
GeneratedSoundStream<int16_t> sound(sineWave); // Stream generated from sine wave
CsvOutput<int32_t> outSound(Serial, channels, buffer_size);
StreamCopy copierSound(outSound, sound); // copies sound to out
void InitSin()
{
  // outSound.begin(info.channels);            // Define CSV Output
  sineWave.begin(info, N_B4); // Setup sine wave
}

I2SStream inMicrophone;
CallbackStream outCallbackStream;
// DynamicMemoryStream out2(true);
StreamCopy copyToCallbackStream(outCallbackStream, inMicrophone); // copies sound into i2s

CsvOutput<int32_t> outSerial(Serial, 1, buffer_size);
StreamCopy copierToSerial(outSerial /*outSound*/, inMicrophone);

AudioRealFFT fft;
StreamCopy copierToFFT(fft, inMicrophone);

MicrophoneSPH0645Reducer<int32_t> XXX;

void PrintVector(double *vData, uint16_t bufferSize, uint8_t scaleType);
void DrawVector(double *vData, uint16_t bufferSize, uint8_t scaleType);
template <class D>
void DrawVector(D *vData, uint16_t bufferSize);
template <class T>
void DrawVector1(const uint8_t *src, uint16_t byte_count, int x, int y, int w, int h, uint16_t color);
template <class T>
void DrawVector2(const uint8_t *src, uint16_t byte_count, int x, int y, int w, int h, uint16_t color);
void fftResult(AudioFFTBase &fft);

void setup()
{
  // Serial.begin(115200);
  Serial.begin(921600);
  while (!Serial)
    ;
  Serial.println("Ready");

  ScreenInit();

  // msetup();
  // AudioLogger::instance().begin(Serial, AudioLogger::Info);
  AudioLogger::instance().begin(Serial, AudioLogger::Error);

  // gfx->setTextColor(gfx->color565(0x00, 0xF0, 0x80));
  // gfx->setCursor(80,80);
  // gfx->print("1");
  // delay(250);

  Serial.println("starting I2S...");
  auto config_in = inMicrophone.defaultConfig(RX_MODE);
  // config_in.rx_tx_mode = RX_MODE;
  config_in.channels = channels;
  config_in.sample_rate = sample_rate;
  config_in.bits_per_sample = bits_per_sample; // I2S_CHANNEL_FMT_RIGHT_LEFT I2S_COMM_FORMAT_I2S  I2S_COMM_FORMAT_I2S_MSB
  config_in.i2s_format = I2S_MSB_FORMAT;       // I2S_PHILIPS_FORMAT;//I2S_PHILIPS_FORMAT; // I2S_MSB_FORMAT; // I2S_STD_FORMAT;
  // config_in.auto_clear = true;
  config_in.buffer_size = buffer_size;
  config_in.is_master = true;
  config_in.port_no = 0;
  config_in.pin_bck = 44;  // 14;
  config_in.pin_ws = 43;   // 15;
  config_in.pin_data = 21; // 22;
  // config_in.pin_data_rx = 21; // 22;
  config_in.use_apll = false; // try with yes

  // нельзя тут! внутри сразу запускает обмен, а он еще не настроенн inMicrophone.setAudioInfo(info);
  outCallbackStream.setAudioInfo(info);

  // config_in.fixed_mclk = sample_rate * 256
  // config_in.pin_mck = 2
  inMicrophone.begin(config_in);
  // REG_SET_BIT(I2S_RX_CONF1_REG(0), I2S_RX_MSB_SHIFT);
  // REG_SET_BIT(I2S_RX_CONF1_REG(1), I2S_RX_MSB_SHIFT);

  InitSin();
  Serial.println("Start Microphone");
  // Включение микрофона
  pinMode(21, INPUT_PULLDOWN);
  pinMode(16, OUTPUT);
  digitalWrite(16, HIGH);

  // Setup FFT
  auto tcfg = fft.defaultConfig();
  tcfg.length = buffer_size; // 4096;
  tcfg.channels = channels;
  tcfg.sample_rate = sample_rate;
  tcfg.bits_per_sample = bits_per_sample;
  tcfg.callback = &fftResult;
  fft.begin(tcfg);

  // auto config_btout= btout.defaultConfig(TX_MODE);
  // config_btout.bufferSize=1024;
  ////config_btout. startLogic = ;
  // btout.begin(config_btout);
}

size_t callbackReader(uint8_t *data, size_t len)
{
  Serial.print("Reader len");
  Serial.println(len);
  return len;
}

Vector<uint8_t> tmp;

size_t callbackWriter(const uint8_t *data, size_t len)
{
  // Serial.print(".");
  // Serial.print(len);
  // Serial.print("Writer len ");
  // Serial.println(len);
  if (tmp.size() != len)
    tmp.resize(len);

  DrawVector1<int32_t>(tmp.data(), (uint16_t)tmp.capacity(), 10, ScreenH - ScreenH / 2 - 5, ScreenW - 20, ScreenH / 2 - 10, 0);

  uint16_t color = gfx->color565(255, 255, 0);
  DrawVector1<int32_t>(data, (uint16_t)len, 10, ScreenH - ScreenH / 2 - 5, ScreenW - 20, ScreenH / 2 - 10, color);

  memcpy((void *)tmp.data(), (void *)data, len);

  return len;
}

void printAudioInfo(const char *text, AudioInfo info)
{
  char str[100];
  sprintf(str, "%s sample_rate: %d  channels: %d  bits_per_sample: %d", text, info.sample_rate, info.channels, info.bits_per_sample);
  Serial.println(str);
}

Vector<uint32_t> fm; // freguence magnitude

// display fft result
void fftResult(AudioFFTBase &fft)
{
  int bins = fft.size();
  if (fm.size() != bins)
    fm.resize(bins);

  DrawVector2<int32_t>((uint8_t *)fm.data(), (uint16_t)fm.capacity(), 10, ScreenH - 5, ScreenW - 20, ScreenH / 2 - 5, 0);

  for (int i = 0; i < bins; i++)
    fm[i] = (100. * 20. * log10(fft.magnitude(i))) - 7700.;
  fm[0] = 0;

  DrawVector2<int32_t>((uint8_t *)fm.data(), (uint16_t)fm.capacity(), 10, ScreenH - 5, ScreenW - 20, ScreenH / 2 - 5, gfx->color565(0, 128, 255));

  // for (int i = 1; i < 10; i++)
  //{
  //   Serial.print(fft.frequency(i));
  //   Serial.print(" ");
  //   Serial.print(fft.magnitude(i));
  //   Serial.print(" ");
  //   Serial.println(fm[i]);
  // }

  // float diff;
  // auto result = fft.result();
  // if (result.magnitude > 100)
  //{
  //   Serial.print(result.frequency);
  //   Serial.print(" ");
  //   Serial.print(result.magnitude);
  //   Serial.print(" => ");
  //   Serial.print(result.frequencyAsNote(diff));
  //   Serial.print(" diff: ");
  //   Serial.println(diff);
  // }
}

uint8_t buf[1024];

void loop()
{
  uint32_t *sample = (uint32_t *)buf;
  int size = sizeof(buf) / sizeof(uint32_t);
  for (uint16_t i = 0; i < size; i++)
  {
    sample[i] = sin(i / 25.) * 1000.;
  }

  DrawVector1<int32_t>(buf, sizeof(buf), 10, ScreenH - 10, ScreenW - 20, ScreenH - 20, gfx->color565(255, 0, 0));

  delay(500);

  outCallbackStream.setReadCallback(callbackReader);
  outCallbackStream.setWriteCallback(callbackWriter);

  printAudioInfo("inMicrophone", inMicrophone.audioInfo());
  printAudioInfo("outCallbackStream", outCallbackStream.audioInfo());

  int d = 0;
  while (1)
  {
    // int aval = in.available();
    //Serial.print("Avail ");
    // Serial.println(aval);

    // while (aval > 100)
    //   ;
    copyToCallbackStream.copy(XXX);
    copierToFFT.copy(XXX);
    // copierToSerial.copy(XXX);

    // copierSound.copy();
    //  Serial.print(  out2.readString());

    // uint8_t size = out2.readBytes(buf, 100);
    // Serial.println(size);
    // if (size < 100)
    //   continue;

    // DrawVector(buf, 100);

    // in.flush();
    // copier.copy();
    // vTaskDelay(10 / portTICK_PERIOD_MS);
    // d++;
    // while (d == 2)
    //  ;
  }
}

void oldMain()
{
  //  /* Build raw data */
  //  double cycles = (((samples - 1) * signalFrequency) / samplingFrequency); // Number of signal cycles that the sampling will read
  //  double cycles2 = (((samples - 1) * 1953) / samplingFrequency);           // Number of signal cycles that the sampling will read
  //  for (uint16_t i = 0; i < samples; i++)
  //  {
  //    vReal[i] = int8_t((amplitude * (sin((i * (twoPi * cycles)) / samples))) / 2.0);       /* Build data with positive and negative values*/
  //    vReal[i] += int8_t((amplitude / 2 * (sin((i * (twoPi * cycles2)) / samples))) / 2.0); /* Build data with positive and negative values*/
  //    // vReal[i] = uint8_t((amplitude * (sin((i * (twoPi * cycles)) / samples) + 1.0)) / 2.0);/* Build data displaced on the Y axis to include only positive values*/
  //    vImag[i] = 0.0; // Imaginary part must be zeroed in case of looping to avoid wrong calculations and overflows
  //  }
  //
  //  FFT = arduinoFFT(vReal, vImag, samples, samplingFrequency); /* Create FFT object */
  //  /* Print the results of the simulated sampling according to time */
  //  Serial.println("Data:");
  //  PrintVector(vReal, samples, SCL_TIME1);
  //
  //  FFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD); /* Weigh data */
  //  Serial.println("Weighed data:");
  //  PrintVector(vReal, samples, SCL_TIME2);
  //
  //  FFT.Compute(FFT_FORWARD); /* Compute FFT */
  //  Serial.println("Computed Real values:");
  //  PrintVector(vReal, samples, SCL_INDEX1);
  //  Serial.println("Computed Imaginary values:");
  //  PrintVector(vImag, samples, SCL_INDEX2);
  //
  //  FFT.ComplexToMagnitude(); /* Compute magnitudes */
  //  Serial.println("Computed magnitudes:");
  //  PrintVector(vReal, (samples >> 1), SCL_FREQUENCY);
  //
  //  double x = FFT.MajorPeak();
  //  Serial.println(x, 6);
  //  while (1)
  //    ; /* Run Once */
  //  // delay(2000); /* Repeat after delay */
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

template <class D>
void DrawVector(D *vData, uint16_t bufferSize)
{
  gfx->fillScreen(gfx->color565(0x00, 0x30, 0x00));

  uint16_t color = gfx->color565(255, 255, 0);
  int yMin = 0, yMax = 100;
  yMax = ScreenH - 150;
  yMin = ScreenH - 30;

  D minVal = 0, maxVal = 0;
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

    // Serial.print("x v=");
    // Serial.print(x);
    // Serial.print(" ");
    // Serial.print(y);
    // Serial.print(" ");
    // Serial.println(vData[i], 4);

    gfx->drawLine(x, yMin, x, yMax, color);

    if (x0 >= 0)
      gfx->drawLine(x0, y0, x, y, color);

    x0 = x;
    y0 = y;
  }
}

template <class T>
void DrawVector1(const uint8_t *src, uint16_t byte_count, int x, int y, int w, int h, uint16_t color)
{
  T *sample = (T *)src;
  int size = byte_count / sizeof(T);

  int yMin = y, yMax = y - h;
  int xMin = x, xMax = x + w;

  T minVal = 0, maxVal = 0;
  for (uint16_t i = 0; i < size; i++)
  {
    minVal = _min(minVal, sample[i]);
    maxVal = _max(maxVal, sample[i]);
  }

  int x0 = -1, y0 = -1;

  for (uint16_t i = 0; i < size; i++)
  {
    int x = map_Generic(i, 0, size, xMin, xMax);
    int y = map_Generic(sample[i], minVal, maxVal, yMin, yMax);

    // Serial.print("x,y=");
    // Serial.print(x);
    // Serial.print(",");
    // Serial.print(y);
    // Serial.print(" ");
    // Serial.println(sample[i]);

    // gfx->drawLine(x, yMin, x, yMax, color);

    if (x0 >= 0)
      gfx->drawLine(x0, y0, x, y, color);

    x0 = x;
    y0 = y;
  }
}

template <class T>
void DrawVector2(const uint8_t *src, uint16_t byte_count, int x, int y, int w, int h, uint16_t color)
{
  T *sample = (T *)src;
  int size = byte_count / sizeof(T);

  int yMin = y, yMax = y - h;
  int xMin = x, xMax = x + w;

  T minVal = 0, maxVal = 2000;
  // for (uint16_t i = 0; i < size; i++)
  //{
  //   minVal = _min(minVal, sample[i]);
  //   maxVal = _max(maxVal, sample[i]);
  // }
  int xWnd = 2;
  int xSize = size / xWnd;
  for (uint16_t i = 0; i < xSize; i += 2)
  {
    int x = map_Generic(i, 0, xSize, xMin, xMax);
    // int y = map_Generic(sample[i], minVal, maxVal, yMin, yMax);
    // gfx->drawLine(x, yMin, x, y, color);

    int val = 0;
    for (int w = 0; w < xWnd; w++)
      val += sample[i + w];

    val /= xWnd;

    int y = map_Generic(sample[i], minVal, maxVal, 0, h);
    gfx->fillRect(x, yMin, 3, -y, color);

    // Serial.print("x,y=");
    // Serial.print(x);
    // Serial.print(",");
    // Serial.print(y);
    // Serial.print(" ");
    // Serial.println(sample[i]);
  }
}
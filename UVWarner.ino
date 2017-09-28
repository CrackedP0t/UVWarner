#include "FastLED.h"

#include "Adafruit_LEDBackpack.h"

#include <VEML6075.h>

#define MAX(a,b) \
  ({ __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b; })

#define MIN(a,b) \
  ({ __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b; })

#define CLAMP(l, n, u) \
  (MAX(l, MIN(n, u)))

#define NP_PIN 12
#define NP_NUM 8

#define UVI_MAX 11

//#define UVI_NUANCE
//#define UVI_GRADIENT

//#define UVI_CYCLE

#if defined(UVI_CYCLE)
#define CYCLE_MIN 0.0
#define CYCLE_MAX 12.0
#define CYCLE_DELAY 1
#endif

#if defined(UVI_NUANCE)
const CRGB uvi_colors[12] = {
  0x00ff00,
  0x4eb400,
  0xa0ce00,
  0xf7e400,
  0xf8b600,
  0xf88700,
  0xf85900,
  0xe82c0e,
  0xd8001d,
  0xff0099,
  0xb54cff,
  0x998cff
};
#else
const CRGB uvi_colors[12] = {
  CRGB::Green,
  CRGB::Green,
  CRGB::Green,
  CRGB::Yellow,
  CRGB::Yellow,
  CRGB::Yellow,
  CRGB::Orange,
  CRGB::Orange,
  CRGB::Red,
  CRGB::Red,
  CRGB::Red,
  CRGB::Purple
};
#endif

void dispString(Adafruit_AlphaNum4 *disp, int8_t off, char *str) {
  uint8_t dotCount = 0;
  size_t len = strlen(str);

  for (uint8_t digit = 0; digit < 4; digit++) {
    int8_t index = digit - off + dotCount;

    if (index >= 0 && index < len) {
      char c = str[index];

      if (c == '.') {
        if (index == 0 || str[index - 1] == '.') {
          disp->writeDigitAscii(digit, ' ', true);
        } else {
          dotCount++;
          digit--;
        }
      } else {
        disp->writeDigitAscii(digit, c, str[index + 1] == '.');
      }
    }
  }

  disp->writeDisplay();
}

uint8_t tween(uint8_t a, uint8_t b, float p) {
  if (a == b) {
    return a;
  } else if (a < b) {
    return a + (uint8_t)((b - a) * p);
  } else {
    return a - (uint8_t)((a - b) * p);
  }
}

CRGB crgb_tween(CRGB a, CRGB b, float p) {
  return CRGB(tween(a.r, b.r, p), tween(a.g, b.g, p), tween(a.b, b.b, p));
}

VEML6075 veml6075 = VEML6075();
bool found = false;

Adafruit_AlphaNum4 alpha4 = Adafruit_AlphaNum4();

CRGB nps[NP_NUM];

void setup() {
  if (!veml6075.begin()) {
    found = false;
  } else {
    found = true;
  }

  alpha4.begin(0x70);

  FastLED.addLeds<NEOPIXEL, NP_PIN>(nps, NP_NUM);

  Serial.begin(9600);
}

void crgb_print(CRGB color) {
  Serial.print("R: "); Serial.print(color.r); Serial.print("; G: "); Serial.print(color.g); Serial.print("; B: "); Serial.print(color.b); Serial.println(";");
}

void loop() {
  float uv_index = 0.0;

#if defined(UVI_CYCLE)
  boolean inverse = false;
  while (true) {
    if (inverse) {
      uv_index -= 0.01;
      if (uv_index < CYCLE_MIN) {
        inverse = false;
        uv_index = CYCLE_MIN + 0.01;
      }
    } else {
      uv_index += 0.01;
      if (uv_index > CYCLE_MAX) {
        inverse = true;
        uv_index = CYCLE_MAX - 0.01;
      }
    }
#else
  if (found) {
    do {
      veml6075.poll();

      uv_index = veml6075.getUVIndex();

    } while (uv_index > 20.0);

    uv_index = MAX(0.0, uv_index);
    uv_index = MIN(uv_index, 11.0);
#endif

    char uv_str[20];

    dtostrf(uv_index, 4, 3, uv_str);

    dispString(&alpha4, 0, uv_str);

    size_t colori = (size_t)uv_index;

#if defined(UVI_GRADIENT)
    CRGB lcolor = uvi_colors[colori];
    CRGB hcolor = uvi_colors[MIN(colori + 1, UVI_MAX)];

    float fpart = uv_index - (uint8_t)uv_index;

    CRGB color = crgb_tween(lcolor, hcolor, fpart);
#else
    CRGB color = uvi_colors[colori];
#endif

    fill_solid(nps, NP_NUM, color);

    FastLED.show();

#if defined(UVI_CYCLE)
    delay(CYCLE_DELAY);
  }
#else
  } else {
    dispString(&alpha4, 0, "Err");
  }
#endif
}

//#define DEBUG_PRINT

//#define UVI_CYCLE

#define DISP_SCREEN
#define DISP_NP

// End config

#define FASTLED_NO_ANNOY

#include <Arduino.h>

#include "util.h"

#define UVI_STAGE_COUNT 5

const uint8_t uvi_stages[UVI_STAGE_COUNT] = {0, 3, 6, 8, 11};

const char *stage_advice[UVI_STAGE_COUNT] = {
	"SUNSCREEN: NO",
	"SUNSCREEN: MAYBE",
	"SUNSCREEN: YES",
	"SUNSCREEN: 100%",
	"AVOID THE SUN"
};

#if defined(UVI_CYCLE)
#include <FastLED.h>

#define CYCLE_MIN 0.0
#define CYCLE_MAX 12.0
#define CYCLE_DELAY 1
#define CYCLE_STEP 2

uint8_t wave_pos = 192;
#else // UVI_CYCLE
#include <VEML6075.h>

VEML6075 veml6075 = VEML6075();
bool veml6075_found = false;
#endif // UVI_CYCLE

#if defined(DISP_NP)
#include <FastLED.h>

#define NP_PIN 12
#define NP_COUNT 8

#define NP_DIM_PIN 0

const CRGB np_colors[UVI_STAGE_COUNT] = {
	CRGB::Green,
	CRGB::Yellow,
	CRGB::Orange,
	CRGB::Red,
	CRGB::Purple
};

CRGB nps[NP_COUNT];
#endif // DISP_NP

#if defined(DISP_SCREEN)
// Include these because of Arduino idiosyncracies
#include <SPI.h>
#include <Wire.h>

#include <U8g2lib.h>

#define SCREEN_W 128
#define SCREEN_H 64

#define U8G2_FONT_ASCENT 13

#define SCREEN_TYPE U8G2_SSD1306_128X64_NONAME_F_HW_I2C

#define SCREEN_UVI_KERN 4

// tf: 32-255; tr: 32-126; tn: 42-58
#define SCREEN_UVI_FONT u8g2_font_osb35_tn
#define SCREEN_ADVICE_FONT u8g2_font_ncenB08_tr

#define SCREEN_RESET_PIN 5

void u8g2PrintCenter(SCREEN_TYPE *disp, u8g2_uint_t x, u8g2_uint_t y, const char *str) {
	u8g2_uint_t width = disp->getStrWidth(str);

	disp->drawStr(x - width / 2, y, str);
}

SCREEN_TYPE screen(U8G2_R0, SCREEN_RESET_PIN);
#endif // DISP_SCREEN

void setup() {
#if !defined(UVI_CYCLE)
	if (!veml6075.begin()) {
		veml6075_found = false;
	} else {
		veml6075_found = true;
	}
#endif // !UVI_CYCLE

#if defined(DISP_SCREEN)
	screen.setI2CAddress(0x7a);
	screen.begin();

	screen.clearBuffer();
#endif // DISP_SCREEN

#if defined(DISP_NP)
    FastLED.setCorrection(CRGB(127, 0, 0));

	FastLED.addLeds<NEOPIXEL, NP_PIN>(nps, NP_COUNT);
#endif // DISP_NP

#if defined(DEBUG_PRINT)
	Serial.begin(9600);
#endif // DEBUG_PRINT
}

float uv_index = 0.0;

void loop() {
#if defined(UVI_CYCLE)
	uv_index = CYCLE_MAX * (float)sin8(wave_pos) / 255.0;

	wave_pos += CYCLE_STEP;
#else // UVI_CYCLE
	if (veml6075_found) {
	do {
		veml6075.poll();

		uv_index = veml6075.getUVIndex();

	} while (uv_index > 20.0);

	uv_index = MAX(0.0, uv_index);
	uv_index = MIN(uv_index, 11.0);
#endif // UVI_CYCLE

	int8_t uvi_stagei = 0;

	for (uvi_stagei = UVI_STAGE_COUNT - 1; uvi_stagei >= 0; uvi_stagei--) {
		if ((float)uvi_stages[uvi_stagei] <= uv_index) {
			break;
		}
	}


#if defined(DISP_SCREEN)
	screen.clearBuffer();

	screen.setFont(SCREEN_UVI_FONT);
	screen.setFontPosTop();

	u8g2_uint_t uvi_x = SCREEN_W / 2;
	u8g2_uint_t uvi_y = 4;

	u8g2_uint_t point_width = screen.getStrWidth(".");

	u8g2PrintCenter(&screen, uvi_x, uvi_y, ".");

	char uv_istr[3];
	snprintf(uv_istr, 3, "%u", (uint8_t)uv_index);

	char uv_fstr[2];
	snprintf(uv_fstr, 2, "%u",
	         (uint8_t)((uv_index - (uint8_t)uv_index) * 10));

	screen.drawStr(uvi_x - point_width / 2
	             - screen.getStrWidth(uv_istr) - SCREEN_UVI_KERN,
	             uvi_y, uv_istr);
	screen.drawStr(uvi_x + point_width / 2 + SCREEN_UVI_KERN,
	             uvi_y, uv_fstr);

	screen.setFont(SCREEN_ADVICE_FONT);
	screen.setFontPosBottom();

	u8g2_uint_t advice_x = SCREEN_W / 2;
	u8g2_uint_t advice_y = SCREEN_H;

	u8g2PrintCenter(&screen, advice_x, advice_y, stage_advice[uvi_stagei]);

	screen.sendBuffer();
#endif // DISP_SCREEN

#if defined(DISP_NP)
	CRGB color = np_colors[uvi_stagei];

	uint16_t pot_reading = analogRead(NP_DIM_PIN);

	uint8_t np_dim = 0;

	if (pot_reading > 4) {
		np_dim = (pot_reading + 1) / 4 - 1;
	}

	color.nscale8(np_dim);

	fill_solid(nps, NP_COUNT, color);

	FastLED.show();
#endif // DISP_NP

#if defined(UVI_CYCLE)
	delay(CYCLE_DELAY);
#else // UVI_CYCLE
} else {
#if defined(DISP_SCREEN)
		u8g2PrintCenter(&screen, 0, 0, "Error");
#endif // DISP_SCREEN
	}
#endif // UVI_CYCLE
}

//#define DEBUG_PRINT

//#define UVI_CYCLE

#define USE_VEML6075

//#define DISP_14SEG
#define DISP_OLED
#define DISP_NP

// End config

#include <Arduino.h>
#include "util.h"

#define UVI_MAX 11

#if defined(DEBUG_PRINT)
#endif // DEBUG_PRINT

#if defined(UVI_CYCLE)
#undef USE_VEML6075

#define CYCLE_MIN 0.0
#define CYCLE_MAX 12.0
#define CYCLE_DELAY 1
#define CYCLE_STEP 2

#include <FastLED.h>
#endif // UVI_CYCLE

#if defined(DISP_NP)
#include <FastLED.h>

#define NP_PIN 12
#define NP_NUM 8
#define NP_BRIGHT 255

//#define NP_NUANCE
//#define NP_GRADIENT

#if defined(NP_NUANCE)
const CRGB np_colors[UVI_MAX + 1] = {
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
#else // NP_NUANCE
const CRGB np_colors[UVI_MAX + 1] = {
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
#endif // NP_NUANCE

CRGB crgb_tween(CRGB a, CRGB b, float p) {
	return CRGB(tween(a.r, b.r, p), tween(a.g, b.g, p), tween(a.b, b.b, p));
}

CRGB nps[NP_NUM];
#endif // DISP_NP

#if defined(DISP_14SEG)
#include <Adafruit_LEDBackpack.h>
void _14segDispString(Adafruit_AlphaNum4 *disp, int8_t off, char *str) {
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

Adafruit_AlphaNum4 alpha4 = Adafruit_AlphaNum4();
#endif // DISP_14SEG

#if defined(DISP_OLED)
#define OLED_W 128
#define OLED_H 64

// Include these because of Arduino idiosyncracies
#include <SPI.h>
#include <Wire.h>

#include <U8g2lib.h>

#define U8G2_FONT_ASCENT 13

#define OLED_TYPE U8G2_SSD1306_128X64_NONAME_F_HW_I2C

#define OLED_UVI_KERN 4

#define OLED_UVI_FONT u8g2_font_osb35_tn
//#define OLED_UVI_FONT u8g2_font_inb33_mn

#define OLED_RESET 5

void u8g2PrintCenter(OLED_TYPE *disp, char * str,
                     u8g2_uint_t x, u8g2_uint_t y) {
	u8g2_uint_t width = disp->getStrWidth(str);

	disp->drawStr(x - width / 2, y, str);
}

OLED_TYPE oled(U8G2_R0, OLED_RESET);
#endif // DISP_OLED

#if defined(USE_VEML6075)
#include <VEML6075.h>

VEML6075 veml6075 = VEML6075();
bool veml6075_found = false;
#endif //  USE_VEML6075


#if defined(DEBUG_PRINT) && defined(DISP_NP)
void crgb_print(CRGB color) {
	Serial.print("R: "); Serial.print(color.r);
	Serial.print("; G: "); Serial.print(color.g);
	Serial.print("; B: "); Serial.print(color.b);
	Serial.println(";");
}
#endif // DEBUG_PRINT && DISP_NP

void setup() {
#if defined(USE_VEML6075)
	if (!veml6075.begin()) {
		veml6075_found = false;
	} else {
		veml6075_found = true;
	}
#endif // USE_VEML6075

#if defined(DISP_14SEG)
	alpha4.begin(0x70);
#endif // DISP_14SEG

#if defined(DISP_OLED)
	oled.setI2CAddress(0x7a);
	oled.begin();

	oled.clearBuffer();
	oled.setFont(OLED_UVI_FONT);

	oled.setFontPosCenter();
#endif // DISP_OLED

#if defined(DISP_NP)
	FastLED.addLeds<NEOPIXEL, NP_PIN>(nps, NP_NUM);
#endif // DISP_NP

#if defined(DEBUG_PRINT)
	Serial.begin(9600);
#endif // DEBUG_PRINT
}

#if defined(UVI_CYCLE)
uint8_t wave_pos = 192;
#endif // UVI_CYCLE

float uv_index = 0.0;

void loop() {
#if defined(UVI_CYCLE)
	uv_index = UVI_MAX * (float)sin8(wave_pos) / 255.0;

	wave_pos += CYCLE_STEP;
#elif defined(USE_VEML6075) // UVI_CYCLE
	if (veml6075_found) {
	do {
		veml6075.poll();

		uv_index = veml6075.getUVIndex();

	} while (uv_index > 20.0);

	uv_index = MAX(0.0, uv_index);
	uv_index = MIN(uv_index, 11.0);
#endif // UVI_CYCLE

#if defined(DISP_14SEG)
	char uv_str[20];

	dtostrf(uv_index, 4, 3, uv_str);

	_14segDispString(&alpha4, 0, uv_str);
#endif // DISP_14SEG

#if defined(DISP_OLED)
	oled.clearBuffer();

	u8g2_uint_t text_x = 64;
	u8g2_uint_t text_y = 32;

	u8g2_uint_t point_width = oled.getStrWidth(".");

	u8g2PrintCenter(&oled, ".", text_x, text_y);

	char uv_istr[3];
	snprintf(uv_istr, 3, "%u", (uint8_t)uv_index);

	char uv_fstr[2];
	snprintf(uv_fstr, 2, "%u",
	         (uint8_t)((uv_index - (uint8_t)uv_index) * 10));

	 oled.drawStr(text_x - point_width / 2
	              - oled.getStrWidth(uv_istr) - OLED_UVI_KERN,
	              text_y, uv_istr);
	 oled.drawStr(text_x + point_width / 2 + OLED_UVI_KERN, text_y, uv_fstr);

	oled.sendBuffer();
#endif

#if defined(DISP_NP)
	size_t colori = (size_t)uv_index;

#if defined(NP_GRADIENT)
	CRGB lcolor = np_colors[colori];
	CRGB hcolor = np_colors[MIN(colori + 1, UVI_MAX)];

	float fpart = uv_index - (uint8_t)uv_index;

	CRGB color = crgb_tween(lcolor, hcolor, fpart);
#else // NP_GRADIENT
	CRGB color = np_colors[colori];
#endif // NP_GRADIENT

	color.nscale8(NP_BRIGHT);

	fill_solid(nps, NP_NUM, color);

	FastLED.show();
#endif // DISP_NP

#if defined(UVI_CYCLE)
	delay(CYCLE_DELAY);
#else // UVI_CYCLE
} else {
#if defined(DISP_14SEG)
		_14segDispString(&alpha4, 0, "Err");
#endif // DISP_14SEG
	}
#endif // UVI_CYCLE
}

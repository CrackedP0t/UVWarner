//#define DEBUG_PRINT

//#define UVI_CYCLE

#define DISP_SCREEN

// End config
#include <math.h>

#include <Arduino.h>

#include "util.h"

#define UVI_PLACES 1

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
#define CYCLE_MIN 0.0
#define CYCLE_MAX 12.0
#define CYCLE_DELAY 25
#define CYCLE_STEP .05

double wave_pos = 0.0;
#else // UVI_CYCLE
#include <VEML6075.h>

VEML6075 veml6075 = VEML6075();
bool veml6075_found = false;
#endif // UVI_CYCLE

#if defined(DISP_SCREEN)
// Include these because of Arduino idiosyncracies
#include <SPI.h>
#include <Wire.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SharpMem.h>
#include <OldStandard_Bold28pt7b.h>

#define BLACK 0
#define WHITE 1

#define SCREEN_W 144
#define SCREEN_H 168

#define SCREEN_SCK 5
#define SCREEN_MOSI 6
#define SCREEN_SS 9

#define SCREEN_UVI_KERN 10
#define SCREEN_UVI_SCALE 1

constexpr auto uvi_font = &OldStandard_Bold28pt7b;

Adafruit_SharpMem screen(SCREEN_SCK, SCREEN_MOSI, SCREEN_SS,
                         SCREEN_W, SCREEN_H);
#endif // DISP_SCREEN

float uv_index = 0.0;
float last_uvi = -1.0;

void setup() {
#if !defined(UVI_CYCLE)
	if (!veml6075.begin()) {
		veml6075_found = false;
	} else {
		veml6075_found = true;
	}
#endif // !UVI_CYCLE

#if defined(DISP_SCREEN)
	screen.begin();
    screen.clearDisplay();

    screen.setTextColor(BLACK);
#endif // DISP_SCREEN

#if defined(DEBUG_PRINT)
	Serial.begin(9600);
#endif // DEBUG_PRINT
}

void loop() {
#if defined(UVI_CYCLE)
	uv_index = CYCLE_MAX * ((sin(wave_pos) + 1.0) / 2.0);

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

    float disp_index = floor(uv_index * powf(10.0, UVI_PLACES)) / powf(10.0, UVI_PLACES);

	int8_t uvi_stagei = 0;

	for (uvi_stagei = UVI_STAGE_COUNT - 1; uvi_stagei >= 0; uvi_stagei--) {
		if ((float)uvi_stages[uvi_stagei] <= uv_index) {
			break;
		}
	}


#if defined(DISP_SCREEN)
    if (disp_index != last_uvi) {
        screen.clearBuffer();

        screen.setFont(uvi_font);

        uint16_t uvi_x = SCREEN_W / 2;
        uint16_t uvi_y = SCREEN_H / 2;

        int16_t d_x, d_y;
        uint16_t d_w, d_h;

        screen.getTextBounds(".", uvi_x, uvi_y, &d_x, &d_y, &d_w, &d_h);

        screen.drawChar(uvi_x - d_w / 2, uvi_y, '.', BLACK, WHITE,
                        SCREEN_UVI_SCALE);

        char uv_istr[3];
        int uv_istr_n = snprintf(uv_istr, 3, "%u", (uint8_t)uv_index);

        char uv_fstr[2];
        snprintf(uv_fstr, 2, "%u",
                 (uint8_t)((uv_index - (uint8_t)uv_index) * 10));

        char c;

        int16_t c_x, c_y;
        uint16_t c_w, c_h;

        char s[2] = {'\0', '\0'};

        uint16_t total_width = 0;

        for (int i = uv_istr_n - 1; i >= 0; i--) {
            c = uv_istr[i];

            s[0] = c;

            screen.getTextBounds(s, 0, 0, &c_x, &c_y, &c_w, &c_h);

            total_width += c_w + SCREEN_UVI_KERN;

            screen.drawChar(d_x - d_w / 2 - total_width, uvi_y, c, BLACK, WHITE,
                            SCREEN_UVI_SCALE);
        }

        c = uv_fstr[0];

        screen.drawChar(d_x + d_w / 2 + SCREEN_UVI_KERN / 2, uvi_y, c, BLACK, WHITE,
                        SCREEN_UVI_SCALE);

        // screen.setFont(SCREEN_ADVICE_FONT);
        // screen.setFontPosBottom();

        // u8g2_uint_t advice_x = SCREEN_W / 2;
        // u8g2_uint_t advice_y = SCREEN_H;

        // u8g2PrintCenter(&screen, advice_x, advice_y, stage_advice[uvi_stagei]);

        screen.refresh();
    }

#endif // DISP_SCREEN
    last_uvi = disp_index;

#if defined(UVI_CYCLE)
    delay(CYCLE_DELAY);
#else // UVI_CYCLE
} else {
#if defined(DISP_SCREEN)
		screen.print("Error");
#endif // DISP_SCREEN
	}
#endif // UVI_CYCLE
}

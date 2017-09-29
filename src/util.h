#include <Arduino.h>

#define MAX(a, b)                                                       \
	({ __typeof__ (a) _a = (a); \
		__typeof__ (b) _b = (b); \
		_a > _b ? _a : _b; })

#define MIN(a, b)                                                       \
	({ __typeof__ (a) _a = (a); \
		__typeof__ (b) _b = (b); \
		_a < _b ? _a : _b; })

#define CLAMP(l, n, u)                                                  \
	(MAX(l, MIN(n, u)))

uint8_t tween(uint8_t a, uint8_t b, float p) {
	if (a == b) {
		return a;
	} else if (a < b) {
		return a + (uint8_t)((b - a) * p);
	} else {
		return a - (uint8_t)((a - b) * p);
	}
}

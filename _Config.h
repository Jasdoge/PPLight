#ifndef __neoPocketConf
#define __neoPocketConf

const uint32_t KEEPALIVE_DURATION = 30000; //150000;	// Use ON_DUR
const uint32_t FADE_TIME = 10000;			// This is appended to KEEPALIVE_DURATION
#define ON_DUR (KEEPALIVE_DURATION+FADE_TIME)

const uint16_t MAX_BRIGHTNESS = 200;

#define PIN_INTERRUPT 0			// Turns on on LOW
#define PIN_CHARGE_DET 1
#define PIN_LED 4

#endif

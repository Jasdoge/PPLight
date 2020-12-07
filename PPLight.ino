#include "_Config.h"
#include <avr/sleep.h>
#define overrideError
#include "Shibetek-KXTJ3.h"

// Variables
uint32_t wake = 0;				// Keepalive timer
uint32_t last_frame = 0;		// Last frame time (used for the animation ticker)
bool charging = false;
bool wokeByTimer = false;		// Tracks if it woke by timer. Used for reading battery charge status
uint32_t last_charge_check = 0;	// Limits how often to read the battery
bool sleepOnLoop = false;		// Used to prevent recursion

// Woke due to pin interrupt
ISR(PORTA_PORT_vect){
	uint8_t flags=PORTA.INTFLAGS;
  	PORTA.INTFLAGS=flags; //clear flags
}

ISR(RTC_PIT_vect){
  	RTC.PITINTFLAGS = RTC_PI_bm;          /* Clear interrupt flag by writing '1' (required) */
	wokeByTimer = true;
}

// Support functions
void setLEDs( uint16_t pwm = 0 ){
	analogWrite(PIN_LED, pwm);
};



// Turns the lights on or off
void toggle( bool on = false ){

	// Off
	if( !on ){

		wake = 0;
		setLEDs();
		sleep();
		return;

	}

	// On
	wake = millis();
	

}



void sleep(){
	
	PORTA.PIN6CTRL = 0b00000001;	// Trigger high
	sleep_cpu();

	// Just on to check if we're charging
	if( wokeByTimer ){
		
		wokeByTimer = false;
		checkCharging(true);
		
		// not plugged in, go back to sleep immediately
		if( !charging ){
			sleepOnLoop = true;
			return;
		}

	}

	// We're either charging or woke by a shake. In either case handle the sensor, otherwise it gets borked when you unplug the charger

	// woek up
	PORTA.PIN7CTRL = 0b00000000;	// Disable pin interrupt

	
	toggle(true);	// Woke, turn on

}

void checkCharging( bool force ){

	#ifdef DEBUG_IGNORE_BATTERY

	charging = false;

	#else

	const uint32_t ms = millis();
	if( !force && ms-last_charge_check < 1000 )
		return;
	
	last_charge_check = ms;

	charging = !digitalRead(PIN_CHARGE_DET);

	#endif

}




void setup(){


	/* Initialize RTC: */
	while (RTC.STATUS > 0){
		;                                   /* Wait for all register to be synchronized */
	}
	RTC.CLKSEL = RTC_CLKSEL_INT32K_gc;    /* 32.768kHz Internal Ultra-Low-Power Oscillator (OSCULP32K) */
	RTC.PITINTCTRL = RTC_PI_bm;           /* PIT Interrupt: enabled */
	RTC.PITCTRLA = RTC_PERIOD_CYC32768_gc /* RTC Clock Cycles 32k, 1Hz ( */
	| RTC_PITEN_bm;                       /* Enable PIT counter: enabled */

	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	sleep_enable();
	
	// The permanent output pins can be set here
	pinMode(PIN_LED, OUTPUT);
	digitalWrite(PIN_LED, HIGH);
	delay(100);
	digitalWrite(PIN_LED, LOW);

	// Interrupt ALWAYS needs to be an input
	pinMode(PIN_INTERRUPT, INPUT);
	pinMode(PIN_CHARGE_DET, INPUT_PULLUP);

	setLEDs(5);
	
	if( !Accelerometer::begin() ){

		for( uint8_t i = 0; i<10; ++i ){

			setLEDs(i%2 ? 5 : 100);
			delay(500);

		}
		digitalWrite(PIN_LED, LOW);
		while(true);

	}

	setLEDs(50);
	delay(100);
	setLEDs(0);

	// sensitivity, duration to trigger, duration to reset
	if( !Accelerometer::setInterrupt( 40, 10, 255 ) ){
		for( uint8_t i = 0; i<20; ++i ){
			setLEDs(i%2 ? 5 : 20);
			delay(100);
		}
		return;
	}

	for( uint8_t i =0; i < 5; ++i ){
		setLEDs(i*50);
		delay(200);
	}
	
	
	toggle();



}

void loop(){


	// This is a recursion breaker
	if( sleepOnLoop ){

		sleepOnLoop = false;
		toggle(false);
		return;

	}



	checkCharging(false);
	
	
	const uint32_t ms = millis();


	// While charging. Only animate charge, regular functionality is disabled
	if( charging ){
		
		const uint16_t ANIM_TIME = 5000;
		const uint16_t ANIM_MAX_BRIGHT = 50;
		const uint16_t ANIM_MIN_BRIGHT = 1;

		uint16_t curTime = ms%(ANIM_TIME*2);
		uint16_t brightness = 0;
		if( curTime <= ANIM_TIME ){

			bool down = curTime > ANIM_TIME/2;
			// Need to animate in and out
			if( down ){
				// Animate down
				curTime = curTime-ANIM_TIME/2;
			}
			
			brightness = map(curTime, 0, ANIM_TIME/2, ANIM_MIN_BRIGHT, ANIM_MAX_BRIGHT);
			if( down )
				brightness = ANIM_MAX_BRIGHT-brightness;

		}

		setLEDs(brightness);

		return;

	}

	// Not charging, do normal stuff
	
	// Animate
	if( ms-last_frame > 10 ){

		last_frame = ms;

		uint16_t brightness = ms%512;	// 2 blinks per second
		if( brightness >= 255 )
			brightness = 255-(brightness-255);

		// Time to fade
		if( ms-wake > KEEPALIVE_DURATION ){

			uint16_t fade = ms-wake-KEEPALIVE_DURATION;	// how far between 0 and FADE_TIME are we in MS?
			fade = map(fade, 0, FADE_TIME, 0, 255);
			if( fade > brightness )
				brightness = 0;
			else
				brightness -= fade;

		}

		setLEDs(brightness);


	}

	// We're still alive
	if( ms-wake < ON_DUR ){

		if( digitalRead(PIN_INTERRUPT) )
			wake = ms;
		
	}
	// Timer has expired, shut down
	else
		toggle();



}

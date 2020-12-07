#include <Wire.h>

namespace Accelerometer{

	const uint8_t XOUT_L = 0x06;
	const uint8_t XOUT_H = 0x07;
	const uint8_t YOUT_L = 0x08;
	const uint8_t YOUT_H = 0x09;
	const uint8_t ZOUT_L = 0x0A;
	const uint8_t ZOUT_H = 0x0B;
	
	const uint8_t DCST_RESP = 0x0C;
	const uint8_t WHO_AM_I = 0x0F;
	const uint8_t INT_SOURCE1 = 0x16;
	const uint8_t INT_SOURCE2 = 0x17;
	const uint8_t STATUS_REG = 0x18;
	const uint8_t INT_REL = 0x1A;
	const uint8_t CTRL_REG1 = 0x1B;
	const uint8_t CTRL_REG2 = 0x1D;
	const uint8_t INT_CTRL_REG1 = 0x1E;
	const uint8_t INT_CTRL_REG2 = 0x1F;
	const uint8_t DATA_CTRL_REG = 0x21;
	const uint8_t WAKEUP_COUNTER = 0x29;
	const uint8_t NA_COUNTER = 0x2A;
	const uint8_t SELF_TEST = 0x3A;
	const uint8_t WAKEUP_THRESHOLD_H = 0x6A;
	const uint8_t WAKEUP_THRESHOLD_L = 0x6B;

	const uint8_t SAMPLE_RATE_0H781 = 0b1000;	// 0.781 hz
	const uint8_t SAMPLE_RATE_1H563 = 0b1001;	// 1.563 hz
	const uint8_t SAMPLE_RATE_3H125 = 0b1010;	// 3.125 hz
	const uint8_t SAMPLE_RATE_6H25 = 0b1011;	// 6.25 hz
	const uint8_t SAMPLE_RATE_12H5 = 0b0010;	// 12.5 hz
	const uint8_t SAMPLE_RATE_25H = 0b0001;	// 25 hz
	const uint8_t SAMPLE_RATE_50H = 0b0010;	//  50 hz
	const uint8_t SAMPLE_RATE_100H = 0b0011;	// 100 hz
	const uint8_t SAMPLE_RATE_200H = 0b0100;	// 200 hz
	const uint8_t SAMPLE_RATE_400H = 0b0101;	// 400 hz
	const uint8_t SAMPLE_RATE_800H = 0b0110;	// 800 hz
	const uint8_t SAMPLE_RATE_1600H = 0b0111;	// 1600 hz

	const uint8_t RANGE_2G = 0b0;		// 2G
	const uint8_t RANGE_4G = 0b01000;		// 2G
	const uint8_t RANGE_8G = 0b010000;		// 2G
	const uint8_t RANGE_16G = 0b100;		// 2G
	
	const uint8_t ERR_READ_FAIL = 1;
	const uint8_t ERR_READ16_FAIL = 2;
	const uint8_t ERR_WRITE_FAIL = 3;
	const uint8_t ERR_STANDBY = 4;
	const uint8_t ERR_CONF_INTERRUPT = 5;
	const uint8_t ERR_WHOAMI_FAIL = 6;
	const uint8_t ERR_SAMPLERATE_FAIL = 7;
	const uint8_t ERR_LOWPOWER_FAIL = 8;
	
	void (*onError)(uint8_t) = NULL;
	#ifndef overrideError
		void handleError( uint8_t err ){

			for( uint8_t i = 0; i<err; ++i ){
				
				digitalWrite(PIN_LED, HIGH);
				delay(400);
				digitalWrite(PIN_LED, LOW);
				delay(100);

			}
			

			delay(1000);

		};
		onError = &handleError;
	#endif

	uint8_t DEVICE_ADDRESS = 0x0F;

	uint8_t readRegister( uint8_t address ){

		//Return value
		uint8_t result = 0;
		Wire.beginTransmission(DEVICE_ADDRESS);
		Wire.write(address);
		if( Wire.endTransmission() != 0 ){

			if( onError )
				(*onError)(ERR_READ_FAIL);
			return 0xFF;

		}
		Wire.requestFrom(DEVICE_ADDRESS, (uint8_t)1);	// Request 1 byte

		while ( Wire.available() )
			result = Wire.read(); 	 // receive a byte as a proper uint8_t


		return result;

	}

	int16_t readRegister16( uint8_t address, bool debug = true ){
	
		int16_t out = 0;


		Wire.beginTransmission(DEVICE_ADDRESS);
		address |= 0x80;	// Should turn auto increment on. Is this part of the i2c spec itself?
		Wire.write(address);
		if( Wire.endTransmission() != 0 ){
			(*onError)(ERR_READ16_FAIL);
			return 0;
		}
		

		Wire.requestFrom(DEVICE_ADDRESS, (uint8_t)2);	// Request 2 bytes
		uint8_t i = 0;
		while( Wire.available() && i < 2 ){

			const uint16_t value = Wire.read();
			
			out = out|(
				value << 8*i
			);
			
			++i;

		}


		return out;

	}

	bool writeRegister( uint8_t address, uint8_t data ){

		Wire.beginTransmission(DEVICE_ADDRESS);

		Wire.write(address);
		Wire.write(data);
		if( Wire.endTransmission() != 0 ){
			(*onError)(ERR_WRITE_FAIL);
			return false;
		}

		return true;

	}

	bool toggleStandby( bool enable = true ){

		// Get current settings
		uint8_t ctrl = readRegister(CTRL_REG1);
		const uint8_t BIT_STANDBY = 0b10000000;

		// Write 0 for standby mode
		if( enable )
			ctrl &= ~BIT_STANDBY;
		// 1 for awake mode
		else
			ctrl |= BIT_STANDBY;

		if( writeRegister(CTRL_REG1, ctrl) )
			return true;

		(*onError)(ERR_STANDBY);
		return false;

	}

	// Use threshold of 0 to turn off
	bool setInterrupt( uint16_t threshold, uint8_t moveDur, uint8_t naDur, bool polarity = HIGH ){

		if( !toggleStandby(true) ){	// Must be put into standby in order to program it
			return false;
		}

		readRegister(INT_REL);	// Clear any existing interrupt

		// Update the main ctrl register	
		uint8_t reg1 = readRegister(CTRL_REG1);
		if( threshold )
			reg1 |= 0b10;	// Enable WUFE (wakeup function)
		else
			reg1 &= ~0b10;	// Disable WUFE
		
		if( !writeRegister(CTRL_REG1, reg1) ){
			(*onError)(ERR_CONF_INTERRUPT);
			return false;
		}


		if( !threshold )
			return true;

		// Set wakeup frequency on ctrl_reg2 to 100hz
		if( !writeRegister(CTRL_REG2, 0b111) )
			return false;
		

		const uint8_t en = 0b00100000;
		const uint8_t pulse = 0b00001000;

		uint8_t intCtrlReg1 = en;	// Enable & latch (by not writing pulse)

		// Set interrupt pin HIGH on interrupt
		if( polarity )
			intCtrlReg1 |= 0b00010000;

	

		// write CTRL_REG_1
		if( !writeRegister(INT_CTRL_REG1, intCtrlReg1) ){
			(*onError)(ERR_CONF_INTERRUPT);
			return false;
		}

		// Enable all direction and unlatched mode
		if( !writeRegister(INT_CTRL_REG2, 0b10111111) ){
			(*onError)(ERR_CONF_INTERRUPT);
			return false;
		}
		
		
		// Write treshold (256/G)
		if( !writeRegister(WAKEUP_THRESHOLD_H, (uint8_t)(threshold >> 4)) ){
			(*onError)(ERR_CONF_INTERRUPT);
			return false;
		}
		
		if( !writeRegister(WAKEUP_THRESHOLD_L, (uint8_t)(threshold << 4)) ){
			(*onError)(ERR_CONF_INTERRUPT);
			return false;
		}
		

		// write wakeup counter (how long does it need to be held at this G before waking up), value is nr of OWUF cycles, so at 100Hz 100 here = 1 sec
		if( !writeRegister(WAKEUP_COUNTER, moveDur) ){
			(*onError)(ERR_CONF_INTERRUPT);
			return false;
		}
		

		// how long inactivity it has to go before another interrupt. Same duration scheme as above
		if( !writeRegister(NA_COUNTER, naDur) ){
			(*onError)(ERR_CONF_INTERRUPT);
			return false;
		}
		

		if( !toggleStandby(false) ){
			(*onError)(ERR_CONF_INTERRUPT);
			return false;
		}
		
		return true;

	}

	// accelRange = accelerometer range in Gs (2, 4, 8, 16)
	// sampleRate = sampleRate in hz ()
	bool begin( uint8_t accelRange = RANGE_8G, uint8_t sampleRate = SAMPLE_RATE_100H ){

		pinMode(SDA, INPUT_PULLUP);
		pinMode(SCL, INPUT_PULLUP);
		Wire.begin();

		delay(2);	// Needs to be higher if you intend to use high power mode. See datasheet.

		
		uint8_t whoami = readRegister(WHO_AM_I);
		
		// Try the other address
		if( whoami != 53 ){
			DEVICE_ADDRESS = 0x0E;
			whoami = readRegister(WHO_AM_I);
		}

		// Failed
		if( whoami != 53 ){
			if( onError )
				(*onError)(ERR_WHOAMI_FAIL);
			return false;
		}

		// Needs to be in standby mode to change settings
		if( !toggleStandby(true) )
			return false;

		// Put sample rate
		if( !writeRegister(DATA_CTRL_REG, sampleRate) ){
			(*onError)(ERR_SAMPLERATE_FAIL);
			return false;
		}

		// Put low power by writing bit 6 low, frequency by using a const, and 0x80 to exit standby mode
		uint8_t ctrlReg1 = accelRange|0x80;
		if( !writeRegister(CTRL_REG1, ctrlReg1) ){
			(*onError)(ERR_LOWPOWER_FAIL);
			return false;
		}

		return true;

	}




};






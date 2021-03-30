#pragma once
/*
	This file is part of Repetier-Firmware.

	Repetier-Firmware is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Repetier-Firmware is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Repetier-Firmware.  If not, see <http://www.gnu.org/licenses/>.

	This firmware is a nearly complete rewrite of the sprinter firmware
	by kliment (https://github.com/kliment/Sprinter)
	which based on Tonokip RepRap firmware rewrite based off of Hydra-mmm firmware.

  Functions in this file are used to communicate using ascii or repetier protocol.
*/

#ifndef HAL_H
#define HAL_H
#include "io.h"
#include "pins.h"
#include "Print.h"
#include <inttypes.h>

/**
  This is the main Hardware Abstraction Layer (HAL).
  To make the firmware work with different processors and toolchains,
  all hardware related code should be packed into the hal files.
*/

#define INLINE __attribute__((always_inline))

#if CPU_ARCH == ARCH_AVR
#include <avr/pgmspace.h>
#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>
#else
#include "eeprom_emulator.h"
#include <inttypes.h>
//STM32
#undef F_CPU
#define F_CPU       18000000        // should be factor of F_CPU_TRUE
#define F_CPU_TRUE  72000000        // actual CPU clock frequency
#define EEPROM_BYTES 4096  // bytes of eeprom we simulate
#define SUPPORT_64_BIT_MATH  // Gives better results with high resultion deltas
// do not use program space memory with STM32F103RE
#endif
#define SPR0    0
#define SPR1    1

#define PACK    __attribute__ ((packed))

#define INLINE __attribute__((always_inline))

#define FSTRINGVALUE(var,value) const char var[] PROGMEM = value;
#define FSTRINGVAR(var) static const char var[] PROGMEM;
#define FSTRINGPARAM(var) PGM_P var

/** \brief Prescale factor, timer0 runs at.

All known arduino boards use 64. This value is needed for the extruder timing. */
#undef  SOFTWARE_SPI
#define TIMER0_PRESCALE 128

#define PROGMEM
#define PGM_P const char *
typedef char prog_char;
#undef PSTR
#define PSTR(s) s
#undef pgm_read_byte_near
#define pgm_read_byte_near(x) (*(int8_t*)x)
#undef pgm_read_byte
#define pgm_read_byte(x) (*(int8_t*)x)
#undef pgm_read_float
#define pgm_read_float(addr) (*(const float *)(addr))
#undef pgm_read_word
//#define pgm_read_word(addr) (*(const unsigned int *)(addr))
#define pgm_read_word(addr) (*(addr))
#undef pgm_read_word_near
#define pgm_read_word_near(addr) pgm_read_word(addr)
#undef pgm_read_dword
#define pgm_read_dword(addr) (*(addr))
//#define pgm_read_dword(addr) (*(const unsigned long *)(addr))
#undef pgm_read_dword_near
#define pgm_read_dword_near(addr) pgm_read_dword(addr)
#define _BV(x) (1 << (x))
#define PULLUP(IO,v)            {pinMode(IO, (v!=LOW ? INPUT_PULLUP : INPUT)); }

#define ANALOG_PRESCALER _BV(ADPS0)|_BV(ADPS1)|_BV(ADPS2)

///#include "Print.h"
#ifdef EXTERNALSERIAL
//#define SERIAL_RX_BUFFER_SIZE 128
#endif
#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#define COMPAT_PRE1
#endif
#if CPU_ARCH==ARCH_AVR
#include "fastio.h"
#else
#define READ(pin) HAL::digitalRead(pin)
#define WRITE(pin,v) HAL::digitalWrite(pin,v)
#define SET_INPUT(pin) pinMode(pin,INPUT)
#define SET_OUTPUT(pin) pinMode(pin,OUTPUT)
#undef LOW
#define LOW         0
#undef HIGH
#define HIGH        1

#endif
union eeval_t {
	uint8_t     b[4];
	float       f;
	uint32_t    i;
	uint16_t    s;
	long        l;
} PACK;

class InterruptProtectedBlock {
public:
	INLINE void protect() {
		nvic_globalirq_disable();
	}

	INLINE void unprotect() {
		nvic_globalirq_enable();
	}

	INLINE InterruptProtectedBlock(bool later = false) {
		if (!later)
			nvic_globalirq_disable();
	}

	INLINE ~InterruptProtectedBlock() {
		nvic_globalirq_enable();
	}
};

#define EEPROM_OFFSET               0
#define SECONDS_TO_TICKS(s) (unsigned long)(s*(float)F_CPU)
#define ANALOG_INPUT_SAMPLE 5
//#define ANALOG_INPUT_MEDIAN 10

// Bits of the ADC converter
#define ANALOG_INPUT_BITS 12
#define ANALOG_REDUCE_BITS 0
#define ANALOG_REDUCE_FACTOR 1

static uint32_t    tone_pin;
// maximum available RAM
#define MAX_RAM 65535

#define bit_clear(x,y) x&= ~(1<<y) //cbi(x,y)
#define bit_set(x,y)   x|= (1<<y)//sbi(x,y)

/** defines the data direction (reading from I2C device) in i2cStart(),i2cRepStart() */
#define I2C_READ    1
/** defines the data direction (writing to I2C device) in i2cStart(),i2cRepStart() */
#define I2C_WRITE   0

#define LIMIT_INTERVAL (F_CPU/500000)

typedef uint16_t speed_t;
typedef uint32_t ticks_t;
typedef uint32_t millis_t;
typedef uint8_t flag8_t;
typedef int8_t fast8_t;
typedef uint8_t ufast8_t;
#define FAST_INTEGER_SQRT

#ifndef EXTERNALSERIAL
// Implement serial communication for one stream only!
/*
  HardwareSerial.h - Hardware serial library for Wiring
  Copyright (c) 2006 Nicholas Zambetti.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  Modified 28 September 2010 by Mark Sproul

  Modified to use only 1 queue with fixed length by Repetier
*/

#define SERIAL_BUFFER_SIZE 128
#define SERIAL_BUFFER_MASK 127
#undef SERIAL_TX_BUFFER_SIZE
#undef SERIAL_TX_BUFFER_MASK

#ifdef BIG_OUTPUT_BUFFER
#define SERIAL_TX_BUFFER_SIZE 128
#define SERIAL_TX_BUFFER_MASK 127
#else
#define SERIAL_TX_BUFFER_SIZE 64
#define SERIAL_TX_BUFFER_MASK 63
#endif
//#define RFSERIAL Serial
#define RFSERIAL Serial
//extern ring_buffer tx_buffer;
#define WAIT_OUT_EMPTY while(tx_buffer.head != tx_buffer.tail) {}
#else
#define RFSERIAL Serial
#endif
#define EXTRUDER_TIMER          TC0
#define EXTRUDER_TIMER_CHANNEL  0
#define EXTRUDER_TIMER_IRQ      ID_TC0
#define EXTRUDER_TIMER_VECTOR   TC0_Handler
#define PWM_TIMER               TC0
#define PWM_TIMER_CHANNEL       1
#define PWM_TIMER_IRQ           ID_TC1
#define PWM_TIMER_VECTOR        TC1_Handler
#define TIMER1_TIMER            TC2
#define TIMER1_TIMER_CHANNEL    2
#define TIMER1_TIMER_IRQ        ID_TC8
#define TIMER1_COMPA_VECTOR     TC8_Handler
#define SERVO_TIMER             TC2
#define SERVO_TIMER_CHANNEL     0
#define SERVO_TIMER_IRQ         ID_TC6
#define SERVO_COMPA_VECTOR      TC6_Handler
#define BEEPER_TIMER            TC1
#define BEEPER_TIMER_CHANNEL    0
#define BEEPER_TIMER_IRQ        ID_TC3
#define BEEPER_TIMER_VECTOR     TC3_Handler
#define DELAY_TIMER             TC1
#define DELAY_TIMER_CHANNEL     1
#define DELAY_TIMER_IRQ         ID_TC4  // IRQ not really used, needed for pmc id
#define DELAY_TIMER_CLOCK       TC_CMR_TCCLKS_TIMER_CLOCK2
#define DELAY_TIMER_PRESCALE    8
#define TWI_INTERFACE   		TWI0
#define TWI_ID  				ID_TWI0
#define SERVO_CLOCK_FREQ        1000
#define SERVO_PRESCALE          2      // Using TCLOCK1 therefore 2
#define SERVO2500US             (((F_CPU_TRUE / SERVO_PRESCALE) / 1000000) * 2500)
#define SERVO5000US             (((F_CPU_TRUE / SERVO_PRESCALE) / 1000000) * 5000)

#define AD_PRESCALE_FACTOR      84  // 500 kHz ADC clock
#define AD_TRACKING_CYCLES      4   // 0 - 15     + 1 adc clock cycles
#define AD_TRANSFER_CYCLES      1   // 0 - 3      * 2 + 3 adc clock cycles

#define ADC_ISR_EOC(channel)    (0x1u << channel)
#define ENABLED_ADC_CHANNELS    {TEMP_0_PIN, TEMP_1_PIN, TEMP_2_PIN}

#define OUT_P_I(p,i) Com::printF(PSTR(p),(int)(i))
#define OUT_P_I_LN(p,i) Com::printFLN(PSTR(p),(int)(i))
#define OUT_P_L(p,i) Com::printF(PSTR(p),(long)(i))
#define OUT_P_L_LN(p,i) Com::printFLN(PSTR(p),(long)(i))
#define OUT_P_F(p,i) Com::printF(PSTR(p),(float)(i))
#define OUT_P_F_LN(p,i) Com::printFLN(PSTR(p),(float)(i))
#define OUT_P_FX(p,i,x) Com::printF(PSTR(p),(float)(i),x)
#define OUT_P_FX_LN(p,i,x) Com::printFLN(PSTR(p),(float)(i),x)
#define OUT_P(p) Com::printF(PSTR(p))
#define OUT_P_LN(p) Com::printFLN(PSTR(p))
#define OUT_ERROR_P(p) Com::printErrorF(PSTR(p))
#define OUT_ERROR_P_LN(p) {Com::printErrorF(PSTR(p));Com::println();}
#define OUT(v) Com::print(v)
#define OUT_LN Com::println()

class HAL
{
public:
	static char virtualEeprom[EEPROM_BYTES];
#if FEATURE_WATCHDOG
	static bool wdPinged;
#endif
	HAL();
	virtual ~HAL();
	static inline void hwSetup(void)
	{
#if EEPROM_MODE == 1
		int i;
		for (i = 0; i < EEPROM_BYTES; i += 4) {
			eeval_t v = eprGetValue(i, 4);
			memcopy4(&virtualEeprom[i], &v.i);
		}
#else
		int i, n = 0;
		for (i = 0; i < EEPROM_BYTES; i += 4) {
			memcopy4(&virtualEeprom[i], &n);
		}
#endif
	}
	// return val'val
	static uint16_t integerSqrt(uint32_t a);
	/** \brief Optimized division

	Normally the C compiler will compute a long/long division, which takes ~670 Ticks.
	This version is optimized for a 16 bit dividend and recognises the special cases
	of a 24 bit and 16 bit dividend, which offen, but not always occur in updating the
	interval.
	*/
	static inline int32_t Div4U2U(uint32_t a, uint16_t b)
	{
		return ((unsigned long)a / (unsigned long)b);
	}
	static inline unsigned long U16SquaredToU32(unsigned int val)
	{
		return (unsigned long)val* (unsigned long)val;
	}
	static inline unsigned int ComputeV(long timer, long accel)
	{
		return static_cast<unsigned int>((static_cast<int64_t>(timer) * static_cast<int64_t>(accel)) >> 18);
	}
	// Multiply two 16 bit values and return 32 bit result
	static inline uint32_t mulu16xu16to32(unsigned int a, unsigned int b)
	{
		return (unsigned long)a* (unsigned long)b;
	}
	// Multiply two 16 bit values and return 32 bit result
	static inline unsigned int mulu6xu16shift16(unsigned int a, unsigned int b)
	{
		return ((unsigned long)a * (unsigned long)b) >> 16;
	}
	static inline void digitalWrite(uint8_t pin, uint8_t value)
	{
		::digitalWrite(pin, value);
	}
	static inline uint8_t digitalRead(uint8_t pin)
	{
		return ::digitalRead(pin);
	}
	static inline void pinMode(uint8_t pin, uint8_t mode)
	{
		if (mode == INPUT) {
			//SET_INPUT(pin);
			::pinMode(pin, INPUT);
		}
		else if (mode == INPUT_PULLUP) {
			::pinMode(pin, INPUT_PULLUP);
		}
		else //SET_OUTPUT(pin);
			::pinMode(pin, OUTPUT);
	}
	static int32_t CPUDivU2(unsigned int divisor) {
		return F_CPU / divisor;
	}
	static inline void delayMicroseconds(unsigned int delayUs)
	{
		uint32_t n = delayUs * (F_CPU_TRUE / 3000000);
		asm volatile(
			"L2_%=_delayMicroseconds:"       "\n\t"
			"subs   %0, #1"                 "\n\t"
			"bge    L2_%=_delayMicroseconds" "\n"
			: "+r" (n) :
			);
	}
	//static uint32_t    tone_pin;
	static inline void delayMilliseconds(unsigned int delayMs)
	{
		unsigned int del;
		while (delayMs > 0) {
			del = delayMs > 100 ? 100 : delayMs;
			delay(del);
			delayMs -= del;
#if FEATURE_WATCHDOG
			HAL::pingWatchdog();
#endif
		}
	}
	static inline void tone(uint8_t pin, int duration)
	{
		pinMode(pin, OUTPUT);
		tone_pin = pin;
	}
	static inline void noTone(uint8_t pin)
	{
		//	::noTone(pin);
	}
	static inline void eprSetByte(unsigned int pos, uint8_t value)
	{
		eeval_t v;
		v.b[0] = value;
		eprBurnValue(pos, 1, v);
		*(uint8_t*)& virtualEeprom[pos] = value;
	}
	static inline void eprSetInt16(unsigned int pos, int16_t value)
	{
		eeval_t v;
		v.s = value;
		eprBurnValue(pos, 2, v);
		memcopy2(&virtualEeprom[pos], &value);
	}
	static inline void eprSetInt32(unsigned int pos, int32_t value)
	{
		eeval_t v;
		v.i = value;
		eprBurnValue(pos, 4, v);
		memcopy4(&virtualEeprom[pos], &value);
	}
	static inline void eprSetFloat(unsigned int pos, float value)
	{
		eeval_t v;
		v.f = value;
		eprBurnValue(pos, sizeof(float), v);
		memcopy4(&virtualEeprom[pos], &value);
	}
	static inline uint8_t eprGetByte(unsigned int pos)
	{
		return *(uint8_t*)& virtualEeprom[pos];
	}
	static inline int16_t eprGetInt16(unsigned int pos)
	{
		int16_t v;
		memcopy2(&v, &virtualEeprom[pos]);
		return v;
	}
	static inline void eprBurnValue(unsigned int pos, int size, union eeval_t newvalue)
	{
		uint8_t n;
		uint16_t valu[4] = { 0 };
		for (n = 0; n < size; n++)
		{
			valu[n] = (uint16_t)newvalue.b[n];
			EE_WriteVariable((uint16_t)(pos + n), valu[n]);
		}
	}

	// Read any data type from EEPROM that was previously written by eprBurnValue
	static inline union eeval_t eprGetValue(unsigned int pos, int size)
	{
		eeval_t v;
		int i;
		uint16_t value_temp = 0xff;
		for (i = 0; i < size; i++) {
			// read an incomming byte
			EE_ReadVariable(pos + i, &value_temp);
			v.b[i] = value_temp;
			value_temp = 0xff;
		}
		return v;
	}

	static inline int32_t eprGetInt32(unsigned int pos)
	{
		int32_t v;
		memcopy4(&v, &virtualEeprom[pos]);
		return v;
	}
	static inline float eprGetFloat(unsigned int pos)
	{
		float v;
		memcopy4(&v, &virtualEeprom[pos]);
		return v;
	}

	// Faster version of InterruptProtectedBlock.
	// For safety it ma yonly be called from within an
	// interrupt handler.
	static inline void allowInterrupts()
	{
		//	sei();
	}

	// Faster version of InterruptProtectedBlock.
	// For safety it ma yonly be called from within an
	// interrupt handler.
	static inline void forbidInterrupts()
	{
		//	cli();
	}
	static inline unsigned long timeInMilliseconds()
	{
		return millis();
	}
	static inline char readFlashByte(PGM_P ptr)
	{
		return pgm_read_byte(ptr);
	}
	static inline void serialSetBaudrate(long baud)
	{
		RFSERIAL.begin(baud);
	}
	static inline bool serialByteAvailable()
	{
		return RFSERIAL.available() > 0;
	}
	static inline uint8_t serialReadByte()
	{
		return RFSERIAL.read();
	}
	static inline void serialWriteByte(char b)
	{
		RFSERIAL.write(b);
	}
	static inline void serialFlush()
	{
		RFSERIAL.flush();
	}
	static void setupTimer();
	static void showStartReason();
	static int getFreeRam();
	static void resetHardware();

	// SPI related functions
	static void spiBegin();
	static inline void spiInit(uint8_t spiRate);

	static inline __attribute__((always_inline))
		void spiSendBlock(uint8_t token, const uint8_t * buf);

	// I2C Support

	static void i2cInit(uint32_t clockSpeedHz);
	static unsigned char i2cStart(uint8_t address);
	static void i2cStartWait(uint8_t address);
	static void i2cStop(void);
	static uint8_t i2cWrite(uint8_t data);
	static uint8_t i2cReadAck(void);
	static uint8_t i2cReadNak(void);

	// Watchdog support

	inline static void startWatchdog()
	{
#if defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__)
		WDTCSR = (1 << WDCE) | (1 << WDE);								// wdt FIX for arduino mega boards
		WDTCSR = (1 << WDIE) | (1 << WDP3);
#else
		//wdt_enable(WDTO_4S);
#endif
};
	inline static void stopWatchdog()
	{
		//wdt_disable();
	}
	inline static void pingWatchdog()
	{
#if FEATURE_WATCHDOG
		wdPinged = true;
#endif
	};
	inline static float maxExtruderTimerFrequency()
	{
		return (float)F_CPU / TIMER0_PRESCALE;
	}
#if FEATURE_SERVO
	static unsigned int servoTimings[4];
	static void servoMicroseconds(uint8_t servo, int ms, uint16_t autoOff);
#endif
	static void analogStart();
#if USE_ADVANCE
	static void resetExtruderDirection();
#endif
	static volatile uint8_t insideTimer1;
protected:
private:
	};
/*#if MOTHERBOARD==6 || MOTHERBOARD==62 || MOTHERBOARD==7
#if MOTHERBOARD!=7
#define SIMULATE_PWM
#endif
#define EXTRUDER_TIMER_VECTOR TIMER2_COMPA_vect
#define EXTRUDER_OCR OCR2A
#define EXTRUDER_TCCR TCCR2A
#define EXTRUDER_TIMSK TIMSK2
#define EXTRUDER_OCIE OCIE2A
#define PWM_TIMER_VECTOR TIMER2_COMPB_vect
#define PWM_OCR OCR2B
#define PWM_TCCR TCCR2B
#define PWM_TIMSK TIMSK2
#define PWM_OCIE OCIE2B
#else*/

//#endif
#endif // HAL_H



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

*/

#define UI_MAIN 1
#include "Repetier.h"
#include "Printer.h"
// The uimenu.h declares static variables of menus, which must be declared only once.
// It does not define interfaces for other modules, so should never be included elsewhere
#include "uimenu.h"
extern const int8_t encoder_table[16] PROGMEM;
#include <math.h>
#include <stdlib.h>
#include <inttypes.h>
#include "itoa.h"
#include <ctype.h>
extern uint8_t pushVar;
extern uint8_t eMaxPos;
float Printer::newECode;
float diffVar;
uint8_t preHeatChoco = 0; // marcel preheat
uint8_t preheatVar = 0;
uint8_t extrudeUp = 0; // marcel preheat
uint8_t extrudeVar = 0;
uint8_t preKartusche = 0; // nach der zeit epos auf geht
uint8_t fileVar = 0; // back im fileselector zu Hauptscreen
uint8_t kartuschenwVar = 0; //nach zeit von preheat im kartuschenw, damit epos memory macht
uint8_t printVar = 0; // nach zeit von preheat im printfile, damit epos fileselector auf macht
uint8_t chocoChooseVar = 0; // choco ausgewählt
uint8_t textChooseVar = 0; // text ausgewählt
uint8_t chooseVar = 0;  // Objekt produzieren gestartet
uint8_t cleanEPos = eMaxPos / 2; // extruder position für reinigung (kleiner als emaxpos)
uint8_t calibrateVar = 0; // wenn mindestens einmal kalibriert wurde
uint8_t zHelpPos = FILAMENTCHANGE_Z_ADD;
uint8_t removeVar = 0;
uint8_t resetVar = 0;
uint8_t standardVar = 0;
uint8_t cookieVar = 0;
uint8_t chocolateVar = 0;
uint8_t holdVar = 0; // temp auf hohe temp halten
uint8_t cleanVar = 0; // im clean menü
uint8_t errorVar = 0; // Error Bildschirm
uint8_t predoseVarPos = 0; // vordosier variable für anzeige
uint8_t oldEposVar = static_cast<float>(Printer::currentPositionSteps[E_AXIS]) * Printer::invAxisStepsPerMM[E_AXIS]; // alte pos für increment auf display
uint8_t oldLevel;
uint8_t cleanEx = 0;
uint8_t eposVariable = 0;
uint8_t startVariable = 0;
uint8_t langQuest = 0;

char fileNameLCD[30];
char fileNameLCDAsk[30];
char symbol;

// marcel preheat zeitvariablen

unsigned long zeitAnfang;
unsigned long zeitEnde = 0;
unsigned long holdAnfang;
unsigned long holdEnde;
unsigned long holdMix;
float zeitChoco;

unsigned long intervall = 10 * 60; // preheat time in s
float zeitMinChoco = (intervall / 60) - 1;
float zeitSecChoco;
float zeitVar = 60;
float zeitSecChoco1 = 0;
unsigned long zeitPercent = 0;
unsigned long fullPercent = 100;
unsigned long fullTest = 100;
unsigned long extrudePercent = 0;
unsigned long x = 0;
unsigned long a = 0;
unsigned long x1 = 0;
float tempPercent = 0;

unsigned long zeitAnfangEx; // marcel Extruder hoch zeit
unsigned long zeitEndeEx = 0;
unsigned long zeitExtrude;
unsigned long intervall_2 = (2 * 60 + 30) * 1000; // eMaxPos time in ms
unsigned long intervall_3 = ((2 * 60 + 30) * 1000) / 2; // cleanPos time in ms
unsigned long intervall_4 = 180 * 1000; // preheat hohe Temp halten in ms
unsigned long zeitPercentEx = 0;

// marcel preheat zeitvariablen

#if FEATURE_SERVO > 0 && UI_SERVO_CONTROL > 0
#if   UI_SERVO_CONTROL == 1 && defined(SERVO0_NEUTRAL_POS)
uint16_t servoPosition = SERVO0_NEUTRAL_POS;
#elif UI_SERVO_CONTROL == 2 && defined(SERVO1_NEUTRAL_POS)
uint16_t servoPosition = SERVO1_NEUTRAL_POS;
#elif UI_SERVO_CONTROL == 3 && defined(SERVO2_NEUTRAL_POS)
uint16_t servoPosition = SERVO2_NEUTRAL_POS;
#elif UI_SERVO_CONTROL == 4 && defined(SERVO3_NEUTRAL_POS)
uint16_t servoPosition = SERVO3_NEUTRAL_POS;
#else
uint16_t servoPosition = 1500;
#endif
#endif

#if BEEPER_TYPE==2 && defined(UI_HAS_I2C_KEYS) && UI_I2C_KEY_ADDRESS!=BEEPER_ADDRESS
#error Beeper address and i2c key address must be identical
#else
#if BEEPER_TYPE==2
#define UI_I2C_KEY_ADDRESS BEEPER_ADDRESS
#endif
#endif

static TemperatureController * currHeaterForSetup;    // pointer to extruder or heatbed temperature controller

#if UI_AUTORETURN_TO_MENU_AFTER != 0
millis_t ui_autoreturn_time = 0;
#endif
#if FEATURE_BABYSTEPPING
int zBabySteps = 0;
#endif

void beep(uint8_t duration, uint8_t count)
{
#if FEATURE_BEEPER
#if BEEPER_TYPE!=0
#if BEEPER_TYPE==1 && defined(BEEPER_PIN) && BEEPER_PIN>=0
	SET_OUTPUT(BEEPER_PIN);
#endif
#if BEEPER_TYPE==2
	HAL::i2cStartWait(BEEPER_ADDRESS + I2C_WRITE);
#if UI_DISPLAY_I2C_CHIPTYPE==1
	HAL::i2cWrite(0x14); // Start at port a
#endif
#endif
	for (uint8_t i = 0; i < count; i++)
	{
#if BEEPER_TYPE==1 && defined(BEEPER_PIN) && BEEPER_PIN>=0
#if defined(BEEPER_TYPE_INVERTING) && BEEPER_TYPE_INVERTING
		WRITE(BEEPER_PIN, LOW);
#else
		WRITE(BEEPER_PIN, HIGH);
#endif
#else
#if UI_DISPLAY_I2C_CHIPTYPE==0
#if BEEPER_ADDRESS == UI_DISPLAY_I2C_ADDRESS
		HAL::i2cWrite(uid.outputMask & ~BEEPER_PIN);
#else
		HAL::i2cWrite(~BEEPER_PIN);
#endif
#endif
#if UI_DISPLAY_I2C_CHIPTYPE==1
		HAL::i2cWrite((BEEPER_PIN) | uid.outputMask);
		HAL::i2cWrite(((BEEPER_PIN) | uid.outputMask) >> 8);
#endif
#endif
		HAL::delayMilliseconds(duration);
#if BEEPER_TYPE==1 && defined(BEEPER_PIN) && BEEPER_PIN>=0
#if defined(BEEPER_TYPE_INVERTING) && BEEPER_TYPE_INVERTING
		WRITE(BEEPER_PIN, HIGH);
#else
		WRITE(BEEPER_PIN, LOW);
#endif
#else
#if UI_DISPLAY_I2C_CHIPTYPE==0

#if BEEPER_ADDRESS == UI_DISPLAY_I2C_ADDRESS
		HAL::i2cWrite((BEEPER_PIN) | uid.outputMask);
#else
		HAL::i2cWrite(255);
#endif
#endif
#if UI_DISPLAY_I2C_CHIPTYPE==1
		HAL::i2cWrite(uid.outputMask);
		HAL::i2cWrite(uid.outputMask >> 8);
#endif
#endif
		HAL::delayMilliseconds(duration);
	}
#if BEEPER_TYPE==2
	HAL::i2cStop();
#endif
#endif
#endif
}

bool UIMenuEntry::showEntry() const
{
	bool ret = true;
	uint8_t f, f2;
	f = HAL::readFlashByte((PGM_P)& filter);
	if (f != 0)
		ret = (f & Printer::menuMode) != 0;
	if (ret && (f2 = HAL::readFlashByte((PGM_P)& nofilter)) != 0)
		ret = (f2 & Printer::menuMode) == 0;
	return ret;
}

#if UI_DISPLAY_TYPE != NO_DISPLAY
UIDisplay uid;
char displayCache[UI_ROWS][MAX_COLS + 1];

// Menu up sign - code 1
// ..*.. 4
// .***. 14
// *.*.* 21
// ..*.. 4
// ..*.. 4
// ..*.. 4
// ***.. 28
// ..... 0
const uint8_t character_back[8] PROGMEM = { 4, 14, 21, 4, 4, 4, 28, 0 };
// Degrees sign - code 2
// ..*.. 4
// .*.*. 10
// ..*.. 4
// ..... 0
// ..... 0
// ..... 0
// ..... 0
// ..... 0
const uint8_t character_degree[8] PROGMEM = { 4, 10, 4, 0, 0, 0, 0, 0 };
// selected - code 3
// ..... 0
// ***** 31
// ***** 31
// ***** 31
// ***** 31
// ***** 31
// ***** 31
// ..... 0
// ..... 0
const uint8_t character_selected[8] PROGMEM = { 0, 31, 31, 31, 31, 31, 0, 0 };
// unselected - code 4
// ..... 0
// ***** 31
// *...* 17
// *...* 17
// *...* 17
// *...* 17
// ***** 31
// ..... 0
// ..... 0
const uint8_t character_unselected[8] PROGMEM = { 0, 31, 17, 17, 17, 31, 0, 0 };
// unselected - code 5
// ..*.. 4
// .*.*. 10
// .*.*. 10
// .*.*. 10
// .*.*. 10
// .***. 14
// ***** 31
// ***** 31
// .***. 14
const uint8_t character_temperature[8] PROGMEM = { 4, 10, 10, 10, 14, 31, 31, 14 };
// unselected - code 6
// ..... 0
// ***.. 28
// ***** 31
// *...* 17
// *...* 17
// ***** 31
// ..... 0
// ..... 0
const uint8_t character_folder[8] PROGMEM = { 0, 28, 31, 17, 17, 31, 0, 0 };

// printer ready - code 7
// *...* 17
// .*.*. 10
// ..*.. 4
// *...* 17
// ..*.. 4
// .*.*. 10
// *...* 17
// *...* 17
const byte character_ready[8] PROGMEM = { 17, 10, 4, 17, 4, 10, 17, 17 };

const long baudrates[] PROGMEM = { 9600, 14400, 19200, 28800, 38400, 56000, 5760, 76800, 111112, 115200, 128000, 230400, 250000, 256000,
								  460800, 500000, 921600, 1000000, 1500000, 0
};

#define LCD_ENTRYMODE			0x04			/**< Set entrymode */

/** @name GENERAL COMMANDS */
/*@{*/
#define LCD_CLEAR			0x01	/**< Clear screen */
#define LCD_HOME			0x02	/**< Cursor move to first digit */
/*@}*/

/** @name ENTRYMODES */
/*@{*/
#define LCD_ENTRYMODE			0x04			/**< Set entrymode */
#define LCD_INCREASE		LCD_ENTRYMODE | 0x02	/**<	Set cursor move direction -- Increase */
#define LCD_DECREASE		LCD_ENTRYMODE | 0x00	/**<	Set cursor move direction -- Decrease */
#define LCD_DISPLAYSHIFTON	LCD_ENTRYMODE | 0x01	/**<	Display is shifted */
#define LCD_DISPLAYSHIFTOFF	LCD_ENTRYMODE | 0x00	/**<	Display is not shifted */
/*@}*/

/** @name DISPLAYMODES */
/*@{*/
#define LCD_DISPLAYMODE			0x08			/**< Set displaymode */
#define LCD_DISPLAYON		LCD_DISPLAYMODE | 0x04	/**<	Display on */
#define LCD_DISPLAYOFF		LCD_DISPLAYMODE | 0x00	/**<	Display off */
#define LCD_CURSORON		LCD_DISPLAYMODE | 0x02	/**<	Cursor on */
#define LCD_CURSOROFF		LCD_DISPLAYMODE | 0x00	/**<	Cursor off */
#define LCD_BLINKINGON		LCD_DISPLAYMODE | 0x01	/**<	Blinking on */
#define LCD_BLINKINGOFF		LCD_DISPLAYMODE | 0x00	/**<	Blinking off */
/*@}*/

/** @name SHIFTMODES */
/*@{*/
#define LCD_SHIFTMODE			0x10			/**< Set shiftmode */
#define LCD_DISPLAYSHIFT	LCD_SHIFTMODE | 0x08	/**<	Display shift */
#define LCD_CURSORMOVE		LCD_SHIFTMODE | 0x00	/**<	Cursor move */
#define LCD_RIGHT		LCD_SHIFTMODE | 0x04	/**<	Right shift */
#define LCD_LEFT		LCD_SHIFTMODE | 0x00	/**<	Left shift */
/*@}*/

/** @name DISPLAY_CONFIGURATION */
/*@{*/
#define LCD_CONFIGURATION		0x20				/**< Set function */
#define LCD_8BIT		LCD_CONFIGURATION | 0x10	/**<	8 bits interface */
#define LCD_4BIT		LCD_CONFIGURATION | 0x00	/**<	4 bits interface */
#define LCD_2LINE		LCD_CONFIGURATION | 0x08	/**<	2 line display */
#define LCD_1LINE		LCD_CONFIGURATION | 0x00	/**<	1 line display */
#define LCD_5X10		LCD_CONFIGURATION | 0x04	/**<	5 X 10 dots */
#define LCD_5X7			LCD_CONFIGURATION | 0x00	/**<	5 X 7 dots */

#define LCD_SETCGRAMADDR 0x40

#define lcdPutChar(value) lcdWriteByte(value,1)
#define lcdCommand(value) lcdWriteByte(value,0)

static const uint8_t LCDLineOffsets[] PROGMEM = UI_LINE_OFFSETS;
static const char versionString[] PROGMEM = UI_VERSION_STRING;

#if UI_DISPLAY_TYPE == DISPLAY_I2C

// ============= I2C LCD Display driver ================
inline void lcdStartWrite()
{
	HAL::i2cStartWait(UI_DISPLAY_I2C_ADDRESS + I2C_WRITE);
#if UI_DISPLAY_I2C_CHIPTYPE == 1
	HAL::i2cWrite(0x14); // Start at port a
#endif
}
inline void lcdStopWrite()
{
	HAL::i2cStop();
}
void lcdWriteNibble(uint8_t value)
{
#if UI_DISPLAY_I2C_CHIPTYPE==0
	value |= uid.outputMask;
#if UI_DISPLAY_D4_PIN==1 && UI_DISPLAY_D5_PIN==2 && UI_DISPLAY_D6_PIN==4 && UI_DISPLAY_D7_PIN==8
	HAL::i2cWrite((value) | UI_DISPLAY_ENABLE_PIN);
	HAL::i2cWrite(value);
#else
	uint8_t v = (value & 1 ? UI_DISPLAY_D4_PIN : 0) | (value & 2 ? UI_DISPLAY_D5_PIN : 0) | (value & 4 ? UI_DISPLAY_D6_PIN : 0) | (value & 8 ? UI_DISPLAY_D7_PIN : 0);
	HAL::i2cWrite((v) | UI_DISPLAY_ENABLE_PIN);
	HAL::i2cWrite(v);
#
#endif
#endif
#if UI_DISPLAY_I2C_CHIPTYPE==1
	unsigned int v = (value & 1 ? UI_DISPLAY_D4_PIN : 0) | (value & 2 ? UI_DISPLAY_D5_PIN : 0) | (value & 4 ? UI_DISPLAY_D6_PIN : 0) | (value & 8 ? UI_DISPLAY_D7_PIN : 0) | uid.outputMask;
	unsigned int v2 = v | UI_DISPLAY_ENABLE_PIN;
	HAL::i2cWrite(v2 & 255);
	HAL::i2cWrite(v2 >> 8);
	HAL::i2cWrite(v & 255);
	HAL::i2cWrite(v >> 8);
#endif
}
void lcdWriteByte(uint8_t c, uint8_t rs)
{
#if UI_DISPLAY_I2C_CHIPTYPE==0
	uint8_t mod = (rs ? UI_DISPLAY_RS_PIN : 0) | uid.outputMask; // | (UI_DISPLAY_RW_PIN);
#if UI_DISPLAY_D4_PIN==1 && UI_DISPLAY_D5_PIN==2 && UI_DISPLAY_D6_PIN==4 && UI_DISPLAY_D7_PIN==8
	uint8_t value = (c >> 4) | mod;
	HAL::i2cWrite((value) | UI_DISPLAY_ENABLE_PIN);
	HAL::i2cWrite(value);
	value = (c & 15) | mod;
	HAL::i2cWrite((value) | UI_DISPLAY_ENABLE_PIN);
	HAL::i2cWrite(value);
#else
	uint8_t value = (c & 16 ? UI_DISPLAY_D4_PIN : 0) | (c & 32 ? UI_DISPLAY_D5_PIN : 0) | (c & 64 ? UI_DISPLAY_D6_PIN : 0) | (c & 128 ? UI_DISPLAY_D7_PIN : 0) | mod;
	HAL::i2cWrite((value) | UI_DISPLAY_ENABLE_PIN);
	HAL::i2cWrite(value);
	value = (c & 1 ? UI_DISPLAY_D4_PIN : 0) | (c & 2 ? UI_DISPLAY_D5_PIN : 0) | (c & 4 ? UI_DISPLAY_D6_PIN : 0) | (c & 8 ? UI_DISPLAY_D7_PIN : 0) | mod;
	HAL::i2cWrite((value) | UI_DISPLAY_ENABLE_PIN);
	HAL::i2cWrite(value);
#endif
#endif
#if UI_DISPLAY_I2C_CHIPTYPE==1
	unsigned int mod = (rs ? UI_DISPLAY_RS_PIN : 0) | uid.outputMask; // | (UI_DISPLAY_RW_PIN);
	unsigned int value = (c & 16 ? UI_DISPLAY_D4_PIN : 0) | (c & 32 ? UI_DISPLAY_D5_PIN : 0) | (c & 64 ? UI_DISPLAY_D6_PIN : 0) | (c & 128 ? UI_DISPLAY_D7_PIN : 0) | mod;
	unsigned int value2 = (value) | UI_DISPLAY_ENABLE_PIN;
	HAL::i2cWrite(value2 & 255);
	HAL::i2cWrite(value2 >> 8);
	HAL::i2cWrite(value & 255);
	HAL::i2cWrite(value >> 8);
	value = (c & 1 ? UI_DISPLAY_D4_PIN : 0) | (c & 2 ? UI_DISPLAY_D5_PIN : 0) | (c & 4 ? UI_DISPLAY_D6_PIN : 0) | (c & 8 ? UI_DISPLAY_D7_PIN : 0) | mod;
	value2 = (value) | UI_DISPLAY_ENABLE_PIN;
	HAL::i2cWrite(value2 & 255);
	HAL::i2cWrite(value2 >> 8);
	HAL::i2cWrite(value & 255);
	HAL::i2cWrite(value >> 8);
#endif
}
void initializeLCD()
{
	HAL::delayMilliseconds(235);
	lcdStartWrite();
	HAL::i2cWrite(uid.outputMask & 255);
#if UI_DISPLAY_I2C_CHIPTYPE==1
	HAL::i2cWrite(uid.outputMask >> 8);
#endif
	HAL::delayMicroseconds(20);
	lcdWriteNibble(0x03);
	HAL::delayMicroseconds(6000); // I have one LCD for which 4500 here was not long enough.
	// second try
	lcdWriteNibble(0x03);
	HAL::delayMicroseconds(180); // wait
	// third go!
	lcdWriteNibble(0x03);
	HAL::delayMicroseconds(180);
	// finally, set to 4-bit interface
	lcdWriteNibble(0x02);
	HAL::delayMicroseconds(180);
	// finally, set # lines, font size, etc.
	lcdCommand(LCD_4BIT | LCD_2LINE | LCD_5X7);
	lcdCommand(LCD_CLEAR);					//-	Clear Screen
	HAL::delayMilliseconds(4); // clear is slow operation
	lcdCommand(LCD_INCREASE | LCD_DISPLAYSHIFTOFF);	//-	Entrymode (Display Shift: off, Increment Address Counter)
	lcdCommand(LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKINGOFF);	//-	Display on
	uid.lastSwitch = uid.lastRefresh = HAL::timeInMilliseconds();
	uid.createChar(1, character_back);
	uid.createChar(2, character_degree);
	uid.createChar(3, character_selected);
	uid.createChar(4, character_unselected);
	uid.createChar(5, character_temperature);
	uid.createChar(6, character_folder);
	uid.createChar(7, character_ready);
	lcdStopWrite();
}
#endif
#if UI_DISPLAY_TYPE == DISPLAY_4BIT || UI_DISPLAY_TYPE == DISPLAY_8BIT

void lcdWriteNibble(uint8_t value)
{
	WRITE(UI_DISPLAY_D4_PIN, value & 1);
	WRITE(UI_DISPLAY_D5_PIN, value & 2);
	WRITE(UI_DISPLAY_D6_PIN, value & 4);
	WRITE(UI_DISPLAY_D7_PIN, value & 8);
	DELAY1MICROSECOND;
	WRITE(UI_DISPLAY_ENABLE_PIN, HIGH);// enable pulse must be >450ns
	HAL::delayMicroseconds(2);
	WRITE(UI_DISPLAY_ENABLE_PIN, LOW);
	HAL::delayMicroseconds(UI_DELAYPERCHAR);
}

void lcdWriteByte(uint8_t c, uint8_t rs)
{
#if false && UI_DISPLAY_RW_PIN >= 0 // not really needed
	SET_INPUT(UI_DISPLAY_D4_PIN);
	SET_INPUT(UI_DISPLAY_D5_PIN);
	SET_INPUT(UI_DISPLAY_D6_PIN);
	SET_INPUT(UI_DISPLAY_D7_PIN);
	WRITE(UI_DISPLAY_RW_PIN, HIGH);
	WRITE(UI_DISPLAY_RS_PIN, LOW);
	uint8_t busy;
	do
	{
		WRITE(UI_DISPLAY_ENABLE_PIN, HIGH);
		DELAY1MICROSECOND;
		busy = READ(UI_DISPLAY_D7_PIN);
		WRITE(UI_DISPLAY_ENABLE_PIN, LOW);
		DELAY2MICROSECOND;

		WRITE(UI_DISPLAY_ENABLE_PIN, HIGH);
		DELAY2MICROSECOND;

		WRITE(UI_DISPLAY_ENABLE_PIN, LOW);
		DELAY2MICROSECOND;
	} while (busy);
	SET_OUTPUT(UI_DISPLAY_D4_PIN);
	SET_OUTPUT(UI_DISPLAY_D5_PIN);
	SET_OUTPUT(UI_DISPLAY_D6_PIN);
	SET_OUTPUT(UI_DISPLAY_D7_PIN);
	WRITE(UI_DISPLAY_RW_PIN, LOW);
#endif
	WRITE(UI_DISPLAY_RS_PIN, rs);

	WRITE(UI_DISPLAY_D4_PIN, c & 0x10);
	WRITE(UI_DISPLAY_D5_PIN, c & 0x20);
	WRITE(UI_DISPLAY_D6_PIN, c & 0x40);
	WRITE(UI_DISPLAY_D7_PIN, c & 0x80);
#if FEATURE_CONTROLLER == CONTROLLER_RADDS
	HAL::delayMicroseconds(10);
#else
	HAL::delayMicroseconds(2);
#endif
	WRITE(UI_DISPLAY_ENABLE_PIN, HIGH);   // enable pulse must be >450ns
#if FEATURE_CONTROLLER == CONTROLLER_RADDS
	HAL::delayMicroseconds(10);
#else
	HAL::delayMicroseconds(2);
#endif
	WRITE(UI_DISPLAY_ENABLE_PIN, LOW);

	WRITE(UI_DISPLAY_D4_PIN, c & 0x01);
	WRITE(UI_DISPLAY_D5_PIN, c & 0x02);
	WRITE(UI_DISPLAY_D6_PIN, c & 0x04);
	WRITE(UI_DISPLAY_D7_PIN, c & 0x08);
	HAL::delayMicroseconds(2);
	WRITE(UI_DISPLAY_ENABLE_PIN, HIGH);   // enable pulse must be >450ns
	HAL::delayMicroseconds(2);
	WRITE(UI_DISPLAY_ENABLE_PIN, LOW);
	HAL::delayMicroseconds(100);
}

#ifdef TRY_AUTOREPAIR_LCD_ERRORS
#define HAS_AUTOREPAIR
/* Fast repair function for displays loosing their settings.
  Do not call this if your display has no problems.
*/
void repairLCD()
{
	// Now we pull both RS and R/W low to begin commands
	WRITE(UI_DISPLAY_RS_PIN, LOW);
	WRITE(UI_DISPLAY_ENABLE_PIN, LOW);

	//put the LCD into 4 bit mode
	// this is according to the hitachi HD44780 datasheet
	// figure 24, pg 46

	// we start in 8bit mode, try to set 4 bit mode
	// at this point we are in 8 bit mode but of course in this
	// interface 4 pins are dangling unconnected and the values
	// on them don't matter for these instructions.
	WRITE(UI_DISPLAY_RS_PIN, LOW);
	HAL::delayMicroseconds(20);
	lcdWriteNibble(0x03);
	HAL::delayMicroseconds(5000); // I have one LCD for which 4500 here was not long enough.
	// second try
	//lcdWriteNibble(0x03);
	//HAL::delayMicroseconds(5000); // wait
	// third go!
	//lcdWriteNibble(0x03);
	//HAL::delayMicroseconds(160);
	// finally, set to 4-bit interface
	lcdWriteNibble(0x02);
	HAL::delayMicroseconds(160);
	// finally, set # lines, font size, etc.
	lcdCommand(LCD_4BIT | LCD_2LINE | LCD_5X7);
	lcdCommand(LCD_INCREASE | LCD_DISPLAYSHIFTOFF);	//-	Entrymode (Display Shift: off, Increment Address Counter)
	lcdCommand(LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKINGOFF);	//-	Display on
	uid.lastSwitch = uid.lastRefresh = HAL::timeInMilliseconds();
	uid.createChar(1, character_back);
	uid.createChar(2, character_degree);
	uid.createChar(3, character_selected);
	uid.createChar(4, character_unselected);
	uid.createChar(5, character_temperature);
	uid.createChar(6, character_folder);
	uid.createChar(7, character_ready);
}
#endif

void initializeLCD()
{
	// SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
	// according to datasheet, we need at least 40ms after power rises above 2.7V
	// before sending commands. Arduino can turn on way before 4.5V.
	// is this delay long enough for all cases??
	HAL::delayMilliseconds(235);
	SET_OUTPUT(UI_DISPLAY_D4_PIN);
	SET_OUTPUT(UI_DISPLAY_D5_PIN);
	SET_OUTPUT(UI_DISPLAY_D6_PIN);
	SET_OUTPUT(UI_DISPLAY_D7_PIN);
	SET_OUTPUT(UI_DISPLAY_RS_PIN);
#if UI_DISPLAY_RW_PIN > -1
	SET_OUTPUT(UI_DISPLAY_RW_PIN);
#endif
	SET_OUTPUT(UI_DISPLAY_ENABLE_PIN);

	// Now we pull both RS and R/W low to begin commands
	WRITE(UI_DISPLAY_RS_PIN, LOW);
	WRITE(UI_DISPLAY_ENABLE_PIN, LOW);

	//put the LCD into 4 bit mode
	// this is according to the hitachi HD44780 datasheet
	// figure 24, pg 46

	// we start in 8bit mode, try to set 4 bit mode
	// at this point we are in 8 bit mode but of course in this
	// interface 4 pins are dangling unconnected and the values
	// on them don't matter for these instructions.
	WRITE(UI_DISPLAY_RS_PIN, LOW);
	HAL::delayMicroseconds(20);
	lcdWriteNibble(0x03);
	HAL::delayMicroseconds(5000); // I have one LCD for which 4500 here was not long enough.
	// second try
	lcdWriteNibble(0x03);
	HAL::delayMicroseconds(5000); // wait
	// third go!
	lcdWriteNibble(0x03);
	HAL::delayMicroseconds(160);
	// finally, set to 4-bit interface
	lcdWriteNibble(0x02);
	HAL::delayMicroseconds(160);
	// finally, set # lines, font size, etc.
	lcdCommand(LCD_4BIT | LCD_2LINE | LCD_5X7);

	lcdCommand(LCD_CLEAR);					//-	Clear Screen
	HAL::delayMilliseconds(3); // clear is slow operation
	lcdCommand(LCD_INCREASE | LCD_DISPLAYSHIFTOFF);	//-	Entrymode (Display Shift: off, Increment Address Counter)
	lcdCommand(LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKINGOFF);	//-	Display on
	uid.lastSwitch = uid.lastRefresh = HAL::timeInMilliseconds();
	uid.createChar(1, character_back);
	uid.createChar(2, character_degree);
	uid.createChar(3, character_selected);
	uid.createChar(4, character_unselected);
	uid.createChar(5, character_temperature);
	uid.createChar(6, character_folder);
	uid.createChar(7, character_ready);
}
// ----------- end direct LCD driver
#endif
#if UI_DISPLAY_TYPE < DISPLAY_ARDUINO_LIB
void UIDisplay::printRow(uint8_t r, char* txt, char* txt2, uint8_t changeAtCol)
{
	changeAtCol = RMath::min(UI_COLS, changeAtCol);
	uint8_t col = 0;
	// Set row
	if (r >= UI_ROWS) return;
#if UI_DISPLAY_TYPE == DISPLAY_I2C
	lcdStartWrite();
#endif
	lcdWriteByte(128 + HAL::readFlashByte((const char*)& LCDLineOffsets[r]), 0); // Position cursor
	char c;
	while ((c = *txt) != 0x00 && col < changeAtCol)
	{
		txt++;
		lcdPutChar(c);
		col++;
	}
	while (col < changeAtCol)
	{
		lcdPutChar(' ');
		col++;
	}
	if (txt2 != NULL)
	{
		while ((c = *txt2) != 0x00 && col < UI_COLS)
		{
			txt2++;
			lcdPutChar(c);
			col++;
		}
		while (col < UI_COLS)
		{
			lcdPutChar(' ');
			col++;
		}
	}
#if UI_DISPLAY_TYPE == DISPLAY_I2C
	lcdStopWrite();
#endif
#if UI_HAS_KEYS==1 && UI_HAS_I2C_ENCODER>0
	uiCheckSlowEncoder();
#endif
}
#endif

#if UI_DISPLAY_TYPE == DISPLAY_ARDUINO_LIB
// Use LiquidCrystal library instead
#include <LiquidCrystal.h>

LiquidCrystal lcd(UI_DISPLAY_RS_PIN, UI_DISPLAY_RW_PIN, UI_DISPLAY_ENABLE_PIN, UI_DISPLAY_D4_PIN, UI_DISPLAY_D5_PIN, UI_DISPLAY_D6_PIN, UI_DISPLAY_D7_PIN);

void UIDisplay::createChar(uint8_t location, const uint8_t charmap[])
{
	location &= 0x7; // we only have 8 locations 0-7
	uint8_t data[8];
	for (int i = 0; i < 8; i++)
	{
		data[i] = pgm_read_byte(&(charmap[i]));
	}
	lcd.createChar(location, data);
}
void UIDisplay::printRow(uint8_t r, char* txt, char* txt2, uint8_t changeAtCol)
{
	changeAtCol = RMath::min(UI_COLS, changeAtCol);
	uint8_t col = 0;
	// Set row
	if (r >= UI_ROWS) return;
	lcd.setCursor(0, r);
	char c;
	while ((c = *txt) != 0x00 && col < changeAtCol)
	{
		txt++;
		lcd.write(c);
		col++;
	}
	while (col < changeAtCol)
	{
		lcd.write(' ');
		col++;
	}
	if (txt2 != NULL)
	{
		while ((c = *txt2) != 0x00 && col < UI_COLS)
		{
			txt2++;
			lcd.write(c);
			col++;
		}
		while (col < UI_COLS)
		{
			lcd.write(' ');
			col++;
		}
	}
#if UI_HAS_KEYS==1 && UI_HAS_I2C_ENCODER>0
	uiCheckSlowEncoder();
#endif
}

void initializeLCD()
{
	lcd.begin(UI_COLS, UI_ROWS);
	uid.lastSwitch = uid.lastRefresh = HAL::timeInMilliseconds();
	uid.createChar(1, character_back);
	uid.createChar(2, character_degree);
	uid.createChar(3, character_selected);
	uid.createChar(4, character_unselected);
}
// ------------------ End LiquidCrystal library as LCD driver
#endif // UI_DISPLAY_TYPE == DISPLAY_ARDUINO_LIB

#if UI_DISPLAY_TYPE == DISPLAY_U8G
//u8glib
#if defined(U8GLIB_ST7920) || defined(U8GLIB_SSD1306_SW_SPI) || defined(MINIPANEL) // marcel lcd || defined(MINIPANEL)
#define UI_SPI_SCK UI_DISPLAY_D4_PIN
#define UI_SPI_MOSI UI_DISPLAY_ENABLE_PIN
#define UI_SPI_CS UI_DISPLAY_RS_PIN
#endif
#include "u8glib_ex.h"
#include "logo.h"

u8g_t u8g;
u8g_uint_t u8_tx = 0, u8_ty = 0;

void u8PrintChar(char c)
{
	switch ((uint8_t)c)
	{
	case 0x7E: // right arrow
		u8g_SetFont(&u8g, u8g_font_6x12_67_75);
		u8_tx += u8g_DrawGlyph(&u8g, u8_tx, u8_ty, 0x52);
		u8g_SetFont(&u8g, UI_FONT_DEFAULT);
		break;
	case CHAR_SELECTOR:
		u8g_SetFont(&u8g, u8g_font_6x12_67_75);
		u8_tx += u8g_DrawGlyph(&u8g, u8_tx, u8_ty, 0xb7);
		u8g_SetFont(&u8g, UI_FONT_DEFAULT);
		break;
	case CHAR_SELECTED:
		u8g_SetFont(&u8g, u8g_font_6x12_67_75);
		u8_tx += u8g_DrawGlyph(&u8g, u8_tx, u8_ty, 0xb6);
		u8g_SetFont(&u8g, UI_FONT_DEFAULT);
		break;
	case 253:      //shift one pixel to right
		u8_tx++;
		break;
	default:
		u8_tx += u8g_DrawGlyph(&u8g, u8_tx, u8_ty, c);
	}
}
void printU8GRow(uint8_t x, uint8_t y, char* text)
{
	char c;
	u8_tx = x;
	u8_ty = y;
	while ((c = *(text++)) != 0) u8PrintChar(c); //version compatible with position adjust
	//        x += u8g_DrawGlyph(&u8g,x,y,c);
}
void UIDisplay::printRow(uint8_t r, char* txt, char* txt2, uint8_t changeAtCol)
{
	changeAtCol = RMath::min(UI_COLS, changeAtCol);
	uint8_t col = 0;
	// Set row
	if (r >= UI_ROWS) return;
	int y = r * UI_FONT_HEIGHT;
	if (!u8g_IsBBXIntersection(&u8g, 0, y, UI_LCD_WIDTH, UI_FONT_HEIGHT + 2)) return; // row not visible
	u8_tx = 0;
	u8_ty = y + UI_FONT_HEIGHT; //set position
	bool highlight = ((uint8_t)(*txt) == CHAR_SELECTOR) || ((uint8_t)(*txt) == CHAR_SELECTED);
	if (highlight)
	{
		u8g_SetColorIndex(&u8g, 1);
		u8g_draw_box(&u8g, 0, y + 1, u8g_GetWidth(&u8g), UI_FONT_HEIGHT + 1);
		u8g_SetColorIndex(&u8g, 0);
	}
	char c;
	while ((c = *(txt++)) != 0 && col < changeAtCol)
	{
		u8PrintChar(c);
		col++;
	}
	if (txt2 != NULL)
	{
		col = changeAtCol;
		u8_tx = col * UI_FONT_WIDTH; //set position
		while ((c = *(txt2++)) != 0 && col < UI_COLS)
		{
			u8PrintChar(c);
			col++;
		}
	}
	if (highlight)
	{
		u8g_SetColorIndex(&u8g, 1);
	}

#if UI_HAS_KEYS==1 && UI_HAS_I2C_ENCODER>0
	uiCheckSlowEncoder();
#endif
}

void initializeLCD()
{
#ifdef U8GLIB_ST7920
	u8g_InitSPI(&u8g, &u8g_dev_st7920_128x64_sw_spi, UI_DISPLAY_D4_PIN, UI_DISPLAY_ENABLE_PIN, UI_DISPLAY_RS_PIN, U8G_PIN_NONE, U8G_PIN_NONE);
#endif
#ifdef U8GLIB_SSD1306_I2C
	u8g_InitI2C(&u8g, &u8g_dev_ssd1306_128x64_i2c, U8G_I2C_OPT_NONE);
#endif
#ifdef U8GLIB_SSD1306_SW_SPI
	u8g_InitSPI(&u8g, &u8g_dev_ssd1306_128x64_sw_spi, UI_DISPLAY_D4_PIN, UI_DISPLAY_ENABLE_PIN, UI_DISPLAY_RS_PIN, U8G_PIN_NONE, U8G_PIN_NONE);
#endif
#ifdef U8GLIB_SH1106_SW_SPI
	u8g_InitSPI(&u8g, &u8g_dev_sh1106_128x64_sw_spi, UI_DISPLAY_D4_PIN, UI_DISPLAY_ENABLE_PIN, UI_DISPLAY_RS_PIN, U8G_PIN_NONE, U8G_PIN_NONE);
#endif
#ifdef U8GLIB_KS0108_FAST
	u8g_Init8Bit(&u8g, &u8g_dev_ks0108_128x64_fast, UI_DISPLAY_D0_PIN, UI_DISPLAY_D1_PIN, UI_DISPLAY_D2_PIN, UI_DISPLAY_D3_PIN, UI_DISPLAY_D4_PIN, UI_DISPLAY_D5_PIN, UI_DISPLAY_D6_PIN, UI_DISPLAY_D7_PIN, UI_DISPLAY_ENABLE_PIN, UI_DISPLAY_CS1, UI_DISPLAY_CS2,
		UI_DISPLAY_DI, UI_DISPLAY_RW_PIN, UI_DISPLAY_RESET_PIN);
#endif
#ifdef U8GLIB_KS0108
	u8g_Init8Bit(&u8g, &u8g_dev_ks0108_128x64, UI_DISPLAY_D0_PIN, UI_DISPLAY_D1_PIN, UI_DISPLAY_D2_PIN, UI_DISPLAY_D3_PIN, UI_DISPLAY_D4_PIN, UI_DISPLAY_D5_PIN, UI_DISPLAY_D6_PIN, UI_DISPLAY_D7_PIN, UI_DISPLAY_ENABLE_PIN, UI_DISPLAY_CS1, UI_DISPLAY_CS2,
		UI_DISPLAY_DI, UI_DISPLAY_RW_PIN, UI_DISPLAY_RESET_PIN);
#endif
#ifdef U8GLIB_ST7565_NHD_C2832_HW_SPI
	u8g_InitHWSPI(&u8g, &u8g_dev_st7565_nhd_c12864_hw_spi, UI_DISPLAY_RS_PIN, UI_DISPLAY_D5_PIN, U8G_PIN_NONE);
#endif
#ifdef U8GLIB_ST7565_NHD_C2832_SW_SPI
	u8g_InitSPI(&u8g, &u8g_dev_st7565_nhd_c12864_sw_spi, UI_DISPLAY_D4_PIN, UI_DISPLAY_ENABLE_PIN, UI_DISPLAY_RS_PIN, UI_DISPLAY_D5_PIN, U8G_PIN_NONE);
#endif
#ifdef MINIPANEL  //marcel lcd
#define DOGLCD_CS PB14
#define DOGLCD_A0 PB12
	u8g_InitHWSPI(&u8g, &u8g_dev_uc1701_mini12864_hw_spi, DOGLCD_CS, DOGLCD_A0, U8G_PIN_NONE);
#define LCD_PIN_RESET PB13
	pinMode(LCD_PIN_RESET, OUTPUT);
	digitalWrite(LCD_PIN_RESET, HIGH);
#endif
	u8g_Begin(&u8g);
#ifdef MINIPANEL
	u8g_SetContrast(&u8g, 160);
#endif
#ifdef UI_ROTATE_180
	u8g_SetRot180(&u8g);
#endif
	u8g_FirstPage(&u8g);
	do
	{
		u8g_SetColorIndex(&u8g, 0);
	} while (u8g_NextPage(&u8g));

	u8g_SetFont(&u8g, UI_FONT_DEFAULT);
	u8g_SetColorIndex(&u8g, 1);
	uid.lastSwitch = uid.lastRefresh = HAL::timeInMilliseconds();
}
// ------------------ End u8GLIB library as LCD driver
#endif // UI_DISPLAY_TYPE == DISPLAY_U8G

#if UI_DISPLAY_TYPE == DISPLAY_GAMEDUINO2
#include "gameduino2.h"
#endif

UIDisplay::UIDisplay()
{
}
#if UI_ANIMATION
void slideIn(uint8_t row, FSTRINGPARAM(text))
{
	char* empty = "";
	int8_t i = 0;
	uid.col = 0;
	uid.addStringP(text);
	uid.printCols[uid.col] = 0;
	for (i = UI_COLS - 1; i >= 0; i--)
	{
		uid.printRow(row, empty, uid.printCols, i);
		HAL::pingWatchdog();
		HAL::delayMilliseconds(10);
	}
}
#endif // UI_ANIMATION
void UIDisplay::initialize()
{
	oldMenuLevel = -2;
#ifdef COMPILE_I2C_DRIVER
	uid.outputMask = UI_DISPLAY_I2C_OUTPUT_START_MASK;
#if UI_DISPLAY_I2C_CHIPTYPE==0 && BEEPER_TYPE==2 && BEEPER_PIN>=0
#if BEEPER_ADDRESS == UI_DISPLAY_I2C_ADDRESS
	uid.outputMask |= BEEPER_PIN;
#endif
#endif
	HAL::i2cInit(UI_I2C_CLOCKSPEED);
#if UI_DISPLAY_I2C_CHIPTYPE==1
	// set direction of pins
	HAL::i2cStart(UI_DISPLAY_I2C_ADDRESS + I2C_WRITE);
	HAL::i2cWrite(0); // IODIRA
	HAL::i2cWrite(~(UI_DISPLAY_I2C_OUTPUT_PINS & 255));
	HAL::i2cWrite(~(UI_DISPLAY_I2C_OUTPUT_PINS >> 8));
	HAL::i2cStop();
	// Set pullups according to  UI_DISPLAY_I2C_PULLUP
	HAL::i2cStart(UI_DISPLAY_I2C_ADDRESS + I2C_WRITE);
	HAL::i2cWrite(0x0C); // GPPUA
	HAL::i2cWrite(UI_DISPLAY_I2C_PULLUP & 255);
	HAL::i2cWrite(UI_DISPLAY_I2C_PULLUP >> 8);
	HAL::i2cStop();
#endif

#endif
	flags = 0;
	menuLevel = 0;
	shift = -2;
	menuPos[0] = 0;
	lastAction = 0;
	delayedAction = 0;
	lastButtonAction = 0;
	activeAction = 0;
	statusMsg[0] = 0;
	uiInitKeys();
	cwd[0] = '/';
	cwd[1] = 0;
	folderLevel = 0;
	UI_STATUS_F(Com::translatedF(UI_TEXT_CLICK_START_ID));
#if UI_DISPLAY_TYPE != NO_DISPLAY
	initializeLCD();
#if defined(USER_KEY1_PIN) && USER_KEY1_PIN > -1
	UI_KEYS_INIT_BUTTON_LOW(USER_KEY1_PIN);
#endif
#if defined(USER_KEY2_PIN) && USER_KEY2_PIN > -1
	UI_KEYS_INIT_BUTTON_LOW(USER_KEY2_PIN);
#endif
#if defined(USER_KEY3_PIN) && USER_KEY3_PIN > -1
	UI_KEYS_INIT_BUTTON_LOW(USER_KEY3_PIN);
#endif
#if defined(USER_KEY4_PIN) && USER_KEY4_PIN > -1
	UI_KEYS_INIT_BUTTON_LOW(USER_KEY4_PIN);
#endif
#if UI_DISPLAY_TYPE == DISPLAY_I2C
	// I don't know why but after power up the lcd does not come up
	// but if I reinitialize i2c and the lcd again here it works.
	HAL::delayMilliseconds(10);
	HAL::i2cInit(UI_I2C_CLOCKSPEED);
	// set direction of pins
	HAL::i2cStart(UI_DISPLAY_I2C_ADDRESS + I2C_WRITE);
	HAL::i2cWrite(0); // IODIRA
	HAL::i2cWrite(~(UI_DISPLAY_I2C_OUTPUT_PINS & 255));
	HAL::i2cWrite(~(UI_DISPLAY_I2C_OUTPUT_PINS >> 8));
	HAL::i2cStop();
	// Set pullups according to  UI_DISPLAY_I2C_PULLUP
	HAL::i2cStart(UI_DISPLAY_I2C_ADDRESS + I2C_WRITE);
	HAL::i2cWrite(0x0C); // GPPUA
	HAL::i2cWrite(UI_DISPLAY_I2C_PULLUP & 255);
	HAL::i2cWrite(UI_DISPLAY_I2C_PULLUP >> 8);
	HAL::i2cStop();
	initializeLCD();
#endif
#if UI_DISPLAY_TYPE == DISPLAY_GAMEDUINO2
	GD2::startScreen();
#else
#if UI_ANIMATION==false || UI_DISPLAY_TYPE == DISPLAY_U8G
#if UI_DISPLAY_TYPE == DISPLAY_U8G
	//u8g picture loop
	u8g_FirstPage(&u8g);
	do
	{
		u8g_DrawBitmapP(&u8g, /*128 - LOGO_WIDTH*/2, 5, ((125) / 8)/*((LOGO_WIDTH + 7) / 8)*/, 43, logo); // marcel logo
		u8g_draw_vline(&u8g, 124, 26, 14);
		u8g_draw_vline(&u8g, 125, 26, 14);
		for (uint8_t y = 0; y < UI_ROWS; y++) displayCache[y][0] = 0;
#ifdef CUSTOM_LOGO
		printRowP(4, PSTR("Repetier"));
		printRowP(5, PSTR("Ver " REPETIER_VERSION));
#else
		/*printRowP(0, PSTR("Repetier"));
		  printRowP(1, PSTR("Ver " REPETIER_VERSION));
		  printRowP(3, PSTR("Machine:"));
		  printRowP(4, PSTR(UI_PRINTER_NAME));
		  printRowP(5, PSTR(UI_PRINTER_COMPANY));*/
#endif
	} while (u8g_NextPage(&u8g)); //end picture loop
#else // not DISPLAY_U8G
	for (uint8_t y = 0; y < UI_ROWS; y++) displayCache[y][0] = 0;
	printRowP(0, versionString);
	printRowP(1, PSTR(UI_PRINTER_NAME));
#if UI_ROWS>2
	printRowP(UI_ROWS - 1, PSTR(UI_PRINTER_COMPANY));
#endif
#endif
#else
	slideIn(0, versionString);
	strcpy(displayCache[0], uid.printCols);
	slideIn(1, PSTR(UI_PRINTER_NAME));
	strcpy(displayCache[1], uid.printCols);
#if UI_ROWS>2
	slideIn(UI_ROWS - 1, PSTR(UI_PRINTER_COMPANY));
	strcpy(displayCache[UI_ROWS - 1], uid.printCols);
#endif
#endif
#endif // gameduino2
	HAL::delayMilliseconds(UI_START_SCREEN_DELAY);
#endif
#if defined(UI_DISPLAY_I2C_CHIPTYPE) && UI_DISPLAY_I2C_CHIPTYPE==0 && (BEEPER_TYPE==2 || defined(UI_HAS_I2C_KEYS))
	// Make sure the beeper is off
	HAL::i2cStartWait(UI_I2C_KEY_ADDRESS + I2C_WRITE);
	HAL::i2cWrite(255); // Disable beeper, enable read for other pins.
	HAL::i2cStop();
#endif
}
#if UI_DISPLAY_TYPE == DISPLAY_4BIT || UI_DISPLAY_TYPE == DISPLAY_8BIT || UI_DISPLAY_TYPE == DISPLAY_I2C
void UIDisplay::createChar(uint8_t location, const uint8_t charmap[])
{
	location &= 0x7; // we only have 8 locations 0-7
	lcdCommand(LCD_SETCGRAMADDR | (location << 3));
	for (int i = 0; i < 8; i++)
	{
		lcdPutChar(pgm_read_byte(&(charmap[i])));
	}
}
#endif
void  UIDisplay::waitForKey()
{
	uint16_t nextAction = 0;

	lastButtonAction = 0;
	while (lastButtonAction == nextAction)
	{
		uiCheckSlowKeys(nextAction);
	}
}

void UIDisplay::printRowP(uint8_t r, PGM_P txt)
{
	if (r >= UI_ROWS) return;
	col = 0;
	addStringP(txt);
	uid.printCols[col] = 0;
	printRow(r, uid.printCols, NULL, UI_COLS);
}
void UIDisplay::addInt(int value, uint8_t digits, char fillChar)
{
	uint8_t dig = 0, neg = 0;
	if (value < 0)
	{
		value = -value;
		neg = 1;
		dig++;
	}
	char buf[7]; // Assumes 8-bit chars plus zero byte.
	char* str = &buf[6];
	buf[6] = 0;
	do
	{
		unsigned int m = value;
		value /= 10;
		char c = m - 10 * value;
		*--str = c + '0';
		dig++;
	} while (value);
	if (neg)
		uid.printCols[col++] = '-';
	if (digits < 6)
		while (dig < digits)
		{
			*--str = fillChar; //' ';
			dig++;
		}
	while (*str && col < MAX_COLS)
	{
		uid.printCols[col++] = *str;
		str++;
	}
}
void UIDisplay::addLong(long value, int8_t digits)
{
	uint8_t dig = 0, neg = 0;
	byte addspaces = digits > 0;
	if (digits < 0) digits = -digits;
	if (value < 0)
	{
		neg = 1;
		value = -value;
		dig++;
	}
	char buf[13]; // Assumes 8-bit chars plus zero byte.
	char* str = &buf[12];
	buf[12] = 0;
	do
	{
		unsigned long m = value;
		value /= 10;
		char c = m - 10 * value;
		*--str = c + '0';
		dig++;
	} while (value);
	if (neg)
		uid.printCols[col++] = '-';
	if (addspaces && digits <= 11)
		while (dig < digits)
		{
			*--str = ' ';
			dig++;
		}
	while (*str && col < MAX_COLS)
	{
		uid.printCols[col++] = *str;
		str++;
	}
}

const float roundingTable[] PROGMEM = { 0.5, 0.05, 0.005, 0.0005 };

UI_STRING(ui_selected, UI_TEXT_SEL);
UI_STRING(ui_unselected, UI_TEXT_NOSEL);

void UIDisplay::addFloat(float number, char fixdigits, uint8_t digits)
{
	// Handle negative numbers
	if (number < 0.0)
	{
		uid.printCols[col++] = '-';
		if (col >= MAX_COLS) return;
		number = -number;
		fixdigits--;
	}
	number += pgm_read_float(&roundingTable[digits]); // for correct rounding

	// Extract the integer part of the number and print it
	unsigned long int_part = (unsigned long)number;
	float remainder = number - (float)int_part;
	addLong(int_part, fixdigits);
	if (col >= UI_COLS) return;

	// Print the decimal point, but only if there are digits beyond
	if (digits > 0)
	{
		uid.printCols[col++] = '.';
	}

	// Extract digits from the remainder one at a time
	while (col < MAX_COLS && digits-- > 0)
	{
		remainder *= 10.0;
		uint8_t toPrint = uint8_t(remainder);
		uid.printCols[col++] = '0' + toPrint;
		remainder -= toPrint;
	}
}

void UIDisplay::addStringP(FSTRINGPARAM(text))
{
	while (col < MAX_COLS)
	{
		uint8_t c = HAL::readFlashByte(text++);
		if (c == 0) return;
		uid.printCols[col++] = c;
	}
}

void UIDisplay::addStringOnOff(uint8_t on)
{
	addStringP(on ? Com::translatedF(UI_TEXT_ON_ID) : Com::translatedF(UI_TEXT_OFF_ID));
}

void UIDisplay::addChar(const char c)
{
	if (col < UI_COLS)
	{
		uid.printCols[col++] = c;
	}
}
void UIDisplay::addGCode(GCode * code)
{
	// assume volatile and make copy so we dont "see" multple code lines as we go.
	//GCode myCode = *code; insuffeicnet memory for this safety check
	//code = &myCode;
	addChar('#');
	addLong(code->N);
	if (code->hasM())
	{
		addChar('M');
		addLong((long)code->M);
	}
	if (code->hasG())
	{
		addChar('G');
		addLong((long)code->G);
	}
	if (code->hasT())
	{
		addChar('T');
		addLong((long)code->T);
	}
	if (code->hasX())
	{
		addChar('X');
		addFloat(code->X);
	}
	if (code->hasY())
	{
		addChar('Y');
		addFloat(code->Y);
	}
	if (code->hasZ())
	{
		addChar('Z');
		addFloat(code->Z);
	}
	if (code->hasE())
	{
		addChar('E');
		addFloat(code->E);
	}
	if (code->hasF())
	{
		addChar('F');
		addFloat(code->F);
	}
	if (code->hasS())
	{
		addChar('S');
		addLong(code->S);
	}
	if (code->hasP())
	{
		addChar('P');
		addLong(code->P);
	}
#ifdef ARC_SUPPORT
	if (code->hasI())
	{
		addChar('I');
		addFloat(code->I);
	}
	if (code->hasJ())
	{
		addChar('J');
		addFloat(code->J);
	}
	if (code->hasR())
	{
		addChar('R');
		addFloat(code->R);
	}
#endif
	// cannot print string, it isnt part of the gcode structure.
	//it points to temp memory in a buffer.
	//if(code->hasSTRING())
}

void UIDisplay::parse(const char* txt, bool ram)
{
	static uint8_t beepdelay = 0;
	int ivalue = 0;
	float fvalue = 0;
	while (col < MAX_COLS)
	{
		char c = (ram ? *(txt++) : pgm_read_byte(txt++));
		if (c == 0) break; // finished
		if (c != '%')
		{
			uid.printCols[col++] = c;
			continue;
		}
		// dynamic parameter, parse meaning and replace
		char c1 = (ram ? *(txt++) : pgm_read_byte(txt++));
		char c2 = (ram ? *(txt++) : pgm_read_byte(txt++));
		switch (c1)
		{
		case '%':
		{
			// print % for input '%%' or '%%%'
			if (col < UI_COLS) uid.printCols[col++] = '%'; // if data = '%%?' escaped percent, with left over ? char
			if (c2 != '%') txt--; // Be flexible and accept 2 or 3 chars
			break;
		} // case '%'

		case '?': // conditional spacer or other char
		{
			// If something has been printed, check if the last char is c2.
			// if not, append c2.
			// otherwise do nothing.
			if (col > 0 && col < UI_COLS)
			{
				if (uid.printCols[col - 1] != c2) uid.printCols[col++] = c2;
			}
			break;
		}
		case 'a': // Acceleration settings
			if (c2 >= 'x' && c2 <= 'z')       addFloat(Printer::maxAccelerationMMPerSquareSecond[c2 - 'x'], 5, 0);
			else if (c2 >= 'X' && c2 <= 'Z') addFloat(Printer::maxTravelAccelerationMMPerSquareSecond[c2 - 'X'], 5, 0);
			else if (c2 == 'j') addFloat(Printer::maxJerk, 3, 1);
#if DRIVE_SYSTEM != DELTA
			else if (c2 == 'J') addFloat(Printer::maxZJerk, 3, 1);
#endif
			break;
		case 'B':
			if (c2 == 'C')	 //Custom coating
			{
				addFloat(Printer::zBedOffset, 3, 2);
				break;
			}
			break;
		case 'd':  // debug boolean
			if (c2 == 'o') addStringOnOff(Printer::debugEcho());
			if (c2 == 'i') addStringOnOff(Printer::debugInfo());
			if (c2 == 'e') addStringOnOff(Printer::debugErrors());
			if (c2 == 'd') addStringOnOff(Printer::debugDryrun());
			if (c2 == 'p') addStringOnOff(Printer::debugEndStop());
			if (c2 == 'x') addStringP(Endstops::xMin() ? ui_selected : ui_unselected);
			if (c2 == 'X') addStringP(Endstops::xMax() ? ui_selected : ui_unselected);
			if (c2 == 'y') addStringP(Endstops::yMin() ? ui_selected : ui_unselected);
			if (c2 == 'Y') addStringP(Endstops::yMax() ? ui_selected : ui_unselected);
			//if (c2 == 'z') addStringP(Endstops::zMin() ? ui_selected : ui_unselected);
			if (c2 == 'Z') addStringP(Endstops::zMax() ? ui_selected : ui_unselected);
			break;
		case 'D':
#if FEATURE_DITTO_PRINTING
			if (c2 >= '0' && c2 <= '9')
			{
				addStringP(Extruder::dittoMode == c2 - '0' ? ui_selected : ui_unselected);
			}
#endif
			break;
		case 'e': // Extruder temperature
		{
			if (c2 == 'I')
			{
				//give integer display
				//char c2 = (ram ? *(txt++) : pgm_read_byte(txt++));
				txt++; // just skip c sign
				ivalue = 0;
			}
			else ivalue = UI_TEMP_PRECISION;

			if (c2 == 'r')  // Extruder relative mode
			{
				addStringP(Printer::relativeExtruderCoordinateMode ? Com::translatedF(UI_TEXT_YES_ID) : Com::translatedF(UI_TEXT_NO_ID));
				break;
			}
#if FEATURE_DITTO_PRINTING
			if (c2 == 'd') { // ditto copy mode
				addInt(Extruder::dittoMode, 1, ' ');
				break;
			}
#endif
#if NUM_TEMPERATURE_LOOPS > 0
			uint8_t eid = NUM_EXTRUDER;    // default = BED if c2 not specified extruder number
			if (c2 == 'c') eid = Extruder::current->id;
			else if (c2 >= '0' && c2 <= '9') eid = c2 - '0';
			if (Printer::isAnyTempsensorDefect())
			{
				if (eid == 0 && ++beepdelay > 30) beepdelay = 0; // beep every 30 seconds
				if (beepdelay == 1) //BEEP_LONG;
				if (tempController[eid]->isSensorDefect())
				{
					addStringP(PSTR(" def "));
					break;
				}
				else if (tempController[eid]->isSensorDecoupled())
				{
					addStringP(PSTR(" dec "));
					break;
				}
			}
#if EXTRUDER_JAM_CONTROL
			if (tempController[eid]->isJammed())
			{
				if (++beepdelay > 10) beepdelay = 0; // beep every 10 seconds
				if (beepdelay == 1) //BEEP_LONG;
				addStringP(PSTR(" jam "));
				break;
			}
#endif
#endif
			if (c2 == 'c') fvalue = Extruder::current->tempControl.currentTemperatureC;
			else if (c2 >= '0' && c2 <= '9') fvalue = extruder[c2 - '0'].tempControl.currentTemperatureC;
			else if (c2 == 'b') fvalue = Extruder::getHeatedBedTemperature();
			else if (c2 == 'B')
			{
				ivalue = 0;
				fvalue = Extruder::getHeatedBedTemperature();
			}
			addFloat(fvalue, 3, ivalue);
			break;
		}
		case 'E': // Target extruder temperature
			if (c2 == 'c') fvalue = Extruder::current->tempControl.targetTemperatureC;
			else if (c2 >= '0' && c2 <= '9') fvalue = extruder[c2 - '0'].tempControl.targetTemperatureC;
#if HAVE_HEATED_BED
			else if (c2 == 'b') fvalue = heatedBedController.targetTemperatureC;
#endif
			addFloat(fvalue, 3, 0 /*UI_TEMP_PRECISION*/);
			break;
#if FAN_PIN > -1 && FEATURE_FAN_CONTROL
		case 'F': // FAN speed
			if (c2 == 's') addInt(floor(Printer::getFanSpeed() * 100 / 255 + 0.5f), 3);
			if (c2 == 'i') addStringP((Printer::flag2 & PRINTER_FLAG2_IGNORE_M106_COMMAND) ? ui_selected : ui_unselected);
			break;
#endif
		case 'f':
			if (c2 >= 'x' && c2 <= 'z') addFloat(Printer::maxFeedrate[c2 - 'x'], 5, 0);
			else if (c2 >= 'X' && c2 <= 'Z') addFloat(Printer::homingFeedrate[c2 - 'X'], 5, 0);
			break;
		case 'i':
			if (c2 == 's') addInt(stepperInactiveTime / 60000, 3);
			else if (c2 == 'p') addInt(maxInactiveTime / 60000, 3);
			break;
		case 'O': // ops related stuff
			break;
		case 'l':
			if (c2 == 'a') addInt(lastAction, 4);
#if defined(CASE_LIGHTS_PIN) && CASE_LIGHTS_PIN >= 0
			else if (c2 == 'o') addStringOnOff(READ(CASE_LIGHTS_PIN));       // Lights on/off
#endif
#if FEATURE_AUTOLEVEL
			else if (c2 == 'l') addStringOnOff((Printer::isAutolevelActive()));       // Autolevel on/off
#endif
			break;
		case 'o':
			if (c2 == 's')
			{
#if SDSUPPORT
				if (sd.sdactive && sd.sdmode)
				{
					//addStringP(Com::translatedF(UI_TEXT_PRINT_POS_ID));
					if (preHeatChoco == 0) {
						addStringP(Com::translatedF(UI_TEXT_PRINT_POS_ID));
						int sizeName = 30;
						for (int j = 0; j < sizeof(fileNameLCD); j++) {
							if (fileNameLCD[j] == '.') {
								sizeName -= 30 - j;
							}
						}
						for (int i = 0; i < sizeName; i++) {
							addChar(fileNameLCD[i]);
              //fileNameLCDAsk[i] = fileNameLCD[i]; 
						}
						float percent;
						if (sd.filesize < 2000000) percent = sd.sdpos * 100.0 / sd.filesize;
						else percent = (sd.sdpos >> 8) * 100.0 / (sd.filesize >> 8);
						addFloat(percent, 3, 1);
						if (col < MAX_COLS)
							uid.printCols[col++] = '%';
					}
				}
				else

					/*#if SDSUPPORT
							if (sd.sdactive && sd.sdmode && Printer::msecondsSession != 0)
							{
										addStringP(Com::translatedF(UI_TEXT_PRINT_POS_ID));

							  long seconds = (HAL::timeInMilliseconds() - Printer::msecondsSession) / 1000;
							  long tmp = seconds / 3600;
							  addInt(tmp,2);
							  addStringP(Com::translatedF(UI_TEXT_PRINTTIME_HOURS_ID));
							  seconds -= tmp * 3600;
							  tmp = seconds / 60;
							  addInt(tmp,2,'0');
							  addStringP(Com::translatedF(UI_TEXT_PRINTTIME_MINUTES_ID));
							}
							else*/
#endif
							//if(preHeatChoco == 0){
					parse(statusMsg, true);
				//}
				break;
			}
			if (c2 == 'c')
			{
				addLong(baudrate, 6);
				break;
			}
			if (c2 == 'e')
			{
				if (errorMsg != 0) addStringP((char PROGMEM*)errorMsg);
				break;
			}
			if (c2 == 'B')
			{
				addInt((int)PrintLine::linesCount, 2);
				break;
			}
			if (c2 == 'f')
			{
				addInt(Printer::extrudeMultiply, 3);
				break;
			}
			if (c2 == 'm')
			{
				addInt(Printer::feedrateMultiply, 3);
				break;
			}
			if (c2 == 'n')
			{
				addInt(Extruder::current->id + 1, 1);
				break;
			}
#if FEATURE_SERVO > 0 && UI_SERVO_CONTROL > 0
			if (c2 == 'S')
			{
				addInt(servoPosition, 4);
				break;
			}
#endif
#if FEATURE_BABYSTEPPING
			if (c2 == 'Y')
			{
				//                addInt(zBabySteps,0);
				addFloat((float)zBabySteps * Printer::invAxisStepsPerMM[Z_AXIS], 2, 2);
				break;
			}
#endif
			// Extruder output level
			if (c2 >= '0' && c2 <= '9') ivalue = pwm_pos[c2 - '0'];
#if HAVE_HEATED_BED
			else if (c2 == 'b') ivalue = pwm_pos[heatedBedController.pwmIndex];
#endif
			else if (c2 == 'C') ivalue = pwm_pos[Extruder::current->id];
			ivalue = (ivalue * 100) / 255;
			addInt(ivalue, 3);
			if (col < MAX_COLS)
				uid.printCols[col++] = '%';
			break;
		case 's': // Endstop positions
			if (c2 == 'x')
			{
#if (X_MIN_PIN > -1) && MIN_HARDWARE_ENDSTOP_X
				addStringOnOff(Endstops::xMin());
#else
				addStringP(Com::translatedF(UI_TEXT_NA_ID));
#endif
			}
			if (c2 == 'X')
#if (X_MAX_PIN > -1) && MAX_HARDWARE_ENDSTOP_X
				addStringOnOff(Endstops::xMax());
#else
				addStringP(Com::translatedF(UI_TEXT_NA_ID));
#endif
			if (c2 == 'y')
#if (Y_MIN_PIN > -1)&& MIN_HARDWARE_ENDSTOP_Y
				addStringOnOff(Endstops::yMin());
#else
				addStringP(Com::translatedF(UI_TEXT_NA_ID));
#endif
			if (c2 == 'Y')
#if (Y_MAX_PIN > -1) && MAX_HARDWARE_ENDSTOP_Y
				addStringOnOff(Endstops::yMax());
#else
				addStringP(Com::translatedF(UI_TEXT_NA_ID));
#endif
			if (c2 == 'z')
#if (Z_MIN_PIN > -1) && MIN_HARDWARE_ENDSTOP_Z
				addStringOnOff(Endstops::zMin());
#else
				addStringP(Com::translatedF(UI_TEXT_NA_ID));
#endif
			if (c2 == 'Z')
#if (Z_MAX_PIN > -1) && MAX_HARDWARE_ENDSTOP_Z
				addStringOnOff(Endstops::zMax());
#else
				addStringP(Com::translatedF(UI_TEXT_NA_ID));
#endif
			if (c2 == 'P')
#if (Z_PROBE_PIN > -1)
				addStringOnOff(Endstops::zProbe());
#else
				addStringP(Com::translatedF(UI_TEXT_NA_ID));
#endif
			break;
		case 'S':
			if (c2 >= 'x' && c2 <= 'z') addFloat(Printer::axisStepsPerMM[c2 - 'x'], 3, 1);
			if (c2 == 'e') addFloat(Extruder::current->stepsPerMM, 3, 1);
			break;
		case 'T': // Print offsets
			if (c2 == '2')
				addFloat(-Printer::coordinateOffset[Z_AXIS], 2, 2);
			else
				addFloat(-Printer::coordinateOffset[c2 - '0'], 4, 0);
			break;
		case 'U':
			if (c2 == 't')  // Printing time
			{
#if EEPROM_MODE
				bool alloff = true;
#if NUM_TEMPERATURE_LOOPS > 0
				for (uint8_t i = 0; i < NUM_EXTRUDER; i++)
					if (tempController[i]->targetTemperatureC > 15) alloff = false;
#endif
				long seconds = (alloff ? 0 : (HAL::timeInMilliseconds() - Printer::msecondsPrinting) / 1000) + HAL::eprGetInt32(EPR_PRINTING_TIME);
				long tmp = seconds / 86400;
				seconds -= tmp * 86400;
				addInt(tmp, 5);
				addStringP(Com::translatedF(UI_TEXT_PRINTTIME_DAYS_ID));
				tmp = seconds / 3600;
				addInt(tmp, 2);
				addStringP(Com::translatedF(UI_TEXT_PRINTTIME_HOURS_ID));
				seconds -= tmp * 3600;
				tmp = seconds / 60;
				addInt(tmp, 2, '0');
				addStringP(Com::translatedF(UI_TEXT_PRINTTIME_MINUTES_ID));
#endif
			}
			else if (c2 == 'f')    // Filament usage
			{
#if EEPROM_MODE
				float dist = Printer::filamentPrinted * 0.001 + HAL::eprGetFloat(EPR_PRINTING_DISTANCE);
#else
				float dist = Printer::filamentPrinted * 0.001;
#endif
				addFloat(dist, 6, 1);
			}
			break;

		case 'x':
			if (c2 >= '0' && c2 <= '4')
			{
				if (c2 == '4') // this sequence save 14 bytes of flash
				{
					addFloat(Printer::filamentPrinted * 0.001, 3, 2);
					break;
				}
				if (c2 == '0')
					fvalue = Printer::realXPosition();
				else if (c2 == '1')
					fvalue = Printer::realYPosition();
				else if (c2 == '2')
					fvalue = Printer::realZPosition();
				else
					fvalue = (float)Printer::currentPositionSteps[E_AXIS] * Printer::invAxisStepsPerMM[E_AXIS];
				addFloat(fvalue, 4, 2);
			}
			break;

		case 'X': // Extruder related
#if NUM_EXTRUDER>0
			if (c2 >= '0' && c2 <= '9')
			{
				addStringP(Extruder::current->id == c2 - '0' ? ui_selected : ui_unselected);
			}
#if TEMP_PID
			else if (c2 == 'i')
			{
				addFloat(currHeaterForSetup->pidIGain, 4, 2);
			}
			else if (c2 == 'p')
			{
				addFloat(currHeaterForSetup->pidPGain, 4, 2);
			}
			else if (c2 == 'd')
			{
				addFloat(currHeaterForSetup->pidDGain, 4, 2);
			}
			else if (c2 == 'm')
			{
				addInt(currHeaterForSetup->pidDriveMin, 3);
			}
			else if (c2 == 'M')
			{
				addInt(currHeaterForSetup->pidDriveMax, 3);
			}
			else if (c2 == 'D')
			{
				addInt(currHeaterForSetup->pidMax, 3);
			}
#endif
			else if (c2 == 'w')
			{
				addInt(Extruder::current->watchPeriod, 4);
			}
#if RETRACT_DURING_HEATUP
			else if (c2 == 'T')
			{
				addInt(Extruder::current->waitRetractTemperature, 4);
			}
			else if (c2 == 'U')
			{
				addInt(Extruder::current->waitRetractUnits, 2);
			}
#endif
			else if (c2 == 'h')
			{
				uint8_t hm = currHeaterForSetup->heatManager;
				if (hm == HTR_PID)
					addStringP(Com::translatedF(UI_TEXT_STRING_HM_PID_ID));
				else if (hm == HTR_DEADTIME)
					addStringP(Com::translatedF(UI_TEXT_STRING_HM_DEADTIME_ID));
				else if (hm == HTR_SLOWBANG)
					addStringP(Com::translatedF(UI_TEXT_STRING_HM_SLOWBANG_ID));
				else
					addStringP(Com::translatedF(UI_TEXT_STRING_HM_BANGBANG_ID));
			}
#if USE_ADVANCE
#if ENABLE_QUADRATIC_ADVANCE
			else if (c2 == 'a')
			{
				addFloat(Extruder::current->advanceK, 3, 0);
			}
#endif
			else if (c2 == 'l')
			{
				addFloat(Extruder::current->advanceL, 3, 0);
			}
#endif
			else if (c2 == 'x')
			{
				addFloat(Extruder::current->xOffset * Printer::invAxisStepsPerMM[X_AXIS], 3, 2);
			}
			else if (c2 == 'y')
			{
				addFloat(Extruder::current->yOffset * Printer::invAxisStepsPerMM[Y_AXIS], 3, 2);
			}
			else if (c2 == 'f')
			{
				addFloat(Extruder::current->maxStartFeedrate, 5, 0);
			}
			else if (c2 == 'F')
			{
				addFloat(Extruder::current->maxFeedrate, 5, 0);
			}
			else if (c2 == 'A')
			{
				addFloat(Extruder::current->maxAcceleration, 5, 0);
			}
#endif
			break;
		case 'y':
#if DRIVE_SYSTEM == DELTA
			if (c2 >= '0' && c2 <= '3') fvalue = (float)Printer::currentNonlinearPositionSteps[c2 - '0'] * Printer::invAxisStepsPerMM[c2 - '0'];
			addFloat(fvalue, 3, 2);
#endif
			break;
		case 'z':
#if EEPROM_MODE != 0 && FEATURE_Z_PROBE
			if (c2 == 'h') { // write z probe height
				addFloat(EEPROM::zProbeHeight(), 3, 2);
				break;
			}
#endif
			if (c2 == '2')
				addFloat(-Printer::coordinateOffset[Z_AXIS], 2, 2);
			else
				addFloat(-Printer::coordinateOffset[c2 - '0'], 4, 0);
			break;
		}
	}
	uid.printCols[col] = 0;
}
void UIDisplay::showLanguageSelectionWizard() {
#if EEPROM_MODE != 0
	pushMenu(&ui_menu_languages_wiz, true);
#endif
}
void UIDisplay::setStatusP(PGM_P txt, bool error)
{
	if (!error && Printer::isUIErrorMessage()) return;
	uint8_t i = 0;
	while (i < 20)
	{
		uint8_t c = pgm_read_byte(txt++);
		if (!c) break;
		statusMsg[i++] = c;
	}
	statusMsg[i] = 0;
	if (error)
		Printer::setUIErrorMessage(true);
}
void UIDisplay::setStatus(const char* txt, bool error)
{
	if (!error && Printer::isUIErrorMessage()) return;
	uint8_t i = 0;
	while (*txt && i < 20)
		statusMsg[i++] = *txt++;
	statusMsg[i] = 0;
	if (error)
		Printer::setUIErrorMessage(true);
}

const UIMenu* const ui_pages[UI_NUM_PAGES] PROGMEM = UI_PAGES;
uint16_t nFilesOnCard;
void UIDisplay::updateSDFileCount()
{
#if SDSUPPORT
	dir_t* p = NULL;
	SdBaseFile* root = sd.fat.vwd();

	root->rewind();
	nFilesOnCard = 0;
	while ((p = root->getLongFilename(p, NULL, 0, NULL)))
	{
		if (!(DIR_IS_FILE(p) || DIR_IS_SUBDIR(p)))
			continue;
		if (folderLevel >= SD_MAX_FOLDER_DEPTH && DIR_IS_SUBDIR(p) && !(p->name[0] == '.' && p->name[1] == '.'))
			continue;
		nFilesOnCard++;
		if (nFilesOnCard > 5000) // Arbitrary maximum, limited only by how long someone would scroll
			return;
	}
#endif
}

void getSDFilenameAt(uint16_t filePos, char* filename)
{
#if SDSUPPORT
	dir_t* p = NULL;
	SdBaseFile* root = sd.fat.vwd();

	root->rewind();
	while ((p = root->getLongFilename(p, tempLongFilename, 0, NULL)) != NULL)
	{
		HAL::pingWatchdog();
		if (!DIR_IS_FILE(p) && !DIR_IS_SUBDIR(p)) continue;
		if (uid.folderLevel >= SD_MAX_FOLDER_DEPTH && DIR_IS_SUBDIR(p) && !(p->name[0] == '.' && p->name[1] == '.')) continue;
		if (filePos--)
			continue;
		strcpy(filename, tempLongFilename);
		if (DIR_IS_SUBDIR(p)) strcat(filename, "/"); // Set marker for directory

		break;
	}
#endif
}

bool UIDisplay::isDirname(char* name)
{
	while (*name) name++;
	name--;
	return *name == '/';
}

void UIDisplay::goDir(char* name)
{
#if SDSUPPORT
	char* p = cwd;
	while (*p)p++;
	if (name[0] == '.' && name[1] == '.')
	{
		if (folderLevel == 0) return;
		p--;
		p--;
		while (*p != '/') p--;
		p++;
		*p = 0;
		folderLevel--;
	}
	else
	{
		if (folderLevel >= SD_MAX_FOLDER_DEPTH) return;
		while (*name) * p++ = *name++;
		*p = 0;
		folderLevel++;
	}
	sd.fat.chdir(cwd);
	updateSDFileCount();
#endif
}
/** write file names at current position to lcd */
void sdrefresh(uint16_t & r, char cache[UI_ROWS][MAX_COLS + 1])
{
#if SDSUPPORT
	dir_t* p = NULL;
	uint16_t offset = uid.menuTop[uid.menuLevel];
	SdBaseFile* root;
	uint16_t length, skip;

	sd.fat.chdir(uid.cwd);
	root = sd.fat.vwd();
	root->rewind();

	skip = (offset > 0 ? offset - 1 : 0);

	while (r + offset < nFilesOnCard + 1 && r < UI_ROWS && (p = root->getLongFilename(p, tempLongFilename, 0, NULL)))
	{
		HAL::pingWatchdog();
		// done if past last used entry
		// skip deleted entry and entries for . and  ..
		// only list subdirectories and files
		if ((DIR_IS_FILE(p) || DIR_IS_SUBDIR(p)))
		{
			if (uid.folderLevel >= SD_MAX_FOLDER_DEPTH && DIR_IS_SUBDIR(p) && !(p->name[0] == '.' && p->name[1] == '.'))
				continue;
			if (skip > 0)
			{
				skip--;
				continue;
			}
			uid.col = 0;
			if (r + offset == uid.menuPos[uid.menuLevel])
				uid.printCols[uid.col++] = CHAR_SELECTOR;
			else
				uid.printCols[uid.col++] = ' ';
			// print file name with possible blank fill
			if (DIR_IS_SUBDIR(p))
				uid.printCols[uid.col++] = bFOLD; // Prepend folder symbol
			length = RMath::min((int)strlen(tempLongFilename), MAX_COLS - uid.col);
			memcpy(uid.printCols + uid.col, tempLongFilename, length);
			uid.col += length;
			uid.printCols[uid.col] = 0;
			strcpy(cache[r++], uid.printCols);
		}
	}
#endif
}

int stat = 0;

// Refresh current menu page
void UIDisplay::refreshPage()
{
	if (stat == 1)
		stat = 0;
	else
		stat = 1;
	Endstops::update();
#if  UI_DISPLAY_TYPE == DISPLAY_GAMEDUINO2
	GD2::refresh();
#else
	uint16_t r;
	uint8_t mtype = UI_MENU_TYPE_INFO;
	char cache[UI_ROWS][MAX_COLS + 1];
	adjustMenuPos();
#if UI_AUTORETURN_TO_MENU_AFTER != 0
	// Reset timeout on menu back when user active on menu
	if (uid.encoderLast != encoderStartScreen)
		ui_autoreturn_time = HAL::timeInMilliseconds() + UI_AUTORETURN_TO_MENU_AFTER;
#endif
	encoderStartScreen = uid.encoderLast;
	// Copy result into cache
	Endstops::update();
	if (menuLevel == 0) // Top level menu
	{
		UIMenu* men = (UIMenu*)pgm_read_word(&(ui_pages[menuPos[0]]));
		uint16_t nr = pgm_read_word_near(&(men->numEntries));
		UIMenuEntry** entries = (UIMenuEntry * *)pgm_read_word(&(men->entries));
		for (r = 0; r < nr && r < UI_ROWS; r++)
		{
			UIMenuEntry* ent = (UIMenuEntry*)pgm_read_word(&(entries[r]));
			col = 0;
			char* text = (char*)pgm_read_word(&(ent->text));
			if (text == NULL)
				text = (char*)Com::translatedF(pgm_read_word(&(ent->translation)));
			parse(text, false);
			strcpy(cache[r], uid.printCols);
		}
	}
	else
	{
		UIMenu* men = (UIMenu*)menu[menuLevel];
		uint16_t nr = pgm_read_word_near(&(men->numEntries));
		mtype = pgm_read_byte((void*) & (men->menuType));
		uint16_t offset = menuTop[menuLevel];
		UIMenuEntry** entries = (UIMenuEntry * *)pgm_read_word(&(men->entries));
		for (r = 0; r + offset < nr && r < UI_ROWS; )
		{
			UIMenuEntry* ent = (UIMenuEntry*)pgm_read_word(&(entries[r + offset]));
			if (!ent->showEntry())
			{
				offset++;
				continue;
			}
			uint8_t entType = pgm_read_byte(&(ent->entryType)) & 127;
			uint16_t entAction = pgm_read_word(&(ent->action));
			col = 0;
			if (entType >= 2 && entType <= 4)
			{
				if (r + offset == menuPos[menuLevel] && activeAction != entAction)
					uid.printCols[col++] = CHAR_SELECTOR;
				else if (activeAction == entAction)
					uid.printCols[col++] = CHAR_SELECTED;
				else
					uid.printCols[col++] = ' ';
			}
			char* text = (char*)pgm_read_word(&(ent->text));
			if (text == NULL)
				text = (char*)Com::translatedF(pgm_read_word(&(ent->translation)));
			parse(text, false);
			if (entType == 2)  // Draw submenu marker at the right side
			{
				while (col < UI_COLS - 1) uid.printCols[col++] = ' ';
				if (col > UI_COLS)
				{
					uid.printCols[RMath::min(UI_COLS - 1, col)] = CHAR_RIGHT;
				}
				else
					uid.printCols[col] = CHAR_RIGHT; // Arrow right
				uid.printCols[++col] = 0;
			}
			strcpy(cache[r], uid.printCols);
			r++;
		}
	}
#if SDSUPPORT
	if (mtype == UI_MENU_TYPE_FILE_SELECTOR)
	{
		sdrefresh(r, cache);
	}
#endif

	uid.printCols[0] = 0;
	while (r < UI_ROWS) // delete trailing empty rows
		strcpy(cache[r++], uid.printCols);
	// cache now contains the data to show
	// Compute transition
	uint8_t transition = 0; // 0 = display, 1 = up, 2 = down, 3 = left, 4 = right
#if UI_ANIMATION
	if (menuLevel != oldMenuLevel && !PrintLine::hasLines())
	{
		if (oldMenuLevel == 0 || oldMenuLevel == -2)
			transition = 1;
		else if (menuLevel == 0)
			transition = 2;
		else if (menuLevel > oldMenuLevel)
			transition = 3;
		else
			transition = 4;
	}
#endif
	uint8_t loops = 1;
	uint8_t dt = 1, y;
	if (transition == 1 || transition == 2) loops = UI_ROWS;
	else if (transition > 2)
	{
		dt = (UI_COLS + UI_COLS - 1) / 16;
		loops = UI_COLS + 1 / dt;
	}
	uint8_t off0 = (shift <= 0 ? 0 : shift);
	uint8_t scroll = dt;
	uint8_t off[UI_ROWS];
	if (transition == 0) // Copy cache to displayCache
	{
		for (y = 0; y < UI_ROWS; y++)
			strcpy(displayCache[y], cache[y]);
	}
	for (y = 0; y < UI_ROWS; y++)
	{
		uint8_t len = strlen(displayCache[y]); // length of line content
		off[y] = len > UI_COLS ? RMath::min(len - UI_COLS, off0) : 0;
		if (len > UI_COLS)
		{
			off[y] = RMath::min(len - UI_COLS, off0);
			if (transition == 0 && (mtype == UI_MENU_TYPE_FILE_SELECTOR || mtype == UI_MENU_TYPE_SUBMENU)) // Copy first char to front
			{
				//displayCache[y][off[y]] = displayCache[y][0];
				cache[y][off[y]] = cache[y][0];
			}
		}
		else off[y] = 0;
#if UI_ANIMATION
		if (transition == 3)
		{
			for (r = len; r < MAX_COLS; r++)
			{
				displayCache[y][r] = 32;
			}
			displayCache[y][MAX_COLS] = 0;
		}
		else if (transition == 4)
		{
			for (r = strlen(cache[y]); r < MAX_COLS; r++)
			{
				cache[y][r] = 32;
			}
			cache[y][MAX_COLS] = 0;
		}
#endif
	}
	for (uint8_t l = 0; l < loops; l++)
	{
		if (uid.encoderLast != encoderStartScreen)
		{
			scroll = 200;
		}
		scroll += dt;
#if UI_DISPLAY_TYPE == DISPLAY_U8G

#define drawHProgressBar(x,y,width,height,progress) \
  {u8g_DrawFrame(&u8g,x,y, width, height);  \
    int p = ceil((width-2) * progress / 100); \
    u8g_DrawBox(&u8g,x+1,y+1, p, height-2);}

#define drawVProgressBar(x,y,width,height,progress) \
  {u8g_DrawFrame(&u8g,x,y, width, height);  \
    int p = height-1 - ceil((height-2) * progress / 100); \
    u8g_DrawBox(&u8g,x+1,y+p, width-2, (height-p));}
#if UI_DISPLAY_TYPE == DISPLAY_U8G

#if SDSUPPORT
		unsigned long sdPercent = 0;
		unsigned long percent = 0;
#endif
		//fan
#if FAN_PIN>-1 && FEATURE_FAN_CONTROL
		int fanPercent = 0;
		char fanString[2];
#endif
		/*if (extrudeUp == 1 && extrudeVar == 0) { // marcel preheat
		  zeitAnfangEx = HAL::timeInMilliseconds();
		  extrudeVar = 1;
		  Printer::currentPosition[E_AXIS] = 0;
		  x = Printer::currentPosition[E_AXIS]; //Printer::currentPositionSteps[E_AXIS] * Printer::invAxisStepsPerMM[E_AXIS];
		  Printer::moveToReal(IGNORE_COORDINATE, IGNORE_COORDINATE, IGNORE_COORDINATE, - 20, EXT0_MAX_FEEDRATE);
		  x1 = Printer::currentPosition[E_AXIS];
		  Com::printFLN(PSTR("anfang1 = "));
			Com::print(Printer::currentPosition[E_AXIS]);
			Com::printFLN(PSTR(" "));
		  //Printer::currentPositionSteps[E_AXIS] = 0;
		} else if (extrudeUp == 1 && extrudeVar == 1) {
			zeitEndeEx = HAL::timeInMilliseconds();
			zeitExtrude = zeitEndeEx - zeitAnfangEx;
			zeitPercentEx = zeitExtrude * 100 / intervall_2;
			a = Printer::currentPosition[E_AXIS] ;//Printer::currentPositionSteps[E_AXIS] * Printer::invAxisStepsPerMM[E_AXIS];
			extrudePercent = 100 - (a * 100 / x);
			if(Endstops::xMax() || Endstops::xMin()){
			  zeitExtrude = intervall_2+1;
			  extrudePercent = 100;
			}
			Com::printFLN(PSTR("anfang = "));
			Com::print(x);
			Com::printFLN(PSTR(" "));
			Com::printFLN(PSTR("ende = "));
			Com::print(a);
			Com::printFLN(PSTR(" "));
		}
		if (zeitExtrude >= intervall_2 || extrudePercent >= 100) {
		  menuLevel = 0;
		  Printer::currentPositionSteps[E_AXIS] = 0;
		  Printer::moveToReal(IGNORE_COORDINATE, IGNORE_COORDINATE, IGNORE_COORDINATE, 10, EXT0_MAX_FEEDRATE);
		  Printer::currentPositionSteps[E_AXIS] = 0;
		  if(removeVar == 1){
			uid.executeAction(UI_ACTION_KARTUSCHENW, false);
		  } else {
			pushMenu(&ui_menu_kartuschenw_3, false);
		  }
		  extrudeVar = 0;
		  zeitExtrude  = 0;
		  zeitAnfangEx = 0;
		  extrudeUp = 0;
		  extrudePercent = 0;
		}*/
    
    //Printer::newECode = 0; code für zurüclsetzten
    //fullTest = 100; code für zurücksetzten
    //diffVar = Printer::destinationSteps[E_AXIS] - Printer::oldCode;
    /*if (Printer::newCode == 92 && Printer::new2Code == 0){
      Printer::oldCode = 0;
    }
    //if (Printer::newCode == 1){
      Printer::newECode += Printer::new2Code - Printer::oldCode;
      Printer::oldCode = Printer::new2Code;
      HAL::eprSetFloat(0, Printer::newECode);
    //}*/
    /*Com::printFLN(PSTR("newCode: "));
     * Printer::newECode += Printer::newCode - Printer::oldCode;
    Printer::oldCode = Printer::newCode;
    HAL::eprSetFloat(0, Printer::newECode);
    Com::print(Printer::newCode);
    Com::printFLN(PSTR("\n"));
    Com::printFLN(PSTR("oldCode: "));
    Com::print(Printer::oldCode);
    Com::printFLN(PSTR("\n"));
    Com::printFLN(PSTR("newECode: "));
    Com::print(Printer::newECode);
    Com::printFLN(PSTR("\n")); */
    HAL::eprSetFloat(999, Printer::newECode);
    if (Printer::newECode >= 0) {
       fullPercent = 100 - (Printer::newECode * 100 / eMaxPos);
       if (fullPercent > fullTest) {
         fullPercent = fullTest;
       }
       else {
         fullTest = fullPercent;
       }
     } else if (Printer::newECode >= eMaxPos){
       fullPercent = 0;
     } else {
       fullPercent = 100;
       //fullPercent = 0;
     }
    
		if (menuLevel == 0 && menuPos[0] == 0) // Main menu with special graphics
		{
			//ext1 and ext2 animation symbols
#if NUM_EXTRUDER < 3
			if (extruder[0].tempControl.targetTemperatureC > 30)
#else
			if (Extruder::current->tempControl.targetTemperatureC > 30)
#endif
				cache[0][0] = ' ';//Printer::isAnimation() ? '\x08' : '\x09'; // marcel Main-screen
			else
				cache[0][0] = ' ';//\x0a'; //off  // marcel Main-screen
#if NUM_EXTRUDER == 2 && MIXING_EXTRUDER == 0
			if (extruder[1].tempControl.targetTemperatureC > 30)
				cache[1][0] = Printer::isAnimation() ? '\x08' : '\x09';
			else
				cache[1][0] = '\x0a'; //off
#endif
#if HAVE_HEATED_BED
	  //heated bed animated icons
			uint8_t lin = 2 - ((NUM_EXTRUDER != 2) ? 1 : 0);
			if (heatedBedController.targetTemperatureC > 30)
				cache[lin][0] = Printer::isAnimation() ? '\x0c' : '\x0d';
			else
				cache[lin][0] = '\x0b';
#endif
#if FAN_PIN > -1 && FEATURE_FAN_CONTROL
			//fan
			fanPercent = Printer::getFanSpeed() * 100 / 255;
			fanString[1] = 0;
			if (fanPercent > 0) //fan running animation
			{
				fanString[0] = Printer::isAnimation() ? '\x0e' : '\x0f';
			}
			else
			{
				fanString[0] = '\x0e';
			}
#endif

			// marcel progressbar Extruderposition
      // g1 befehl nehemen wenn übersetzt wird dann variable hochzählen vielleicht
      //if(static_cast<float>(Printer::currentPositionSteps[E_AXIS]) * Printer::invAxisStepsPerMM[E_AXIS] == 0){
      //  oldEposVar = 0;
      //} 
      //Printer::currentPosition[E_AXIS] = Printer::currentPosition[E_AXIS] + static_cast<float>(Printer::currentPositionSteps[E_AXIS]) * Printer::invAxisStepsPerMM[E_AXIS] - oldEposVar;
      //Printer::currentPosition[E_AXIS] = static_cast<float>(Printer::currentPositionSteps[E_AXIS]) * Printer::invAxisStepsPerMM[E_AXIS];
      //oldEposVar = static_cast<float>(Printer::currentPositionSteps[E_AXIS]) * Printer::invAxisStepsPerMM[E_AXIS];
      //Com::print(Printer::currentPosition[E_AXIS]);
        
      
			// marcel progressbar Extruderposition
#if SDSUPPORT
	  //SD Card
			if (sd.sdactive)
			{
				if (sd.sdactive && sd.sdmode)
				{
					if (sd.filesize < 20000000) sdPercent = sd.sdpos * 100 / sd.filesize;
					else sdPercent = (sd.sdpos >> 8) * 100 / (sd.filesize >> 8);
				}
				else
				{
					sdPercent = 0;
				}
			}
#endif
			if (preHeatChoco == 1 && preheatVar == 0) { // marcel preheat
				zeitAnfang = HAL::timeInMilliseconds();
				preheatVar = 1;
				Printer::setMenuMode(MENU_MODE_PREHEAT, true);
				Printer::moveToReal(15, IGNORE_COORDINATE, IGNORE_COORDINATE, IGNORE_COORDINATE, Printer::homingFeedrate[X_AXIS]);
				if (printVar == 1 || kartuschenwVar == 1) {
					//Printer::currentPositionSteps[E_AXIS] = 0;
				}
				if (Extruder::current->tempControl.currentTemperatureC >= 30.5) {
					pushMenu(&ui_menu_preask, false);
					//zeitChoco = -1;
				}
			}
			else if (preHeatChoco == 1 && preheatVar == 1) {
				zeitEnde = HAL::timeInMilliseconds();
				zeitSecChoco1 = zeitEnde;
				zeitChoco = zeitEnde - zeitAnfang;
				//Com::print(zeitChoco);
				zeitChoco /= 1000;
				zeitChoco = round(zeitChoco);
				zeitSecChoco1 = zeitChoco;
				zeitSecChoco = zeitVar - zeitSecChoco1;
				zeitPercent = zeitChoco * 100 / intervall;
				zeitChoco = intervall - zeitChoco;
        UI_STATUS_F(Com::translatedF(UI_TEXT_PREHEATING_ID));
        HAL::delayMilliseconds(990);
				if (chooseVar == 1) {
					//pushMenu(&ui_menu_calibrate, false);
					chooseVar = 0;
				}

				if (zeitSecChoco <= 0) {
					zeitVar += 60;
				}
				for (int i = 0; i < intervall; i += 60) {
					if (zeitChoco == i) {
						zeitMinChoco -= 1;
						HAL::delayMilliseconds(1000);
					}
				}
			}
			if (zeitChoco < 0/*intervall*/ && preKartusche == 1) {
				kartuschenwVar = 1;
				pushMenu(&ui_menu_sd_fileselector, true);
				preheatVar = 0;
				zeitChoco = 0;
				zeitAnfang = 0;
				preHeatChoco = 0;
				preKartusche = 0;
				zeitPercent = 100;
        zeitVar = 60;
        zeitMinChoco = (intervall / 60) - 1;
        BEEP_LONG;
        BEEP_LONG;
				UI_STATUS_F(Com::translatedF(UI_TEXT_PREHEAT_DONE_ID));
				Printer::setMenuMode(MENU_MODE_PREHEAT, false);
        GCode::executeFString("M104 S31");
			}
			else if (zeitChoco < 0 && printVar == 1 && errorVar != 1) {
        fileVar = 1;
				pushMenu(&ui_menu_sd_fileselector, false);
				preheatVar = 0;
				zeitChoco = 0;
				zeitAnfang = 0;
				preHeatChoco = 0;
				preKartusche = 0;
				zeitPercent = 100;
        zeitVar = 60;
        zeitMinChoco = (intervall / 60) - 1;
        BEEP_LONG;
        BEEP_LONG;
				UI_CLEAR_STATUS;
				Printer::setMenuMode(MENU_MODE_PREHEAT, false);
        GCode::executeFString("M104 S31");
				//UI_STATUS_F(Com::translatedF(UI_TEXT_PREHEAT_DONE_ID));
			}
			else if (zeitChoco < 0/*intervall*/) {
				//UI_CLEAR_STATUS;
				UI_STATUS_F(Com::translatedF(UI_TEXT_PREHEAT_DONE_ID));
				preheatVar = 0;
				zeitChoco = 0;
				zeitAnfang = 0;
				preHeatChoco = 0;
				preKartusche = 0;
				zeitPercent = 100;
        zeitMinChoco = (intervall / 60) - 1;
        zeitVar = 60;
        BEEP_LONG;
        BEEP_LONG;
        GCode::executeFString("M104 S31");
				Printer::setMenuMode(MENU_MODE_PREHEAT, false);
			}
			if (Extruder::current->tempControl.currentTemperatureC >= 38 && holdVar == 0) {
				holdVar = 1;
				holdAnfang = HAL::timeInMilliseconds();
			}
			else if (Extruder::current->tempControl.currentTemperatureC >= 38 && holdVar == 1) {
				holdEnde = HAL::timeInMilliseconds();
				holdMix = holdEnde - holdAnfang;
			}
			if (holdMix >= intervall_4) {
				holdAnfang = 0;
				holdEnde = 0;
				holdMix = 0;
				holdVar = 0;
				if (textChooseVar = 1) {
					//Extruder::setTemperatureForExtruder(UI_SET_PRESET_EXTRUDER_TEMP_ABS, 0);
				}
				if (chocoChooseVar = 1) {
					//Extruder::setTemperatureForExtruder(UI_SET_PRESET_EXTRUDER_TEMP_PLA, 0);
				}
				//Extruder::setTemperatureForExtruder(UI_SET_PRESET_EXTRUDER_TEMP_PLA, 0);
			}
      tempPercent = (100 * (Extruder::current->tempControl.currentTemperatureC - 20)) / 11;
      if (tempPercent > 100){
        tempPercent = 100;    
      }
      if (tempPercent <= 0) {
        tempPercent = 0;  
      }
		}
#endif

		//u8g picture loop
		u8g_FirstPage(&u8g);
		do
		{
			/*if(kartuschenwVar == 1 && menuLevel != 0){ // für u8g in jedem Menü
			  printU8GRow(0, 10, const_cast<char *>("Test"));
			}*/
#endif
			if (transition == 0)
			{
#if UI_DISPLAY_TYPE == DISPLAY_U8G
				/*if (extrudeUp == 1 && extrudeVar == 1) {
				  printU8GRow(0, 10, const_cast<char *>("Extruder is moving up"));
				  drawHProgressBar(0, 51, u8g_GetWidth(&u8g), 13, extrudePercent);//zeitPercentEx);
				}*/
        if (startVariable == 1){
          printU8GRow(0, 20, const_cast<char*>(fileNameLCDAsk));
        }
        if (eposVariable == 1){
          //printU8GRow(0, 61, const_cast<char*>("www.mycusini.com")); // language ready machen UI_TEXT_SERVICE_2_ID);// 
          u8g_SetFont(&u8g, UI_FONT_SMALL);
          char fullPercentC[3] = { '\0' };
          itoa(fullPercent, fullPercentC, 10);
          /*
          drawHProgressBar(57, 4, 47, 6, fullPercent);
          printU8GRow(0, 10, const_cast<char*>("Refill"));
          printU8GRow(108, 10, const_cast<char*>(fullPercentC));
          printU8GRow(124, 10, const_cast<char*>("%"));*/
          drawHProgressBar(0, 4, 47, 6, fullPercent);
          printU8GRow(70, 10, const_cast<char*>("Refill:"));
          printU8GRow(108, 10, const_cast<char*>(fullPercentC));
          printU8GRow(124, 10, const_cast<char*>("%"));
          /*drawHProgressBar(72, 4, 47, 6, fullPercent);
          printU8GRow(0, 20, const_cast<char*>("Refill"));
          printU8GRow(108, 20, const_cast<char*>(fullPercentC));
          printU8GRow(124, 20, const_cast<char*>("%"));*/
          u8g_SetFont(&u8g, UI_FONT_DEFAULT);
          
          /*int sizeName = 10;
          for (int j = 0; j < sizeof(fileNameLCD); j++) {
            if (fileNameLCD[j] == '.') {
              sizeName -= 10 - j;
            }
          }
          for (int i = 0; i < sizeName; i++) {
            addChar(fileNameLCD[i]);           
          }*/
        }
				if (menuLevel == 0 && menuPos[0] == 0)
				{
					if (errorVar == 1) {
						u8g_SetFont(&u8g, UI_FONT_DEFAULT);
            /*printU8GRow(20,  8, const_cast<char*>("Error - Stopped"));// language ready machen UI_TEXT_ERROR_TEXT_ID);//
            printU8GRow(32, 22, const_cast<char*>("Temp Sensor"));// language ready machen UI_TEXT_ERROR_TEMP_ID);//
            printU8GRow(0, 43, const_cast<char*>("Restart Printer")); 
            printU8GRow(0, 52, const_cast<char*>("otherwise Contact us")); // language ready machen UI_TEXT_SERVICE_2_ID);//
            printU8GRow(0, 61, const_cast<char*>("at: www.mycusini.com")); // language ready machen UI_TEXT_SERVICE_2_ID);//*/
						printU8GRow(20,  8, const_cast<char*>(Com::translatedF(UI_TEXT_ERROR_TEXT_ID)));// language ready machen );//
						printU8GRow(32, 22, const_cast<char*>(Com::translatedF(UI_TEXT_ERROR_TEMP_ID)));// language ready machen UI_TEXT_ERROR_TEMP_ID);//
            printU8GRow(0, 43, const_cast<char*>(Com::translatedF(UI_TEXT_TEMP_2_ID))); 
						printU8GRow(0, 52, const_cast<char*>(Com::translatedF(UI_TEXT_SERVICE_2_ID))); // language ready machen UI_TEXT_SERVICE_2_ID);//
						printU8GRow(0, 61, const_cast<char*>(Com::translatedF(UI_TEXT_SERVICE_3_ID))); // language ready machen UI_TEXT_SERVICE_3_ID);//
            eposVariable = 0;
					}
          else if (HAL::eprGetFloat(1021) == 0) {
            uid.showLanguageSelectionWizard();
          }
					else {
					  if (HAL::eprGetFloat(1020) == 1){             
                Com::selectLanguage(1);
                HAL::eprSetByte(EPR_SELECTED_LANGUAGE, 1);
                //Com::printFLN(PSTR("im sparch"));
            }
						u8g_SetFont(&u8g, UI_FONT_SMALL);
						uint8_t py = 8;
						for (uint8_t r = 0; r < 3; r++)
						{
							if (u8g_IsBBXIntersection(&u8g, 0, py - UI_FONT_SMALL_HEIGHT, 1, UI_FONT_SMALL_HEIGHT))
								printU8GRow(0, py, cache[r]);
							py += 10;
						}

#if FAN_PIN>-1 && FEATURE_FAN_CONTROL
						//fan
						if (u8g_IsBBXIntersection(&u8g, 0, 30 - UI_FONT_SMALL_HEIGHT, 1, UI_FONT_SMALL_HEIGHT))
							printU8GRow(117, 30, fanString);
						drawVProgressBar(116, 0, 9, 20, fanPercent);
#endif
						if (u8g_IsBBXIntersection(&u8g, 0, 42 - UI_FONT_SMALL_HEIGHT, 1, UI_FONT_SMALL_HEIGHT))
							printU8GRow(0, 42, cache[3]); //multiplier + extruded
						if (u8g_IsBBXIntersection(&u8g, 0, 52 - UI_FONT_SMALL_HEIGHT, 1, UI_FONT_SMALL_HEIGHT))
							printU8GRow(0, 52, cache[4]); //buffer usage
						drawHProgressBar(57, 28, 47, 6, fullPercent);
						u8g_DrawBitmapP(&u8g, 57, 5, ((22) / 8), 7, muetze); // marcel logo
						u8g_SetFont(&u8g, UI_FONT_DEFAULT);
						printU8GRow(43, 18, const_cast<char*>("mycusini")); // languag'e ready machen UI_TEXT_MYCUSINI_ID);//
						u8g_SetFont(&u8g, UI_FONT_SMALL);
						// Main-Screen Test

						printU8GRow(0, 34, const_cast<char*>("Refill")); // language ready machen UI_TEXT_REFILL_ID);//

						//printU8GRow(-1, 48, const_cast<char*>("Temperature")); // language ready machen UI_TEXT_TEMPERATURE_ID);//
            printU8GRow(-1, 48, const_cast<char*>(Com::translatedF(UI_TEXT_MAIN_TEMP_ID)));
						printU8GRow(122, 49, cache[0]);
						float zeitPercentO = 0;
						drawHProgressBar(57, 43, 47, 6, zeitPercentO);
						char fullPercentC[3] = { '\0' };
						itoa(fullPercent, fullPercentC, 10);

						printU8GRow(108, 34, const_cast<char*>(fullPercentC));
						printU8GRow(124, 34, const_cast<char*>("%"));
						drawHProgressBar(57, 43, 47, 6, tempPercent);//zeitPercent);

						if (sdPercent >= 100) {
							chocoChooseVar = 0;
							textChooseVar = 0;
						}
						// Main-Screen Test Ende
						if (preHeatChoco == 1)//marcel preheat
						{
							printU8GRow(-1, 48, const_cast<char*>("Temperature")); // language ready machen UI_TEXT_TEMPERATURE_ID);//
							//drawHProgressBar(57, 43, 47, 6, zeitPercent);
							char zeitTable[10] = { '\0' };
							itoa(zeitSecChoco, zeitTable, 10);
							//Com::print(zeitTable);
							printU8GRow(115, 62, const_cast<char*>(zeitTable));
							printU8GRow(125, 62, const_cast<char*>("s"));
							char zeitTableMin[10] = { '\0' };
							itoa(zeitMinChoco, zeitTableMin, 10);
							printU8GRow(98, 62, const_cast<char*>(zeitTableMin));
							printU8GRow(108, 62, const_cast<char*>("m"));
						}

#if SDSUPPORT
						//SD Card
						if (sd.sdactive && u8g_IsBBXIntersection(&u8g, 66, 52 - UI_FONT_SMALL_HEIGHT, 1, UI_FONT_SMALL_HEIGHT))
						{
							//printU8GRow(0, 48, const_cast<char *>("Progress"));
							//drawHProgressBar(57, 43, 47, 6, sdPercent);
						}
#endif
						//Status
						py = u8g_GetHeight(&u8g) - 2;
						if (u8g_IsBBXIntersection(&u8g, 70, py - UI_FONT_SMALL_HEIGHT, 1, UI_FONT_SMALL_HEIGHT))
							printU8GRow(0, py, cache[5]);

						//divider lines
						//u8g_DrawHLine(&u8g, 0, 32, u8g_GetWidth(&u8g));
						if (u8g_IsBBXIntersection(&u8g, 54, 0, 1, 55))
						{
							//u8g_draw_vline(&u8g,112, 0, 32);
							//u8g_draw_vline(&u8g, 62, 0, 54);
						}
						u8g_SetFont(&u8g, UI_FONT_DEFAULT);
					}
				}
				else
				{
#endif
					for (y = 0; y < UI_ROWS; y++)
						printRow(y, &cache[y][off[y]], NULL, UI_COLS);
#if UI_DISPLAY_TYPE == DISPLAY_U8G
				}
#endif
			}
#if UI_ANIMATION
			else
			{
				if (transition == 1)  // up
				{
					if (scroll > UI_ROWS)
					{
						scroll = UI_ROWS;
						l = loops;
					}
					for (y = 0; y < UI_ROWS - scroll; y++)
					{
						r = y + scroll;
						printRow(y, &displayCache[r][off[r]], NULL, UI_COLS);
					}
					for (y = 0; y < scroll; y++)
					{
						printRow(UI_ROWS - scroll + y, cache[y], NULL, UI_COLS);
					}
				}
				else if (transition == 2)    // down
				{
					if (scroll > UI_ROWS)
					{
						scroll = UI_ROWS;
						l = loops;
					}
					for (y = 0; y < scroll; y++)
					{
						printRow(y, cache[UI_ROWS - scroll + y], NULL, UI_COLS);
					}
					for (y = 0; y < UI_ROWS - scroll; y++)
					{
						r = y + scroll;
						printRow(y + scroll, &displayCache[y][off[y]], NULL, UI_COLS);
					}
				}
				else if (transition == 3)    // left
				{
					if (scroll > UI_COLS)
					{
						scroll = UI_COLS;
						l = loops;
					}
					for (y = 0; y < UI_ROWS; y++)
					{
						printRow(y, &displayCache[y][off[y] + scroll], cache[y], UI_COLS - scroll);
					}
				}
				else     // right
				{
					if (scroll > UI_COLS)
					{
						scroll = UI_COLS;
						l = loops;
					}
					for (y = 0; y < UI_ROWS; y++)
					{
						printRow(y, cache[y] + UI_COLS - scroll, &displayCache[y][off[y]], scroll);
					}
				}
#if UI_DISPLAY_TYPE != DISPLAY_U8G
				HAL::delayMilliseconds(transition < 3 ? 200 : 70);
#endif
				HAL::pingWatchdog();
			}
#endif
#if UI_DISPLAY_TYPE == DISPLAY_U8G
		} while (u8g_NextPage(&u8g)); //end picture loop
		Printer::toggleAnimation();
#endif
			} // for l
#if UI_ANIMATION
  // copy to last cache
	if (transition != 0)
		for (y = 0; y < UI_ROWS; y++)
			strcpy(displayCache[y], cache[y]);
	oldMenuLevel = menuLevel;
#endif
#endif
}

void UIDisplay::pushMenu(const UIMenu * men, bool refresh)
{
	if (men == menu[menuLevel])
	{
		refreshPage();
		return;
	}
	if (menuLevel == 4) return; // Max. depth reached. No more memory to down further.
	menuLevel++;
	menu[menuLevel] = men;
	menuTop[menuLevel] = menuPos[menuLevel] = 0;
#if SDSUPPORT
	UIMenu * men2 = (UIMenu*)menu[menuLevel];
	if (pgm_read_byte(&(men2->menuType)) == 1)
	{
		// Menu is Open files list
		updateSDFileCount();
		// Keep menu positon in file list, more user friendly.
		// If file list changed, still need to reset position.
		if (menuPos[menuLevel] > nFilesOnCard)
		{
			//This exception can happen if the card was unplugged or modified.
			menuTop[menuLevel] = 0;
			menuPos[menuLevel] = UI_MENU_BACKCNT; // if top entry is back, default to next useful item
		}
	}
	else
#endif
	{
		// With or without SDCARD, being here means the menu is not a files list
		// Reset menu to top
		menuTop[menuLevel] = 0;

		UIMenuEntry** entries = (UIMenuEntry * *)pgm_read_word(&(men->entries));
		UIMenuEntry* ent = (UIMenuEntry*)pgm_read_word(&(entries[0]));
		uint16_t entAction = pgm_read_word(&(ent->action));
		menuPos[menuLevel] = entAction == UI_ACTION_BACK ? 1 : 0; // if top entry is back, default to next useful item
	}
	if (refresh)
		refreshPage();
}

void UIDisplay::popMenu(bool refresh)
{
	if (menuLevel > 0) menuLevel--;
	Printer::setAutomount(false);
	activeAction = 0;
	if (refresh)
		refreshPage();
}
int UIDisplay::okAction(bool allowMoves)
{
	if (Printer::isUIErrorMessage())
	{
		Printer::setUIErrorMessage(false);
		return 0;
	}
	//BEEP_SHORT
#if UI_HAS_KEYS == 1
		if (menuLevel == 0)  // Enter menu
		{
			menuLevel = 1;
			menuTop[1] = 0;
			menuPos[1] = UI_MENU_BACKCNT; // if top entry is back, default to next useful item
			menu[1] = &ui_menu_main;
      UI_STATUS_F(Com::translatedF(UI_TEXT_CLICK_START_ID));
      
			if (resetVar == 0) {
				menuLevel = 0;
				Printer::homeAxis(false, false, true);
				Printer::homeAxis(true, true, false);
				UI_STATUS_F(Com::translatedF(UI_TEXT_CLICK_START_ID));
        //Printer::currentPosition[E_AXIS] = 0;
        
				resetVar = 1;
			}
			return 0;
		}
	const UIMenu* men = (const UIMenu*)menu[menuLevel];
	//uint8_t nr = pgm_read_word_near(&(menu->numEntries));
	uint8_t mtype = pgm_read_byte(&(men->menuType));
	UIMenuEntry** entries;
	UIMenuEntry* ent;
	unsigned char entType;
	unsigned int action;
#if SDSUPPORT
	if (mtype == UI_MENU_TYPE_FILE_SELECTOR)
	{
		if (menuPos[menuLevel] == 0)  // Selected back instead of file
		{
			return executeAction(UI_ACTION_BACK, allowMoves);
		}

		if (!sd.sdactive)
			return 0;
		uint8_t filePos = menuPos[menuLevel] - 1;
		char filename[LONG_FILENAME_LENGTH + 1];

		getSDFilenameAt(filePos, filename);
		if (isDirname(filename))  // Directory change selected
		{
			goDir(filename);
			menuTop[menuLevel] = 0;
			menuPos[menuLevel] = 1;
			refreshPage();
			oldMenuLevel = -1;
			return 0;
		}

		int16_t shortAction; // renamed to avoid scope confusion
		if (Printer::isAutomount())
			shortAction = UI_ACTION_SD_PRINT; // marcel sd direkt print
		else
		{
			men = menu[menuLevel - 1];
			entries = (UIMenuEntry * *)pgm_read_word(&(men->entries));
			ent = (UIMenuEntry*)pgm_read_word(&(entries[menuPos[menuLevel - 1]]));
			shortAction = pgm_read_word(&(ent->action));
		}
		shortAction = UI_ACTION_SD_PRINT;
		sd.file.close();
		sd.fat.chdir(cwd);
		Com::printFLN(PSTR("sd print"));
		EVENT_START_UI_ACTION(shortAction);
		Com::printFLN(PSTR("n short action"));
		switch (shortAction)
		{
		case UI_ACTION_SD_PRINT:
			if (sd.selectFile(filename, false))
			{
				strcpy(fileNameLCD, filename); //marcel filename auf lcd
        int sizeNameAsk = 30;
        for (int j = 0; j < sizeof(fileNameLCD); j++) {
          if (fileNameLCD[j] == '.') {
            sizeNameAsk -= 30 - j;
          }
        }
        for (int i = 0; i < sizeNameAsk; i++) {
          addChar(fileNameLCD[i]);
          fileNameLCDAsk[i] = fileNameLCD[i]; 
        }
				//sd.startPrint();
				//BEEP_LONG;
				//menuLevel = 0;
        startVariable = 1;
        fileVar = 0;
				pushMenu(&ui_menu_start_quest, false);
				//pushMenu(&ui_menu_epos_kart, false);
			}
			break;

		case UI_ACTION_EPOS_KART:
			if (sd.selectFile(filename, false))
			{
        //GCode::executeFString("G1 E-0.8");
				sd.startPrint();
				//BEEP_LONG;
				menuLevel = 0;
				
			}
			Com::printFLN(PSTR("unter sd print"));
			break;

		case UI_ACTION_CARTRIDGE_INSERTED:
			popMenu(false);
			menuLevel = 0;
			preHeatChoco = 1;
			Com::printFLN(PSTR("klick done inst"));
			break;
		case UI_ACTION_SD_DELETE:
			if (sd.sdactive)
			{
				sd.sdmode = 0;
				sd.file.close();
				if (sd.fat.remove(filename))
				{
					Com::printFLN(Com::tFileDeleted);
					BEEP_LONG
						if (menuPos[menuLevel] > 0)
							menuPos[menuLevel]--;
					updateSDFileCount();
				}
				else
				{
					Com::printFLN(Com::tDeletionFailed);
				}
			}
			break;
		}
		return 0;
	}
#endif
	entries = (UIMenuEntry * *)pgm_read_word(&(men->entries));
	ent = (UIMenuEntry*)pgm_read_word(&(entries[menuPos[menuLevel]]));
	entType = pgm_read_byte(&(ent->entryType));// 0 = Info, 1 = Headline, 2 = submenu ref, 3 = direct action command, 4 = modify action
	action = pgm_read_word(&(ent->action));
	if (mtype == UI_MENU_TYPE_MODIFICATION_MENU)  // action menu
	{
		action = pgm_read_word(&(men->id));
		finishAction(action);
		return executeAction(UI_ACTION_BACK, true);
	}
	if (mtype == UI_MENU_TYPE_SUBMENU && entType == 4)  // Modify action
	{
		if (activeAction)  // finish action
		{
			finishAction(action);
			activeAction = 0;
		}
		else
			activeAction = action;
		return 0;
	}
	if (mtype == UI_MENU_TYPE_WIZARD)
	{
		action = pgm_read_word(&(men->id));
		switch (action)
		{
#if FEATURE_RETRACTION
		case UI_ACTION_WIZARD_FILAMENTCHANGE: // filament change is finished
		  //            BEEP_SHORT;
			popMenu(true);
			Extruder::current->retractDistance(EEPROM_FLOAT(RETRACTION_LENGTH));
#if FILAMENTCHANGE_REHOME
#if Z_HOME_DIR > 0
			Printer::homeAxis(true, true, FILAMENTCHANGE_REHOME == 2);
#else
			Printer::homeAxis(true, true, false);
#endif
#endif
		case UI_ACTION_CHOCO_CHOOSE:
			//menuLevel = 0;
			popMenu(false);
			chocoChooseVar = 1;
			printVar = 1;
			fileVar = 1;
			//pushMenu(&ui_menu_calibrate, false);
			uid.executeAction(UI_ACTION_PREHEAT_PLA, false);
			//pushMenu(&ui_menu_calibrate, false);
			//Printer::moveToReal(Printer::currentPosition[X_AXIS], Printer::currentPosition[Y_AXIS],Printer::currentPosition[Z_AXIS], Printer::currentPosition[Z_AXIS] + 10, Printer::homingFeedrate[X_AXIS]);
			break;
		case UI_ACTION_TEXT_CHOOSE:
			popMenu(false);
			textChooseVar = 1;
			printVar = 1;
			fileVar = 1;
			uid.executeAction(UI_ACTION_PREHEAT_ABS, false);
			//pushMenu(&ui_menu_calibrate, false);
			//Printer::moveToReal(Printer::currentPosition[X_AXIS], Printer::currentPosition[Y_AXIS],Printer::currentPosition[Z_AXIS], Printer::currentPosition[Z_AXIS] + 10, Printer::homingFeedrate[X_AXIS]);
			break;
		case UI_ACTION_CALIBRATE_Z:
			popMenu(false);
			pushMenu(&ui_menu_calibrate_z_small, false);
			break;
		case UI_ACTION_CALIBRATE_Z_SMALL:
			if (printVar == 1) {
				popMenu(false);
				menuLevel = 0;//pushMenu(&ui_menu_preheat_ask, false);
				Printer::homeAxis(true, true, false);
			}
			else {
				popMenu(true);
				menuLevel = 0;
				Printer::homeAxis(true, true, true);
				standardVar = 0;
				cookieVar = 0;
				chocolateVar = 0;
			}
			calibrateVar = 1;
			break;
		case UI_ACTION_PRE_CALIBRATE:
			popMenu(false);
			if (standardVar == 1) {
				uid.executeAction(UI_ACTION_CALIBRATE_Z, false);
			}
			else {
				pushMenu(&ui_menu_place_plate, false);
			}
			break;
		case UI_ACTION_PLACE_PLATE:
			popMenu(false);
			uid.executeAction(UI_ACTION_CALIBRATE_Z, false);
			break;
		case UI_ACTION_CLEAN_1:
			popMenu(false);
			extrudeUp = 1;
			uid.executeAction(UI_ACTION_CLEAN_2, false);
			break;
		case UI_ACTION_CLEAN_3:
			menuLevel = 0;
			popMenu(false);
			extrudeUp = 1;
			UI_STATUS_F(Com::translatedF(UI_TEXT_PRINTER_READY_ID));
			uid.executeAction(UI_ACTION_CLEAN_4, false);
			break;
		case UI_ACTION_KARTUSCHENW: //Marcel Kartuschenwechsel
			/*Extruder::tempTestFunction(0);
			UI_STATUS_F(Com::translatedF(UI_TEXT_PRINTER_READY_ID));
			popMenu(false);
			menuLevel = 0;
			if (removeVar == 1) {
				removeVar = 0;
			}
			else {
				preHeatChoco = 1;
				zeitPercent = 0;
				preKartusche = 1;
				//UI_STATUS_F(Com::translatedF(UI_TEXT_PREHEAT_PLA_ID));
			}
			fullPercent = 0;
			fullTest = 100;
      Printer::newCode = 0;
      //Printer::oldCode = 0;
      //Printer::oldCode = Printer::newECode;
			Printer::currentPosition[E_AXIS] = 0;
			Printer::currentPositionSteps[E_AXIS] = 0;
      predoseVarPos = 0;*/
			break;
		case UI_ACTION_KARTUSCHENR:
			UI_STATUS_F(Com::translatedF(UI_TEXT_PRINTER_READY_ID));
			popMenu(false);
			menuLevel = 0;
			removeVar = 0;
			fullPercent = 0;
			fullTest = 100;
      Printer::newCode = 0;
      //Printer::oldCode = 0;
      //Printer::oldCode = Printer::newECode;
			Printer::currentPosition[E_AXIS] = 0;
			Printer::currentPositionSteps[E_AXIS] = 0;
      predoseVarPos = 0;
			break;
		case UI_ACTION_EPOS_KART: //marcel Kartuschenwechsel
			UI_STATUS_F(Com::translatedF(UI_TEXT_PRINTER_READY_ID));
			//Com::printFLN(PSTR("in epos kart"));
			if (printVar == 1) {
        //richtiges 0.9.3
				popMenu(false);
				//pushMenu(&ui_menu_sd, false);
        GCode::executeFString("G91");
        GCode::executeFString("G1 E-0.8");
        GCode::executeFString("G90");
				sd.startPrint();
				printVar = 0;
				menuLevel = 0;
				Com::printFLN(PSTR("weiter unten"));
				break;
			}
			if (kartuschenwVar == 1) {
				menuLevel = 0;
				popMenu(false);
				Printer::homeAxis(true, true, false);
				Printer::GoToMemoryPosition(true, true, false, false, Printer::homingFeedrate[X_AXIS]);
				Printer::GoToMemoryPosition(false, false, true, false, Printer::homingFeedrate[Z_AXIS]);
				Printer::setBlockingReceive(false);
				kartuschenwVar = 0;
				Printer::setMenuMode(MENU_MODE_PREHEAT, false);
        Printer::newCode = 0;
        //Printer::oldCode = 0;
        //Printer::oldCode = Printer::newECode;
				Printer::currentPosition[E_AXIS] = 0;
				Printer::currentPositionSteps[E_AXIS] = 0;
        predoseVarPos = 0;
				break;
			}
			menuLevel = 0;
			if (Endstops::yMax()) { // marcel extra endstop
				//Printer::moveToReal(IGNORE_COORDINATE, IGNORE_COORDINATE, IGNORE_COORDINATE, 10, EXT0_MAX_FEEDRATE);
				//Printer::currentPositionSteps[E_AXIS] = 0;
			}
			popMenu(true);
      //Printer::currentPositionSteps[E_AXIS] = 0;
			break;

			/*Printer::GoToMemoryPosition(true, true, false, false, Printer::homingFeedrate[X_AXIS]);
			  Printer::GoToMemoryPosition(false, false, true, false, Printer::homingFeedrate[Z_AXIS]);
			  Extruder::current->retractDistance(-EEPROM_FLOAT(RETRACTION_LENGTH));
			  Printer::currentPositionSteps[E_AXIS] = Printer::popWizardVar().l; // set e to starting position
			*/
#if EXTRUDER_JAM_CONTROL
			Extruder::markAllUnjammed();
#endif
			Printer::setJamcontrolDisabled(false);
			break;
#if EXTRUDER_JAM_CONTROL
		case UI_ACTION_WIZARD_JAM_REHEAT: // user saw problem and takes action
			popMenu(false);
			pushMenu(&ui_wiz_jamwaitheat, true);
			Extruder::unpauseExtruders();
			popMenu(false);
			pushMenu(&ui_wiz_filamentchange, true);
			break;
		case UI_ACTION_WIZARD_JAM_WAITHEAT: // called while heating - should do nothing user must wait
			BEEP_LONG;
			break;
#endif // EXTRUDER_JAM_CONTROL
#endif
		}
		return 0;
		}
	if (entType == 2)  // Enter submenu
	{
		pushMenu((UIMenu*)action, false);
		//        BEEP_SHORT
#if FEATURE_BABYSTEPPING
		zBabySteps = 0;
#endif
#if HAVE_HEATED_BED
		if (action == pgm_read_word(&ui_menu_conf_bed.action)) // enter Bed configuration menu
			currHeaterForSetup = &heatedBedController;
		else
#endif
			currHeaterForSetup = &(Extruder::current->tempControl);
		Printer::setMenuMode(MENU_MODE_FULL_PID, currHeaterForSetup->heatManager == 1);
		Printer::setMenuMode(MENU_MODE_DEADTIME, currHeaterForSetup->heatManager == 3);
		return 0;
	}
	if (entType == 3)
	{
		return executeAction(action, allowMoves);
	}
	return executeAction(UI_ACTION_BACK, allowMoves);
#endif
	}

//#define INCREMENT_MIN_MAX(a,steps,_min,_max) if ( (increment<0) && (_min>=0) && (a<_min-increment*steps) ) {a=_min;} else { a+=increment*steps; if(a<_min) a=_min; else if(a>_max) a=_max;};

// this version not have single byte variable rollover bug
#define INCREMENT_MIN_MAX(a,steps,_min,_max) a = constrain((a + increment*steps), _min, _max);

void UIDisplay::adjustMenuPos()
{
	if (menuLevel == 0) return;
	UIMenu * men = (UIMenu*)menu[menuLevel];
	UIMenuEntry * *entries = (UIMenuEntry * *)pgm_read_word(&(men->entries));
	uint8_t mtype = HAL::readFlashByte((PGM_P) & (men->menuType)) & 127;
	uint16_t numEntries = pgm_read_word(&(men->numEntries));
	if (mtype != 2) return;
	UIMenuEntry * entry;
	while (menuPos[menuLevel] > 0) // Go up until we reach visible position
	{
		entry = (UIMenuEntry*)pgm_read_word(&(entries[menuPos[menuLevel]]));
		if (pgm_read_byte((void*) & (entry->entryType)) == 1) // skip headlines
			menuPos[menuLevel]--;
		else if (entry->showEntry())
			break;
		else
			menuPos[menuLevel]--;
	}

	// with bad luck the only visible option was in the opposite direction
	while (menuPos[menuLevel] < numEntries - 1) // Go down until we reach visible position
	{
		entry = (UIMenuEntry*)pgm_read_word(&(entries[menuPos[menuLevel]]));
		if (pgm_read_byte((void*) & (entry->entryType)) == 1) // skip headlines
			menuPos[menuLevel]++;
		else if (entry->showEntry())
			break;
		else
			menuPos[menuLevel]++;
	}

	uint8_t skipped = 0;
	bool modified;
	if (menuTop[menuLevel] > menuPos[menuLevel])
		menuTop[menuLevel] = menuPos[menuLevel];
	do
	{
		skipped = 0;
		modified = false;
		for (uint8_t r = menuTop[menuLevel]; r < menuPos[menuLevel]; r++)
		{
			UIMenuEntry* ent = (UIMenuEntry*)pgm_read_word(&(entries[r]));
			if (!ent->showEntry())
				skipped++;
		}
		if (menuTop[menuLevel] + skipped + UI_ROWS - 1 < menuPos[menuLevel])
		{
			menuTop[menuLevel] = menuPos[menuLevel] + 1 - UI_ROWS;
			modified = true;
		}
	} while (modified);
}

bool UIDisplay::isWizardActive()
{
	UIMenu* men = (UIMenu*)menu[menuLevel];
	return (HAL::readFlashByte((PGM_P) & (men->menuType)) & 127) == 5;
}
bool UIDisplay::isSticky() {
	UIMenu* men = (UIMenu*)menu[menuLevel];
	uint8_t mt = HAL::readFlashByte((PGM_P) & (men->menuType));
	return ((mt & 128) == 128) || mt == 5;
}

bool UIDisplay::nextPreviousAction(int16_t next, bool allowMoves)
{
	if (Printer::isUIErrorMessage())
	{
		Printer::setUIErrorMessage(false);
		return true;
	}
	millis_t actTime = HAL::timeInMilliseconds();
	millis_t dtReal;
	millis_t dt = dtReal = actTime - lastNextPrev;
	lastNextPrev = actTime;
	if (dt < SPEED_MAX_MILLIS) dt = SPEED_MAX_MILLIS;
	if (dt > SPEED_MIN_MILLIS)
	{
		dt = SPEED_MIN_MILLIS;
		lastNextAccumul = 1;
	}
	float f = (float)(SPEED_MIN_MILLIS - dt) / (float)(SPEED_MIN_MILLIS - SPEED_MAX_MILLIS);
	lastNextAccumul = 1.0f + (float)SPEED_MAGNIFICATION * f * f;
#if UI_DYNAMIC_ENCODER_SPEED
	int16_t dynSp = lastNextAccumul / 16;
	if (dynSp < 1)  dynSp = 1;
	if (dynSp > 30) dynSp = 30;
	next *= dynSp;
#endif

#if UI_HAS_KEYS == 1
	if (menuLevel == 0)
	{
		lastSwitch = HAL::timeInMilliseconds();
		if ((UI_INVERT_MENU_DIRECTION && next < 0) || (!UI_INVERT_MENU_DIRECTION && next > 0))
		{
			menuPos[0]++;
			if (menuPos[0] >= UI_NUM_PAGES)
				menuPos[0] = 0;
		}
		else
		{
			menuPos[0] = (menuPos[0] == 0 ? UI_NUM_PAGES - 1 : menuPos[0] - 1);
		}
		return true;
	}
	UIMenu* men = (UIMenu*)menu[menuLevel];
	uint8_t nr = pgm_read_word_near(&(men->numEntries));
	uint8_t mtype = HAL::readFlashByte((PGM_P) & (men->menuType)) & 127;
	UIMenuEntry** entries = (UIMenuEntry * *)pgm_read_word(&(men->entries));
	UIMenuEntry* ent = (UIMenuEntry*)pgm_read_word(&(entries[menuPos[menuLevel]]));
	UIMenuEntry* testEnt;
	// 0 = Info, 1 = Headline, 2 = submenu ref, 3 = direct action command
	//uint8_t entType = HAL::readFlashByte((PGM_P)&(ent->entryType));
	unsigned int action = pgm_read_word(&(ent->action));
	if (mtype == UI_MENU_TYPE_SUBMENU && activeAction == 0)  // browse through menu items
	{
		if ((UI_INVERT_MENU_DIRECTION && next < 0) || (!UI_INVERT_MENU_DIRECTION && next > 0))
		{
			while (menuPos[menuLevel] + 1 < nr)
			{
				menuPos[menuLevel]++;
				testEnt = (UIMenuEntry*)pgm_read_word(&(entries[menuPos[menuLevel]]));
				if (testEnt->showEntry())
					break;
			}
		}
		else if (menuPos[menuLevel] > 0)
		{
			while (menuPos[menuLevel] > 0)
			{
				menuPos[menuLevel]--;
				testEnt = (UIMenuEntry*)pgm_read_word(&(entries[menuPos[menuLevel]]));
				if (testEnt->showEntry())
					break;
			}
		}
		shift = -2; // reset shift position
		adjustMenuPos();
		return true;
	}
#if SDSUPPORT
	if (mtype == UI_MENU_TYPE_FILE_SELECTOR)  // SD listing
	{
		if ((UI_INVERT_MENU_DIRECTION && next < 0) || (!UI_INVERT_MENU_DIRECTION && next > 0))
		{
			menuPos[menuLevel] += abs(next);
			if (menuPos[menuLevel] > nFilesOnCard) menuPos[menuLevel] = nFilesOnCard;
		}
		else if (menuPos[menuLevel] > 0)
		{
			if (menuPos[menuLevel] > abs(next))
				menuPos[menuLevel] -= abs(next);
			else
				menuPos[menuLevel] = 0;
		}
		if (menuTop[menuLevel] > menuPos[menuLevel])
			menuTop[menuLevel] = menuPos[menuLevel];
		else if (menuTop[menuLevel] + UI_ROWS - 1 < menuPos[menuLevel])
			menuTop[menuLevel] = menuPos[menuLevel] + 1 - UI_ROWS;
		shift = -2; // reset shift position
		return true;
	}
#endif
	if (mtype == UI_MENU_TYPE_MODIFICATION_MENU || mtype == UI_MENU_TYPE_WIZARD) action = pgm_read_word(&(men->id));
	else action = activeAction;
	int16_t increment = next;
	EVENT_START_NEXTPREVIOUS(action, increment);
	switch (action)
	{
	case UI_ACTION_FANSPEED:
		Commands::setFanSpeed(Printer::getFanSpeed() + increment * 3, true);
		break;
	case UI_ACTION_XPOSITION:
		if (!allowMoves) return false;
#if UI_SPEEDDEPENDENT_POSITIONING
		{
			float d = 0.01 * (float)increment * lastNextAccumul;
			if (fabs(d) * 1000 > Printer::maxFeedrate[X_AXIS] * dtReal)
				d *= Printer::maxFeedrate[X_AXIS] * dtReal / (1000 * fabs(d));
			long steps = (long)(d * Printer::axisStepsPerMM[X_AXIS]);
			steps = (increment < 0 ? RMath::min(steps, (long)increment) : RMath::max(steps, (long)increment));
			PrintLine::moveRelativeDistanceInStepsReal(steps, 0, 0, 0, Printer::maxFeedrate[X_AXIS], false, false);
		}
#else
		PrintLine::moveRelativeDistanceInStepsReal(increment, 0, 0, 0, Printer::homingFeedrate[X_AXIS], false, false);
#endif
		Commands::printCurrentPosition(PSTR("UI_ACTION_XPOSITION "));
		break;
	case UI_ACTION_YPOSITION:
		if (!allowMoves) return false;
#if UI_SPEEDDEPENDENT_POSITIONING
		{
			float d = 0.01 * (float)increment * lastNextAccumul;
			if (fabs(d) * 1000 > Printer::maxFeedrate[Y_AXIS] * dtReal)
				d *= Printer::maxFeedrate[Y_AXIS] * dtReal / (1000 * fabs(d));
			long steps = (long)(d * Printer::axisStepsPerMM[Y_AXIS]);
			steps = (increment < 0 ? RMath::min(steps, (long)increment) : RMath::max(steps, (long)increment));
			PrintLine::moveRelativeDistanceInStepsReal(0, steps, 0, 0, Printer::maxFeedrate[Y_AXIS], false, false);
		}
#else
		PrintLine::moveRelativeDistanceInStepsReal(0, increment, 0, 0, Printer::homingFeedrate[Y_AXIS], false, false);
#endif
		Commands::printCurrentPosition(PSTR("UI_ACTION_YPOSITION "));
		break;
	case UI_ACTION_ZPOSITION_NOTEST:
		if (!allowMoves) return false;
		Printer::setNoDestinationCheck(true);
		goto ZPOS1;
	case UI_ACTION_ZPOSITION:
		if (!allowMoves) return false;
	ZPOS1:
#if UI_SPEEDDEPENDENT_POSITIONING
		{
			float d = 0.01 * (float)increment * lastNextAccumul;
			if (fabs(d) * 1000 > Printer::maxFeedrate[Z_AXIS] * dtReal)
				d *= Printer::maxFeedrate[Z_AXIS] * dtReal / (1000 * fabs(d));
			long steps = (long)(d * Printer::axisStepsPerMM[Z_AXIS]);
			steps = (increment < 0 ? RMath::min(steps, (long)increment) : RMath::max(steps, (long)increment));
			PrintLine::moveRelativeDistanceInStepsReal(0, 0, steps, 0, Printer::maxFeedrate[Z_AXIS], false, false);
		}
#else
		PrintLine::moveRelativeDistanceInStepsReal(0, 0, ((long)increment * Printer::axisStepsPerMM[Z_AXIS]) / 100, 0, Printer::homingFeedrate[Z_AXIS], false, false);
#endif
		Printer::setNoDestinationCheck(false);
		Commands::printCurrentPosition(PSTR("UI_ACTION_ZPOSITION "));
		break;

		// marcel calibrate

	case UI_ACTION_CALIBRATE_Z:
		if (!allowMoves) return false;
		PrintLine::moveRelativeDistanceInStepsReal(0, 0, Printer::axisStepsPerMM[Z_AXIS] * increment, 0, Printer::homingFeedrate[Z_AXIS], true, false);
		Printer::setNoDestinationCheck(false);
		Commands::printCurrentPosition(PSTR("UI_ACTION_ZPOSITION_FAST "));
		break;
	case UI_ACTION_CALIBRATE_Z_SMALL:
		if (!allowMoves) return false;
#if UI_SPEEDDEPENDENT_POSITIONING
		{
			float d = 0.01 * (float)increment * lastNextAccumul;
			if (fabs(d) * 1000 > Printer::maxFeedrate[Z_AXIS] * dtReal)
				d *= Printer::maxFeedrate[Z_AXIS] * dtReal / (1000 * fabs(d));
			long steps = (long)(d * Printer::axisStepsPerMM[Z_AXIS]);
			steps = (increment < 0 ? RMath::min(steps, (long)increment) : RMath::max(steps, (long)increment));
			PrintLine::moveRelativeDistanceInStepsReal(0, 0, steps, 0, Printer::maxFeedrate[Z_AXIS], false, false);
		}
#else
		PrintLine::moveRelativeDistanceInStepsReal(0, 0, ((long)increment * Printer::axisStepsPerMM[Z_AXIS]) / 100, 0, Printer::homingFeedrate[Z_AXIS], false, false);
		//PrintLine::moveRelativeDistanceInStepsReal(MIDDLE_X_POS, MIDDLE_Y_POS, ((long)increment * Printer::axisStepsPerMM[Z_AXIS]) / 100, 0, Printer::homingFeedrate[Z_AXIS], false, false);
#endif
		Printer::setNoDestinationCheck(false);
		Commands::printCurrentPosition(PSTR("UI_ACTION_ZPOSITION "));
		break;

		// marcel calibrate ende

	case UI_ACTION_XPOSITION_FAST:
		if (!allowMoves) return false;
		PrintLine::moveRelativeDistanceInStepsReal(Printer::axisStepsPerMM[X_AXIS] * increment, 0, 0, 0, Printer::homingFeedrate[X_AXIS], true, false);
		Commands::printCurrentPosition(PSTR("UI_ACTION_XPOSITION_FAST "));
		break;
	case UI_ACTION_YPOSITION_FAST:
		if (!allowMoves) return false;
		PrintLine::moveRelativeDistanceInStepsReal(0, Printer::axisStepsPerMM[Y_AXIS] * increment, 0, 0, Printer::homingFeedrate[Y_AXIS], true, false);
		Commands::printCurrentPosition(PSTR("UI_ACTION_YPOSITION_FAST "));
		break;
	case UI_ACTION_ZPOSITION_FAST_NOTEST:
		if (!allowMoves) return false;
		Printer::setNoDestinationCheck(true);
		goto ZPOS2;
	case UI_ACTION_ZPOSITION_FAST:
		if (!allowMoves) return false;

	ZPOS2:
		PrintLine::moveRelativeDistanceInStepsReal(0, 0, Printer::axisStepsPerMM[Z_AXIS] * increment, 0, Printer::homingFeedrate[Z_AXIS], true, false);
		Printer::setNoDestinationCheck(false);
		Commands::printCurrentPosition(PSTR("UI_ACTION_ZPOSITION_FAST "));
		break;
	case UI_ACTION_EPOSITION:
		if (!allowMoves) return false;
		PrintLine::moveRelativeDistanceInSteps(0, 0, 0, Printer::axisStepsPerMM[E_AXIS] * increment / Printer::extrusionFactor, UI_SET_EXTRUDER_FEEDRATE, true, false, false);
		Commands::printCurrentPosition(PSTR("UI_ACTION_EPOSITION "));
		break;
	case UI_ACTION_EPOS_KART:
		if (!allowMoves) return false;
    if (increment > 0){
      GCode::executeFString("G91");
      GCode::executeFString("G1 E1");
      GCode::executeFString("G90");
    } else {
      GCode::executeFString("G91");
      GCode::executeFString("G1 E-1");
      GCode::executeFString("G90");
    }
		//PrintLine::moveRelativeDistanceInSteps(0, 0, 0, 0.5*Printer::axisStepsPerMM[E_AXIS] * increment / Printer::extrusionFactor, UI_SET_EXTRUDER_FEEDRATE, true, true, false);
		Commands::printCurrentPosition(PSTR("UI_ACTION_EPOS_KART"));
    //Commands::printCurrentPosition(PSTR("Turn right to Extrude"));
    predoseVarPos = static_cast<float>(Printer::currentPositionSteps[E_AXIS]) * Printer::invAxisStepsPerMM[E_AXIS];
    Printer::currentPositionSteps[E_AXIS] = 0;
    //EEPROM::writeNewEpos(Printer::newECode);
    HAL::eprSetFloat(999, Printer::newECode);
    Com::print(HAL::eprGetFloat(0));
		break;
#if FEATURE_RETRACTION
	case UI_ACTION_WIZARD_FILAMENTCHANGE: // filament change is finished
		Extruder::current->retractDistance(-increment);
		Commands::waitUntilEndOfAllMoves();
		Extruder::current->disableCurrentExtruderMotor();
		break;
#endif
	case UI_ACTION_Z_BABYSTEPS:
#if FEATURE_BABYSTEPPING
	{
		previousMillisCmd = HAL::timeInMilliseconds();
#if UI_DYNAMIC_ENCODER_SPEED
		increment /= dynSp; // we need fixed speeds or we get in trouble here!
#endif
		if ((abs((int)Printer::zBabystepsMissing + (increment * BABYSTEP_MULTIPLICATOR))) < 20000)
		{
			InterruptProtectedBlock noint;
			Printer::zBabystepsMissing += increment * BABYSTEP_MULTIPLICATOR;
			zBabySteps += increment * BABYSTEP_MULTIPLICATOR;
		}
	}
#endif
	break;
	case UI_ACTION_HEATED_BED_TEMP:
#if HAVE_HEATED_BED
	{
		int tmp = (int)heatedBedController.targetTemperatureC;
		if (tmp < UI_SET_MIN_HEATED_BED_TEMP) tmp = 0;
		if (tmp == 0 && increment > 0) tmp = UI_SET_MIN_HEATED_BED_TEMP;
		else tmp += increment;
		if (tmp < UI_SET_MIN_HEATED_BED_TEMP) tmp = 0;
		else if (tmp > UI_SET_MAX_HEATED_BED_TEMP) tmp = UI_SET_MAX_HEATED_BED_TEMP;
		Extruder::setHeatedBedTemperature(tmp);
	}
#endif
	break;

	case UI_ACTION_EXTRUDER0_TEMP:
#if NUM_EXTRUDER > 1
	case UI_ACTION_EXTRUDER1_TEMP:
#endif
#if NUM_EXTRUDER > 2
	case UI_ACTION_EXTRUDER2_TEMP:
#endif
#if NUM_EXTRUDER > 3
	case UI_ACTION_EXTRUDER3_TEMP:
#endif
#if NUM_EXTRUDER > 4
	case UI_ACTION_EXTRUDER4_TEMP:
#endif
#if NUM_EXTRUDER > 5
	case UI_ACTION_EXTRUDER5_TEMP:
#endif
	{
		int tmp = (int)extruder[action - UI_ACTION_EXTRUDER0_TEMP].tempControl.targetTemperatureC;
		if (tmp < UI_SET_MIN_EXTRUDER_TEMP) tmp = 0;
		if (tmp == 0 && increment > 0) tmp = UI_SET_MIN_EXTRUDER_TEMP;
		else tmp += increment;
		if (tmp < UI_SET_MIN_EXTRUDER_TEMP) tmp = 0;
		else if (tmp > UI_SET_MAX_EXTRUDER_TEMP) tmp = UI_SET_MAX_EXTRUDER_TEMP;
		Extruder::setTemperatureForExtruder(tmp, action - UI_ACTION_EXTRUDER0_TEMP);
	}
	break;
	case UI_ACTION_FEEDRATE_MULTIPLY:
	{
		int fr = Printer::feedrateMultiply;
		INCREMENT_MIN_MAX(fr, 1, 25, 500);
		Commands::changeFeedrateMultiply(fr);
	}
	break;
	case UI_ACTION_FLOWRATE_MULTIPLY:
	{
		INCREMENT_MIN_MAX(Printer::extrudeMultiply, 1, 25, 500);
		Commands::changeFlowrateMultiply(Printer::extrudeMultiply);
	}
	break;
#if UI_BED_COATING
	case UI_ACTION_COATING_CUSTOM:
		INCREMENT_MIN_MAX(Printer::zBedOffset, 0.01, -1.0, 199.0);
		break;
#endif
	case UI_ACTION_STEPPER_INACTIVE:
	{
		uint8_t inactT = stepperInactiveTime / 60000;
		INCREMENT_MIN_MAX(inactT, 1, 0, 240);
		stepperInactiveTime = inactT * 60000;
	}
	break;
	case UI_ACTION_MAX_INACTIVE:
	{
		uint8_t inactT = maxInactiveTime / 60000;
		INCREMENT_MIN_MAX(inactT, 1, 0, 240);
		maxInactiveTime = inactT * 60000;
	}
	break;
	case UI_ACTION_PRINT_ACCEL_X:
	case UI_ACTION_PRINT_ACCEL_Y:
	case UI_ACTION_PRINT_ACCEL_Z:
#if DRIVE_SYSTEM != DELTA
		INCREMENT_MIN_MAX(Printer::maxAccelerationMMPerSquareSecond[action - UI_ACTION_PRINT_ACCEL_X], ((action == UI_ACTION_PRINT_ACCEL_Z) ? 1 : 100), 0, 10000);
#else
		INCREMENT_MIN_MAX(Printer::maxAccelerationMMPerSquareSecond[action - UI_ACTION_PRINT_ACCEL_X], 100, 0, 10000);
#endif
		Printer::updateDerivedParameter();
		break;
	case UI_ACTION_MOVE_ACCEL_X:
	case UI_ACTION_MOVE_ACCEL_Y:
	case UI_ACTION_MOVE_ACCEL_Z:
#if DRIVE_SYSTEM != DELTA
		INCREMENT_MIN_MAX(Printer::maxTravelAccelerationMMPerSquareSecond[action - UI_ACTION_MOVE_ACCEL_X], ((action == UI_ACTION_MOVE_ACCEL_Z) ? 1 : 100), 0, 10000);
#else
		INCREMENT_MIN_MAX(Printer::maxTravelAccelerationMMPerSquareSecond[action - UI_ACTION_MOVE_ACCEL_X], 100, 0, 10000);
#endif
		Printer::updateDerivedParameter();
		break;
	case UI_ACTION_MAX_JERK:
		INCREMENT_MIN_MAX(Printer::maxJerk, 0.1, 1, 99.9);
		break;
#if DRIVE_SYSTEM != DELTA
	case UI_ACTION_MAX_ZJERK:
		INCREMENT_MIN_MAX(Printer::maxZJerk, 0.1, 0.1, 99.9);
		break;
#endif
	case UI_ACTION_HOMING_FEEDRATE_X:
	case UI_ACTION_HOMING_FEEDRATE_Y:
	case UI_ACTION_HOMING_FEEDRATE_Z:
		INCREMENT_MIN_MAX(Printer::homingFeedrate[action - UI_ACTION_HOMING_FEEDRATE_X], 1, 1, 1000);
		break;

	case UI_ACTION_MAX_FEEDRATE_X:
	case UI_ACTION_MAX_FEEDRATE_Y:
	case UI_ACTION_MAX_FEEDRATE_Z:
		INCREMENT_MIN_MAX(Printer::maxFeedrate[action - UI_ACTION_MAX_FEEDRATE_X], 1, 1, 1000);
		break;

	case UI_ACTION_STEPS_X:
	case UI_ACTION_STEPS_Y:
	case UI_ACTION_STEPS_Z:
		INCREMENT_MIN_MAX(Printer::axisStepsPerMM[action - UI_ACTION_STEPS_X], 0.1, 0, 999);
		Printer::updateDerivedParameter();
		break;

	case UI_ACTION_XOFF:
	case UI_ACTION_YOFF:
	{
		float tmp = -Printer::coordinateOffset[action - UI_ACTION_XOFF];
		INCREMENT_MIN_MAX(tmp, 1, -999, 999);
		Printer::coordinateOffset[action - UI_ACTION_XOFF] = -tmp;
	}
	break;
	case UI_ACTION_ZOFF:
	{
		float tmp = -Printer::coordinateOffset[Z_AXIS];
		INCREMENT_MIN_MAX(tmp, 0.01, -9.99, 9.99);
		Printer::coordinateOffset[Z_AXIS] = -tmp;
	}
	break;

	case UI_ACTION_BAUDRATE:
#if EEPROM_MODE != 0
	{
		int16_t p = 0;
		int32_t rate;
		do
		{
			rate = pgm_read_dword(&(baudrates[(uint8_t)p]));
			if (rate == baudrate) break;
			p++;
		} while (rate != 0);
		if (rate == 0) p -= 2;
		p += increment;
		if (p < 0) p = 0;
		if (p > static_cast<int16_t>(sizeof(baudrates) / 4) - 2)
			p = sizeof(baudrates) / 4 - 2;
		baudrate = pgm_read_dword(&(baudrates[p]));
	}
#endif
	break;
	case UI_ACTION_SERVOPOS:
#if FEATURE_SERVO > 0  && UI_SERVO_CONTROL > 0
		INCREMENT_MIN_MAX(servoPosition, 5, 500, 2500);
		HAL::servoMicroseconds(UI_SERVO_CONTROL - 1, servoPosition, 500);
#endif
		break;
#if TEMP_PID
	case UI_ACTION_PID_PGAIN:
		INCREMENT_MIN_MAX(currHeaterForSetup->pidPGain, 0.1, 0, 200);
		break;
	case UI_ACTION_PID_IGAIN:
		INCREMENT_MIN_MAX(currHeaterForSetup->pidIGain, 0.01, 0, 100);
		if (&Extruder::current->tempControl == currHeaterForSetup)
			Extruder::selectExtruderById(Extruder::current->id);
		break;
	case UI_ACTION_PID_DGAIN:
		INCREMENT_MIN_MAX(currHeaterForSetup->pidDGain, 0.1, 0, 200);
		break;
	case UI_ACTION_DRIVE_MIN:
		INCREMENT_MIN_MAX(currHeaterForSetup->pidDriveMin, 1, 1, 255);
		break;
	case UI_ACTION_DRIVE_MAX:
		INCREMENT_MIN_MAX(currHeaterForSetup->pidDriveMax, 1, 1, 255);
		break;
	case UI_ACTION_PID_MAX:
		INCREMENT_MIN_MAX(currHeaterForSetup->pidMax, 1, 1, 255);
		break;
#endif
	case UI_ACTION_X_OFFSET:
		INCREMENT_MIN_MAX(Extruder::current->xOffset, 1, -99999, 99999);
		Extruder::selectExtruderById(Extruder::current->id);
		break;
	case UI_ACTION_Y_OFFSET:
		INCREMENT_MIN_MAX(Extruder::current->yOffset, 1, -99999, 99999);
		Extruder::selectExtruderById(Extruder::current->id);
		break;
	case UI_ACTION_EXTR_STEPS:
		INCREMENT_MIN_MAX(Extruder::current->stepsPerMM, 0.1, 1, 9999);
		Extruder::selectExtruderById(Extruder::current->id);
		break;
	case UI_ACTION_EXTR_ACCELERATION:
		INCREMENT_MIN_MAX(Extruder::current->maxAcceleration, 10, 10, 99999);
		Extruder::selectExtruderById(Extruder::current->id);
		break;
	case UI_ACTION_EXTR_MAX_FEEDRATE:
		INCREMENT_MIN_MAX(Extruder::current->maxFeedrate, 1, 1, 999);
		Extruder::selectExtruderById(Extruder::current->id);
		break;
	case UI_ACTION_EXTR_START_FEEDRATE:
		INCREMENT_MIN_MAX(Extruder::current->maxStartFeedrate, 1, 1, 999);
		Extruder::selectExtruderById(Extruder::current->id);
		break;
	case UI_ACTION_EXTR_HEATMANAGER:
		INCREMENT_MIN_MAX(currHeaterForSetup->heatManager, 1, 0, 3);
		Printer::setMenuMode(MENU_MODE_FULL_PID, currHeaterForSetup->heatManager == 1); // show PIDS only with PID controller selected
		Printer::setMenuMode(MENU_MODE_DEADTIME, currHeaterForSetup->heatManager == 3);
		break;
	case UI_ACTION_EXTR_WATCH_PERIOD:
		INCREMENT_MIN_MAX(Extruder::current->watchPeriod, 1, 0, 999);
		break;
#if RETRACT_DURING_HEATUP
	case UI_ACTION_EXTR_WAIT_RETRACT_TEMP:
		INCREMENT_MIN_MAX(Extruder::current->waitRetractTemperature, 1, 100, UI_SET_MAX_EXTRUDER_TEMP);
		break;
	case UI_ACTION_EXTR_WAIT_RETRACT_UNITS:
		INCREMENT_MIN_MAX(Extruder::current->waitRetractUnits, 1, 0, 99);
		break;
#endif
#if USE_ADVANCE
#if ENABLE_QUADRATIC_ADVANCE
	case UI_ACTION_ADVANCE_K:
		INCREMENT_MIN_MAX(Extruder::current->advanceK, 1, 0, 200);
		break;
#endif
	case UI_ACTION_ADVANCE_L:
		INCREMENT_MIN_MAX(Extruder::current->advanceL, 1, 0, 600);
		break;
#endif
	}
#if UI_AUTORETURN_TO_MENU_AFTER!=0
	ui_autoreturn_time = HAL::timeInMilliseconds() + UI_AUTORETURN_TO_MENU_AFTER;
#endif
#endif
	return true;
}

#if UI_BED_COATING
void UIDisplay::menuAdjustHeight(const UIMenu * men, float offset)
{
#if EEPROM_MODE != 0
	//If there is something to change
	if (EEPROM::zProbeZOffset() != offset)
	{
		HAL::eprSetFloat(EPR_Z_PROBE_Z_OFFSET, offset);
		EEPROM::storeDataIntoEEPROM(false);
	}
#endif
	Printer::zBedOffset = offset;
	//Display message
	pushMenu(men, false);
	BEEP_SHORT;
	Printer::homeAxis(true, true, true);
	Commands::printCurrentPosition(PSTR("UI_ACTION_HOMEALL "));
	menuLevel = 0;
	activeAction = 0;
	//UI_STATUS_UPD_F(Com::translatedF(UI_TEXT_PRINTER_READY_ID));
	UI_STATUS_UPD_F(Com::translatedF(UI_TEXT_CLICK_START_ID));
}
#endif

void UIDisplay::finishAction(unsigned int action)
{
#if UI_BED_COATING
	if (action == UI_ACTION_COATING_CUSTOM)
	{
		menuAdjustHeight(&ui_menu_coating_custom, Printer::zBedOffset);
	}
#endif
}
// Actions are events from user input. Depending on the current state, each
// action can behave differently. Other actions do always the same like home, disable extruder etc.

int UIDisplay::executeAction(unsigned int action, bool allowMoves)
{
	int ret = 0;
#if UI_HAS_KEYS == 1
	if (action & UI_ACTION_TOPMENU)  // Go to start menu
	{
		menuLevel = 0;
	}
	action &= 8191; // strip out higher level flags
	if (action >= 2000 && action < 3000)
	{
		setStatusP(Com::translatedF(UI_TEXT_STRING_ACTION_ID));
	}
	else
		switch (action)
		{
		case UI_ACTION_OK:
			ret = okAction(allowMoves);
			break;
		case UI_ACTION_BACK:
			if (uid.isWizardActive()) break; // wizards can not exit before finished
			if (fileVar == 1) {
				//menuLevel = 0;
				//fileVar = 0;
        pushMenu(&ui_menu_sd_fileselector, false);
        //popMenu(false);
			} else {
			//pushMenu(&ui_menu_sd_fileselector, false);
			popMenu(false);
			}
			break;
		case UI_ACTION_NEXT:
			if (!nextPreviousAction(1, allowMoves))
				ret = UI_ACTION_NEXT;
			break;
		case UI_ACTION_PREVIOUS:
			if (!nextPreviousAction(-1, allowMoves))
				ret = UI_ACTION_PREVIOUS;
			break;
		case UI_ACTION_MENU_UP:
			if (menuLevel > 0) menuLevel--;
			break;
		case UI_ACTION_TOP_MENU:
			menuLevel = 0;
			break;
		case UI_ACTION_EMERGENCY_STOP:
			Commands::emergencyStop();
			break;
		case UI_ACTION_HOME_ALL:
			if (!allowMoves) return UI_ACTION_HOME_ALL;
			menuLevel = 0;
			Printer::homeAxis(true, true, true);
			Commands::printCurrentPosition(PSTR("UI_ACTION_HOMEALL "));
			break;
		case UI_ACTION_HOME_X:
			if (!allowMoves) return UI_ACTION_HOME_X;
			Printer::homeAxis(true, false, false);
			Commands::printCurrentPosition(PSTR("UI_ACTION_HOME_X "));
			break;
		case UI_ACTION_HOME_Y:
			if (!allowMoves) return UI_ACTION_HOME_Y;
			Printer::homeAxis(false, true, false);
			Commands::printCurrentPosition(PSTR("UI_ACTION_HOME_Y "));
			break;
		case UI_ACTION_HOME_Z:
			if (!allowMoves) return UI_ACTION_HOME_Z;
			Printer::homeAxis(false, false, true);
			Commands::printCurrentPosition(PSTR("UI_ACTION_HOME_Z "));
			break;
		case UI_ACTION_SET_ORIGIN:
			if (!allowMoves) return UI_ACTION_SET_ORIGIN;
			Printer::setOrigin(0, 0, 0);
			break;
		case UI_ACTION_DEBUG_ECHO:
			Printer::toggleEcho();
			break;
		case UI_ACTION_DEBUG_INFO:
			Printer::toggleInfo();
			break;
		case UI_ACTION_DEBUG_ERROR:
			Printer::toggleErrors();
			break;
		case UI_ACTION_DEBUG_ENDSTOP:
			Printer::toggleEndStop();
			break;
		case UI_ACTION_DEBUG_DRYRUN:
			Printer::toggleDryRun();
			if (Printer::debugDryrun())  // simulate movements without printing
			{
				for (int i = 0; i < NUM_EXTRUDER; i++)
					Extruder::setTemperatureForExtruder(0, i);
#if HAVE_HEATED_BED
				Extruder::setHeatedBedTemperature(0);
#endif
			}
			break;
		case UI_ACTION_POWER:
#if PS_ON_PIN >= 0 // avoid compiler errors when the power supply pin is disabled
			Commands::waitUntilEndOfAllMoves();
			//SET_OUTPUT(PS_ON_PIN); //GND
			TOGGLE(PS_ON_PIN);
#endif
			break;
#if CASE_LIGHTS_PIN >= 0
		case UI_ACTION_LIGHTS_ONOFF:
			TOGGLE(CASE_LIGHTS_PIN);
#ifdef CASE_LIGHTS2_PIN
			TOGGLE(CASE_LIGHTS2_PIN);
#endif
			Printer::reportCaseLightStatus();
			UI_STATUS_F(Com::translatedF(UI_TEXT_LIGHTS_ONOFF_ID));
			break;
#endif
		case UI_ACTION_PREHEAT_PLA: // Marcel preheat choco normal (33°C)
			//UI_STATUS_F(Com::translatedF(UI_TEXT_PREHEAT_PLA_ID));
			//Extruder::setTemperatureForExtruder(UI_SET_PRESET_EXTRUDER_TEMP_PLA, 0);
			Extruder::setTemperatureForExtruder(UI_SET_PRESET_EXTRUDER_TEMP_CHOCO, 0);
			preHeatChoco = 1;
			chocoChooseVar = 1;
#if NUM_EXTRUDER > 1
			Extruder::setTemperatureForExtruder(UI_SET_PRESET_EXTRUDER_TEMP_PLA, 1);
#endif
#if NUM_EXTRUDER > 2
			Extruder::setTemperatureForExtruder(UI_SET_PRESET_EXTRUDER_TEMP_PLA, 2);
#endif
#if HAVE_HEATED_BED
			Extruder::setHeatedBedTemperature(UI_SET_PRESET_HEATED_BED_TEMP_PLA);
#endif
			menuLevel = 0;
			break;
		case UI_ACTION_PREHEAT_ABS: // Marcel choco schrift (32°C)
			UI_STATUS_F(Com::translatedF(UI_TEXT_PREHEAT_ABS_ID));
			//Extruder::setTemperatureForExtruder(UI_SET_PRESET_EXTRUDER_TEMP_ABS, 0);
			Extruder::setTemperatureForExtruder(UI_SET_PRESET_EXTRUDER_TEMP_CHOCO, 0);
			menuLevel = 0;
			preHeatChoco = 1;
			textChooseVar = 1;
#if NUM_EXTRUDER > 1
			Extruder::setTemperatureForExtruder(UI_SET_PRESET_EXTRUDER_TEMP_ABS, 1);
#endif
#if NUM_EXTRUDER > 2
			Extruder::setTemperatureForExtruder(UI_SET_PRESET_EXTRUDER_TEMP_ABS, 2);
#endif
#if HAVE_HEATED_BED
			Extruder::setHeatedBedTemperature(UI_SET_PRESET_HEATED_BED_TEMP_ABS);
#endif
			break;
		case UI_ACTION_COOLDOWN:
			UI_STATUS_F(Com::translatedF(UI_TEXT_COOLDOWN_ID));
			for (int i = 0; i < NUM_EXTRUDER; i++)
				Extruder::setTemperatureForExtruder(0, i);
#if HAVE_HEATED_BED
			Extruder::setHeatedBedTemperature(0);
#endif
			break;
		case UI_ACTION_HEATED_BED_OFF:
#if HAVE_HEATED_BED
			Extruder::setHeatedBedTemperature(0);
#endif
			break;
		case UI_ACTION_EXTRUDER0_OFF:
#if NUM_EXTRUDER > 1
		case UI_ACTION_EXTRUDER1_OFF:
#endif
#if NUM_EXTRUDER > 2
		case UI_ACTION_EXTRUDER2_OFF:
#endif
#if NUM_EXTRUDER > 3
		case UI_ACTION_EXTRUDER3_OFF:
#endif
#if NUM_EXTRUDER > 4
		case UI_ACTION_EXTRUDER4_OFF:
#endif
#if NUM_EXTRUDER > 5
		case UI_ACTION_EXTRUDER5_OFF:
#endif
			Extruder::setTemperatureForExtruder(0, action - UI_ACTION_EXTRUDER0_OFF);
			break;
		case UI_ACTION_DISABLE_STEPPER:
			Printer::kill(true);
			break;
		case UI_ACTION_RESET_EXTRUDER:
			Printer::currentPositionSteps[E_AXIS] = 0;
			break;
		case UI_ACTION_EXTRUDER_RELATIVE:
			Printer::relativeExtruderCoordinateMode = !Printer::relativeExtruderCoordinateMode;
			break;
		case UI_ACTION_SELECT_EXTRUDER0:
#if NUM_EXTRUDER > 1
		case UI_ACTION_SELECT_EXTRUDER1:
#endif
#if NUM_EXTRUDER > 2
		case UI_ACTION_SELECT_EXTRUDER2:
#endif
#if NUM_EXTRUDER > 3
		case UI_ACTION_SELECT_EXTRUDER3:
#endif
#if NUM_EXTRUDER > 4
		case UI_ACTION_SELECT_EXTRUDER4:
#endif
#if NUM_EXTRUDER > 5
		case UI_ACTION_SELECT_EXTRUDER5:
#endif
			if (!allowMoves) return action;
			Extruder::selectExtruderById(action - UI_ACTION_SELECT_EXTRUDER0);
			currHeaterForSetup = &(Extruder::current->tempControl);
			Printer::setMenuMode(MENU_MODE_FULL_PID, currHeaterForSetup->heatManager == 1);
			Printer::setMenuMode(MENU_MODE_DEADTIME, currHeaterForSetup->heatManager == 3);
			break;
#if FEATURE_DITTO_PRINTING
		case UI_DITTO_0:
		case UI_DITTO_1:
		case UI_DITTO_2:
		case UI_DITTO_3:
			Extruder::dittoMode = action - UI_DITTO_0;
			break;
#endif
#if EEPROM_MODE != 0
		case UI_ACTION_STORE_EEPROM:
			EEPROM::storeDataIntoEEPROM(false);
			pushMenu(&ui_menu_eeprom_saved, false);
			BEEP_LONG;
			break;
		case UI_ACTION_LOAD_EEPROM:
			EEPROM::readDataFromEEPROM(true);
			Extruder::selectExtruderById(Extruder::current->id);
			pushMenu(&ui_menu_eeprom_loaded, false);
			BEEP_LONG;
			break;
#endif
#if SDSUPPORT
		case UI_ACTION_SD_DELETE:
			if (sd.sdactive)
			{
        fileVar = 1;
				pushMenu(&ui_menu_sd_fileselector, false);
			}
			else
			{
				UI_ERROR_P(Com::translatedF(UI_TEXT_NOSDCARD_ID));
			}
			break;
		case UI_ACTION_SD_PRINT:
			if (sd.sdactive)
			{
        fileVar = 1;
				pushMenu(&ui_menu_sd_fileselector, false);
			}
			break;
		case UI_ACTION_SD_PAUSE:
			if (!allowMoves)
				ret = UI_ACTION_SD_PAUSE;
			else
				sd.pausePrint(true);
			break;
		case UI_ACTION_SD_CONTINUE:
			if (!allowMoves) ret = UI_ACTION_SD_CONTINUE;
			else sd.continuePrint(true);
			break;
		case UI_ACTION_SD_PRI_PAU_CONT:
			if (!allowMoves) ret = UI_ACTION_SD_PRI_PAU_CONT;
			else
			{
				if (Printer::isMenuMode(MENU_MODE_SD_PRINTING + MENU_MODE_SD_PAUSED))
					sd.continuePrint();
				else if (Printer::isMenuMode(MENU_MODE_SD_PRINTING))
					sd.pausePrint(true);
				else if (sd.sdactive)
          fileVar = 1;
					pushMenu(&ui_menu_sd_fileselector, false);
			}
			break;
	  case UI_ACTION_STOP_EXTRUDE:
      menuLevel = 0;
      HAL::eprSetFloat(100, 0);
      pushMenu(&ui_menu_sd_askstop, false);
      break;
      
		case UI_ACTION_SD_STOP:
			if (!allowMoves) ret = UI_ACTION_SD_STOP;
			else sd.stopPrint();
			chocoChooseVar = 0;
			textChooseVar = 0;
			Extruder::setTemperatureForExtruder(UI_SET_PRESET_EXTRUDER_TEMP_PLA, 0);
			menuLevel = 0;
			uid.executeAction(UI_ACTION_ANOTHER_OBJECT, false);
      Com::printFLN(PSTR("im stop action"));
      GCode::executeFString("G91");
      GCode::executeFString("G1 E-0.8");
      GCode::executeFString("G1 Z5");
      GCode::executeFString("G90");
      Printer::homeAxis(true,true,false);
      HAL::eprSetFloat(999, Printer::newECode);
			//pushMenu(&ui_menu_another, false);
			break;
    
		case UI_ACTION_SD_UNMOUNT:
			sd.unmount();
			break;
		case UI_ACTION_SD_MOUNT:
			sd.mount();
			break;
		case UI_ACTION_MENU_SDCARD:
			pushMenu(&ui_menu_sd, false);
			break;
#endif
#if FAN_PIN>-1 && FEATURE_FAN_CONTROL
		case UI_ACTION_FAN_OFF:
		case UI_ACTION_FAN_25:
		case UI_ACTION_FAN_50:
		case UI_ACTION_FAN_75:
			Commands::setFanSpeed((action - UI_ACTION_FAN_OFF) * 64, true);
			break;
		case UI_ACTION_FAN_FULL:
			Commands::setFanSpeed(255, true);
			break;
		case UI_ACTION_FAN_SUSPEND:
		{
			static uint8_t lastFanSpeed = 255;
			if (Printer::getFanSpeed() == 0)
				Commands::setFanSpeed(lastFanSpeed, true);
			else
			{
				lastFanSpeed = Printer::getFanSpeed();
				Commands::setFanSpeed(0, true);
			}
		}
		break;
		case UI_ACTION_IGNORE_M106:
			Printer::flag2 ^= PRINTER_FLAG2_IGNORE_M106_COMMAND;
			break;
#endif
		case UI_ACTION_MENU_XPOS:
			pushMenu(&ui_menu_xpos, false);
			break;
		case UI_ACTION_MENU_YPOS:
			pushMenu(&ui_menu_ypos, false);
			break;
		case UI_ACTION_MENU_ZPOS:
			pushMenu(&ui_menu_zpos, false);
			break;
		case UI_ACTION_MENU_XPOSFAST:
			pushMenu(&ui_menu_xpos_fast, false);
			break;
		case UI_ACTION_MENU_YPOSFAST:
			pushMenu(&ui_menu_ypos_fast, false);
			break;
		case UI_ACTION_MENU_ZPOSFAST:
			pushMenu(&ui_menu_zpos_fast, false);
			break;
		case UI_ACTION_MENU_QUICKSETTINGS:
			pushMenu(&ui_menu_quick, false);
			break;
		case UI_ACTION_MENU_EXTRUDER:
			pushMenu(&ui_menu_extruder, false);
			break;
		case UI_ACTION_MENU_POSITIONS:
			pushMenu(&ui_menu_positions, false);
			break;
#ifdef UI_USERMENU1
		case UI_ACTION_SHOW_USERMENU1:
			Com::printFLN(PSTR("menu aktiv"));
			pushMenu(&UI_USERMENU1, false);
			break;
#endif
#ifdef UI_USERMENU2
		case UI_ACTION_SHOW_USERMENU2:
			pushMenu(&UI_USERMENU2, false);
			break;
#endif
#ifdef UI_USERMENU3
		case UI_ACTION_SHOW_USERMENU3:
			pushMenu(&UI_USERMENU3, false);
			break;
#endif
#ifdef UI_USERMENU4
		case UI_ACTION_SHOW_USERMENU4:
			pushMenu(&UI_USERMENU4, false);
			break;
#endif
#ifdef UI_USERMENU5
		case UI_ACTION_SHOW_USERMENU5:
			pushMenu(&UI_USERMENU5, false);
			break;
#endif
#ifdef UI_USERMENU6
		case UI_ACTION_SHOW_USERMENU6:
			pushMenu(&UI_USERMENU6, false);
			break;
#endif
#ifdef UI_USERMENU7
		case UI_ACTION_SHOW_USERMENU7:
			pushMenu(&UI_USERMENU7, false);
			break;
#endif
#ifdef UI_USERMENU8
		case UI_ACTION_SHOW_USERMENU8:
			pushMenu(&UI_USERMENU8, false);
			break;
#endif
#ifdef UI_USERMENU9
		case UI_ACTION_SHOW_USERMENU9:
			pushMenu(&UI_USERMENU9, false);
			break;
#endif
#ifdef UI_USERMENU10
		case UI_ACTION_SHOW_USERMENU10:
			pushMenu(&UI_USERMENU10, false);
			break;
#endif
			//new menus

		case UI_ACTION_TURN_OFF_NO:
			menuLevel = 0;
      startVariable = 0;
			//UI_STATUS_F(Com::translatedF(UI_TEXT_PRINTER_READY_ID));
      UI_STATUS_F(Com::translatedF(UI_TEXT_CLICK_START_ID));
			break;
      
		case UI_ACTION_OPEN_FILES:
			printVar = 1;
      startVariable = 0;
      fileVar = 1;
      GCode::commandsReceivingWritePosition = 0;
      startVariable = 0;
      sd.stopPrint();
      Printer::setMenuMode(MENU_MODE_SD_PRINTING, false);
      menuLevel = 0;
      sd.sdmode = 0;
			pushMenu(&ui_menu_sd_fileselector, false);
      for (int i; i < 29; i++){
        fileNameLCD[i] = ' ';
        fileNameLCDAsk[i] = ' ';  
      }
			break;

    case UI_ACTION_EMPTY_CART_NOW:
      menuLevel = 0;
      uid.executeAction(UI_ACTION_REMOVE_CART, false);
      //pushMenu(&ui_menu_remove_cart, true);
      break;

    case UI_ACTION_EMPTY_CART_OPEN:
      sd.stopPrint();
      GCode::executeFString("G91");
      GCode::executeFString("G1 E-0.8");
      GCode::executeFString("G90");
      pushMenu(&ui_menu_empty_cart, true);
      //PrintLine::moveRelativeDistanceInSteps(0, 0, 0, -0.8*Printer::axisStepsPerMM[E_AXIS], UI_SET_EXTRUDER_FEEDRATE, true, true, false);
      Printer::moveToReal(IGNORE_COORDINATE, IGNORE_COORDINATE, 48, IGNORE_COORDINATE, Printer::homingFeedrate[Z_AXIS]);
      Printer::homeAxis(true,true,false);
      HAL::eprSetFloat(999, Printer::newECode);
      break;

    case UI_ACTION_CALI:
      //Printer::homeAxis(false, false, true);
      //Printer::moveToReal(IGNORE_COORDINATE, IGNORE_COORDINATE, 5, IGNORE_COORDINATE, Printer::homingFeedrate[Z_AXIS]);
      Printer::homeAxis(false, false, true);
      menuLevel = 0;
      pushMenu(&ui_menu_cali_2, false);
      break;

    case UI_ACTION_CALI_AGAIN:  
      Printer::moveToReal(IGNORE_COORDINATE, IGNORE_COORDINATE, 5, IGNORE_COORDINATE, Printer::homingFeedrate[Z_AXIS]);
      Printer::homeAxis(false, false, true);
      break;

    case UI_ACTION_GO_PRODUCE:
      pushMenu(&ui_menu_choose, false);
      break;

    case UI_ACTION_GO_PRODUCE_NO:
      menuLevel = 0;
      break;

    case UI_ACTION_CALI_BACK:
      UI_STATUS_F(Com::translatedF(UI_TEXT_CLICK_START_ID));
      menuLevel = 0;
      break;

    case UI_ACTION_BACK_MAINT:
      menuLevel = 0;
      pushMenu(&ui_menu_maint, false);
      break;

    case UI_ACTION_CALI_ASK:
      pushMenu(&ui_menu_cali, false);
      break;

    case UI_ACTION_CLEAN_EX:
      pushMenu(&ui_menu_clean_pre, false);
      cleanEx = 1;
      break;

    case UI_ACTION_CLEAN_GO:
      eposVariable = 0;
      uid.executeAction(UI_ACTION_YES_CLEAN_NOW_1, false);
      break;

    case UI_ACTION_FACTORY_RESET:
      menuLevel = 0;
      HAL::eprSetFloat(1021, 0);
      break;

    case UI_ACTION_NEW_CART_YES:
      break;

		case UI_ACTION_START_PRINT:
			menuLevel = 0;
      startVariable = 0;
      if (HAL::eprGetFloat(100) == 0){
        pushMenu(&ui_menu_wait, false);
        Printer::homeAxis(true, true, false);
        //PrintLine::moveRelativeDistanceInSteps(10 * Printer::axisStepsPerMM[X_AXIS], 0, 0, 0, Printer::homingFeedrate[X_AXIS], true, true, false);
        //PrintLine::moveRelativeDistanceInSteps(0, 0, 40 * Printer::axisStepsPerMM[Z_AXIS], 0, Printer::homingFeedrate[Z_AXIS], true, true, false);
        Printer::moveToReal(10, IGNORE_COORDINATE, IGNORE_COORDINATE, IGNORE_COORDINATE, Printer::homingFeedrate[X_AXIS]);
        Printer::moveToReal(IGNORE_COORDINATE, IGNORE_COORDINATE, 40, IGNORE_COORDINATE, Printer::homingFeedrate[Z_AXIS]);
        Printer::updateCurrentPosition(true);
        //Printer::currentPositionSteps[X_AXIS] = 10 * Printer::axisStepsPerMM[X_AXIS];
        //Printer::currentPositionSteps[Z_AXIS] = 40 * Printer::axisStepsPerMM[Z_AXIS];
        HAL::delayMilliseconds(3000);
			  pushMenu(&ui_menu_epos_kart, false);
			} else {
        sd.startPrint();
        menuLevel = 0;
			}
			break;

		case UI_ACTION_CLEAN_OPEN:
      //sd.stopPrint();
      Extruder::setTemperatureForExtruder(0, 0);
      printVar = 0;
			Extruder::tempTestFunction(1);
			menuLevel = 0;
      pushMenu(&ui_menu_wait, false);
      Printer::homeAxis(true, true, true);
      Printer::moveToReal(Printer::currentPosition[X_AXIS], Printer::currentPosition[Y_AXIS], 35, IGNORE_COORDINATE, Printer::homingFeedrate[Z_AXIS]);
      Printer::moveToReal(10, 10, IGNORE_COORDINATE, IGNORE_COORDINATE, Printer::homingFeedrate[X_AXIS]);
      //Printer::currentPositionSteps[E_AXIS] = 0;
      //GCode::executeFString("G91");
      //GCode::executeFString("G1 E-100");
      //GCode::executeFString("G90");
      //GCode::executeFString("G92 E0");
			Printer::moveToReal(IGNORE_COORDINATE, IGNORE_COORDINATE, IGNORE_COORDINATE, -100, EXT0_MAX_FEEDRATE); // bis endstop hoch (wert)
      //GCode::executeFString("G92 E0");
      HAL::delayMilliseconds(13000);
      HAL::eprSetFloat(100, 0);
			//pushMenu(&ui_menu_close_cart, false);
      pushMenu(&ui_menu_load_insert, false);
      Printer::newECode = 0;
      HAL::eprSetFloat(999, Printer::newECode); 
      fullTest = 100;
      zeitPercent = 0;
			break;

		case UI_ACTION_GO_ABORT:
			GCode::commandsReceivingWritePosition = 0;
      startVariable = 0;
			sd.stopPrint();
			Printer::setMenuMode(MENU_MODE_SD_PRINTING, false);
			menuLevel = 0;
			sd.sdmode = 0;
			pushMenu(&ui_menu_abort, false);
			Com::printFLN(PSTR("open abort"));

			break;

		case UI_ACTION_YES_CLEAN_NOW_1:
      //sd.stopPrint();
      Extruder::setTemperatureForExtruder(0, 0);
      printVar = 0;
      Extruder::tempTestFunction(1);
      menuLevel = 0;
      pushMenu(&ui_menu_wait, false); 
      Printer::homeAxis(true, true, true);
      Printer::moveToReal(Printer::currentPosition[X_AXIS], Printer::currentPosition[Y_AXIS], 35, IGNORE_COORDINATE, Printer::homingFeedrate[Z_AXIS]);
      Printer::moveToReal(10, 10, IGNORE_COORDINATE, IGNORE_COORDINATE, Printer::homingFeedrate[X_AXIS]);
      //Printer::currentPositionSteps[E_AXIS] = 0;
      //GCode::executeFString("G91");
      //GCode::executeFString("G1 E-100");
      //GCode::executeFString("G90");
      Printer::moveToReal(IGNORE_COORDINATE, IGNORE_COORDINATE, IGNORE_COORDINATE, -100, EXT0_MAX_FEEDRATE); // bis endstop hoch (wert)
      //GCode::executeFString("G92 E0");
      //Printer::currentPositionSteps[E_AXIS] = 0;
      HAL::delayMilliseconds(13000);
      HAL::eprSetFloat(100, 0);
      pushMenu(&ui_menu_clean_safe, false);
      Printer::newECode = 0;
      HAL::eprSetFloat(999, Printer::newECode); 
      fullTest = 100;
      zeitPercent = 0;
			break;

		case UI_ACTION_YES_CLEAN_NOW:
      menuLevel = 0;
      if (cleanEx == 1){
        pushMenu(&ui_menu_clean_ex, true);
        Printer::moveToReal(IGNORE_COORDINATE, IGNORE_COORDINATE, IGNORE_COORDINATE, -40, EXT0_MAX_FEEDRATE);
        HAL::delayMilliseconds(20000);
      } else {
			  pushMenu(&ui_menu_wait_extruder_down, true);
			  Printer::moveToReal(IGNORE_COORDINATE, IGNORE_COORDINATE, IGNORE_COORDINATE, -70, EXT0_MAX_FEEDRATE);
        HAL::delayMilliseconds(13000);
      }
			pushMenu(&ui_menu_done_quest, false); // ganz runter bis stempel locker (wert)
			break;

		case UI_ACTION_INSERT_NEW:
      //oldLevel = menuLevel;
			//menuLevel = 51;
      //HAL::delayMilliseconds(2000);
      //menuLevel = oldLevel;
      pushMenu(&ui_menu_insert_new_cart, false);
			break;

		case UI_ACTION_INSERT_NEW_DONE:
      if (cleanEx == 1){
        menuLevel = 0;
        pushMenu(&ui_menu_insert_stamp, false);
      } else {
			  menuLevel = 0;
        pushMenu(&ui_menu_wait_extruder_up, true);
        //GCode::executeFString("G91");
        //GCode::executeFString("G1 E-100");
        //GCode::executeFString("G90");
        GCode::executeFString("G92 E0");
        Printer::moveToReal(IGNORE_COORDINATE, IGNORE_COORDINATE, IGNORE_COORDINATE, -100, EXT0_MAX_FEEDRATE); // bis endstop hoch (wert)
        //GCode::executeFString("G92 E0");
        Printer::newECode = 0;
        HAL::eprSetFloat(999, Printer::newECode); 
        fullTest = 100;
        HAL::eprSetFloat(100, 0);
        HAL::delayMilliseconds(15000);
        pushMenu(&ui_menu_close_cart, false);
			  //pushMenu(&ui_menu_load_insert, false);
			  //Printer::moveToReal(IGNORE_COORDINATE, IGNORE_COORDINATE, IGNORE_COORDINATE, -40, EXT0_MAX_FEEDRATE); // bis endstop hoch (wert)
        //Printer::newECode = 0; 
        //fullTest = 100;
      }
			break;

    case UI_ACTION_CLOSE_CART:
      eposVariable = 0;
      pushMenu(&ui_menu_turn_off, false);
      break;

		case UI_ACTION_GO_TURN_OFF:
			pushMenu(&ui_menu_turn_off, false);
      //pushMenu(&ui_menu_inserted, false);
			break;

		case UI_ACTION_NEW_CARTRIDGE_YES:
      //sd.stopPrint();
      Extruder::setTemperatureForExtruder(0, 0);
      printVar = 0;
      Extruder::tempTestFunction(1);
      menuLevel = 0;
      pushMenu(&ui_menu_wait, false); 
      //uid.executeAction(UI_ACTION_GO_TURN_OFF, false);
      Printer::homeAxis(true, true, true);
      Printer::moveToReal(Printer::currentPosition[X_AXIS], Printer::currentPosition[Y_AXIS], 40, IGNORE_COORDINATE, Printer::homingFeedrate[Z_AXIS]);
			Printer::moveToReal(10, 10, IGNORE_COORDINATE, IGNORE_COORDINATE, Printer::homingFeedrate[X_AXIS]);
      //Printer::currentPositionSteps[E_AXIS] = 0;
      //GCode::executeFString("G91");
      //GCode::executeFString("G1 E-100");
      //GCode::executeFString("G90");
      //GCode::executeFString("G92 E0");
			Printer::moveToReal(IGNORE_COORDINATE, IGNORE_COORDINATE, IGNORE_COORDINATE, -100, EXT0_MAX_FEEDRATE); // bis endstop hoch
      //GCode::executeFString("G92 E0");
      //Printer::currentPositionSteps[E_AXIS] = 0;
      HAL::delayMilliseconds(13000);
      HAL::eprSetFloat(100, 0);
      pushMenu(&ui_menu_inserted, false);
      Printer::newECode = 0;
      HAL::eprSetFloat(999, Printer::newECode); 
      fullTest = 100; 
      zeitPercent = 0;
			break;

		case UI_ACTION_CHOOSE_OBJECT:
			break;

		case UI_ACTION_TURN_OFF_LAST:
			pushMenu(&ui_menu_turn_off_last, false);
			break;

		case UI_ACTION_REMOVE_CART:
      Extruder::tempTestFunction(1);
			//pushMenu(&ui_wiz_remove_cart, false);
			menuLevel = 0;
      eposVariable = 0;
      uid.executeAction(UI_ACTION_CLEAN_OPEN, false);
			//pushMenu(&ui_menu_remove_cart, true);
      //Printer::homeAxis(true, true, false);
      //Printer::moveToReal(5, IGNORE_COORDINATE, IGNORE_COORDINATE, IGNORE_COORDINATE, Printer::homingFeedrate[X_AXIS]);
			//UI_STATUS_F(Com::translatedF("test2"));
			break;

		case UI_ACTION_ANOTHER_OBJECT:
			menuLevel = 0;
			printVar = 1;
      HAL::eprSetFloat(999, Printer::newECode);
      eposVariable = 1;
			pushMenu(&ui_menu_another, false);
			break;

    case UI_ACTION_NEW_OBJECT:
      eposVariable = 0;
      fileVar = 1;
      for (int i; i < sizeof(fileNameLCD); i++){
        fileNameLCD[i] = ' ';
        fileNameLCDAsk[i] = ' ';  
      }
      pushMenu(&ui_menu_sd_fileselector, false);
      break;

		case UI_ACTION_SERVICE:
			pushMenu(&ui_menu_service, false);
			break;

    case UI_ACTION_INSERT_STAMP:
      /*pushMenu(&ui_menu_turn_off, false);
      //pushMenu(&ui_menu_insert_new_cart, false);
      GCode::executeFString("G92 E0");
      Printer::moveToReal(IGNORE_COORDINATE, IGNORE_COORDINATE, IGNORE_COORDINATE, -100, EXT0_MAX_FEEDRATE); // bis endstop hoch (wert)
      Printer::newECode = 0;
      HAL::eprSetFloat(0, Printer::newECode); 
      fullTest = 100;
      HAL::eprSetFloat(100, 0);*/
      cleanEx = 0;
      uid.executeAction(UI_ACTION_INSERT_NEW_DONE, false);
      break;
    

    case UI_ACTION_LOAD_NOW:
      pushMenu(&ui_menu_load_now, false);
      break;

    case UI_ACTION_LOAD_NOW_NO:
      menuLevel = 0;
      break;

    case UI_ACTION_LOAD_DONE:
      Com::printFLN(PSTR("im load"));
      Extruder::setTemperatureForExtruder(0, 0);
      printVar = 0;
      Extruder::tempTestFunction(1);
      menuLevel = 0;
      pushMenu(&ui_menu_wait, false); 
      //uid.executeAction(UI_ACTION_GO_TURN_OFF, false);
      Printer::homeAxis(true, true, true);
      Printer::moveToReal(Printer::currentPosition[X_AXIS], Printer::currentPosition[Y_AXIS], 40, IGNORE_COORDINATE, Printer::homingFeedrate[Z_AXIS]);
      Printer::moveToReal(10, 10, IGNORE_COORDINATE, IGNORE_COORDINATE, Printer::homingFeedrate[X_AXIS]);
      //Printer::currentPositionSteps[E_AXIS] = 0;
      //Printer::moveToReal(IGNORE_COORDINATE, IGNORE_COORDINATE, IGNORE_COORDINATE, -100, EXT0_MAX_FEEDRATE); // bis endstop hoch
      //Printer::currentPositionSteps[E_AXIS] = 0;
      HAL::delayMilliseconds(12000);
      //pushMenu(&ui_menu_load_insert, false);
      pushMenu(&ui_menu_insert_stamp, false);
      Printer::newECode = 0;
      HAL::eprSetFloat(999, Printer::newECode); 
      fullTest = 100; 
      zeitPercent = 0;
      break;

    case UI_ACTION_LOAD_INSERT:
      menuLevel = 0;
      pushMenu(&ui_menu_close_cart, false);
      break;

    case UI_ACTION_PRE_YES:
      uid.executeAction(UI_ACTION_CHOCO_CHOOSE, false);
      break;

    case UI_ACTION_PRE_NO:
      //zeitChoco = -1;
      fileVar = 1;
      pushMenu(&ui_menu_sd_fileselector, false);
      preheatVar = 0;
      zeitChoco = 0;
      zeitAnfang = 0;
      preHeatChoco = 0;
      preKartusche = 0;
      zeitPercent = 100;
      zeitVar = 60;
      zeitMinChoco = (intervall / 60) - 1;
      UI_CLEAR_STATUS;
      Printer::setMenuMode(MENU_MODE_PREHEAT, false);
      break;
      
    // new menu ende
    
		case UI_ACTION_TEMP_DEFECT_ERR:
			menuLevel = 0;
			zeitChoco = -1;
			errorVar = 1;
			UI_STATUS_F(Com::translatedF(UI_TEXT_TEMP_DEFECT_ID));
			break;

		case UI_ACTION_KARTUSCHENW_MAIN:
			extrudeUp = 1;
			menuLevel = 0;
			pushMenu(&ui_menu_kartuschenw_2, false);
			if (calibrateVar == 1) {
				zHelpPos = FILAMENTCHANGE_Z_ADD - 10;
			}
			if (zHelpPos - Printer::currentPosition[Z_AXIS] <= 0) {
				Printer::moveToReal(Printer::currentPosition[X_AXIS], Printer::currentPosition[Y_AXIS], IGNORE_COORDINATE, IGNORE_COORDINATE, Printer::homingFeedrate[Z_AXIS]);
			}
			else {
				Printer::moveToReal(Printer::currentPosition[X_AXIS], Printer::currentPosition[Y_AXIS], zHelpPos - Printer::currentPosition[Z_AXIS], IGNORE_COORDINATE, Printer::homingFeedrate[Z_AXIS]);
			}
			Printer::moveToReal(FILAMENTCHANGE_X_POS, FILAMENTCHANGE_Y_POS, IGNORE_COORDINATE, IGNORE_COORDINATE, Printer::homingFeedrate[X_AXIS]);
			Commands::waitUntilEndOfAllMoves();
			uid.executeAction(UI_ACTION_KARTUSCHENW_2, false);
			break;
		case UI_ACTION_KARTUSCHENW_1: //Marcel Kartuschenwechsel
		  //pushMenu(&ui_menu_kartuschenw_1, false);
			menuLevel = 0;
			pushMenu(&ui_menu_remove_cart, false);
			//pushMenu(&ui_menu_abort, false);
			//pushMenu(&ui_menu_remove_cart, false);

			sd.stopPrint();
			//Printer::setBlockingReceive(true);
			/*Printer::MemoryPosition();
			 * extrudeUp = 1;
			if(calibrateVar == 1){
			  zHelpPos = FILAMENTCHANGE_Z_ADD - 10;
			}
			if(zHelpPos - Printer::currentPosition[Z_AXIS] <= 0){
			  Printer::moveToReal(Printer::currentPosition[X_AXIS], Printer::currentPosition[Y_AXIS], IGNORE_COORDINATE, IGNORE_COORDINATE, Printer::homingFeedrate[Z_AXIS]);
			} else {
			  Printer::moveToReal(Printer::currentPosition[X_AXIS], Printer::currentPosition[Y_AXIS], zHelpPos - Printer::currentPosition[Z_AXIS], IGNORE_COORDINATE, Printer::homingFeedrate[Z_AXIS]);
			}
			Printer::moveToReal(FILAMENTCHANGE_X_POS, FILAMENTCHANGE_Y_POS,IGNORE_COORDINATE, IGNORE_COORDINATE, Printer::homingFeedrate[X_AXIS]);*/

			break;
		case UI_ACTION_KARTUSCHENW_2:
			kartuschenwVar = 1;
			pushMenu(&ui_menu_kartuschenw_2, false);
			if (extrudeUp == 1 && extrudeVar == 0) { // marcel preheat
				zeitAnfangEx = HAL::timeInMilliseconds();
				extrudeVar = 1;
				Printer::moveToReal(IGNORE_COORDINATE, IGNORE_COORDINATE, IGNORE_COORDINATE, -1000, EXT0_MAX_FEEDRATE);
				Printer::currentPositionSteps[E_AXIS] = 0;
				Printer::updateCurrentPosition(true);
				uid.executeAction(UI_ACTION_KARTUSCHENW_2, false);
			}
			else if (extrudeUp == 1 && extrudeVar == 1) {
				do {
					zeitEndeEx = HAL::timeInMilliseconds();
					zeitExtrude = zeitEndeEx - zeitAnfangEx;
					zeitPercentEx = zeitExtrude * 100 / intervall_2;
					u8g_FirstPage(&u8g);
					do
					{
						printU8GRow(0, 10, const_cast<char*>("Extruder is moving up")); // language ready machen UI_TEXT_EXTR_UP_ID);//
						drawHProgressBar(0, 51, u8g_GetWidth(&u8g), 13, zeitPercentEx);//extrudePercent);
						if (Endstops::yMax() || Endstops::yMin()) {
							zeitExtrude = intervall_2 + 1;
						}
					} while (u8g_NextPage(&u8g));
				} while (zeitExtrude <= intervall_2);//while (extrudePercent < 100);
			}
			if (zeitExtrude >= intervall_2) {
				Printer::currentPositionSteps[E_AXIS] = 0;
				Printer::moveToReal(IGNORE_COORDINATE, IGNORE_COORDINATE, IGNORE_COORDINATE, 20, EXT0_MAX_FEEDRATE);
				Printer::currentPositionSteps[E_AXIS] = 0;
				menuLevel = 0;
				if (cleanVar == 1) {
					uid.executeAction(UI_ACTION_CLEAN_1, false);
					cleanVar = 0;
					extrudeVar = 0;
					zeitExtrude = 0;
					zeitAnfangEx = 0;
					extrudeUp = 0;
					break;
				}
				if (removeVar == 1) {
					uid.executeAction(UI_ACTION_KARTUSCHENW, false);
				}
				else {
					pushMenu(&ui_menu_kartuschenw_3, false);
				}
				extrudeVar = 0;
				zeitExtrude = 0;
				zeitAnfangEx = 0;
				extrudeUp = 0;
			}
			break;
		case UI_ACTION_PREHEAT_CHOC: // Marcel preheat choco normal (33°C)
			//UI_STATUS_F(Com::translatedF(UI_TEXT_PREHEAT_PLA_ID));
			Extruder::setTemperatureForExtruder(UI_SET_PRESET_EXTRUDER_TEMP_PLA, 0);
			//Extruder::setTemperatureForExtruder(UI_SET_PRESET_EXTRUDER_TEMP_CHOC, 0);
			preHeatChoco = 1;
			menuLevel = 0;
			break;
		case UI_ACTION_PREHEAT_SKIP:
      //Extruder::tempTestFunction(1);
			menuLevel = 0;
			preHeatChoco = 0;
			preheatVar = 0;
			zeitChoco = 0;
			zeitAnfang = 0;
			zeitPercent = 100;
			Printer::setMenuMode(MENU_MODE_PREHEAT, false);
			UI_STATUS_F(Com::translatedF(UI_TEXT_PREHEAT_DONE_ID));
			if (textChooseVar = 1) {
				Extruder::setTemperatureForExtruder(UI_SET_PRESET_EXTRUDER_TEMP_ABS, 0);
			}
			if (chocoChooseVar = 1) {
				Extruder::setTemperatureForExtruder(UI_SET_PRESET_EXTRUDER_TEMP_PLA, 0);
			}
			Extruder::setTemperatureForExtruder(UI_SET_PRESET_EXTRUDER_TEMP_PLA, 0);
			if (printVar == 1) {
        fileVar = 1;
				pushMenu(&ui_menu_sd_fileselector, false);
			}
			else if (kartuschenwVar == 1) {
				pushMenu(&ui_menu_epos_kart, false);
			}
			else {
				menuLevel = 0;
			}
			break;
		case UI_ACTION_PREHEAT_YES:
			preKartusche = 1;
			uid.executeAction(UI_ACTION_PREHEAT_CHOC, false);
      Com::print(HAL::eprGetFloat(0));
      Com::printFLN(PSTR(" eeprom wert für epos"));
			break;
		case UI_ACTION_PREHEAT_ASK:
			fileVar = 1;// marcel action before print
			printVar = 1;
			pushMenu(&ui_menu_preheat_ask, false);
			break;

		case UI_ACTION_SKIP_NO:
			menuLevel = 0;
			break;

		case UI_ACTION_PRE_CALIBRATE:
			menuLevel = 0;
			pushMenu(&ui_menu_pre_calibrate, false);
			break;
		case UI_ACTION_CALIBRATE_COOKIE:
			cookieVar = 1;
			calibrateVar = 1;
			uid.executeAction(UI_ACTION_PRE_CALIBRATE, false);
			break;
		case UI_ACTION_CALIBRATE_CHOCOLATE:
			calibrateVar = 1;
			chocolateVar = 1;
			uid.executeAction(UI_ACTION_PRE_CALIBRATE, false);
			break;
		case UI_ACTION_CALIBRATE_STANDARD:
			calibrateVar = 0;
			standardVar = 1;
			uid.executeAction(UI_ACTION_PRE_CALIBRATE, false);
			break;
		case UI_ACTION_CALIBRATE_Z:
			pushMenu(&ui_menu_calibrate_z, false);
			Printer::moveToReal(MIDDLE_X_POS, MIDDLE_Y_POS, Printer::currentPosition[Z_AXIS], 0, Printer::homingFeedrate[X_AXIS]);
			Printer::lastCmdPos[X_AXIS] = MIDDLE_X_POS;
			Printer::lastCmdPos[Y_AXIS] = MIDDLE_Y_POS;
			if (standardVar == 1) {
				Printer::moveToReal(Printer::currentPosition[X_AXIS], Printer::currentPosition[Y_AXIS], CALIBRATE_Z_POS, 0, Printer::homingFeedrate[Z_AXIS]);
				Printer::lastCmdPos[Z_AXIS] = CALIBRATE_Z_POS;
			}
			if (cookieVar == 1) {
				Printer::moveToReal(Printer::currentPosition[X_AXIS], Printer::currentPosition[Y_AXIS], CALIBRATE_Z_POS_COOKIE, 0, Printer::homingFeedrate[Z_AXIS]);
				Printer::lastCmdPos[Z_AXIS] = CALIBRATE_Z_POS_COOKIE;
			}
			if (chocolateVar == 1) {
				Printer::moveToReal(Printer::currentPosition[X_AXIS], Printer::currentPosition[Y_AXIS], CALIBRATE_Z_POS_CHOCOLATE, 0, Printer::homingFeedrate[Z_AXIS]);
				Printer::lastCmdPos[Z_AXIS] = CALIBRATE_Z_POS_CHOCOLATE;
			}
			break;

		case UI_ACTION_NO_ALREADY:
			if (printVar == 1) {
				menuLevel = 0;
				//pushMenu(&ui_menu_preheat_ask, false);
			}
			else {
				menuLevel = 0;
			}
			break;
		case UI_ACTION_CHOOSE:
			if (sd.sdactive) {
				pushMenu(&ui_menu_produce_ask, false);
				chooseVar = 1;
				chocoChooseVar = 0;
				textChooseVar = 0;
			}
			else {
				pushMenu(&ui_menu_no_sd, false);
			}
			break;

		case UI_ACTION_CHOCO_CHOOSE:

			/*if(calibrateVar == 1){
			  zHelpPos = FILAMENTCHANGE_Z_ADD - 10;
			}
			if(zHelpPos - Printer::currentPosition[Z_AXIS] <= 0){
			  Printer::moveToReal(Printer::currentPosition[X_AXIS], Printer::currentPosition[Y_AXIS], IGNORE_COORDINATE, IGNORE_COORDINATE, Printer::homingFeedrate[Z_AXIS]);
			} else {
			  Printer::moveToReal(Printer::currentPosition[X_AXIS], Printer::currentPosition[Y_AXIS], zHelpPos - Printer::currentPosition[Z_AXIS], IGNORE_COORDINATE, Printer::homingFeedrate[Z_AXIS]);
			}
			Printer::moveToReal(FILAMENTCHANGE_X_POS, FILAMENTCHANGE_Y_POS,IGNORE_COORDINATE, IGNORE_COORDINATE, Printer::homingFeedrate[X_AXIS]);*/
			//
			//pushMenu(&ui_menu_choco, false);
			//menuLevel = 0;
			popMenu(false);
			chocoChooseVar = 1;
			printVar = 1;
			fileVar = 1;
			//pushMenu(&ui_menu_calibrate, false);
			uid.executeAction(UI_ACTION_PREHEAT_PLA, false);
      //Printer::currentPositionSteps[E_AXIS] = 0;
			break;

		case UI_ACTION_TEXT_CHOOSE:

			if (calibrateVar == 1) {
				zHelpPos = FILAMENTCHANGE_Z_ADD - 10;
			}
			if (zHelpPos - Printer::currentPosition[Z_AXIS] <= 0) {
				Printer::moveToReal(Printer::currentPosition[X_AXIS], Printer::currentPosition[Y_AXIS], IGNORE_COORDINATE, IGNORE_COORDINATE, Printer::homingFeedrate[Z_AXIS]);
			}
			else {
				Printer::moveToReal(Printer::currentPosition[X_AXIS], Printer::currentPosition[Y_AXIS], zHelpPos - Printer::currentPosition[Z_AXIS], IGNORE_COORDINATE, Printer::homingFeedrate[Z_AXIS]);
			}
			Printer::moveToReal(FILAMENTCHANGE_X_POS, FILAMENTCHANGE_Y_POS, IGNORE_COORDINATE, IGNORE_COORDINATE, Printer::homingFeedrate[X_AXIS]);
			pushMenu(&ui_menu_text, false);
			break;

		case UI_ACTION_YES_CLEAN:
			cleanVar = 1;
			extrudeUp = 1;
			pushMenu(&ui_menu_kartuschenw_2, false);
			if (calibrateVar == 1) {
				zHelpPos = FILAMENTCHANGE_Z_ADD - 10;
			}
			if (zHelpPos - Printer::currentPosition[Z_AXIS] <= 0) {
				Printer::moveToReal(Printer::currentPosition[X_AXIS], Printer::currentPosition[Y_AXIS], IGNORE_COORDINATE, IGNORE_COORDINATE, Printer::homingFeedrate[Z_AXIS]);
			}
			else {
				Printer::moveToReal(Printer::currentPosition[X_AXIS], Printer::currentPosition[Y_AXIS], zHelpPos - Printer::currentPosition[Z_AXIS], IGNORE_COORDINATE, Printer::homingFeedrate[Z_AXIS]);
			}
			Printer::moveToReal(MIDDLE_X_POS, MIDDLE_Y_POS, IGNORE_COORDINATE, IGNORE_COORDINATE, Printer::homingFeedrate[X_AXIS]);
			Printer::updateCurrentPosition(true);
			Commands::waitUntilEndOfAllMoves();
			Extruder::tempTestFunction(1); //Marcel Temp
			uid.executeAction(UI_ACTION_KARTUSCHENW_2, false);
			break;

		case UI_ACTION_CLEAN_1:
			pushMenu(&ui_menu_clean_1, false);
			break;

		case UI_ACTION_CLEAN_2:
			pushMenu(&ui_menu_clean_2, false);
			if (extrudeUp == 1 && extrudeVar == 0) { // marcel preheat
				zeitAnfangEx = HAL::timeInMilliseconds();
				extrudeVar = 1;
				Printer::moveToReal(IGNORE_COORDINATE, IGNORE_COORDINATE, IGNORE_COORDINATE, cleanEPos, EXT0_MAX_FEEDRATE);
				uid.executeAction(UI_ACTION_CLEAN_2, false);
			}
			else if (extrudeUp == 1 && extrudeVar == 1) {
				do {
					zeitEndeEx = HAL::timeInMilliseconds();
					zeitExtrude = zeitEndeEx - zeitAnfangEx;
					zeitPercentEx = zeitExtrude * 100 / intervall_3;
					u8g_FirstPage(&u8g);
					do
					{
						printU8GRow(0, 10, const_cast<char*>("Extruder moving down")); // language ready machen UI_TEXT_EXTR_DOWN_ID);//
						drawHProgressBar(0, 51, u8g_GetWidth(&u8g), 13, zeitPercentEx);
					} while (u8g_NextPage(&u8g));
				} while (zeitExtrude <= intervall_3);
			}
			if (zeitExtrude >= intervall_3) {
				pushMenu(&ui_menu_clean_3, false);
				extrudeVar = 0;
				zeitExtrude = 0;
				zeitAnfangEx = 0;
				extrudeUp = 0;
			}
			break;
		case UI_ACTION_CLEAN_3:
			pushMenu(&ui_menu_clean_2, false);
			break;
		case UI_ACTION_CLEAN_4:
			pushMenu(&ui_menu_clean_4, false);
			if (extrudeUp == 1 && extrudeVar == 0) { // marcel preheat
				zeitAnfangEx = HAL::timeInMilliseconds();
				extrudeVar = 1;
				Printer::moveToReal(IGNORE_COORDINATE, IGNORE_COORDINATE, IGNORE_COORDINATE, -cleanEPos, EXT0_MAX_FEEDRATE);
				uid.executeAction(UI_ACTION_CLEAN_4, false);
			}
			else if (extrudeUp == 1 && extrudeVar == 1) {
				do {
					zeitEndeEx = HAL::timeInMilliseconds();
					zeitExtrude = zeitEndeEx - zeitAnfangEx;
					zeitPercentEx = zeitExtrude * 100 / intervall_3;
					u8g_FirstPage(&u8g);
					do
					{
						printU8GRow(0, 10, const_cast<char*>("Extruder moving up")); // language ready machen UI_TEXT_EXTR_UP_ID);//
						drawHProgressBar(0, 51, u8g_GetWidth(&u8g), 13, zeitPercentEx);
						if (Endstops::yMax() || Endstops::yMin()) {
							zeitExtrude = intervall_3 + 1;
						}
					} while (u8g_NextPage(&u8g));
				} while (zeitExtrude <= intervall_3);
			}
			if (zeitExtrude >= intervall_3) {
				menuLevel = 0;
				Printer::currentPositionSteps[E_AXIS] = 0;
				Printer::moveToReal(IGNORE_COORDINATE, IGNORE_COORDINATE, IGNORE_COORDINATE, 20, EXT0_MAX_FEEDRATE);
				Printer::currentPositionSteps[E_AXIS] = 0;
				extrudeVar = 0;
				zeitExtrude = 0;
				zeitAnfangEx = 0;
				extrudeUp = 0;
				fullPercent = 100;
				fullTest = 100;
				UI_STATUS_F(Com::translatedF(UI_TEXT_PRINTER_READY_ID));
			}
			break;

		case UI_ACTION_REMOVE_KART:
			//extrudeUp = 1;
			removeVar = 1;
			pushMenu(&ui_menu_kartuschenw_2, false);
			if (calibrateVar == 1) {
				zHelpPos = FILAMENTCHANGE_Z_ADD - 10;
			}
			if (zHelpPos - Printer::currentPosition[Z_AXIS] <= 0) {
				Printer::moveToReal(Printer::currentPosition[X_AXIS], Printer::currentPosition[Y_AXIS], IGNORE_COORDINATE, 0, Printer::homingFeedrate[Z_AXIS]);
			}
			else {
				Printer::moveToReal(Printer::currentPosition[X_AXIS], Printer::currentPosition[Y_AXIS], zHelpPos - Printer::currentPosition[Z_AXIS], 0, Printer::homingFeedrate[Z_AXIS]);
			}
			Printer::moveToReal(FILAMENTCHANGE_X_POS, FILAMENTCHANGE_Y_POS, IGNORE_COORDINATE, 0, Printer::homingFeedrate[X_AXIS]);
			Commands::waitUntilEndOfAllMoves();
			uid.executeAction(UI_ACTION_KARTUSCHENW_2, false);
			break;

		case UI_ACTION_NO_SD:
			pushMenu(&ui_menu_no_sd, false);
			break;

		case UI_ACTION_EPOS_KART:
			menuLevel = 0;
			pushMenu(&ui_menu_epos_kart, false);
			/*if (calibrateVar == 1) {
				zHelpPos = FILAMENTCHANGE_Z_ADD - 10;
			}
			if (zHelpPos - Printer::currentPosition[Z_AXIS] <= 0) {
				Printer::moveToReal(Printer::currentPosition[X_AXIS], Printer::currentPosition[Y_AXIS], IGNORE_COORDINATE, IGNORE_COORDINATE, Printer::homingFeedrate[Z_AXIS]);
			}
			else {
				Printer::moveToReal(Printer::currentPosition[X_AXIS], Printer::currentPosition[Y_AXIS], zHelpPos - Printer::currentPosition[Z_AXIS], IGNORE_COORDINATE, Printer::homingFeedrate[Z_AXIS]);
			}
			Printer::moveToReal(MIDDLE_X_POS, MIDDLE_Y_POS, IGNORE_COORDINATE, IGNORE_COORDINATE, Printer::homingFeedrate[X_AXIS]);
			Printer::updateCurrentPosition(true);
			Commands::waitUntilEndOfAllMoves();*/
			break;
		case UI_ACTION_KARTUSCHENR:
			Extruder::tempTestFunction(1);
			pushMenu(&ui_wiz_kartuschenr, true);
			break;

		case UI_ACTION_KARTUSCHENW: //Marcel Kartuschenwechsel
			Extruder::tempTestFunction(1); //Marcel Temp
			pushMenu(&ui_wiz_kartuschenw, true);
			break;
			/*Printer::resetWizardStack();
			  Printer::pushWizardVar(Printer::currentPositionSteps[E_AXIS]);
			  Printer::MemoryPosition();
			  Printer::currentPositionSteps[E_AXIS] = 0;
			  if(Printer::isBlockingReceive()) break;
			  Printer::setJamcontrolDisabled(true);
			  Com::printFLN(PSTR("important: Filament change required!"));
			  Printer::setBlockingReceive(true);
			  BEEP_LONG;
			  Printer::resetWizardStack();
			  Printer::pushWizardVar(Printer::currentPositionSteps[E_AXIS]);
			  Printer::MemoryPosition();
			  Extruder::current->retractDistance(FILAMENTCHANGE_SHORTRETRACT);
			  float newZ = FILAMENTCHANGE_Z_ADD + Printer::currentPosition[Z_AXIS];
			  Printer::currentPositionSteps[E_AXIS] = 0;
			  Printer::moveToReal(Printer::currentPosition[X_AXIS], Printer::currentPosition[Y_AXIS], newZ, 0, Printer::homingFeedrate[Z_AXIS]);
			  Printer::moveToReal(FILAMENTCHANGE_X_POS, FILAMENTCHANGE_Y_POS, newZ, 0, Printer::homingFeedrate[X_AXIS]);
			  Extruder::current->retractDistance(FILAMENTCHANGE_LONGRETRACT);
			  Extruder::current->disableCurrentExtruderMotor();
			  break;*/

			  /*case UI_ACTION_NEW_CARTRIDGE_YES:
				 Com::printFLN(PSTR("NEW cart yes"));
				 pushMenu(&ui_menu_inserted, false);
				 //uid.executeAction(UI_ACTION_CARTRIDGE_INSERTED, false);
				 break;*/

		case UI_ACTION_CARTRIDGE_INSERTED:
			//preHeatChoco = 1;
			Com::printFLN(PSTR("klick done inst"));
			break;

#if FEATURE_RETRACTION
		case UI_ACTION_WIZARD_FILAMENTCHANGE:
		{
			if (Printer::isBlockingReceive()) break;
			Printer::setJamcontrolDisabled(true);
			Com::printFLN(PSTR("important: Filament change required!"));
			Printer::setBlockingReceive(true);
			BEEP_LONG;
			pushMenu(&ui_wiz_filamentchange, true);
			Printer::resetWizardStack();
			Printer::pushWizardVar(Printer::currentPositionSteps[E_AXIS]);
			Printer::MemoryPosition();
			Extruder::current->retractDistance(FILAMENTCHANGE_SHORTRETRACT);
			float newZ = FILAMENTCHANGE_Z_ADD + Printer::currentPosition[Z_AXIS];
			Printer::currentPositionSteps[E_AXIS] = 0;
			Printer::moveToReal(Printer::currentPosition[X_AXIS], Printer::currentPosition[Y_AXIS], newZ, 0, Printer::homingFeedrate[Z_AXIS]);
			Printer::moveToReal(FILAMENTCHANGE_X_POS, FILAMENTCHANGE_Y_POS, newZ, 0, Printer::homingFeedrate[X_AXIS]);
			Extruder::current->retractDistance(FILAMENTCHANGE_LONGRETRACT);
			Extruder::current->disableCurrentExtruderMotor();
		}
		break;
#if EXTRUDER_JAM_CONTROL
		case UI_ACTION_WIZARD_JAM_EOF:
		{
			Extruder::markAllUnjammed();
			Printer::setJamcontrolDisabled(true);
			Printer::setBlockingReceive(true);
			pushMenu(&ui_wiz_jamreheat, true);
			Printer::resetWizardStack();
			Printer::pushWizardVar(Printer::currentPositionSteps[E_AXIS]);
			Printer::MemoryPosition();
			Extruder::current->retractDistance(FILAMENTCHANGE_SHORTRETRACT);
			float newZ = FILAMENTCHANGE_Z_ADD + Printer::currentPosition[Z_AXIS];
			Printer::currentPositionSteps[E_AXIS] = 0;
			Printer::moveToReal(Printer::currentPosition[X_AXIS], Printer::currentPosition[Y_AXIS], newZ, 0, Printer::homingFeedrate[Z_AXIS]);
			Printer::moveToReal(FILAMENTCHANGE_X_POS, FILAMENTCHANGE_Y_POS, newZ, 0, Printer::homingFeedrate[X_AXIS]);
			//Extruder::current->retractDistance(FILAMENTCHANGE_LONGRETRACT);
			Extruder::pauseExtruders();
			Commands::waitUntilEndOfAllMoves();
#if FILAMENTCHANGE_REHOME
			Printer::disableXStepper();
			Printer::disableYStepper();
#if Z_HOME_DIR > 0 && FILAMENTCHANGE_REHOME == 2
			Printer::disableZStepper();
#endif
#endif
		}
		break;
#endif // EXTRUDER_JAM_CONTROL
#endif // FEATURE_RETRACTION

		case UI_ACTION_X_UP:
		case UI_ACTION_X_DOWN:
			if (!allowMoves) return action;
			PrintLine::moveRelativeDistanceInStepsReal(((action == UI_ACTION_X_UP) ? 1.0 : -1.0) * Printer::axisStepsPerMM[X_AXIS], 0, 0, 0, Printer::homingFeedrate[X_AXIS], false, false);
			break;
		case UI_ACTION_Y_UP:
		case UI_ACTION_Y_DOWN:
			if (!allowMoves) return action;
			PrintLine::moveRelativeDistanceInStepsReal(0, ((action == UI_ACTION_Y_UP) ? 1.0 : -1.0) * Printer::axisStepsPerMM[Y_AXIS], 0, 0, Printer::homingFeedrate[Y_AXIS], false, false);
			break;
		case UI_ACTION_Z_UP:
		case UI_ACTION_Z_DOWN:
			if (!allowMoves) return action;
			PrintLine::moveRelativeDistanceInStepsReal(0, 0, ((action == UI_ACTION_Z_UP) ? 1.0 : -1.0) * Printer::axisStepsPerMM[Z_AXIS], 0, Printer::homingFeedrate[Z_AXIS], false, false);
			break;
		case UI_ACTION_EXTRUDER_UP:
		case UI_ACTION_EXTRUDER_DOWN:
			if (!allowMoves) return action;
			PrintLine::moveRelativeDistanceInStepsReal(0, 0, 0, ((action == UI_ACTION_EXTRUDER_UP) ? 1.0 : -1.0) * Printer::axisStepsPerMM[E_AXIS], UI_SET_EXTRUDER_FEEDRATE, false, false);
			break;
		case UI_ACTION_EXTRUDER_TEMP_UP:
		{
			int tmp = (int)(Extruder::current->tempControl.targetTemperatureC) + 1;
			if (tmp == 1) tmp = UI_SET_MIN_EXTRUDER_TEMP;
			else if (tmp > UI_SET_MAX_EXTRUDER_TEMP) tmp = UI_SET_MAX_EXTRUDER_TEMP;
			Extruder::setTemperatureForExtruder(tmp, Extruder::current->id);
		}
		break;
		case UI_ACTION_EXTRUDER_TEMP_DOWN:
		{
			int tmp = (int)(Extruder::current->tempControl.targetTemperatureC) - 1;
			if (tmp < UI_SET_MIN_EXTRUDER_TEMP) tmp = 0;
			Extruder::setTemperatureForExtruder(tmp, Extruder::current->id);
		}
		break;
		case UI_ACTION_HEATED_BED_UP:
#if HAVE_HEATED_BED
		{
			int tmp = (int)heatedBedController.targetTemperatureC + 1;
			if (tmp == 1) tmp = UI_SET_MIN_HEATED_BED_TEMP;
			else if (tmp > UI_SET_MAX_HEATED_BED_TEMP) tmp = UI_SET_MAX_HEATED_BED_TEMP;
			Extruder::setHeatedBedTemperature(tmp);
		}
#endif
		break;
#if MAX_HARDWARE_ENDSTOP_Z
		case UI_ACTION_SET_MEASURED_ORIGIN:
		{
			Printer::updateCurrentPosition();
			Printer::zLength -= Printer::currentPosition[Z_AXIS];
			Printer::currentPositionSteps[Z_AXIS] = 0;
			Printer::updateDerivedParameter();
#if NONLINEAR_SYSTEM
			transformCartesianStepsToDeltaSteps(Printer::currentPositionSteps, Printer::currentNonlinearPositionSteps);
#endif
			Printer::updateCurrentPosition(true);
			Com::printFLN(Com::tZProbePrinterHeight, Printer::zLength);
#if EEPROM_MODE != 0
			EEPROM::storeDataIntoEEPROM(false);
			Com::printFLN(Com::tEEPROMUpdated);
#endif
			Commands::printCurrentPosition(PSTR("UI_ACTION_SET_MEASURED_ORIGIN "));
		}
		break;
#endif
		case UI_ACTION_SET_P1:
#if SOFTWARE_LEVELING
			for (uint8_t i = 0; i < 3; i++)
			{
				Printer::levelingP1[i] = Printer::currentPositionSteps[i];
			}
#endif
			break;
		case UI_ACTION_SET_P2:
#if SOFTWARE_LEVELING
			for (uint8_t i = 0; i < 3; i++)
			{
				Printer::levelingP2[i] = Printer::currentPositionSteps[i];
			}
#endif
			break;
		case UI_ACTION_SET_P3:
#if SOFTWARE_LEVELING
			for (uint8_t i = 0; i < 3; i++)
			{
				Printer::levelingP3[i] = Printer::currentPositionSteps[i];
			}
#endif
			break;
		case UI_ACTION_CALC_LEVEL:
#if SOFTWARE_LEVELING
			int32_t factors[4];
			PrintLine::calculatePlane(factors, Printer::levelingP1, Printer::levelingP2, Printer::levelingP3);
			Com::printFLN(Com::tLevelingCalc);
			Com::printFLN(Com::tTower1, PrintLine::calcZOffset(factors, Printer::deltaAPosXSteps, Printer::deltaAPosYSteps) * Printer::invAxisStepsPerMM[Z_AXIS]);
			Com::printFLN(Com::tTower2, PrintLine::calcZOffset(factors, Printer::deltaBPosXSteps, Printer::deltaBPosYSteps) * Printer::invAxisStepsPerMM[Z_AXIS]);
			Com::printFLN(Com::tTower3, PrintLine::calcZOffset(factors, Printer::deltaCPosXSteps, Printer::deltaCPosYSteps) * Printer::invAxisStepsPerMM[Z_AXIS]);
#endif
			break;
		case UI_ACTION_HEATED_BED_DOWN:
#if HAVE_HEATED_BED
		{
			int tmp = (int)heatedBedController.targetTemperatureC - 1;
			if (tmp < UI_SET_MIN_HEATED_BED_TEMP) tmp = 0;
			Extruder::setHeatedBedTemperature(tmp);
		}
#endif
		break;
		case UI_ACTION_FAN_UP:
			Commands::setFanSpeed(Printer::getFanSpeed() + 32, true);
			break;
		case UI_ACTION_FAN_DOWN:
			Commands::setFanSpeed(Printer::getFanSpeed() - 32, true);
			break;
		case UI_ACTION_KILL:
			Commands::emergencyStop();
			break;
		case UI_ACTION_RESET:
			HAL::resetHardware();
			break;
		case UI_ACTION_PAUSE:
			Com::printFLN(PSTR("RequestPause:"));
			break;
#if UI_BED_COATING
		case UI_ACTION_NOCOATING:
			menuAdjustHeight(&ui_menu_nocoating_action, 0);
			break;
		case UI_ACTION_BUILDTAK:
			menuAdjustHeight(&ui_menu_buildtak_action, 0.4);
			break;
		case UI_ACTION_KAPTON:
			menuAdjustHeight(&ui_menu_kapton_action, 0.04);
			break;
		case UI_ACTION_GLUESTICK:
			menuAdjustHeight(&ui_menu_gluestick_action, 0.04);
			break;
		case UI_ACTION_BLUETAPE:
			menuAdjustHeight(&ui_menu_bluetape_action, 0.15);
			break;
		case UI_ACTION_PETTAPE:
			menuAdjustHeight(&ui_menu_pettape_action, 0.09);
			break;
#endif
#if FEATURE_AUTOLEVEL
		case UI_ACTION_AUTOLEVEL_ONOFF:
			Printer::setAutolevelActive(!Printer::isAutolevelActive());
			break;
#endif
#ifdef DEBUG_PRINT
		case UI_ACTION_WRITE_DEBUG:
			Com::printF(PSTR("Buf. Read Idx:"), (int)GCode::bufferReadIndex);
			Com::printF(PSTR(" Buf. Write Idx:"), (int)GCode::bufferWriteIndex);
			Com::printF(PSTR(" Comment:"), (int)GCode::commentDetected);
			Com::printF(PSTR(" Buf. Len:"), (int)GCode::bufferLength);
			Com::printF(PSTR(" Wait resend:"), (int)GCode::waitingForResend);
			Com::printFLN(PSTR(" Recv. Write Pos:"), (int)GCode::commandsReceivingWritePosition);
			Com::printF(PSTR("Min. XY Speed:"), Printer::minimumSpeed);
			Com::printF(PSTR(" Min. Z Speed:"), Printer::minimumZSpeed);
			Com::printF(PSTR(" Buffer:"), PrintLine::linesCount);
			Com::printF(PSTR(" Lines pos:"), (int)PrintLine::linesPos);
			Com::printFLN(PSTR(" Write Pos:"), (int)PrintLine::linesWritePos);
			Com::printFLN(PSTR("Wait loop:"), debugWaitLoop);
			Com::printF(PSTR("sd mode:"), (int)sd.sdmode);
			Com::printF(PSTR(" pos:"), sd.sdpos);
			Com::printFLN(PSTR(" of "), sd.filesize);
			break;
#endif
		case UI_ACTION_TEMP_DEFECT:
			Printer::setAnyTempsensorDefect();
			break;

    case UI_ACTION_SET_FURTHER:
      break;
		
		case UI_ACTION_LANGUAGE_EN:
		case UI_ACTION_LANGUAGE_DE:
		case UI_ACTION_LANGUAGE_NL:
		case UI_ACTION_LANGUAGE_PT:
		case UI_ACTION_LANGUAGE_IT:
		case UI_ACTION_LANGUAGE_ES:
		case UI_ACTION_LANGUAGE_SE:
		case UI_ACTION_LANGUAGE_FR:
		case UI_ACTION_LANGUAGE_CZ:
		case UI_ACTION_LANGUAGE_PL:
		case UI_ACTION_LANGUAGE_TR:
		case UI_ACTION_LANGUAGE_FI:
      HAL::eprSetFloat(1021, 1);
			Com::selectLanguage(action - UI_ACTION_LANGUAGE_EN);
			HAL::eprSetFloat(1020, action - UI_ACTION_LANGUAGE_EN);
      UI_STATUS_F(Com::translatedF(UI_TEXT_CLICK_START_ID));
      Com::printFLN(PSTR(" in action sp "));
#if EEPROM_MODE != 0
      
			EEPROM::storeDataIntoEEPROM(0); // remember for next start
#endif
			break;
		}
	refreshPage();
#if UI_AUTORETURN_TO_MENU_AFTER!=0
	ui_autoreturn_time = HAL::timeInMilliseconds() + UI_AUTORETURN_TO_MENU_AFTER;
#endif
#endif
	return ret;
}
void UIDisplay::mediumAction()
{
#if UI_HAS_I2C_ENCODER>0
	uiCheckSlowEncoder();
#endif
}

// Gets calles from main tread
void UIDisplay::slowAction(bool allowMoves)
{
	millis_t time = HAL::timeInMilliseconds();
	uint8_t refresh = 0;
#if UI_HAS_KEYS == 1
	// delayed action open?
	if (allowMoves && delayedAction != 0)
	{
		executeAction(delayedAction, true);
		delayedAction = 0;
	}

	// Update key buffer
	InterruptProtectedBlock noInts;
	if ((flags & (UI_FLAG_FAST_KEY_ACTION + UI_FLAG_KEY_TEST_RUNNING)) == 0)
	{
		flags |= UI_FLAG_KEY_TEST_RUNNING;
		noInts.unprotect();
#if defined(UI_I2C_HOTEND_LED) || defined(UI_I2C_HEATBED_LED) || defined(UI_I2C_FAN_LED)
		{
			// check temps and set appropriate leds
			int led = 0;
#if NUM_EXTRUDER>0 && defined(UI_I2C_HOTEND_LED)
			led |= (tempController[Extruder::current->id]->targetTemperatureC > 0 ? UI_I2C_HOTEND_LED : 0);
#endif
#if HAVE_HEATED_BED && defined(UI_I2C_HEATBED_LED)
			led |= (heatedBedController.targetTemperatureC > 0 ? UI_I2C_HEATBED_LED : 0);
#endif
#if FAN_PIN>=0 && defined(UI_I2C_FAN_LED)
			led |= (Printer::getFanSpeed() > 0 ? UI_I2C_FAN_LED : 0);
#endif
			// update the leds
			uid.outputMask = ~led & (UI_I2C_HEATBED_LED | UI_I2C_HOTEND_LED | UI_I2C_FAN_LED);
		}
#endif
		uint16_t nextAction = 0;
		uiCheckSlowKeys(nextAction);
#ifdef HAS_USER_KEYS
		ui_check_Ukeys(nextAction);
#endif
		if (lastButtonAction != nextAction)
		{
			lastButtonStart = time;
			lastButtonAction = nextAction;
			noInts.protect();
			flags |= UI_FLAG_SLOW_KEY_ACTION; // Mark slow action
		}
		noInts.protect();
		flags &= ~UI_FLAG_KEY_TEST_RUNNING;
	}
	noInts.protect();
	if ((flags & UI_FLAG_SLOW_ACTION_RUNNING) == 0)
	{
		flags |= UI_FLAG_SLOW_ACTION_RUNNING;
		// Reset click encoder
		noInts.protect();
		int16_t encodeChange = encoderPos;
		encoderPos = 0;
		noInts.unprotect();
		int newAction;
		if (encodeChange) // encoder changed
		{
			nextPreviousAction(encodeChange, allowMoves);
			BEEP_SHORT
				refresh = 1;
		}
		if (lastAction != lastButtonAction)
		{
			if (lastButtonAction == 0)
			{
				if (lastAction >= 2000 && lastAction < 3000)
					statusMsg[0] = 0;
				lastAction = 0;
				noInts.protect();
				flags &= ~(UI_FLAG_FAST_KEY_ACTION + UI_FLAG_SLOW_KEY_ACTION);
			}
			else if (time - lastButtonStart > UI_KEY_BOUNCETIME)    // New key pressed
			{
				lastAction = lastButtonAction;
				BEEP_SHORT
					if ((newAction = executeAction(lastAction, allowMoves)) == 0)
					{
						nextRepeat = time + UI_KEY_FIRST_REPEAT;
						repeatDuration = UI_KEY_FIRST_REPEAT;
					}
					else
					{
						if (delayedAction == 0)
							delayedAction = newAction;
					}
			}
		}
		else if (lastAction < 1000 && lastAction)    // Repeatable key
		{
			if (time - nextRepeat < 10000)
			{
				if (delayedAction == 0)
					delayedAction = executeAction(lastAction, allowMoves);
				else
					executeAction(lastAction, allowMoves);
				repeatDuration -= UI_KEY_REDUCE_REPEAT;
				if (repeatDuration < UI_KEY_MIN_REPEAT) repeatDuration = UI_KEY_MIN_REPEAT;
				nextRepeat = time + repeatDuration;
				BEEP_SHORT
			}
		}
		noInts.protect();
		flags &= ~UI_FLAG_SLOW_ACTION_RUNNING;
	}
	noInts.unprotect();
#endif
#if UI_AUTORETURN_TO_MENU_AFTER != 0
	if (menuLevel > 0 && ui_autoreturn_time < time && !uid.isSticky()) // Go to top menu after x seoonds
	{
		lastSwitch = time;
		menuLevel = 0;
		activeAction = 0;
	}
#endif
	if (uid.isWizardActive())
		previousMillisCmd = HAL::timeInMilliseconds(); // prevent stepper/heater disable from timeout during active wizard
	if (menuLevel == 0 && time > 4000) // Top menu refresh/switch
	{
		if (time - lastSwitch > UI_PAGES_DURATION)
		{
			lastSwitch = time;
#if !defined(UI_DISABLE_AUTO_PAGESWITCH) || !UI_DISABLE_AUTO_PAGESWITCH
			menuPos[0]++;
			if (menuPos[0] >= UI_NUM_PAGES)
				menuPos[0] = 0;
#endif
			refresh = 1;
		}
		else if (time - lastRefresh >= 1000) refresh = 1;
		}
	else if (time - lastRefresh >= 800)
	{
		//UIMenu *men = (UIMenu*)menu[menuLevel];
		//uint8_t mtype = pgm_read_byte((void*)&(men->menuType));
		//if(mtype!=1)
		refresh = 1;
	}
	if (refresh) // does lcd need a refresh?
	{
#if defined(TRY_AUTOREPAIR_LCD_ERRORS)
#if defined(HAS_AUTOREPAIR)
		repairLCD();
#else
#error TRY_AUTOREPAIR_LCD_ERRORS is not supported for your display type!
#endif
#endif

		if (menuLevel > 1 || Printer::isAutomount())
		{
			shift++;
			if (shift + UI_COLS > MAX_COLS + 1)
				shift = -2;
	}
		else
			shift = -2;

		refreshPage();
		lastRefresh = time;
	}
	}

// Gets called from inside an interrupt with interrupts allowed!
void UIDisplay::fastAction()
{
#if UI_HAS_KEYS == 1
	// Check keys
//	InterruptProtectedBlock noInts;
	if ((flags & (UI_FLAG_KEY_TEST_RUNNING + UI_FLAG_SLOW_KEY_ACTION)) == 0)
	{
		flags |= UI_FLAG_KEY_TEST_RUNNING;
		uint16_t nextAction = 0;
		uiCheckKeys(nextAction);
		//        ui_check_Ukeys(nextAction);
		if (lastButtonAction != nextAction)
		{
			lastButtonStart = HAL::timeInMilliseconds();
			lastButtonAction = nextAction;
			flags |= UI_FLAG_FAST_KEY_ACTION;
		}
		flags &= ~UI_FLAG_KEY_TEST_RUNNING;
	}
#endif
}

#if defined(UI_REVERSE_ENCODER) && UI_REVERSE_ENCODER == 1
#if UI_ENCODER_SPEED==0
const int8_t encoder_table[16] PROGMEM = { 0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0 }; // Full speed
#elif UI_ENCODER_SPEED==1
const int8_t encoder_table[16] PROGMEM = { 0, 0, 1, 0, 0, 0, 0, -1, -1, 0, 0, 0, 0, 1, 0, 0 }; // Half speed
#else
const int8_t encoder_table[16] PROGMEM = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, -1, 0 }; // Quart speed
#endif
#else
#if UI_ENCODER_SPEED==0
const int8_t encoder_table[16] PROGMEM = { 0, 1, -1, 0, -1, 0, 0, 1, 1, 0, 0, -1, 0, -1, 1, 0 }; // Full speed
#elif UI_ENCODER_SPEED==1
const int8_t encoder_table[16] PROGMEM = { 0, 0, -1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, -1, 0, 0 }; // Half speed
#else
//const int8_t encoder_table[16] PROGMEM = {0,0,0,0,0,0,0,0,1,0,0,0,0,-1,0,0}; // Quart speed
//const int8_t encoder_table[16] PROGMEM = {0,1,0,0,-1,0,0,0,0,0,0,0,0,0,0,0}; // Quart speed
const int8_t encoder_table[16] PROGMEM = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, 1, 0 }; // Quart speed
#endif
#endif
#endif

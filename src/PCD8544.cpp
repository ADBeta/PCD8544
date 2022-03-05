/*
	PCD8544 LCD a.k.a Nokia 5110 LCD driver library. 
	Includes low-power CPU options and memory/cycle saving options 
	
	Last modified 13 Feb 2022
	Written by ADBeta
	
	Add dumb functions to just write data to the screen
	
	Add font array that can be disabled to save space
	

*/

#include "Arduino.h"
#include "PCD8544.h"

/*****************    Hardware Control    *****************/
void PCD8544::writeBit(byte bit) {
	//Safety check on the given bit
	bit &= 0x01;
	
	//Write bit to the bus
	digitalWrite(p_DAT, bit);
	
	//Cycle clock pin to latch bit
	digitalWrite(p_CLK, HIGH);
	digitalWrite(p_CLK, LOW);
}

void PCD8544::writeByte(byte data) {
	digitalWrite(p_CE, LOW); //Start transmission
	
	//Send bits MSB First
	for(int8_t cbit = 7; cbit >= 0; cbit--) {
		uint8_t dbit = (data >> cbit) & 0x01;
		
		writeBit(dbit);
	}
	digitalWrite(p_CE, HIGH); //End transmission	
}

/*****************    Screen Control    *****************/
void PCD8544::sendCMD(byte data) {
	digitalWrite(p_DC, LOW);
	writeByte(data);
}

void PCD8544::sendDAT(byte data) {
	digitalWrite(p_DC, HIGH);
	writeByte(data);
	
	//Incriment position value only if MEMARRAY is not defined (raw write mode)
	#ifdef DIRECT_ADDR
		incPos();
	#endif
}

void PCD8544::setPos(byte x, byte y) {
	//Limit values to correct range
	if(x > 83) x = 83;
	if(y > 5) y = 5;
	
	sendCMD(0x80 + x);
	sendCMD(0x40 + y);
	
	screenX = x;
	screenY = y;
}

void PCD8544::incPos() {
	++screenX;
	
	//Skip to next line if x is past 83	
	if(screenX == 84) {
		screenX = 0;
		
		++screenY;
		//If Y pos is greater than 6, reset its position
		if(screenY == 6) {
			screenY = 0;
		}
	}
}

void PCD8544::clear() {
	setPos(0, 0);
	
	//go through each column of the LCD and blank it   84 * 6 = 504
	for(uint16_t col = 0; col < 504; col++) {
		sendDAT(0x00);
	}
	
	//Reset position after clear -- done automatically --
	//setPos(0, 0);
}

//TODO Add compile flag to this
void PCD8544::printChar(char input) {
	//Make sure input char is within usable range
	if(input < 0x20 || input > 0x7E) return;
	
	//align input char with the index of the array
	input = input - 0x20;
	
	//Output all 5 bytes of the font char at position [input]
	for(byte charByte = 0; charByte < 5; charByte++) {
		sendDAT(pgm_read_byte(&(font[input][charByte])));
	}
}

void PCD8544::printString(const char *input) {
	byte crrChar = 0;
	while(input[crrChar] != '\0') {
		printChar(input[crrChar]);
		sendDAT(0x00); //Add a space
		
		++crrChar;
	}
}

void PCD8544::printIcon(const byte *icon, const byte length, const byte height) {
	//Keep track of the XPosition we started at to print icon corrctly
	byte startXPos = screenX;
	
	for(byte col = 0; col < height; col++) {
		
		//Get offset position of array, length + current collumn position
		unsigned int arrayOffset = length * col;
		
		for(byte row = 0; row < length; row++) {
			
			sendDAT(pgm_read_byte(&(icon[row + arrayOffset])));
		}
		
		//Set the position to the original X position, down 1 Y position
		setPos(startXPos, ++screenY);	
	}
}

/*****************    Configuration    *****************/
void PCD8544::init(byte RST, byte CLK, byte DAT, byte DC, byte CE) {
	//Set global pins
	p_RST = RST;
	p_CLK = CLK;
	p_DAT = DAT;
	p_DC = DC;
	p_CE = CE;
	
	//Pin states
	pinMode(p_RST, OUTPUT);
	pinMode(p_CLK, OUTPUT);
	pinMode(p_DAT, OUTPUT);
	pinMode(p_DC, OUTPUT);
	pinMode(p_CE, OUTPUT);
	
	//Set pin states
	digitalWrite(p_CE, HIGH);
	digitalWrite(p_CLK, LOW);
	digitalWrite(p_DAT, LOW);
	//Reset
	digitalWrite(p_RST, LOW);
	delay(1);
	digitalWrite(p_RST, HIGH);
	
	//LCD Init commands
	sendCMD(0x21); //Open Extended Commands
	sendCMD(contrast); //Set LCD VOP (Contrast)
	sendCMD(tempCoeff); //Set temp coefficient
	sendCMD(bias); //Set LCD Bias
	sendCMD(0x20); //Close Extended Commands, Basic Commands
	sendCMD(0x0C); //Normal mode
	
}

void PCD8544::invert(bool inv) {
	if(inv == 1) {
		sendCMD(0x0D);
	} 
	
	if(inv == 0) {
		sendCMD(0x0C);
	}
}

/*****************    Data Structures    *****************/
#ifdef FONT_EN
const PROGMEM byte font[100][5] = {
{0x00, 0x00, 0x00, 0x00, 0x00}, // 20
{0x00, 0x00, 0x5f, 0x00, 0x00}, // 21 ! 
{0x00, 0x07, 0x00, 0x07, 0x00}, // 22 ?
{0x14, 0x7f, 0x14, 0x7f, 0x14}, // 23 #
{0x24, 0x2a, 0x7f, 0x2a, 0x12}, // 24 $
{0x23, 0x13, 0x08, 0x64, 0x62}, // 25 %
{0x36, 0x49, 0x55, 0x22, 0x50}, // 26 &
{0x00, 0x05, 0x03, 0x00, 0x00}, // 27 ?
{0x00, 0x1c, 0x22, 0x41, 0x00}, // 28 (
{0x00, 0x41, 0x22, 0x1c, 0x00}, // 29 )
{0x14, 0x08, 0x3e, 0x08, 0x14}, // 2a *
{0x08, 0x08, 0x3e, 0x08, 0x08}, // 2b +
{0x00, 0x50, 0x30, 0x00, 0x00}, // 2c ,
{0x08, 0x08, 0x08, 0x08, 0x08}, // 2d ?
{0x00, 0x60, 0x60, 0x00, 0x00}, // 2e .
{0x20, 0x10, 0x08, 0x04, 0x02}, // 2f /
{0x3e, 0x51, 0x49, 0x45, 0x3e}, // 30 0
{0x00, 0x42, 0x7f, 0x40, 0x00}, // 31 1
{0x42, 0x61, 0x51, 0x49, 0x46}, // 32 2
{0x21, 0x41, 0x45, 0x4b, 0x31}, // 33 3
{0x18, 0x14, 0x12, 0x7f, 0x10}, // 34 4
{0x27, 0x45, 0x45, 0x45, 0x39}, // 35 5
{0x3c, 0x4a, 0x49, 0x49, 0x30}, // 36 6
{0x01, 0x71, 0x09, 0x05, 0x03}, // 37 7
{0x36, 0x49, 0x49, 0x49, 0x36}, // 38 8
{0x06, 0x49, 0x49, 0x29, 0x1e}, // 39 9
{0x00, 0x36, 0x36, 0x00, 0x00}, // 3a :
{0x00, 0x56, 0x36, 0x00, 0x00}, // 3b ;
{0x08, 0x14, 0x22, 0x41, 0x00}, // 3c <
{0x14, 0x14, 0x14, 0x14, 0x14}, // 3d =
{0x00, 0x41, 0x22, 0x14, 0x08}, // 3e >
{0x02, 0x01, 0x51, 0x09, 0x06}, // 3f ?
{0x32, 0x49, 0x79, 0x41, 0x3e}, // 40 @
{0x7e, 0x11, 0x11, 0x11, 0x7e}, // 41 A
{0x7f, 0x49, 0x49, 0x49, 0x36}, // 42 B
{0x3e, 0x41, 0x41, 0x41, 0x22}, // 43 C
{0x7f, 0x41, 0x41, 0x22, 0x1c}, // 44 D
{0x7f, 0x49, 0x49, 0x49, 0x41}, // 45 E
{0x7f, 0x09, 0x09, 0x09, 0x01}, // 46 F
{0x3e, 0x41, 0x49, 0x49, 0x7a}, // 47 G
{0x7f, 0x08, 0x08, 0x08, 0x7f}, // 48 H
{0x00, 0x41, 0x7f, 0x41, 0x00}, // 49 I
{0x20, 0x40, 0x41, 0x3f, 0x01}, // 4a J
{0x7f, 0x08, 0x14, 0x22, 0x41}, // 4b K
{0x7f, 0x40, 0x40, 0x40, 0x40}, // 4c L
{0x7f, 0x02, 0x0c, 0x02, 0x7f}, // 4d M
{0x7f, 0x04, 0x08, 0x10, 0x7f}, // 4e N
{0x3e, 0x41, 0x41, 0x41, 0x3e}, // 4f O
{0x7f, 0x09, 0x09, 0x09, 0x06}, // 50 P
{0x3e, 0x41, 0x51, 0x21, 0x5e}, // 51 Q
{0x7f, 0x09, 0x19, 0x29, 0x46}, // 52 R
{0x46, 0x49, 0x49, 0x49, 0x31}, // 53 S
{0x01, 0x01, 0x7f, 0x01, 0x01}, // 54 T
{0x3f, 0x40, 0x40, 0x40, 0x3f}, // 55 U
{0x1f, 0x20, 0x40, 0x20, 0x1f}, // 56 V
{0x3f, 0x40, 0x38, 0x40, 0x3f}, // 57 W
{0x63, 0x14, 0x08, 0x14, 0x63}, // 58 X
{0x07, 0x08, 0x70, 0x08, 0x07}, // 59 Y
{0x61, 0x51, 0x49, 0x45, 0x43}, // 5a Z
{0x00, 0x7f, 0x41, 0x41, 0x00}, // 5b [
{0x02, 0x04, 0x08, 0x10, 0x20}, // 5c forward slash
{0x00, 0x41, 0x41, 0x7f, 0x00}, // 5d ]
{0x04, 0x02, 0x01, 0x02, 0x04}, // 5e ^
{0x40, 0x40, 0x40, 0x40, 0x40}, // 5f _
{0x00, 0x01, 0x02, 0x04, 0x00}, // 60 `
{0x20, 0x54, 0x54, 0x54, 0x78}, // 61 a
{0x7f, 0x48, 0x44, 0x44, 0x38}, // 62 b
{0x38, 0x44, 0x44, 0x44, 0x20}, // 63 c
{0x38, 0x44, 0x44, 0x48, 0x7f}, // 64 d
{0x38, 0x54, 0x54, 0x54, 0x18}, // 65 e
{0x08, 0x7e, 0x09, 0x01, 0x02}, // 66 f
{0x0c, 0x52, 0x52, 0x52, 0x3e}, // 67 g
{0x7f, 0x08, 0x04, 0x04, 0x78}, // 68 h
{0x00, 0x44, 0x7d, 0x40, 0x00}, // 69 i
{0x20, 0x40, 0x44, 0x3d, 0x00}, // 6a j
{0x7f, 0x10, 0x28, 0x44, 0x00}, // 6b k
{0x00, 0x41, 0x7f, 0x40, 0x00}, // 6c l
{0x7c, 0x04, 0x18, 0x04, 0x78}, // 6d m
{0x7c, 0x08, 0x04, 0x04, 0x78}, // 6e n
{0x38, 0x44, 0x44, 0x44, 0x38}, // 6f o
{0x7c, 0x14, 0x14, 0x14, 0x08}, // 70 p
{0x08, 0x14, 0x14, 0x18, 0x7c}, // 71 q
{0x7c, 0x08, 0x04, 0x04, 0x08}, // 72 r
{0x48, 0x54, 0x54, 0x54, 0x20}, // 73 s
{0x04, 0x3f, 0x44, 0x40, 0x20}, // 74 t
{0x3c, 0x40, 0x40, 0x20, 0x7c}, // 75 u
{0x1c, 0x20, 0x40, 0x20, 0x1c}, // 76 v
{0x3c, 0x40, 0x30, 0x40, 0x3c}, // 77 w
{0x44, 0x28, 0x10, 0x28, 0x44}, // 78 x
{0x0c, 0x50, 0x50, 0x50, 0x3c}, // 79 y
{0x44, 0x64, 0x54, 0x4c, 0x44}, // 7a z
{0x00, 0x08, 0x36, 0x41, 0x00}, // 7b {
{0x00, 0x00, 0x7f, 0x00, 0x00}, // 7c |
{0x00, 0x41, 0x36, 0x08, 0x00} // 7d }
};
#else 
const PROGMEM byte font[0][0];
#endif

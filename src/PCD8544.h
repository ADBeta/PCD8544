/*
	Other information is in PCD8544.cpp
	ADBeta
*/

#include "Arduino.h"

#ifndef PCD8544_h
#define PCD8544_h

/** Define Options **/
//Enable font memory
#define FONT_EN
//Enable direct display control addressing without a large memory bank
#define DIRECT_ADDR

/** font **/
#ifdef FONT_EN
extern const PROGMEM byte font[100][5];
#else
extern const PROGMEM byte font[0][0];
#endif

class PCD8544 {
	public: 
	/** Basic functions **/
	void sendCMD(byte);
	void sendDAT(byte);
	
	/** Screen Control **/
	void setPos(byte, byte); //X, Y
	void incPos();
	void clear();
	
	/** IDK What to call this **/
	void printChar(char);
	void printString(const char*);
	
	void printIcon(const byte*, const byte, const byte); //&IconByte, length, height
	
	/** Configuation Methods **/
	void init(byte, byte, byte, byte, byte); //RST, CLK, DAT, DC, CE
	void invert(bool);
	
	//private:
	/** Hardware level control **/
	void writeBit(byte);
	void writeByte(byte);

	/** Screen Parameters **/
	byte screenX, screenY;
	
	/** Pins **/
	uint8_t p_RST, p_CLK, p_DAT, p_DC, p_CE;

	/** LCD Parameters **/
	byte bias = 0x14;
	byte contrast = 0xB9;
	byte tempCoeff = 0x04;
	
	
	
};
#endif

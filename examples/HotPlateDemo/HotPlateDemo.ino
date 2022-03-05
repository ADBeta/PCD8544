#include "PCD8544.h"

#define RST 8
#define CLK 12
#define DAT 11
#define DC 10
#define CE 9

PCD8544 LCD;

//Plate is currently idle but HOT
const byte PROGMEM idleHOT[32] = {
	0x00, 0x01, 0x07, 0x1E, 0xF2, 0x06, 0x04, 0x04,
	0x0C, 0x18, 0x70, 0xC0, 0x80, 0x00, 0x00, 0x00,
	0xF0, 0xF0, 0xC0, 0xC7, 0xCC, 0xD8, 0xD8, 0xD5,
	0xD6, 0xD8, 0xD8, 0xCD, 0xC7, 0xC0, 0xF0, 0xF0
};

//Plate is currently idle and cold
const byte PROGMEM idleRDY[32] = {
	0x00, 0xFE, 0x62, 0xA2, 0x1C, 0x00, 0xFE, 0x02,
	0x04, 0xF8, 0x00, 0x1E, 0x20, 0xE0, 0x1E, 0x00,
	0xF0, 0xF7, 0xC0, 0xC0, 0xC7, 0xC0, 0xC7, 0xC4,
	0xC2, 0xC1, 0xC0, 0xC0, 0xC0, 0xC7, 0xF0, 0xF0
};

//Plate is currently heating
const byte PROGMEM heat[32] = {
0x00, 0x00, 0x00, 0x86, 0xCE, 0xFC, 0x78, 0x00,
0x00, 0x86, 0xCE, 0xFC, 0x78, 0x00, 0x00, 0x00,
0xF8, 0xF8, 0xC0, 0xC7, 0xCF, 0xDC, 0xD8, 0xC0, 
0xC0, 0xC7, 0xCF, 0xDC, 0xD8, 0xC0, 0xF8, 0xF8
};

//Strings to be held in progmem 
const char PROGMEM status[] = "Status:";
const char PROGMEM status_0[] = "READY";
const char PROGMEM status_1[] = "HEATING";
const char PROGMEM status_2[] = "HOT!";
const char PROGMEM status_3[] = "ERROR";

const char PROGMEM menu_0[] = "Select Temp";


const char PROGMEM temp_0[] = "T-Centre:";
const char PROGMEM temp_1[] = "T-Edge:";
const char PROGMEM temp_2[] = "T-Set:";

//String list containing all strings needed.
const char *const stringList[] PROGMEM = {
	status, status_0, status_1, status_2, status_3, 
	menu_0, 
	temp_0, temp_1, temp_2
};

#define TEMP_X 61
#define CNTR_Y 3
#define EDGE_Y 4
#define TSET_Y 5

#define STATUS_X 35
#define STATUS_Y 1

#define ICON_X 0
#define ICON_Y 0


//TODO Make functions to setup and update the screen

void tempScreenSetup() {
	//Sets up the temp screen with overlays and values. does not check temps
	//or status
	
	//Print default icon
	LCD.setPos(ICON_X, ICON_Y);
	LCD.printIcon(idleRDY, 16, 2);
	
	//Print Status
	LCD.setPos(STATUS_X, STATUS_Y -1);
	LCD.printString(fetchString(0));
	
	
	//Print temperature overlay
	LCD.setPos(0, CNTR_Y);
	LCD.printString(fetchString(6));

	LCD.setPos(0, EDGE_Y);
	LCD.printString(fetchString(7));
	
	LCD.setPos(0, TSET_Y);
	LCD.printString(fetchString(8));
	
	
	LCD.setPos(TEMP_X, CNTR_Y);
	LCD.printString("210C");
}


void tempScreenUpdate() {

}


char * fetchString(byte index) {
	static char buffer[20];
	
	strcpy_P(buffer, (char *)pgm_read_word(&(stringList[index])));

	return buffer;
}

void setup() {
	LCD.init(RST, CLK, DAT, DC, CE);
	LCD.clear();
	Serial.begin(9600);
	
	
	delay(1000);
	tempScreenSetup();
}

void loop() {
	
}

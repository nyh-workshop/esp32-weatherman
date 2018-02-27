#ifndef _I2C_LCD_H
#define _I2C_LCD_H

#include "driver/i2c.h"

#define NOP() asm volatile ("nop")

class I2CLCD
{

  public:
    // Basic driver for the I2C LCD:
    I2CLCD();
    I2CLCD(unsigned char inputSDA, unsigned char inputSCL, unsigned char address, unsigned char i2cNum);
    void init_i2c_hw(unsigned char inputSDA, unsigned char inputSCL,  unsigned char i2cNum);
    inline void backlightOn();
    inline void backlightOff();
    void init();
    void command(unsigned char inputCommand);
    void data(unsigned char inputData);

    // Print a text in the LCD w/ row and column:
    void writeText(const char* text, unsigned char inputRow, unsigned char inputCol);

    // ESP32 I2C write data: 
    esp_err_t write_byte(unsigned char inputByte);

    // HD44780 commands:
    inline void clearDisplay()      { command(0b00000001); }
    inline void rtnHome()           { command(0b00000010); } 
    inline void dispOn();
    inline void dispOff();
    inline void cursorOn();
    inline void cursorOff();
    inline void cursorBlinkOn();
    inline void cursorBlinkOff();
    inline void entryModeIncr();
    inline void entryModeDecr();
    inline void shiftDispOn();
    inline void shiftDispOff();
    inline void cursorMove();
    inline void dispShift();
    inline void cursorMovesLeft();
    inline void cursorMovesRight();

    // Microsecond delay routines taken from ESP32's Arduino Github:
    unsigned long IRAM_ATTR micros();
    void IRAM_ATTR delayMicroseconds(uint32_t us);

  private:
    unsigned char I2C_master_num;
    unsigned char SDA;
    unsigned char SCL;
    unsigned char i2clcd_address;
    unsigned int timeout = 1000;
    
    // LCD variables: 
    unsigned char backlight = 0x08;
    unsigned char E = 0x04;
    unsigned char RS = 0x01;
    unsigned char dispCtrl = 0x08;      // Display on/off control.
    unsigned char cursorCtrl = 0x10;    // Cursor or display shift.
    unsigned char entryCtrl = 0x04;     // Entry mode set.

    unsigned char row;
    unsigned char col;  

    // For microsecond delays:
    portMUX_TYPE microsMux = portMUX_INITIALIZER_UNLOCKED;
};

inline void I2CLCD::backlightOn() {
    backlight = 0x08;
    write_byte(backlight);
}

inline void I2CLCD::backlightOff() {
    backlight = 0x00;
    write_byte(backlight);
}

inline void I2CLCD::cursorBlinkOn() {
    dispCtrl = dispCtrl | 0x01;
    command(dispCtrl);
}

inline void I2CLCD::cursorBlinkOff() {
    dispCtrl = dispCtrl & ~0x01;
    command(dispCtrl);
}

inline void I2CLCD::cursorOn() {
    dispCtrl = dispCtrl | 0x02;
    command(dispCtrl);
}

inline void I2CLCD::cursorOff() {
    dispCtrl = dispCtrl & ~0x02;
    command(dispCtrl);
}

inline void I2CLCD::dispOn() {
    dispCtrl = dispCtrl | 0x04;
    command(dispCtrl);
}

inline void I2CLCD::dispOff() {
    dispCtrl = dispCtrl & ~0x04;
    command(dispCtrl);
}

inline void I2CLCD::entryModeIncr() {
    entryCtrl = entryCtrl | 0x02;
    command(entryCtrl);
}

inline void I2CLCD::entryModeDecr() {
    entryCtrl = entryCtrl & ~0x02;
    command(entryCtrl);
}

inline void I2CLCD::shiftDispOn(){ 
    entryCtrl = entryCtrl | 0x01;
    command(entryCtrl);
}

inline void I2CLCD::shiftDispOff(){ 
    entryCtrl = entryCtrl & ~0x01;
    command(entryCtrl);
}

inline void I2CLCD::cursorMove() {
    cursorCtrl = cursorCtrl & ~0x08;
    command(cursorCtrl);
}

inline void I2CLCD::dispShift() {
    cursorCtrl = cursorCtrl | 0x08;
    command(cursorCtrl);
}

inline void I2CLCD::cursorMovesLeft() {
    cursorCtrl = cursorCtrl & ~0x04;
    command(cursorCtrl);
}

inline void I2CLCD::cursorMovesRight() {
    cursorCtrl = cursorCtrl | 0x04;
    command(cursorCtrl);
}

#endif
#include "i2c_lcd.h"

// Setting default values for the I2C LCD:
I2CLCD::I2CLCD()
{

    SCL = (gpio_num_t)21;
    SDA = (gpio_num_t)22;
    i2clcd_address = 0x3f;
    I2C_master_num = (i2c_port_t)I2C_NUM_0;
    timeout = 1000;
    backlightOn();

    init_i2c_hw(SDA, SCL, I2C_master_num);

    // Then init the I2C LCD module:
    init();
}

I2CLCD::I2CLCD(unsigned char inputSDA, unsigned char inputSCL, unsigned char address, unsigned char i2cNum)
{
    i2clcd_address = 0x3f;
    timeout = 1000;
    I2C_master_num = (i2c_port_t)i2cNum;
    backlightOn();

    init_i2c_hw(inputSDA, inputSCL, i2cNum);

    // Then init the I2C LCD module:
    init();
}

void I2CLCD::init_i2c_hw(unsigned char inputSDA, unsigned char inputSCL, unsigned char i2cNum)
{

    // Init ESP32 I2C hardware:
    int i2c_master_port = (i2c_port_t)i2cNum;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = (gpio_num_t)inputSDA;
    conf.sda_pullup_en = GPIO_PULLUP_DISABLE;
    conf.scl_io_num = (gpio_num_t)inputSCL;
    conf.scl_pullup_en = GPIO_PULLUP_DISABLE;
    conf.master.clk_speed = 100000;
    i2c_param_config((i2c_port_t)i2c_master_port, &conf);
    i2c_driver_install((i2c_port_t)i2c_master_port, conf.mode,
                       0,
                       0, 0);
}

void I2CLCD::init()
{
    write_byte(0x30);
    write_byte(0x30 | E);
    delayMicroseconds(20);
    write_byte(0x30);

    vTaskDelay(1);

    write_byte(0x30);
    write_byte(0x30 | E);
    delayMicroseconds(20);
    write_byte(0x30);

    vTaskDelay(1);

    write_byte(0x30);
    write_byte(0x30 | E);
    delayMicroseconds(20);
    write_byte(0x30);

    write_byte(0x20);
    write_byte(0x20 | E);
    delayMicroseconds(20);
    write_byte(0x20);

    vTaskDelay(1);

    //command(0b00000001);
    //command(0b00000110);
    //command(0b00001100); // Display ON, blink off, cursor off.

    clearDisplay();
    entryModeIncr();
    cursorOff();
    cursorBlinkOff();
    dispOn();
}

esp_err_t I2CLCD::write_byte(unsigned char inputByte)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (i2clcd_address << 1) | I2C_MASTER_WRITE, 1);
    i2c_master_write_byte(cmd, inputByte, 1);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin((i2c_port_t)I2C_master_num, cmd, timeout / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

void I2CLCD::command(unsigned char cmd)
{

    unsigned char highNibble = (cmd & 0xF0) >> 4;
    unsigned char lowNibble = cmd & 0x0F;

    // Send high nibble
    //write_byte(highNibble);
    write_byte((highNibble << 4) | E | backlight);
    delayMicroseconds(20);
    write_byte((highNibble << 4) | backlight);

    delayMicroseconds(100);

    // Send low nibble
    //write_byte(lowNibble);
    write_byte((lowNibble << 4) | E | backlight);
    delayMicroseconds(20);
    write_byte((lowNibble << 4) | backlight);

    delayMicroseconds(5000);
}

void I2CLCD::data(unsigned char dt)
{

    unsigned char highNibble = (dt & 0xF0) >> 4;
    unsigned char lowNibble = dt & 0x0F;

    // Send high nibble
    //write_byte(highNibble);
    write_byte((highNibble << 4) | E | RS | backlight);
    delayMicroseconds(20);
    write_byte((highNibble << 4) | RS | backlight);

    delayMicroseconds(100);

    // Send low nibble
    //write_byte(lowNibble);
    write_byte((lowNibble << 4) | E | RS | backlight);
    delayMicroseconds(20);
    write_byte((lowNibble << 4) | RS | backlight);

    delayMicroseconds(5000);
}

void I2CLCD::writeText(const char *text, unsigned char inputRow, unsigned char inputCol)
{

    unsigned char address_d = 0; // address of the data in the screen.
    switch (inputRow)
    {
    case 0:
        address_d = 0x80 + inputCol; // at zeroth row
        break;
    case 1:
        address_d = 0xC0 + inputCol; // at first row
        break;
    case 2:
        address_d = 0x94 + inputCol; // at second row
        break;
    case 3:
        address_d = 0xD4 + inputCol; // at third row
        break;
    default:
        address_d = 0x80 + inputCol; // returns to first row if invalid row number is detected
        break;
    }

    command(address_d);

    while (*text) // Place a string, letter by letter.
        data(*text++);
}

unsigned long IRAM_ATTR I2CLCD::micros()
{
    static unsigned long lccount = 0;
    static unsigned long overflow = 0;
    unsigned long ccount;
    portENTER_CRITICAL_ISR(&microsMux);
    __asm__ __volatile__("rsr     %0, ccount"
                         : "=a"(ccount));
    if (ccount < lccount)
    {
        overflow += UINT32_MAX / CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ;
    }
    lccount = ccount;
    portEXIT_CRITICAL_ISR(&microsMux);
    return overflow + (ccount / CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ);
}

void IRAM_ATTR I2CLCD::delayMicroseconds(uint32_t us)
{
    uint32_t m = micros();
    if(us){
        uint32_t e = (m + us);
        if(m > e){ //overflow
            while(micros() > e){
                NOP();
            }
        }
        while(micros() < e){
            NOP();
        }
    }
}
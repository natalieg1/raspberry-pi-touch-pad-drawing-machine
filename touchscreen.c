// This is a driver for the tsc2007 i2c resistive touch screen controller
// Datasheet: https://www.ti.com/lit/ds/symlink/tsc2007.pdf?ts=1710438763717
// By Natalie Greenfield, March 17 2024

#include "rpi.h"
#include "libc/helper-macros.h"
#include "i2c.h"
#include "bit-support.h"
#include "rgb.h"
#include "touchscreen.h"

enum {
    // Functions
    SETUP = 0b1011,
    READ_X = 0b1100,
    READ_Y = 0b1101,
    READ_Z = 0b1110,
};

// Power setting
uint8_t power = 0;

// Resolution mode 
uint8_t mode = 0;

int rd_n_wait(uint8_t addr, uint8_t base_reg, uint8_t *v, uint32_t n) {
    i2c_write(addr, (void*) &base_reg, 1);
    // Delay required by manual before read 
    delay_us(500);
    return i2c_read(addr, v, n);
}

void TSC2007_init(uint32_t addr, uint32_t pwr, uint32_t res_mod) {
    power = pwr;
    mode = res_mod;
    delay_ms(100);
    // Signal that you are done setting up 
    uint8_t command = TSC2007_command(0, 0, 0);
    i2c_write(addr, &command, 1);
}

// Creates command byte
uint8_t TSC2007_command(uint8_t function, uint8_t power, uint8_t mode) {
    uint8_t command = (function << 4);
    command |= (power << 2);
    command |= (mode << 1);
    return command;
}

// Send command byte and read values back 
uint16_t read_val(uint32_t addr, uint8_t com){
    uint8_t byte[2];
    rd_n_wait(addr, com, byte, 2);
    uint16_t result = ((uint16_t)byte[0] << 4) + (byte[1] >> 4);
    return result;
}

// Sheck if touch pad has been pressed
int TSC2007_touched(uint32_t addr) {
    uint8_t z_com = TSC2007_command(READ_Z, power, mode);
    uint16_t z = read_val(addr, z_com);

    // Signal that you are done reading 
    uint8_t command = TSC2007_command(0, 0, 0);
    i2c_write(addr, &command, 1);
    return z > 100;
}

// Read x, y, and z values 
touch_point TSC2007_read(uint32_t addr) {
    touch_point p;
    uint8_t x_com = TSC2007_command(READ_X, power, mode);
    uint8_t y_com = TSC2007_command(READ_Y, power, mode);
    uint8_t z_com = TSC2007_command(READ_Z, power, mode);
    p.x = read_val(addr, x_com);
    p.y = read_val(addr, y_com);
    p.z = read_val(addr, z_com);

    // Signal that you are done reading 
    uint8_t command = TSC2007_command(0, 0, 0);
    i2c_write(addr, &command, 1);
    return p;
}


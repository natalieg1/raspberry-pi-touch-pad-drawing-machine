// This page contains the driver code for a TCS24725 rgb sensor 
// Data sheet: https://cdn-shop.adafruit.com/datasheets/TCS34725.pdf
// By Natalie Greenfield, March 17 

#include "rpi.h"
#include "libc/helper-macros.h"
#include "i2c.h"
#include "bit-support.h"
#include "rgb.h"
#include "i2c-helpers.h"

enum {
    // p3
    dev_address = 0x29,

    // pg 13 - table 3 

    // Set up 
    ENABLE = 0xa0,
    ATIME = 0xa1,
    WTIME = 0xa3,

    // Sonfig 
    CONFIG = 0xaD,
    CONTROL = 0xaF,
    ID = 0xb2,
    STATUS = 0xb3,

    // Solor registers 
    COLOR_START = 0xb4,
    // CDATAH = 0xb5,
    // RDATAL = 0x16,
    // RDATAH = 0x17,
    // GDATAL = 0x18,
    // GDATAH = 0x19,
    // BDATAL = 0x1A,
    // BDATAH = 0x1B,

    // Settings
    integration_time = 0xd5, 
    gain = 0x01, 
};

void TCS34725_init(uint32_t addr) {
    // Set integration time
    imu_wr(addr, ATIME, integration_time);
    delay_ms(100);
    // Set gain
    imu_wr(addr, CONTROL, gain);
    delay_ms(100);
    // Enable AEN and PON 
    imu_wr(addr, ENABLE, 0x03);
    delay_ms(100);
}
 
color_vals TCS34725_read(uint32_t addr) {
    // Wait till status is 1 such that rgb values are valid 
    while(!(imu_rd(addr, STATUS) & 1)){};
    uint8_t regs[8];
    imu_rd_n(addr, COLOR_START , regs, 8);
    color_vals colors;
    // Add color vals to struct 
    colors.clear = ((uint16_t)regs[1] << 8) + regs[0];
    colors.red = ((uint16_t)regs[3] << 8) + regs[2];
    colors.green = ((uint16_t)regs[5] << 8) + regs[4];
    colors.blue = ((uint16_t)regs[7] << 8) + regs[6];
    return colors;
}



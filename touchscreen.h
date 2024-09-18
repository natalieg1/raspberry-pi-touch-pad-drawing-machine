#ifndef __TOUCHSCREEN_H__
#define __TOUCHSCREEN_H__

typedef struct { 
    uint16_t x, y, z;
} touch_point;

void TSC2007_init(uint32_t addr, uint32_t pwr, uint32_t res_mod);

// Create command byte
uint8_t TSC2007_command(uint8_t function, uint8_t power, uint8_t mode);

int TSC2007_touched(uint32_t addr);

touch_point TSC2007_read(uint32_t addr);

#endif

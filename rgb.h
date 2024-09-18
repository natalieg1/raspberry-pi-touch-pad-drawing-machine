#ifndef __RGB_H__
#define __RGB_H__


typedef struct { 
    uint16_t clear, red, green, blue;
} color_vals;

// Intialize the color sensor 
void TCS34725_init(uint32_t addr);

// Read from the color sensor into a color_vals struct
color_vals TCS34725_read(uint32_t addr);

#endif

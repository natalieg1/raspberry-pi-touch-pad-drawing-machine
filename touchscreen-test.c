// This page of code is a basic test for the tsc2007 touchscreen
// Written by Natalie Greenfield, March 17 2024
#include "rpi.h"
#include "i2c.h"
#include "touchscreen.h"

void notmain(void) {
    uint8_t addr = 0x48;
    delay_ms(100);   // Allow time for i2c/device to boot up.
    i2c_init(); 
    delay_ms(100);   // Allow time for i2c/dev to settle after init.

    TSC2007_init(addr, 1, 0);
    delay_ms(100); 

    for (int i = 0; i < 30; i++){
        while(!TSC2007_touched(addr)){};
        printk("touched\n");
        touch_point touch = TSC2007_read(addr);
        printk("x: %d\n", touch.x);
        printk("y: %d\n", touch.y);
        printk("z: %d\n", touch.z);
        printk("\n");
        delay_ms(100);
    }
}

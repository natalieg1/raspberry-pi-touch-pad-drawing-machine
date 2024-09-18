// Initial simple test for a TCS24725 rgb sensor 
// By Natalie Greenfield, March 17 2024
#include "rpi.h"
#include "i2c.h"
#include "rgb.h"

// Read a single device register <reg> from i2c device 
// <addr> and return the result.
static uint8_t i2c_get_reg(uint8_t addr, uint8_t reg) {
    i2c_write(addr, &reg, 1);
    uint8_t v;
    i2c_read(addr,  &v, 1);
    return v;
}

void notmain(void) {
    uint8_t addr = 0x29;
    delay_ms(100);   // Allow time for i2c/device to boot up.
    i2c_init(); 
    delay_ms(100);   // Allow time for i2c/dev to settle after init.

    TCS34725_init(addr);

    enum {
        WHO_AM_I_REG = 0xb2,
        WHO_AM_I_VAL = 0x44,
    };

    uint8_t v = i2c_get_reg(addr, WHO_AM_I_REG);
    if(v != WHO_AM_I_VAL)
        panic("Initial probe failed: expected %b (%x), have %b (%x)\n",
            WHO_AM_I_VAL, WHO_AM_I_VAL, v, v);
    printk("SUCCESS:  acknowledged our ping: WHO_AM_I=%b, (%x)!!\n", v);

    for (int i = 0; i < 20; i++){
        color_vals color = TCS34725_read(addr);
        printk("clear: %d\n", color.clear);
        printk("red: %d\n", color.red);
        printk("green: %d\n", color.green);
        printk("blue: %d\n", color.blue);
        printk("\n");
        delay_ms(1000);
    }
}

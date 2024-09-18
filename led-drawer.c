// This page contains the code to run an LED matrix, RGB sensor and touchpad drawing system 
// By Natalie Greenfield, March 17 2024

#include "rpi-thread.h"
#include "rpi.h"
#include "WS2812B.h"
#include "neopixel.h"
#include "i2c.h"
#include "rgb.h"
#include "touchscreen.h"
#include "pi-sd.h"

// Shared values struct between threads 
typedef struct {
    uint16_t x, y, z; // Touch coordinates
    uint16_t r, g, b; // Color values
    int valid; // Indicates if the data is ready to used
} sharedValues;

sharedValues state = {0, 0, 0, 0, 0, 0, 0};

neo_t h;

// Adresses for i2c devices 
uint32_t addr_color = 0x29;
uint32_t addr_touch = 0x48;

// pix_pin used to control the light strip.
// Save pin used to save image
enum { pix_pin = 21, save_pin = 23, view_save_pin = 24};

void enable_cache(void) {
    unsigned r;
    asm volatile ("MRC p15, 0, %0, c1, c0, 0" : "=r" (r));
	r |= (1 << 12); // l1 instruction cache
	r |= (1 << 11); // Branch prediction
    asm volatile ("MCR p15, 0, %0, c1, c0, 0" :: "r" (r));
}

// Check if the touch screen or the save button has been pressed 
void touchScreenReader(void * temp_buf){
    while (1) {
        // Touch screen
        if (TSC2007_touched((uint32_t)addr_touch)) {
            touch_point touch = TSC2007_read((uint32_t)addr_touch);
            state.x = touch.x;
            state.y = touch.y;
            state.z = touch.z;
            state.valid = 1;
        }
        // Save button 
        if (gpio_read(save_pin) == 0) {
            uint32_t lba_addr = 29900 + (4089 - 2) * 16;
            memcpy(temp_buf, &h->pixels, 192);
            pi_sd_write(temp_buf, lba_addr, 1);
            printk("saved\n");
        };
        rpi_yield();
    }
}

// Read RGBC values from the color sensor and update state 
void colorSensorReader() {
    while (1) {
        color_vals color = TCS34725_read((uint32_t)addr_color);
        // Reset state
        state.r = 0;
        state.g = 0;
        state.b = 0;
        // Update state
        if (((color.red > color.green) && (color.red > color.blue))){
            state.r = 0xff;
        }
        else if ((color.green > color.red) && (color.green > color.blue)) {
            state.g = 0xff;
        }
        else{
            state.b = 0xff;
        }

        // Clear LED matrix if color sensor is covered 
        if (color.clear <= 300) {
            for(int i = 0; i < 256; i++)
                pix_sendpixel(h->pin, 0,0,0);
            neopix_flush(h);
        }
        rpi_yield();
    }
}
// Update LED matrix based on touch point location and color 
void ledMatrixUpdater(void * temp_buf) {
    while (1) {
        // Check if save button has been pressed and update LED matrix to saved image 
        if (gpio_read(view_save_pin) == 0){
            uint32_t lba_addr = 29900 + (4089 - 2) * 16;
            pi_sd_read(temp_buf, lba_addr, 1);
            //num bytes = 3 bytes per pixel * 64 pixels in light array 
            memcpy(&h->pixels, temp_buf, 192);

            // Show stored drawing 
            neopix_flush_buffer_same(h);
        }

        if (state.valid) {
            uint32_t index = 0;
            if (((state.x/500)%2) == 1) {
                index = ((state.x/500) * 8) + 7 - (state.y/500);
            }
            else {
                index = ((state.x/500) * 8) + (state.y/500);
            }
            if (state.x > 4000 || state.y > 4000) {
                 neopix_write(h, 0, state.r, state.g, state.b);
            }
            else {
            neopix_write(h, index, state.r, state.g, state.b);
            }
            neopix_flush_buffer_same(h);
            state.valid = 0;
        }
        rpi_yield();
    }
}

void notmain() {
    // Set up i2c
    delay_ms(100);   // Allow time for i2c/device to boot up.
    i2c_init(); 
    delay_ms(100);   // Allow time for i2c/dev to settle after init.

    // Setup lights 
    enable_cache(); 
    gpio_set_output(pix_pin);
    // Full board is 256
    unsigned npixels = 64; 
    h = neopix_init(pix_pin, npixels);

    // Set up color sensor 
    TCS34725_init(addr_color);

    // Set up touchscreen 
    TSC2007_init(addr_touch, 1, 0);
    delay_ms(100); 

    // Set up save button
    gpio_set_input(save_pin);
    gpio_set_pullup(save_pin);

    // Set up view saved image button
    gpio_set_input(view_save_pin);
    gpio_set_pullup(view_save_pin);

    // Init sd card writing 
    pi_sd_init();
    uint32_t lba_addr = 29900 + (4089 - 2) * 16;
    uint8_t * temp_buf = kmalloc(512);
    pi_sd_read(temp_buf, lba_addr, 1);

    // num bytes = 3 bytes per pixel * 64 pixels in light array 
    memcpy(&h->pixels, temp_buf, 192);

    // Show stored drawing 
    neopix_flush_buffer_same(h);
    
    // Initiate threads
    unsigned oneMB = 1024*1024;
    kmalloc_init_set_start((void*)oneMB, oneMB);

    // Fork threads
    rpi_fork(touchScreenReader, (void*)temp_buf);
    rpi_fork(colorSensorReader, (void*)0);
    rpi_fork(ledMatrixUpdater,(void *)temp_buf);
    rpi_thread_start();  
    panic("should not reach here");
}

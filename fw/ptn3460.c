//
// Copyright 2021 Wenting Zhang <zephray@outlook.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ptn3460.h"
#include "syslog.h"
#include "utils.h"
//#include "edid.h"

#define PTN3460_I2C_ADDRESS (0x60)
#define PTN3460_I2C         (i2c0)
#define PTN3460_HPD_PIN     (12)
#define PTN3460_PDN_PIN     (13)

void ptn3460_select_edid_emulation(uint8_t id) {
    uint8_t buf[2];
    int result;
    buf[0] = (uint8_t)0x84;
    buf[1] = (uint8_t)0x01 | (id << 1);
    result = i2c_write_blocking(PTN3460_I2C, PTN3460_I2C_ADDRESS,
            buf, 2, false);
    if (result != 2) {
        fatal("Failed writing data to PTN3460");
    }
}

void ptn3460_load_edid(uint8_t *edid) {
    uint8_t buf[1];
    int result;
    buf[0] = 0;
    result = i2c_write_blocking(PTN3460_I2C, PTN3460_I2C_ADDRESS,
            buf, 1, true);
    result = i2c_write_blocking(PTN3460_I2C, PTN3460_I2C_ADDRESS,
            edid, 128, false);
    if (result != 128) {
        fatal("Failed writing data to PTN3460");
    }
}

void ptn3460_init() {
    gpio_init(PTN3460_HPD_PIN);
    gpio_set_dir(PTN3460_HPD_PIN, GPIO_IN);
    gpio_pull_down(PTN3460_HPD_PIN);
    gpio_init(PTN3460_PDN_PIN);
    gpio_put(PTN3460_PDN_PIN, 1);
    gpio_set_dir(PTN3460_PDN_PIN, GPIO_OUT);
    sleep_ms(100);
    // wait for HPD to become high
    int ticks = 0;
    while (gpio_get(PTN3460_HPD_PIN) != true) {
        ticks ++;
        if (ticks > 500) {
            fatal("PTN3460 boot timeout");
        }
        sleep_ms(1);
    }
    syslog_printf("PTN3460 up after %d ms", ticks);
    // Enable EDID emulation
    ptn3460_select_edid_emulation(0);
    //ptn3460_load_edid();

    uint8_t buf[2];
    int result;
    buf[0] = (uint8_t)0x80;
    buf[1] = (uint8_t)0x02;
    result = i2c_write_blocking(PTN3460_I2C, PTN3460_I2C_ADDRESS,
            buf, 2, false);
    if (result != 2) {
        fatal("Failed writing data to PTN3460");
    }
}
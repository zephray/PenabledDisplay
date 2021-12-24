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
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"
#include "lcd.h"
#include "ui.h"
#include "syslog.h"
#include "utils.h"
#include "tcpm_driver.h"
#include "usb_pd.h"
#include "ptn3460.h"

uint16_t colors[3] = {0xf800, 0x07e0, 0x001f};

bool reserved_addr(uint8_t addr) {
    return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

void i2c_scan() {
    ui_printf(0, 0, "I2C Bus Scan");
    ui_printf(0, 8, "   0123456789ABCDEF");
    int x = 0, y = 16;

    for (int addr = 0; addr < (1 << 7); ++addr) {
        if (addr % 16 == 0) {
            ui_printf(x, y, "%02x ", addr);
            x += 18;
        }

        // Perform a 1-byte dummy read from the probe address. If a slave
        // acknowledges this address, the function returns the number of bytes
        // transferred. If the address byte is ignored, the function returns
        // -1.

        // Skip over any reserved addresses.
        int ret;
        uint8_t rxdata;
        if (reserved_addr(addr))
            ret = PICO_ERROR_GENERIC;
        else
            ret = i2c_read_blocking(i2c0, addr, &rxdata, 1, false);

        ui_printf(x, y, ret < 0 ? "." : "@");
        x += 6;
        if(addr % 16 == 15) {
            x = 0; y += 8;
        }
    }
}

int main()
{
    stdio_init_all();

    lcd_init();
    ui_init();
    lcd_update();

    int result = tcpm_init(0);
    if (result)
        fatal("Failed to initialize TCPC");

    int cc1, cc2;
    tcpc_config[0].drv->get_cc(0, &cc1, &cc2);
    syslog_printf("CC status %d %d", cc1, cc2);

    ptn3460_init();
    pd_init(0);
    sleep_ms(50);

    syslog_disp();

    const uint LED_PIN = 22;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, true);
    int i = 0;

    extern int dp_enabled;
    bool hpd_sent = false;

    while (1) {
        // Interrupt is not available, polling
        fusb302_tcpc_alert(0);
        pd_run_state_machine(0);
        if (dp_enabled && !hpd_sent && !pd_is_vdm_busy(0)) {
            syslog_printf("DP enabled\n");
            pd_send_hpd(0, hpd_high);
            hpd_sent = true;
        }
        syslog_disp();
    }

    return 0;
}

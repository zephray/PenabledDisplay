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
#include "lcd.h"
#include "ui.h"
#include "syslog.h"
#include "utils.h"
#include "tcpm_driver.h"
#include "usb_pd.h"

uint16_t colors[3] = {0xf800, 0x07e0, 0x001f};

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

    syslog_disp();

    const uint LED_PIN = 22;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    int i = 0;
    while (1) {
        gpio_put(LED_PIN, 1);
        sleep_ms(250);
        gpio_put(LED_PIN, 0);
        sleep_ms(250);
        pd_run_state_machine(0);
    }

    return 0;
}

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
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "lcd.h"
#include "ui.h"
#include "syslog.h"

typedef struct _msg_t {
    struct _msg_t *prev;
    struct _msg_t *next;
    char text[];
} msg_t;

// States
static msg_t *head;
static msg_t *tail;
static msg_t *viewable_first;
static int offset;
static int count;

void syslog_init(void) {
    head = NULL;
    tail = NULL;
    count = 0;
}

void syslog_disp(void) {
    ui_clear(0x0000);
    ui_disp_string(0, 0, "System log", 0xffff);
    
    msg_t *msg = viewable_first;
    int y = UI_HEIGHT;
    while ((msg) && (y > 8)) {
        int h = ui_disp_string_bb(msg->text, UI_WIDTH);
        y -= h;
        if (y >= 8)
            ui_disp_string(0, y, msg->text, 0xffff);
        msg = msg->next;
    }
    lcd_update();
}

static void syslog_add_to_tail(msg_t *msg) {
    msg->prev = tail;
    if (tail) {
        tail->next = msg;
    }
    else {
        head = msg;
        viewable_first = msg;
    }
    tail = msg;
    count++;
}

static void syslog_del_from_head(void) {
    msg_t *old = head;
    if (head) {
        head = head->next;
        if (head)
            head->prev = NULL;
        free(old);
        count--;
    }
}

int syslog_printf(const char *format, ...) {
    char printf_buffer[SYSLOG_PRINTF_BUFFER_SIZE];

    int length = 0;

    va_list ap;
    va_start(ap, format);

    length = vsnprintf(printf_buffer, SYSLOG_PRINTF_BUFFER_SIZE, format, ap);

    va_end(ap);

    msg_t *msg = malloc(sizeof(msg_t) + length + 1);
    memcpy(msg->text, printf_buffer, length + 1);
    msg->next = NULL;
    msg->prev = NULL;

    if (count >= SYSLOG_MAX_LINES)
        syslog_del_from_head();
    syslog_add_to_tail(msg);

    return length;
}
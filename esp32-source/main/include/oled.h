#ifndef OLED_H_
#define OLED_H_

#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include "sdkconfig.h"
#include <string.h>

#include <ssd1306.h>
// #include <font8x8_basic.h>

esp_err_t i2c_master_init(void);
void ssd1306_init();
void task_ssd1306_display_text(const void *arg_text);
void task_ssd1306_display_clear();

#endif
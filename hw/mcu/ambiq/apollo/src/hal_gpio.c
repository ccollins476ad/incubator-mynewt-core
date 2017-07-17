/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <assert.h>
#include <os/os.h>
#include <am_mcu_apollo.h>
#include <am_hal_pin.h>
#include <am_hal_gpio.h>
#include <hal/hal_gpio.h>

int
hal_gpio_init_in(int pin, hal_gpio_pull_t pull)
{
    uint32_t cfg;

    cfg = AM_HAL_PIN_INPUT;

    switch (pull)  {
        case HAL_GPIO_PULL_UP:
            cfg |= AM_HAL_GPIO_PULLUP;
            break;
        default:
            break;
    }
    am_hal_gpio_pin_config((uint32_t) pin, cfg);

    return (0);
}

int
hal_gpio_init_out(int pin, int val)
{
    am_hal_gpio_pin_config((uint32_t) pin, AM_HAL_GPIO_OUTPUT);
    hal_gpio_write(pin, val);

    return (0);
}

void
hal_gpio_write(int pin, int val)
{
    if (val) {
        am_hal_gpio_out_bit_set((uint32_t) pin);
    } else {
        am_hal_gpio_out_bit_clear((uint32_t) pin);
    }
}

int
hal_gpio_read(int pin)
{
    int state;

    state = am_hal_gpio_input_bit_read((uint32_t) pin);

    return (state);
}

int
hal_gpio_toggle(int pin)
{
    am_hal_gpio_out_bit_toggle(pin);

    return (0);
}

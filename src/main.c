// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText 2023 Kristin Ebuengan
// SPDX-FileCopyrightText 2023 Melody Gill
// SPDX-FileCopyrightText 2023 Gabriel Marcano

/* 
* This is an edited file of main.c from https://github.com/gemarcano/redboard_lora_example
* which this repo was forked from
*
* Uses GPIO interrupts from the redboard to alert when an alarm set off from the RTC
*/

#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"

#include <string.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <inttypes.h>

#include <uart.h>
#include <adc.h>
#include <syscalls.h>
#include <gpio.h>

#define CHECK_ERRORS(x)\
	if ((x) != AM_HAL_STATUS_SUCCESS)\
	{\
		error_handler(x);\
	}

volatile uint32_t count;
volatile uint32_t val;

static void error_handler(uint32_t error)
{
	(void)error;
	for(;;)
	{
		am_devices_led_on(am_bsp_psLEDs, 0);
		am_util_delay_ms(500);
		am_devices_led_off(am_bsp_psLEDs, 0);
		am_util_delay_ms(500);
	}
}

/* added gpio_handler which differs from the original file */
void gpio_handler(void)
{
    am_hal_gpio_state_read(23,  AM_HAL_GPIO_INPUT_READ, &val);
    count++;
}

/* 
* removed anything using LoRa from original file and used the file as a
* template for GPIO interrupts 
*/
static struct uart uart;
static struct gpio alarm;

int main(void)
{
	// Prepare MCU by init-ing clock, cache, and power level operation
	am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_SYSCLK_MAX, 0);
	am_hal_cachectrl_config(&am_hal_cachectrl_defaults);
	am_hal_cachectrl_enable();
	am_bsp_low_power_init();
	am_hal_sysctrl_fpu_enable();
	am_hal_sysctrl_fpu_stacking_enable(true);

	// Enabling the GPIO interrupt (added from original file)
	am_hal_gpio_interrupt_register(23, gpio_handler);
	am_hal_gpio_interrupt_clear(((uint64_t) 0x1) << 23);
	gpio_init(&alarm, 23, GPIO_MODE_INPUT, false);
	am_hal_interrupt_master_enable();

	// Init UART, registers with SDK printf
	syscalls_uart_init(&uart);
	uart_init(&uart, UART_INST0);

	// Wait till an interrupt happens (changed from original file)
	int counter = 0;
	while (1)
	{
		// tells us when the alarm stops
		am_util_stdio_printf("alarm went off %d\r\n\r\n", counter);
		am_util_delay_ms(10);
		counter += 10;
		
		// code for gpio_handler
		int currentCount = count;
		while (currentCount == count) {
			am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
		}
	}
}

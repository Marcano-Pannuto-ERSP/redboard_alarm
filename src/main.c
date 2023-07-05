// SPDX-License-Identifier: Apache-2.0
// Copyright: Gabriel Marcano, 2023

#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"

#include <string.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>

#include <uart.h>
#include <adc.h>
#include <spi.h>
#include <lora.h>
#include <gpio.h>

#define CHECK_ERRORS(x)\
	if ((x) != AM_HAL_STATUS_SUCCESS)\
	{\
		error_handler(x);\
	}

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

	// Init UART, registers with SDK printf
	uart_init(&uart, UART_INST0);

	gpio_init(&alarm, 23, GPIO_MODE_INPUT, false);

	// Wait here for the ISR to grab a buffer of samples.
	while (1)
	{
		// Print the battery voltage and temperature for each interrupt
		//
		uint32_t data = 0;
		if (gpio_read(&alarm) == false){
			am_util_stdio_terminal_clear();
			am_util_stdio_printf("alarm went off\r\n\r\n");
		}
		am_util_delay_ms(200);
	}
}

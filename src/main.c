// SPDX-License-Identifier: Apache-2.0
// Copyright: Gabriel Marcano, 2023

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

void gpio_handler(void)
{
    uint32_t count;
    uint32_t val;

    //
    // Debounce for 20 ms.
    // We're triggered for rising edge - so we expect a consistent HIGH here
    //
    for (count = 0; count < 10; count++)
    {
        am_hal_gpio_state_read(23,  AM_HAL_GPIO_INPUT_READ, &val);
        if (!val)
        {
            return; // State not high...must be result of debounce
        }
        am_util_delay_ms(2);
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

	am_hal_gpio_interrupt_register(23, gpio_handler);

	am_hal_gpio_interrupt_clear(((uint64_t) 0x1) << 23);
	gpio_init(&alarm, 23, GPIO_MODE_INPUT, false);

	// Init UART, registers with SDK printf
	uart_init(&uart, UART_INST0);

	am_hal_interrupt_master_enable();

	int counter = 0;
	// Wait here for the ISR to grab a buffer of samples.
	while (1)
	{
		uint32_t data = 0;
		if (gpio_read(&alarm) == false){
			am_util_stdio_printf("alarm went off %d\r\n\r\n", counter);
		}
		am_util_delay_ms(10);
		counter += 10;

		uint64_t ui64Status;
		am_hal_gpio_interrupt_status_get(false, &ui64Status);
		am_util_stdio_printf("status: %08llX\r\n\r\n", ui64Status);
		am_util_stdio_printf("status: %"PRIx64"\r\n\r\n", ui64Status);
		am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
	}
}

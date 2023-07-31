#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"

#include <sys/time.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>

#include <uart.h>
#include <spi.h>
#include <adc.h>
#include <am1815.h>
#include <bmp280.h>
#include <flash.h>
#include <pdm.h>
#include <syscalls.h>
#include <asimple_littlefs.h>

uint8_t am1815_read_timer(struct am1815 *rtc)
{
	uint32_t buffer[1];
	uint8_t *data = (uint8_t*)buffer;
	spi_device_cmd_read(rtc->spi, 0x19, data, 1);
	memcpy(data, buffer, 1);

    return data;
}

double find_timer(double timer)
{
    if(timer <= 0.0625){
        timer = ((int)(timer * 4096))/4096.0;
	}
    else if(timer <= 4){
        timer = ((int)(timer * 64))/64.0;
	}
    else if(timer <= 256){
        timer = (int)timer;
	}
    else if(timer <= 15360){
        timer = ((int)(timer/60)) * 60;
	}
    else{
        timer = 15360;
	}

    // if(timer == 0){
    //     printf("Timer disabled (set to 0 seconds)");
	// }
    // else{
    //     printf("Timer set to: %u", timer);
	// }
    return timer;
}

void am1815_write_timer2(struct am1815 *rtc, double timer)
{
	double finalTimer = find_timer(timer);

	if(finalTimer == 0){
        am_util_stdio_printf("Timer disabled (set to 0 seconds)\r\n");
		return;
	}
    else{
        am_util_stdio_printf("Timer set to: %f\r\n", finalTimer);
	}

	// TE (enables countdown timer)
    // Sets the Countdown Timer Frequency and
    // the Timer Initial Value
	am_util_stdio_printf("RTC ID: %02X\r\n", am1815_read_register(rtc, 0x28));
    uint8_t countdowntimer = am1815_read_register(rtc, 0x18);
    // clear TE first
	am1815_write_register(rtc, 0x18, countdowntimer & ~0b10000000);
    uint8_t RPT = countdowntimer & 0b00011100;
    uint8_t timerResult = 0b10100000 + RPT;
    uint32_t timerinitial = 0;
    if(finalTimer <= 0.0625){
        timerResult += 0b00;
        timerinitial = ((int)(finalTimer * 4096)) - 1;
	}
    else if(finalTimer <= 4){
        timerResult += 0b01;
        timerinitial = ((int)(finalTimer * 64)) - 1;
	}
    else if(finalTimer <= 256){
        timerResult += 0b10;
        timerinitial = ((int)timer) - 1;
	}
    else{
        timerResult += 0b11;
        timerinitial = ((int)(finalTimer * (1/60))) - 1;
	}

	am1815_write_register(rtc, 0x19, timerinitial);
	am1815_write_register(rtc, 0x1A, timerinitial);
	am1815_write_register(rtc, 0x18, timerResult);

	am_util_stdio_printf("18: %02X\r\n", am1815_read_register(rtc, 0x18));
	am_util_stdio_printf("19: %02X\r\n", am1815_read_register(rtc, 0x19));
	am_util_stdio_printf("1A: %02X\r\n", am1815_read_register(rtc, 0x1A));

	return;
}

struct am1815 rtc;
struct spi_bus spi;
struct spi_device rtc_spi;
static struct uart uart;

int main(void){
	// Prepare MCU by init-ing clock, cache, and power level operation
	am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_SYSCLK_MAX, 0);
	am_hal_cachectrl_config(&am_hal_cachectrl_defaults);
	am_hal_cachectrl_enable();
	am_bsp_low_power_init();
	am_hal_sysctrl_fpu_enable();
	am_hal_sysctrl_fpu_stacking_enable(true);

	// Init UART, registers with SDK printf
	syscalls_uart_init(&uart);
	uart_init(&uart, UART_INST0);

	am_util_stdio_printf("hello!\r\n");

	spi_bus_init(&spi, 0);
	spi_bus_enable(&spi);
	spi_bus_init_device(&spi, &rtc_spi, SPI_CS_3, 2000000u);
    am1815_init(&rtc, &rtc_spi);
    am1815_read_timer(&rtc);

	am_util_stdio_printf("Read Register: %02X\r\n", am1815_read_register(&rtc, 0x19));
	am_util_stdio_printf("RTC ID: %02X\r\n", am1815_read_register(&rtc, 0x28));

	am1815_write_timer2(&rtc, 0.4);

	am_util_stdio_printf("done!\r\n");
}
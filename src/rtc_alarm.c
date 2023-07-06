// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText 2023 Kristin Ebuengan
// SPDX-FileCopyrightText 2023 Melody Gill
// SPDX-FileCopyrightText 2023 Gabriel Marcano

/*
* Allows to change the alarm configurations on the RTC
*/

#include "am_util.h"

#include <am1815.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>

#include <uart.h>

static uint8_t from_bcd(uint8_t bcd)
{
	return (bcd & 0xF) + ((bcd >> 4) * 10);
}

/**
 * Reads the time in the RTC's alarm registers
 * Since there is no year, we set the year to 0 (i.e., 0 years since 1900)
*/
struct timeval am1815_read_alarm(struct am1815 *rtc)
{
	struct spi *spi = rtc->spi;
	uint32_t buffer[2];
	uint8_t *data = (uint8_t*)buffer;
	spi_read(spi, 0x8, buffer, 7);
	memcpy(data, buffer, 7);

	struct tm date = {
        .tm_year = 0,
		.tm_mon = from_bcd(data[5] & 0x1F) - 1,
		.tm_mday = from_bcd(data[4] & 0x3F),
		.tm_hour = from_bcd(data[3] & 0x3F),
		.tm_min = from_bcd(data[2] & 0x7F),
		.tm_sec = from_bcd(data[1] & 0x7F),
		.tm_wday = from_bcd(data[7] & 0x07),
	};

	time_t time = mktime(&date);

	struct timeval result = {
		.tv_sec = time,
		.tv_usec = from_bcd(data[0]) * 10000,
	};
	return result;
}

static struct uart uart;
static struct spi spi;

int main(void){
	spi_init(&spi, 0);
	spi_enable(&spi);

    struct am1815 rtc;
    am1815_init(&rtc, &spi);
    // initialize_time(&rtc);

    uart_init(&uart, UART_INST0);

    struct timeval toPrint = am1815_read_alarm(&rtc);
    // struct timeval toPrint = am1815_read_time(&rtc);

    while (1) {
		am_util_stdio_printf("in while loop \r\n");

		am_util_stdio_printf("seconds: %lld\r\n", toPrint.tv_sec);
   		am_util_stdio_printf("ms: %lld\r\n", toPrint.tv_usec);
		am_util_delay_ms(250);
    }
   
}
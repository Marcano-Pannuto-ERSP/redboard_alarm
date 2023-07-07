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

uint8_t to_bcd(uint8_t value)
{
	uint8_t result = 0;
	uint8_t decade = 0;
	while(value != 0){
		uint8_t digit = value % 10;
		value = value / 10;
		result |= digit << (4 * decade);
		decade += 1;
	}
	return result;
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

void am1815_write_alarm(struct am1815 *rtc, struct timeval *atime)
{
    struct tm date;
    gmtime_r(&(atime->tv_sec), &date);
    int hundredths = atime->tv_usec / 10000;

    am1815_write_register(rtc, 0x8, to_bcd((uint8_t)hundredths));
    am1815_write_register(rtc, 0x9, to_bcd((uint8_t)date.tm_sec));
    am1815_write_register(rtc, 0xA, to_bcd((uint8_t)date.tm_min));
    am1815_write_register(rtc, 0xB, to_bcd((uint8_t)date.tm_hour));
    am1815_write_register(rtc, 0xC, to_bcd((uint8_t)date.tm_mday));
    am1815_write_register(rtc, 0xD, to_bcd((uint8_t)date.tm_mon));
    am1815_write_register(rtc, 0xE, to_bcd((uint8_t)date.tm_wday));
}

static struct uart uart;
static struct spi spi;

int main(void){
	spi_init(&spi, 0, 2000000u);
	spi_enable(&spi);

    struct am1815 rtc;
    am1815_init(&rtc, &spi);

    uart_init(&uart, UART_INST0);

	// Create a timeval struct to set the RTC alarm to
	struct tm date = {
		.tm_year = 123,
		.tm_mon = 7,
		.tm_mday = 10,
		.tm_hour = 7,
		.tm_min = 30,
		.tm_sec = 45,
	};
	time_t time = mktime(&date);
	struct timeval atime = {.tv_sec = time, .tv_usec = 0};
	// Write to alarm
	am1815_write_alarm(&rtc, &atime);

	// Read what alarm says now
    struct timeval toPrint = am1815_read_alarm(&rtc);

	// Debug: write to ram registers
	am1815_write_register(&rtc, 0x40, 16);

    while (1) {
		am_util_stdio_printf("in while loop \r\n");

		am_util_stdio_printf("seconds: %lld\r\n", toPrint.tv_sec);
   		am_util_stdio_printf("ms: %lld\r\n", toPrint.tv_usec);

		// debug : read hundredths register
		uint8_t reg = am1815_read_register(&rtc, 0x1);
		am_util_stdio_printf("register 1: %d\r\n", reg);

		// read ram register
		uint8_t reg2 = am1815_read_register(&rtc, 0x40);
		am_util_stdio_printf("register 40: %d\r\n", reg2);

		// read oscillator status register
		uint8_t reg3 = am1815_read_register(&rtc, 0x1D);
		am_util_stdio_printf("register 1D: %d\r\n", reg3);

		am_util_delay_ms(250);
    }
   
}
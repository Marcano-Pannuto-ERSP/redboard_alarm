#include <am1815.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>

int am1815_read_timer(struct am1815 *rtc)
{
	uint8_t buffer;
	uint8_t *data = (uint8_t*)buffer;
	spi_device_cmd_read(rtc->spi, 0x19, buffer, 1);
	memcpy(data, buffer, 1);
	
    return (int)data;
	// struct tm date = {
    //     .tm_year = 0,
	// 	.tm_mon = from_bcd(data[5] & 0x1F) - 1,
	// 	.tm_mday = from_bcd(data[4] & 0x3F),
	// 	.tm_hour = from_bcd(data[3] & 0x3F),
	// 	.tm_min = from_bcd(data[2] & 0x7F),
	// 	.tm_sec = from_bcd(data[1] & 0x7F),
	// 	.tm_wday = from_bcd(data[7] & 0x07),
	// };

	// time_t time = mktime(&date);

	// struct timeval result = {
	// 	.tv_sec = time,
	// 	.tv_usec = from_bcd(data[0]) * 10000,
	// };
	// return result;
}

// void am1815_write_alarm(struct am1815 *rtc, struct timeval *atime)
// {
//     // struct tm date;
//     // gmtime_r(&(atime->tv_sec), &date);
//     // int hundredths = atime->tv_usec / 10000;

//     // am1815_write_register(rtc, 0x8, to_bcd((uint8_t)hundredths));
//     // am1815_write_register(rtc, 0x9, to_bcd((uint8_t)date.tm_sec));
//     // am1815_write_register(rtc, 0xA, to_bcd((uint8_t)date.tm_min));
//     // am1815_write_register(rtc, 0xB, to_bcd((uint8_t)date.tm_hour));
//     // am1815_write_register(rtc, 0xC, to_bcd((uint8_t)date.tm_mday));
//     // am1815_write_register(rtc, 0xD, to_bcd((uint8_t)date.tm_mon));
//     // am1815_write_register(rtc, 0xE, to_bcd((uint8_t)date.tm_wday));
// }

struct am1815 rtc;
struct spi_bus spi;
struct spi_device rtc_spi;

int main(void){
	spi_bus_init(&spi, 0);
	spi_bus_enable(&spi);
	spi_bus_init_device(&spi, &rtc_spi, SPI_CS_3, 2000000u);
    am1815_init(&rtc, &rtc_spi);
    am1815_read_timer(&rtc);
}
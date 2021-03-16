#ifndef RTC_H_INCLUDED
#define RTC_H_INCLUDED

#include "common.h"

#define RTC_EEP_SIZE				56		// RTC EEPROM size
#define RTC_EEP_START				0x08	// RTC EEPROM start address
#define ADVANCED_RTC_DATA_ADDRESS	0 + RTC_EEP_START

typedef struct sTimeData
{
	uint8_t hours;
	uint8_t minutes;
	uint8_t seconds;
} sTimeData_t;

typedef struct sDateData
{
	uint8_t day;
	uint8_t date;
	uint8_t month;
	uint8_t year;
} sDateData_t;

typedef struct sAdvancedRTCData 
{
	uint8_t leapYear;
	uint8_t daylightSavingTime;
}sAdvancedRTCData_t;

// Spare bytes at the end of EEPROM
#define RTC_EEP_SPARE       ( RTC_EEP_SIZE		\
							- sizeof(sAdvancedRTCData_t))

typedef struct sEeprom
{
	sAdvancedRTCData_t advancedRTCData;
	uint8_t spare[RTC_EEP_SPARE];    // Spare bytes
} sEeprom_t;

void rtcInit(void);
eStatusYesNo_t rtcLoadTime(sTimeData_t *timeData);
eStatusYesNo_t rtcSaveTime(sTimeData_t *timeData);
eStatusYesNo_t rtcLoadDate(sDateData_t *dateData);
eStatusYesNo_t rtcSaveDate(sDateData_t *dateData);
eStatusYesNo_t rtcSaveEEP(uint8_t address, void *data, int8_t size);
eStatusYesNo_t rtcLoadEEP(uint8_t address, void *data, int8_t size);

#endif // RTC_H_INCLUDED

#include <stdio.h>

#include "rtc.h"
#include "i2cmaster.h"

// RTC Addresses
#define DS1307				0xD0//0x68	// chip address
#define RTC_SEC				0x00		// seconds
#define RTC_MIN				0x01		// minutes
#define RTC_HOD				0x02		// hours
#define RTC_DAY				0x03		// day
#define RTC_DATE			0x04		// date
#define RTC_MONTH			0x05		// month
#define RTC_YEAR			0x06		// year

#define MO 1
#define TU 2
#define WE 3
#define TH 4
#define FR 5
#define SA 6
#define SU 7

#define DAYLIGHTS_BEGIN_DAY SU
#define DAYLIGHTS_BEGIN_MONTH	3
#define DAYLIGHTS_BEGIN_HOUR	2
#define DAYLIGHTS_END_DAY SU
#define DAYLIGHTS_END_MONTH		10
#define DAYLIGHTS_END_HOUR		3


eStatusYesNo_t read(uint8_t address, uint8_t *readData)
{
	uint8_t status;
	
	status = i2c_start(DS1307+I2C_WRITE);		// zapis do DS3231
	
	if (status == 0)
	{
		i2c_write(address);									// adresa bunky
		i2c_rep_start(DS1307+I2C_READ);					// cteni z DS3231
		*readData = i2c_readNak();						// ulozit prectenou hodnotu do globalni promenne
		i2c_stop();		               					// stop
	}
	else
	{
		return NO;
	}
	
	return YES;
}

eStatusYesNo_t write(uint8_t address, uint8_t dataToWrite)
{
	uint8_t status;
	
	status = i2c_start(DS1307+I2C_WRITE);
	
	if (status == 0)
	{
		i2c_write(address);	        						// 	vyber adresy( hodiny minuty atd)
		i2c_write(dataToWrite);								// 	zapise BCD na vybranou adresu
		i2c_stop(); 									//	stop
	}
	else
	{
		return NO;
	}
	
	return YES;	
}

eStatusYesNo_t rtcLoadTime(sTimeData_t *timeData)
{
	uint8_t i2cData = 0;
	eStatusYesNo_t status;
	
	// Seconds
	status = read(RTC_SEC, &i2cData);
	if (status == NO)
	{
		return NO;
	}
	timeData->seconds = ((i2cData >> 4) & 0b00000111) * 10 + (i2cData & 0x0F);
	
	// Minutes
	status = read(RTC_MIN, &i2cData);
	if (status == NO)
	{
		return NO;
	}
	timeData->minutes = ((i2cData >> 4)& 0b00000111) * 10 + (i2cData & 0x0F);
	
	// Hours
	status = read(RTC_HOD, &i2cData);
	if (status == NO)
	{
		return NO;
	}
	timeData->hours = ((i2cData >> 4)& 0b00000011) * 10 + (i2cData & 0x0F);
	
	return YES;
}

eStatusYesNo_t rtcSaveTime(sTimeData_t *timeData)
{
	uint8_t i2cData = 0;
	eStatusYesNo_t status;

	// Seconds
	i2cData = ( (timeData->seconds / 10) << 4 ) + (timeData->seconds % 10);
	status = write(RTC_SEC, i2cData);
	if (status == NO)
	{
		return NO;
	}
	
	// Minutes
	i2cData = ( (timeData->minutes / 10 ) << 4 ) + (timeData->minutes % 10);
	status = write(RTC_MIN, i2cData);
	if (status == NO)
	{
		return NO;
	}
	
	// Hours
	i2cData = ( (timeData->hours / 10) << 4 ) + (timeData->hours % 10);
	status = write(RTC_HOD, i2cData);
	if (status == NO)
	{
		return NO;
	}
		
	return YES;
}

eStatusYesNo_t rtcLoadDate(sDateData_t *dateData)
{
	uint8_t i2cData = 0;
	eStatusYesNo_t status;
	
	// Day	
	status = read(RTC_DAY, &i2cData);
	if (status == NO)
	{
		return NO;
	}	
	dateData->day = i2cData;
	
	// Date
	status = read(RTC_DATE, &i2cData);
	if (status == NO)
	{
		return NO;
	}	
	dateData->date = ( (i2cData >> 4) & 0b00000011 ) * 10 + (i2cData & 0x0F);
	
	// Month
	status = read(RTC_MONTH, &i2cData);
	if (status == NO)
	{
		return NO;
	}	
	dateData->month = ( (i2cData >> 4) & 0x00000001 ) * 10 + (i2cData & 0x0F);
	
	// Year
	status = read(RTC_YEAR, &i2cData);
	if (status == NO)
	{
		return NO;
	}	
	dateData->year = ( (i2cData >> 4) & 0x0F ) * 10 + (i2cData & 0x0F);
	
	return YES;
}

eStatusYesNo_t rtcSaveDate(sDateData_t *dateData)
{
	uint8_t i2cData = 0;
	eStatusYesNo_t status;
	
	// Day
	i2cData = dateData->day;
	status = write(RTC_DAY, i2cData);
	if (status == NO)
	{
		return NO;
	}
	
	// Date
	i2cData = ( (dateData->date / 10) << 4) + (dateData->date % 10);
	status = write(RTC_DATE, i2cData);
	if (status == NO)
	{
		return NO;
	}
		
	// Month
	i2cData = ( (dateData->month / 10) << 4 ) + (dateData->month % 10);
	status = write(RTC_MONTH, i2cData);
	if (status == NO)
	{
		return NO;
	}
		
	// Year
	i2cData = ( (dateData->year / 10) << 4 ) + (dateData->year % 10);	
	status = write(RTC_YEAR, i2cData);
	if (status == NO)
	{
		return NO;
	}	

	return YES;
}

eStatusYesNo_t rtcSaveEEP(uint8_t address, void *data, int8_t size)
{
    eStatusYesNo_t status;
	const uint8_t *p;

    p = data;
    
    while (size-- > 0) 
	{
	    status = write(address, *p);
	    if (status == NO)
	    {
		    return NO;
	    }
		
	    p++;
	    address++;
    }

	return YES;	
}

eStatusYesNo_t rtcLoadEEP(uint8_t address, void *data, int8_t size)
{
	eStatusYesNo_t status;
	uint8_t *p;

	p = data;
	
	while (size-- > 0) 
	{
		status = read(address, p);
		if (status == NO)
		{
			return NO;
		}
		p++;
		address++;
	}
	
	return YES;	
}

void rtcInit(void)
{
	i2c_init();	
}
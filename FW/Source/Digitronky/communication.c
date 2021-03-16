#include <stdio.h>
#include <string.h>

#include "common.h"
#include "communication.h"
#include "uart.h"
#include "rtc.h"

typedef enum eStatusAT
{
	AT_ERR_SYNTAX = -1,
	AT_READ = 0,
	AT_WRITE = 1
} eStatusAT_t;

typedef enum eStates
{
	AT_STATE_DATA_WAIT,
	AT_STATE_VALIDATE_CMD,
	AT_STATE_READ,
	AT_STATE_WRITE,
	AT_STATE_SAVE,
	AT_STATE_REPLY,
	AT_STATE_HELP,
	AT_STATE_ERROR
} eStates_t;

typedef enum eATcmdIDs
{
	AT_CMD_HELP_ID,
	AT_CMD_DAY_ID,
	AT_CMD_DATE_ID,
	AT_CMD_MONTH_ID,
	AT_CMD_YEAR_ID,
	AT_CMD_LEAP_ID,
	AT_CMD_DAYLIGHT_SAVING_TIME_ID,
	AT_CMD_LAST_ID
} eATcmdIDs_t;

typedef struct sATcommands
{
	eATcmdIDs_t	cmdID;
	char*	cmdString;
	uint8_t	cmdStringLen;
	char*	helpString;
	uint16_t minParameterRange;
	uint16_t maxParameterRange;
} sATcommands_t;

#define PARSE_ERR	255
#define RANGE_NA								0xFFFF
// Commands
#define AT_CMD_HELP_STRING					"HELP"
#define AT_CMD_DAY_STRING					"DAY"
#define AT_CMD_DATE_STRING					"DATE"
#define AT_CMD_MONTH_STRING					"MONTH"
#define AT_CMD_YEAR_STRING					"YEAR"
#define AT_CMD_LEAP_YEAR_STRING				"LEAP_YEAR"
#define AT_CMD_DAYLIGHT_SAVING_TIME_STRING	"DAYLIGHT_SAVING_TIME"

#define AT_HELP_STRING						"Try it again or use"
#define AT_HELP_DAY_STRING					"(starts Monday)"
#define AT_HELP_DATE_STRING					NULL
#define AT_HELP_MONTH_STRING				NULL
#define AT_HELP_YEAR_STRING					NULL
#define AT_HELP_LEAP_YEAR_STRING			"NOT SUPPORTED (False/True)"
#define AT_HELP_DAYLIGHT_SAVING_TIME_STRING	"NOT SUPPORTED (Spring/Winter)"

#define AT_CMD_PREFIX_STRING				"AT"
#define AT_CMD_DIVIDER_STRING				"$"

#define AT_OK_STRING						"OK"
#define AT_ERROR_GENERAL_STRING				"ERROR"
#define AT_CMD_AVAILABLE_STRING				"Available commands:"

// AT commands ranges
#define AT_CMD_YEAR_RANGE_OFFSET				2000

#define AT_CMD_DAY_RANGE_MIN					1
#define AT_CMD_DAY_RANGE_MAX					7
#define AT_CMD_DATE_RANGE_MIN					1
#define AT_CMD_DATE_RANGE_MAX					31
#define AT_CMD_MONTH_RANGE_MIN					1
#define AT_CMD_MONTH_RANGE_MAX					12
#define AT_CMD_YEAR_RANGE_MIN					0 + AT_CMD_YEAR_RANGE_OFFSET
#define AT_CMD_YEAR_RANGE_MAX					99 + AT_CMD_YEAR_RANGE_OFFSET
#define AT_CMD_LEAP_YEAR_RANGE_MIN				0
#define AT_CMD_LEAP_YEAR_RANGE_MAX				1
#define AT_CMD_DAYLIGHT_SAVING_TIME_RANGE_MIN	0
#define AT_CMD_DAYLIGHT_SAVING_TIME_RANGE_MAX	1

// AT command string length
#define AT_CMD_HELP_LEN						4
#define AT_CMD_DAY_LEN						3
#define AT_CMD_DATE_LEN						4
#define AT_CMD_MONTH_LEN					5
#define AT_CMD_YEAR_LEN						4
#define AT_CMD_LEAP_YEAR_LEN				9
#define AT_CMD_DAYLIGHT_SAVING_TIME_LEN		20

#define AT_CMD_PREFIX_LEN					2
#define AT_CMD_DIVIDER_LEN					1

char rx_buffer[BUFFER_SIZE];	// UARTu buffer
eStates_t state;

static const sATcommands_t ATcommands[] = {
	{AT_CMD_HELP_ID,					 AT_CMD_HELP_STRING,				 AT_CMD_HELP_LEN,				  AT_HELP_STRING,					   RANGE_NA,							  RANGE_NA},
	{AT_CMD_DAY_ID,						 AT_CMD_DAY_STRING,				   	 AT_CMD_DAY_LEN,				  AT_HELP_DAY_STRING,				   AT_CMD_DAY_RANGE_MIN,				  AT_CMD_DAY_RANGE_MAX},
	{AT_CMD_DATE_ID,					 AT_CMD_DATE_STRING,				 AT_CMD_DATE_LEN,				  AT_HELP_DATE_STRING,				   AT_CMD_DATE_RANGE_MIN,				  AT_CMD_DATE_RANGE_MAX},
	{AT_CMD_MONTH_ID,					 AT_CMD_MONTH_STRING,				 AT_CMD_MONTH_LEN,				  AT_HELP_MONTH_STRING,				   AT_CMD_MONTH_RANGE_MIN,				  AT_CMD_MONTH_RANGE_MAX},
	{AT_CMD_YEAR_ID,					 AT_CMD_YEAR_STRING,				 AT_CMD_YEAR_LEN,				  AT_HELP_YEAR_STRING,				   AT_CMD_YEAR_RANGE_MIN,				  AT_CMD_YEAR_RANGE_MAX},
	{AT_CMD_LEAP_ID,					 AT_CMD_LEAP_YEAR_STRING,			 AT_CMD_LEAP_YEAR_LEN,			  AT_HELP_LEAP_YEAR_STRING,			   AT_CMD_LEAP_YEAR_RANGE_MIN,			  AT_CMD_LEAP_YEAR_RANGE_MAX},
	{AT_CMD_DAYLIGHT_SAVING_TIME_ID,	 AT_CMD_DAYLIGHT_SAVING_TIME_STRING, AT_CMD_DAYLIGHT_SAVING_TIME_LEN, AT_HELP_DAYLIGHT_SAVING_TIME_STRING, AT_CMD_DAYLIGHT_SAVING_TIME_RANGE_MIN, AT_CMD_DAYLIGHT_SAVING_TIME_RANGE_MAX},
	{AT_CMD_LAST_ID,					 NULL,								 0,								  NULL,								   RANGE_NA,							  RANGE_NA}};
	
void println(char *s)
{
	uart_puts(s);
	uart_putc('\n');
}

void printATcmd(uint8_t cmdIndex)
{
	uart_puts(AT_CMD_DIVIDER_STRING);
	uart_puts(ATcommands[cmdIndex].cmdString);
	uart_putc(':');
}

void helpATcmd(void)
{
	uint8_t i = 0;
	char buff[16];

	println(AT_CMD_AVAILABLE_STRING);
	while (ATcommands[i].cmdString != NULL)
	{

		uart_puts(AT_CMD_PREFIX_STRING);
		uart_puts(AT_CMD_DIVIDER_STRING);
		uart_puts(ATcommands[i].cmdString);
		if ( ATcommands[i].cmdID != AT_CMD_HELP_ID )
		{
			uart_puts(" - ");
			if (ATcommands[i].helpString != NULL)
			{
				uart_puts(ATcommands[i].helpString);
			}
			
			sprintf(buff, " %d to %d", ATcommands[i].minParameterRange, ATcommands[i].maxParameterRange);	
			println(buff);
		}
		else
		{
			uart_putc('\n');
		}
		
		i++;
	}
}

eStatusAT_t validATcmdRequest(const char* string, uint8_t cmdIndex)
{
	uint8_t cmdLen = 0;
	
	cmdLen = ATcommands[cmdIndex].cmdStringLen + AT_CMD_PREFIX_LEN + AT_CMD_DIVIDER_LEN;
	
	if(string[cmdLen] == '?')
	{
		return AT_READ;
	}
	else if (string[cmdLen] == '=')
	{
		return AT_WRITE;
	}
	
	return AT_ERR_SYNTAX;
}

eStatusAT_t validATcmd(const char* string, uint8_t* cmdIndex)
{
	char cmd[BUFFER_SIZE];
	uint8_t i = 0;
	uint8_t cmdLen = 0;
	eStatusAT_t status = AT_ERR_SYNTAX;
	
	while (ATcommands[i].cmdString != NULL)
	{
		cmdLen = AT_CMD_PREFIX_LEN + AT_CMD_DIVIDER_LEN + ATcommands[i].cmdStringLen;
		strcpy (cmd, AT_CMD_PREFIX_STRING);
		strcat (cmd, AT_CMD_DIVIDER_STRING);
		strcat (cmd, ATcommands[i].cmdString);
		
		if ( strncmp(string, cmd, cmdLen) == 0 )
		{
			status = validATcmdRequest(string, i);
			if(status  != AT_ERR_SYNTAX )
			{
				*cmdIndex = i;
				return status;	
			}
		}
		
		i++;
	}

	*cmdIndex = PARSE_ERR;	
	return status;
}

eStatusYesNo_t validATcmdRange(uint8_t cmdIndex, uint16_t numberToValidate)
{
	if ( (ATcommands[cmdIndex].minParameterRange <= numberToValidate) &&
	     (ATcommands[cmdIndex].maxParameterRange >= numberToValidate) )
	{
		return YES;	  
	}
	
	return NO;
}

uint16_t power(uint8_t x, uint8_t y)
{
	uint16_t temp;
	
	if( y == 0)
	{
		return 1;
	}
	
	temp = power(x, (y / 2) );
	
	if ( (y % 2) == 0)
	{
		return temp*temp;	
	}
	else
	{
		if(y > 0)
		{
			return x * temp * temp;	
		}
		else
		{
			return (temp * temp) / x;	
		}
	}
}

eStatusYesNo_t my_atoi(const char* snum, uint8_t snumLen ,uint16_t* num)
{
	int8_t idx = 0;
	*num = 0;
	uint16_t pwr = 0;

	for(idx = snumLen - 1; idx >= 0; idx--)
	{
		// Only process numbers from 0 through 9
		if(snum[idx] >= 0x30 && snum[idx] <= 0x39)
		{
			pwr = power(10, snumLen - idx - 1);
			*num += (snum[idx] - 0x30) * pwr;
		}
		else
		{
			*num = RANGE_NA;
			return NO;
		}
	}

	return YES;
}

eStatusYesNo_t getNumFromString(const char* string, uint16_t* num)
{
	// NOTE: Call this function only if string contains char '='
	uint8_t i = 0, j = 0;
	uint8_t len = 0;
	char buff[BUFFER_SIZE];
		
	while( string[i] != '=')
	{
		i++;
	}
	
	while ( (string[j] != '\n') && (string[j] != '\r') )
	{
		j++;
	}
	
	len = j - i - 1;

	memcpy(buff, string + i + 1, len);
	
	if ( my_atoi(buff, len, num) == YES)
	{
		return YES;
	}

	return NO;
}

void commInit(void)
{
	uart_init(9600); // uart init
	memset(rx_buffer,0, BUFFER_SIZE);
	uart_flush();	 // clear buffer
	state = AT_STATE_DATA_WAIT;
}

void loadDateParameter(eATcmdIDs_t id, uint8_t* data)
{
	sDateData_t date;
	sAdvancedRTCData_t advancedRTCData;
	uint8_t size = 0;

	size = sizeof(sAdvancedRTCData_t);
	rtcLoadDate(&date);
	rtcLoadEEP(ADVANCED_RTC_DATA_ADDRESS, &advancedRTCData, size);
	
	switch(id)
	{
		case AT_CMD_HELP_ID:
			break;
		case AT_CMD_DAY_ID:
			*data = date.day;
			break;
		case AT_CMD_DATE_ID:
			*data = date.date;
			break;
		case AT_CMD_MONTH_ID:
			*data = date.month;
			break;
		case AT_CMD_YEAR_ID:
			*data = date.year;
			break;
		case AT_CMD_LEAP_ID:
			*data = advancedRTCData.leapYear;
			println("KUNDA1");
			break;
		case AT_CMD_DAYLIGHT_SAVING_TIME_ID:
			*data = advancedRTCData.daylightSavingTime;
			println("KUNDA2");
			break;
		case AT_CMD_LAST_ID:
			break;
	}
}

void saveDateParameter(eATcmdIDs_t id, uint8_t data)
{
	sDateData_t date;
	sAdvancedRTCData_t advancedRTCData;
	uint8_t size = 0;

	size = sizeof(sAdvancedRTCData_t);
	rtcLoadDate(&date);
	rtcLoadEEP(ADVANCED_RTC_DATA_ADDRESS, &advancedRTCData, size);
	
	switch(id)
	{
		case AT_CMD_HELP_ID:
			break;
		case AT_CMD_DAY_ID:
			date.day = data;
			break;
		case AT_CMD_DATE_ID:
			date.date = data;
			break;
		case AT_CMD_MONTH_ID:
			date.month = data; 
			break;
		case AT_CMD_YEAR_ID:
			date.year = data;
			break;
		case AT_CMD_LEAP_ID:
			advancedRTCData.leapYear = data;
			println("KUNDA1");
			break;
		case AT_CMD_DAYLIGHT_SAVING_TIME_ID:
			advancedRTCData.daylightSavingTime = data;
			println("KUNDA2");
			break;
		case AT_CMD_LAST_ID:
			break;
	}
	
	rtcSaveDate(&date);
	rtcSaveEEP(ADVANCED_RTC_DATA_ADDRESS, &advancedRTCData, size);
}

void commProcess(void)
{
	uint8_t cmdIndex = 0;
	uint8_t numberRTC = 0;
	uint16_t number = 0;
	uint8_t exit = 1;

	char buff[10];

	while (exit)
	{
		switch(state)
		{
			case AT_STATE_DATA_WAIT:
				if (uart_gets(rx_buffer))
				{
					state = AT_STATE_VALIDATE_CMD;
				}
				else
				{
					exit = 0;
					state = AT_STATE_DATA_WAIT;	
				}
				
				break;
		
			case AT_STATE_VALIDATE_CMD:
				switch(validATcmd(rx_buffer, &cmdIndex) )
				{
					case AT_READ:
						state = AT_STATE_READ;
						break;
					case AT_WRITE:
						state = AT_STATE_WRITE;
						break;
					case AT_ERR_SYNTAX:
						state = AT_STATE_HELP;
						break;
				}
	
				break;
		
			case AT_STATE_READ:
				loadDateParameter(ATcommands[cmdIndex].cmdID, &numberRTC);
			
				number = numberRTC;
				
				if (ATcommands[cmdIndex].cmdID == AT_CMD_YEAR_ID)
				{
					number = numberRTC + AT_CMD_YEAR_RANGE_OFFSET;
				}
			
				state = AT_STATE_REPLY;
				break;
		
			case AT_STATE_WRITE:
				if ( getNumFromString(rx_buffer, &number) == YES)
				{
					if( validATcmdRange(ATcommands[cmdIndex].cmdID, number) == YES)
					{
						// TODO for now only reply what you have read
						state = AT_STATE_SAVE;
						//state = AT_STATE_REPLY;						
					}
					else
					{
						state = AT_STATE_ERROR;
					}				
				}
				else
				{
					state = AT_STATE_ERROR;
				}
		
				break;
		
			case AT_STATE_SAVE:
				if (ATcommands[cmdIndex].cmdID == AT_CMD_YEAR_ID)
				{
					numberRTC = number - AT_CMD_YEAR_RANGE_OFFSET;
				}
				else
				{
					numberRTC = number;
				}

				saveDateParameter(ATcommands[cmdIndex].cmdID, numberRTC);
				state = AT_STATE_REPLY;
				break;
			
			case AT_STATE_HELP:
				uart_puts(AT_HELP_STRING);
				uart_putc(' ');
				uart_puts(AT_CMD_PREFIX_STRING);
				uart_puts(AT_CMD_DIVIDER_STRING);
				uart_puts(AT_CMD_HELP_STRING);
				println("?");
				
				state = AT_STATE_ERROR;
				break;
						
			case AT_STATE_REPLY:
				printATcmd(cmdIndex);
				
				if(ATcommands[cmdIndex].cmdID != AT_CMD_HELP_ID)
				{
					sprintf(buff,"%d",number);
					println(buff);
				}
				else
				{
					helpATcmd();
				}
				
				println(AT_OK_STRING);
				exit = 0;
				state = AT_STATE_DATA_WAIT;	
				uart_flush();
				break;
		
			case AT_STATE_ERROR:
				println(AT_ERROR_GENERAL_STRING);
				exit = 0;
				state = AT_STATE_DATA_WAIT;	
				uart_flush();
				break;
		}
	}
}
		
		
					
					
/*	
	int16_t data = 0, len = 0;
	uint8_t i2cADDR = 0, i2cSTATE = I2C_STOP;
	
			if( uart_gets(rx_buffer) )
			{
				// TODO read is working, write is problem
				// TODO divide it to functions and create buffer for messages
				
				if( (strcmp(rx_buffer,"AT$HELP?\n") == 0) | (strcmp(rx_buffer,"AT$HELP?\r") == 0) )
				{
					uart_puts("Available commands:\n");
					uart_puts("AT$DAY - 1 to 7 (starts Monday)\n");
					uart_puts("AT$DATE - 1 to 31\n");
					uart_puts("AT$MONTH - 1 to 12\n");
					uart_puts("AT$YEAR - 2000 to 2099\n");
					uart_puts("AT$LEAP_YEAR - 0/1 (False/True)\n");
					uart_puts("AT$DAYLIGHT_SAVING_TIME - 0/1 (Spring/Winter)\n");
					uart_puts("\nTo read the parameter use e.g. AT$DAY?\n");
					uart_puts("\nTo write the parameter use e.g. AT$DAY=7\n");
					uart_puts("OK\n");
				}
				else if(strncmp(rx_buffer,"AT$LEAP_YEAR?",13) == 0)
				{
					uart_puts("OK\n");
				}
				else if(strncmp(rx_buffer,"AT$DAYLIGHT_SAVING_TIME?",24) == 0)
				{
					uart_puts("OK\n");
				}
				else if(string_compare("AT$DAY", &len) == 0)
				{
					uart_puts("$DAY: ");
					
					if( ( rx_buffer[len] == '?') && ( (rx_buffer[len + 1] == '\n') || (rx_buffer[len + 1] == '\r') ) )
					{
						i2cADDR = RTC_DAY;
						
						i2cSTATE = I2C_READ;
					}
					else if (rx_buffer[len] == '=')
					{
						i2cSTATE = I2C_WRITE;
					}
					else
					{
						uart_puts("ERROR\n");
					}
				}
				else if(string_compare("AT$DATE", &len) == 0)
				{
					uart_puts("$DATE: ");
					
					if( ( rx_buffer[len] == '?') && ( (rx_buffer[len + 1] == '\n') || (rx_buffer[len + 1] == '\r') ) )
					{
						i2cADDR = RTC_DATE;
						
						i2cSTATE = I2C_READ;
					}
					else if (rx_buffer[len] == '=')
					{
						i2cSTATE = I2C_WRITE;
					}
					else
					{
						uart_puts("ERROR\n");
					}
				}
				else if(string_compare("AT$MONTH", &len) == 0)
				{
					uart_puts("$MONTH: ");
					
					if( ( rx_buffer[len] == '?') && ( (rx_buffer[len + 1] == '\n') || (rx_buffer[len + 1] == '\r') ) )
					{
						i2cADDR = RTC_MONTH;
						
						i2cSTATE = I2C_READ;
					}
					else if (rx_buffer[len] == '=')
					{
						i2cSTATE = I2C_WRITE;
					}
					else
					{
						uart_puts("ERROR\n");
					}
				}
				else if(string_compare("AT$YEAR", &len) == 0)
				{
					uart_puts("$YEAR: ");
					
					if( ( rx_buffer[len] == '?') && ( (rx_buffer[len + 1] == '\n') || (rx_buffer[len + 1] == '\r') ) )
					{
						i2cADDR = RTC_YEAR;
						
						i2cSTATE = I2C_READ;
					}
					else if (rx_buffer[len] == '=')
					{
						i2cSTATE = I2C_WRITE;
					}
					else
					{
						uart_puts("ERROR\n");
					}
				}
				else
				{
					uart_puts("ERROR. Try it again or use help ""AT$HELP?"".\n");
				}

				if( (i2cSTATE == I2C_READ) )
				{
					data = read_data(i2cADDR);
					if(data >= 0)
					{
						sprintf (tx_buffer, "%d\n", data);
						uart_puts(tx_buffer);
						uart_puts("Read OK\n");
					}
					else
					{
						uart_puts("I2C read ERR\n");
					}
					
					i2cSTATE = I2C_STOP;
				}
				else if( (i2cSTATE == I2C_WRITE) )
				{

					len += sscanf(rx_buffer, "%*[^0123456789]%d", &data);
					
					//TODO this is not working
					if( (rx_buffer[len - 1] == '\n') || (rx_buffer[len - 1] == '\r') )
					{
						sprintf (tx_buffer, "%d\n", data);
						uart_puts(tx_buffer);
						uart_puts("Write OK\n");
					}
					else
					{
						uart_puts("I2C write ERR\n");
					}
					
					i2cSTATE = I2C_STOP;
				}


				uart_flush();
			}	
			
*/

/*void clockCheckDaylightSaving(void)
{
	if (SR_GetBool(SR_TIME_YR,SR_TIME_YR_TIME_DAYLGHT_S) == false)
	{
		if ( (clock_now.month == DAYLIGHTS_BEGIN_MONTH) && 
			 (clock_now.dayOfWeek == DAYLIGHTS_BEGIN_DAY) && 
			 ((clock_now.day + 7) > 31) && 
			 (clock_now.hours == DAYLIGHTS_BEGIN_HOUR) && 
			 (clock_now.minutes == 0) && 
			 (clock_now.seconds == 0))
		{
			clock_now.hours += 1;
			clockSetRTC();
			SR_SetBool(SR_TIME_YR,SR_TIME_YR_TIME_DAYLGHT_S,true);
		}
		if ( ( (clock_now.month > DAYLIGHTS_BEGIN_MONTH) && (clock_now.month < DAYLIGHTS_END_MONTH)) || 
			 ( (clock_now.month == DAYLIGHTS_BEGIN_MONTH) && ((clock_now.day - clock_now.dayOfWeek + 7) > 31)) || 
			 ( (clock_now.month == DAYLIGHTS_END_MONTH) && ( ( (clock_now.day - clock_now.dayOfWeek + 7) < 31) && (clock_now.dayOfWeek != SU)) || (clock_now.day < 25)) || 
			 ( ( (clock_now.month == DAYLIGHTS_END_MONTH) && (clock_now.dayOfWeek == DAYLIGHTS_END_DAY) && (clock_now.day + 7 > 31) && (((uint32_t)clock_now.hours * 3600 + clock_now.minutes * 60 + clock_now.seconds) < 7200)) || ((clock_now.month == DAYLIGHTS_BEGIN_MONTH) && (clock_now.dayOfWeek == DAYLIGHTS_BEGIN_DAY) && (clock_now.day + 7 > 31) && (((uint32_t)clock_now.hours * 3600 + clock_now.minutes * 60 + clock_now.seconds) > 10800))))
		{
			SR_SetBool(SR_TIME_YR,SR_TIME_YR_TIME_DAYLGHT_S,true);
		}

	}
	else
	{
		if ( (clock_now.month == DAYLIGHTS_END_MONTH) && 
			 (clock_now.dayOfWeek == DAYLIGHTS_END_DAY) && 
			 (clock_now.day + 7 > 31) && 
			 (clock_now.hours == DAYLIGHTS_END_HOUR) && 
			 (clock_now.minutes == 0) && (clock_now.seconds == 0))
		{
			clock_now.hours -= 1;
			clockSetRTC();
			SR_SetBool(SR_TIME_YR,SR_TIME_YR_TIME_DAYLGHT_S,false);
		}
		else if ( ((clock_now.month < DAYLIGHTS_BEGIN_MONTH) && (clock_now.month > DAYLIGHTS_END_MONTH)) || ((clock_now.month == DAYLIGHTS_END_MONTH) && ((clock_now.day - clock_now.dayOfWeek + 7) > 31)) || ((clock_now.month == DAYLIGHTS_BEGIN_MONTH) && (((clock_now.day - clock_now.dayOfWeek + 7) < 31) && (clock_now.dayOfWeek != SU)) && (clock_now.day < 25)) || (((clock_now.month == DAYLIGHTS_BEGIN_MONTH) && (clock_now.dayOfWeek == DAYLIGHTS_BEGIN_DAY) && (clock_now.day + 7 > 31) && (((uint32_t)clock_now.hours * 3600 + clock_now.minutes * 60 + clock_now.seconds) < 7200)) || ((clock_now.month == DAYLIGHTS_END_MONTH) && (clock_now.dayOfWeek == DAYLIGHTS_END_DAY) && (clock_now.day + 7 > 31) && (((uint32_t)clock_now.hours * 3600 + clock_now.minutes * 60 + clock_now.seconds) > 10800))))
		{
			SR_SetBool(SR_TIME_YR,SR_TIME_YR_TIME_DAYLGHT_S,false);
		}
	}
}*/
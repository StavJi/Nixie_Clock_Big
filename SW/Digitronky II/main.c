#ifdef F_CPU
#undef F_CPU
#endif

#define F_CPU 8000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <avr/pgmspace.h>
#include "i2cmaster.h"

//************* KATODY ****************
#define KATODA_0_PORT		PORTB		
#define KATODA_0_DDR		DDRB
#define KATODA_0_NUMBER		0

#define KATODA_1_PORT		PORTB		
#define KATODA_1_DDR		DDRB
#define KATODA_1_NUMBER		2

#define KATODA_2_PORT		PORTB		
#define KATODA_2_DDR		DDRB
#define KATODA_2_NUMBER		3

#define KATODA_3_PORT		PORTB			
#define KATODA_3_DDR		DDRB
#define KATODA_3_NUMBER		1

#define KATODA_4_PORT		PORTD
#define KATODA_4_DDR		DDRB
#define KATODA_4_NUMBER		7

#define KATODA_5_PORT		PORTD
#define KATODA_5_DDR		DDRD
#define KATODA_5_NUMBER		5

#define KATODA_6_PORT		PORTB
#define KATODA_6_DDR		DDRB
#define KATODA_6_NUMBER		7

#define KATODA_7_PORT		PORTB
#define KATODA_7_DDR		DDRB
#define KATODA_7_NUMBER		5

#define KATODA_8_PORT		PORTB
#define KATODA_8_DDR		DDRB
#define KATODA_8_NUMBER		6

#define KATODA_9_PORT		PORTD
#define KATODA_9_DDR		DDRD
#define KATODA_9_NUMBER		6
//************* KATODY ****************

//************* ANODY ****************
#define ANODA_1_PORT		PORTC		
#define ANODA_1_DDR			DDRC
#define ANODA_1_NUMBER		0

#define ANODA_2_PORT		PORTC		
#define ANODA_2_DDR			DDRC
#define ANODA_2_NUMBER		1

#define ANODA_3_PORT		PORTC		
#define ANODA_3_DDR			DDRC
#define ANODA_3_NUMBER		2

#define ANODA_4_PORT		PORTC		
#define ANODA_4_DDR			DDRC
#define ANODA_4_NUMBER		3
//************* ANODY ****************

//************* DOUTNAVKA ****************
#define DOUTNAVKA_PORT		PORTB
#define DOUTNAVKA_DDR		DDRB
#define DOUTNAVKA_NUMBER	4
//************* DOUTNAVKA ****************

//************* TLACITKA ****************
#define TLACITKO_1_PORT		PORTD		// tlacitko 1
#define TLACITKO_1_DDR		DDRD
#define TLACITKO_1_PIN		PIND
#define TLACITKO_1_NUMBER	4

#define TLACITKO_2_PORT		PORTD		// tlacitko 2
#define TLACITKO_2_DDR		DDRD
#define TLACITKO_2_PIN		PIND
#define TLACITKO_2_NUMBER	3

#define TLACITKO_3_PORT		PORTD		// tlacitko 3
#define TLACITKO_3_DDR		DDRD
#define TLACITKO_3_PIN		PIND
#define TLACITKO_3_NUMBER	2
//************* TLACITKA ****************

//************* LEDky ****************
#define LED_1_PORT		PORTD
#define LED_1_DDR		DDRD
#define LED_1_NUMBER	1

#define LED_2_PORT		PORTD
#define LED_2_DDR		DDRD
#define LED_2_NUMBER	0
//************* LEDky ****************

// ******* NEMENIT ******* //
#define TLACITKO_1_ZMACKNUTO	(TLACITKO_1_PIN & (1 << TLACITKO_1_NUMBER) ) 					// zjisteni zda doslo ke stisku tlacitka
#define TLACITKO_2_ZMACKNUTO	(TLACITKO_2_PIN & (1 << TLACITKO_2_NUMBER) )
#define TLACITKO_3_ZMACKNUTO	(TLACITKO_3_PIN & (1 << TLACITKO_3_NUMBER) )

#define TLACITKO_MAX_TIME	15000

#define DIGITS_OFF			KATODA_0_PORT &= ~ (1 << KATODA_0_NUMBER);\
							KATODA_1_PORT &= ~ (1 << KATODA_1_NUMBER);\
							KATODA_2_PORT &= ~ (1 << KATODA_2_NUMBER);\
							KATODA_3_PORT &= ~ (1 << KATODA_3_NUMBER);\
							KATODA_4_PORT &= ~ (1 << KATODA_4_NUMBER);\
							KATODA_5_PORT &= ~ (1 << KATODA_5_NUMBER);\
							KATODA_6_PORT &= ~ (1 << KATODA_6_NUMBER);\
							KATODA_7_PORT &= ~ (1 << KATODA_7_NUMBER);\
							KATODA_8_PORT &= ~ (1 << KATODA_8_NUMBER);\
							KATODA_9_PORT &= ~ (1 << KATODA_9_NUMBER);
							
#define ANODE_OFF			ANODA_1_PORT &= ~(1 << ANODA_1_NUMBER);\
							ANODA_2_PORT &= ~(1 << ANODA_2_NUMBER);\
							ANODA_3_PORT &= ~(1 << ANODA_3_NUMBER);\
							ANODA_4_PORT &= ~(1 << ANODA_4_NUMBER);
#define ANODE_1_ON			ANODA_1_PORT |= (1 << ANODA_1_NUMBER)
#define ANODE_2_ON			ANODA_2_PORT |= (1 << ANODA_2_NUMBER)
#define ANODE_3_ON			ANODA_3_PORT |= (1 << ANODA_3_NUMBER)
#define ANODE_4_ON			ANODA_4_PORT |= (1 << ANODA_4_NUMBER)

#define LED_1_ON			LED_1_PORT |= (1 << LED_1_NUMBER);
#define LED_2_ON			LED_2_PORT |= (1 << LED_2_NUMBER);
#define DOUTNAVKA_ON		DOUTNAVKA_PORT |= (1 << DOUTNAVKA_NUMBER);

#define LED_1_OFF			LED_1_PORT &= ~ (1 << LED_1_NUMBER);
#define LED_2_OFF			LED_2_PORT &= ~ (1 << LED_2_NUMBER);
#define DOUTNAVKA_OFF		DOUTNAVKA_PORT &= ~ (1 << DOUTNAVKA_NUMBER);

#define CAS_NECINNOST		10000		// cca 10s

// Adresy RTC

#define DS1307				0xD0//0x68	// adresa obvodu
#define RTC_SEC				0x00		// adresa sekund
#define RTC_MIN				0x01		// adresa minut
#define RTC_HOD				0x02		// adresa hodin
#define RTC_DAY				0x04		// den
#define RTC_WEEK_DAY		0x03		// den v tydnu
#define RTC_MONTH			0x05		// mesic
#define RTC_YEAR			0x06		// rok

#define MO 1
#define TU 2
#define WE 3
#define TH 4
#define FR 5
#define SA 6
#define SU 7

#define DAYLIGHTS_BEGIN_DAY SU
#define DAYLIGHTS_BEGIN_MONTH 3
#define DAYLIGHTS_BEGIN_HOUR 2
#define DAYLIGHTS_END_DAY SU
#define DAYLIGHTS_END_MONTH 10
#define DAYLIGHTS_END_HOUR 3

// ******* NEMENIT ******* //

volatile unsigned char disp[6] = {10,10,10,10,10,10};	// globalni promenna pro zobrazovaci rutinu, zapsanim 10 se cislice zhasne
volatile uint8_t pos;									// index prave zobrazovane pozice
volatile uint8_t menu = 0;								// informuje o vstupu do menu 
volatile uint8_t blikej = 0b10000000;					// promenna urcujici zda se ma blikat a kterou cislici aktivni spodni polovina bajtu
														// horni polovina tecky
volatile uint16_t rok = 0;								// den													
														
														
volatile uint8_t blikej_cnt = 0;						// pocitadlo pro blikani
volatile uint8_t temp = 0;								// pomocne promenne
volatile uint16_t timer = 0;							// pocitadlo necinosti
volatile uint16_t time_tl_1 = 0;						// promenna urcujici delku zmacknuteho tlacitka
volatile uint16_t time_tl_2 = 0;						// promenna urcujici delku zmacknuteho tlacitka
volatile uint16_t time_tl_3 = 0;						// promenna urcujici delku zmacknuteho tlacitka
volatile uint8_t tlacitko_1 = 0, tlacitko_1_old = 0;	// co ma dane tlacitko vykonavat
volatile uint8_t tlacitko_2 = 0, tlacitko_2_old = 0;	// co ma dane tlacitko vykonavat
volatile uint8_t tlacitko_3 = 0, tlacitko_3_old = 0;	// co ma dane tlacitko vykonavat

volatile uint16_t sec = 0;
volatile uint8_t i2cdata = 0, i2cERR = 0;

signed char UART_test[30];								// promenna na test programu po UARTu

void init(void);
void zobraz_cislo(uint8_t cislo);
void read(unsigned char reg);
void write(unsigned char reg);
void read_time(char *dest);
void read_date(char *dest);
void clockCheckDaylightSaving(void);

int main(void)
{
	i2c_init();
	init();
	
	read_date((char*)&disp[0]);
	
	_delay_ms(10000);
	
	for(;;)
	{
		if (menu)						// nastaveni casu
		{
			blikej = 0b00001111;		// blikej hodinama i minutama
			
			while (menu)
			{
				if (tlacitko_1)			// nastaveni hodin
				{
					timer = CAS_NECINNOST;

					disp[1]++;

					if( (disp[1] > 3) && (disp[0] > 1) )
					{
						disp[1] = 0;
						disp[0]++;
					}

					if( (disp[1] > 9) && (disp[0] < 2) )
					{
						disp[1] = 0;
						disp[0]++;
					}

					if(disp[0] > 2) disp[0] = 0;		
					
					tlacitko_1 = 0;
				}
				
				if (tlacitko_3)			// nastaveni minut
				{
					timer = CAS_NECINNOST;
					
					disp[3]++;

					if (disp[3] > 9)
					{
						disp[3] = 0;
						disp[2]++;
					}

					if(disp[2] > 5) disp[2] = 0;
					
					tlacitko_3 = 0;					
				}				
			}
			
			blikej = 0b10000000;
			sec = 0;
			
			i2cdata = ((disp[0]<<4)+disp[1]);	// slozit BCD hodnotu
			write(RTC_HOD);
			i2cdata = ((disp[2]<<4)+disp[3]);	// slozit BCD hodnotu
			write(RTC_MIN);
			i2cdata = 0;
			write(RTC_SEC);			
		}
		else
		{								// normalni beh hodin
			
			read_time((char*)&disp[0]);
				
			if (i2cERR > 0)
			{
				LED_1_ON;
				blikej = 0b00001111;		// blikej hodinama i minutama
				disp[0] = 0;
				disp[1] = 0;
				disp[2] = 0;
				disp[3] = 0;
				disp[4] = 0;
				disp[5] = 0;
			}
			else
			{
				LED_1_OFF;
				blikej = 0b10000000;		
			}
		}
	}
	
	return 0;										// nikdy sem nedojne ale nejsem prase
}

void init (void)
{
	// vystupy - katody
	KATODA_0_DDR |= (1 << KATODA_0_NUMBER);
	KATODA_1_DDR |= (1 << KATODA_1_NUMBER);
	KATODA_2_DDR |= (1 << KATODA_2_NUMBER);
	KATODA_3_DDR |= (1 << KATODA_3_NUMBER);
	KATODA_4_DDR |= (1 << KATODA_4_NUMBER);
	KATODA_5_DDR |= (1 << KATODA_5_NUMBER);
	KATODA_6_DDR |= (1 << KATODA_6_NUMBER);
	KATODA_7_DDR |= (1 << KATODA_7_NUMBER);
	KATODA_8_DDR |= (1 << KATODA_8_NUMBER);
	KATODA_9_DDR |= (1 << KATODA_9_NUMBER);
	
	// vystupy anody
	ANODA_1_DDR |= (1 << ANODA_1_NUMBER);
	ANODA_2_DDR |= (1 << ANODA_2_NUMBER);
	ANODA_3_DDR |= (1 << ANODA_3_NUMBER);
	ANODA_4_DDR |= (1 << ANODA_4_NUMBER);
	
	// vstupy pro tlacitka
	TLACITKO_1_DDR &= ~(1 << TLACITKO_1_NUMBER);
	TLACITKO_2_DDR &= ~(1 << TLACITKO_2_NUMBER);
	TLACITKO_3_DDR &= ~(1 << TLACITKO_3_NUMBER);
	
	//pull up pro tlacitka
	TLACITKO_1_PORT |= (1 << TLACITKO_1_NUMBER);
	TLACITKO_2_PORT |= (1 << TLACITKO_2_NUMBER);
	TLACITKO_3_PORT |= (1 << TLACITKO_3_NUMBER);
	
	// vystup doutnavka
	DOUTNAVKA_DDR |= (1 << DOUTNAVKA_NUMBER);
	
	// zhasnuti doutnavky
	DOUTNAVKA_OFF;
	
	// vystupy LEDky
	LED_1_DDR |= (1 << LED_1_NUMBER);
	LED_2_DDR |= (1 << LED_2_NUMBER);
	
	// zhasnuti LEDek
	LED_1_OFF;
	LED_2_OFF;
	
	// timer 0
	TCCR0 |= (1 << CS02);						// preddelicka 256
	TIMSK |= (1 << TOIE0);						// povoleni casovace 2
	TCNT0 = 162;									
	
	//Timner2 - 1ms
	TCCR2 |= (1 << WGM21);						// rezim CTC
	TCCR2	|= (1 << CS22);						// predelicka 64
	OCR2 = 124;									// prednaplneni registru
	TIMSK |= (1 << OCIE2);						// Output Compare A Match Interrupt Enable*/
	
	sei();										// povol globalni preruseni
}

void zobraz_cislo(uint8_t cislo)
{
	switch (cislo)			// vybrat cislo
	{
		case 0:				
			KATODA_0_PORT |= (1 << KATODA_0_NUMBER);
			break;
		case 1:
			KATODA_1_PORT |= (1 << KATODA_1_NUMBER);
			break;
		case 2:
			KATODA_2_PORT |= (1 << KATODA_2_NUMBER);
			break;
		case 3:
			KATODA_3_PORT |= (1 << KATODA_3_NUMBER);
			break;
		case 4:
			KATODA_4_PORT |= (1 << KATODA_4_NUMBER);
			break;
		case 5:
			KATODA_5_PORT |= (1 << KATODA_5_NUMBER);
			break;
		case 6:
			KATODA_6_PORT |= (1 << KATODA_6_NUMBER);
			break;
		case 7:
			KATODA_7_PORT |= (1 << KATODA_7_NUMBER);
			break;
		case 8:
			KATODA_8_PORT |= (1 << KATODA_8_NUMBER);
			break;
		case 9:
			KATODA_9_PORT |= (1 << KATODA_9_NUMBER);
			break;
		default:
			break;				
	}// end switch
}

void read(unsigned char reg)
{
	i2cERR = i2c_start(DS1307+I2C_WRITE);		// zapis do DS3231
	
	if (i2cERR == 0)
	{
		i2c_write(reg);									// adresa bunky
		i2c_rep_start(DS1307+I2C_READ);					// cteni z DS3231
		i2cdata = i2c_readNak();						// ulozit prectenou hodnotu do globalni promenne
		i2c_stop();		               					// stop
	}
}

void write(unsigned char reg)
{
	i2cERR = i2c_start(DS1307+I2C_WRITE);
	
	if (i2cERR == 0)
	{
		i2c_write(reg);	        						// 	vyber adresy( hodiny minuty atd)
		i2c_write(i2cdata);								// 	zapise BCD na vybranou adresu
		i2c_stop(); 									//	stop
	}
}

void read_time(char *dest)							// funkce pro vycteni casu z RTC do zvoleneho pole
{
	read(RTC_SEC);
	if (i2cERR == 0)
	{
		dest[5]=(i2cdata & 0x0F);
		dest[4]=((i2cdata >>4)& 0b00000111);
		read(RTC_MIN);
		dest[3]=(i2cdata & 0x0F);
		dest[2]=((i2cdata >>4)& 0b00000111);
		read(RTC_HOD);
		dest[1]=(i2cdata & 0x0F);
		dest[0]=((i2cdata >>4)& 0b00000011);
	}
}

void read_date(char *dest)
{
	read(RTC_DAY);
	if (i2cERR == 0)
	{
//		rok = i2cdata;
		dest[3]=(i2cdata & 0x0F);
		dest[2]=((i2cdata >>4)& 0x0F);
	}
}

void clockCheckDaylightSaving(void)
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
}

ISR(TIMER2_COMP_vect)							// tlacitka
{
	tlacitko_1_old <<= 1;
	tlacitko_1_old |= (TLACITKO_1_ZMACKNUTO >> TLACITKO_1_NUMBER);
	tlacitko_1_old &= 0b11111111;
	
	tlacitko_2_old <<= 1;
	tlacitko_2_old |= (TLACITKO_2_ZMACKNUTO >> TLACITKO_2_NUMBER);
	tlacitko_2_old &= 0b11111111;
	
	tlacitko_3_old <<= 1;
	tlacitko_3_old |= (TLACITKO_3_ZMACKNUTO >> TLACITKO_3_NUMBER);
	tlacitko_3_old &= 0b11111111;

	if (i2cERR == 0)
	{
		if (tlacitko_2_old == 0b11110000)
		{
			menu ^= 1;
			timer = CAS_NECINNOST;
		}
	}
	
	if(menu)
	{
		if (tlacitko_1_old == 0b11110000)
		{
			tlacitko_1 = 1;
		}
		
		if (tlacitko_3_old == 0b11110000)
		{
			tlacitko_3 = 1;
		}
	}
	
	if(menu)
	{
		if(timer != 0)
		{
			timer--;
		}
		else
		{
			menu = 0;
		}
	}
	else
	{
		if (sec > 999)					// 1s
		{
			sec = 0;
		}
		
		sec++;
	}	
}			

ISR(TIMER0_OVF_vect)								// multiplex
{
	TCNT0 = 162;			
	
	ANODE_OFF;
	DIGITS_OFF;				// zhasnout vsechno
	LED_1_OFF;
	DOUTNAVKA_OFF;

	temp = (1 << pos);
	
	switch (pos)			// vybrat anodu, skenovani tlacitek
	{
		case 0:					// sileny if osetruje situaci, aby nesvitila nula (napr. 09:42), ale aby svitila v situaci 00:15
			if( ( (disp[0] < 10) && (disp[0] > 0) ) || ( (disp[0] < 10) && (disp[0] == 0) && (disp[1] == 0)) )			// pokud neni nic zobrazeno, nepripoji se ani anoda,
			{							// aby neprosvitala sousedni cislice
				ANODE_1_ON;
			}
			break;
		case 1:
			if (disp[1] < 10)
			{
				ANODE_2_ON;
			}
			break;
		case 2:
			if (disp[2] < 10)
			{
				ANODE_3_ON;
			}
			break;
		case 3:
			if (disp[3] < 10)
			{
				ANODE_4_ON;
			}
			break;
	}// end switch

	blikej_cnt++;
	
	if(blikej_cnt > 340) blikej_cnt = 0;			// perioda blikani cca 1 sekunda

	if( ( (blikej & temp) & 0x0f) )					// blikat zvolenou cislici
	{
		if(blikej_cnt > 50)							// sviti dele nez je zhasnuto
		{
			zobraz_cislo(disp[pos]);
		}
		else
		{
			ANODE_OFF;
		}
	}
	else
	{												// blikani cislici je vypnuto
		zobraz_cislo(disp[pos]);					// zobraz trvale
	}
	
	if( blikej & 0b10000000 )
	{
		if(sec < 499)							// sviti dele nez je zhasnuto
		{
			DOUTNAVKA_ON;
		}
	}
	else
	{												// blikani cislici je vypnuto
		DOUTNAVKA_ON;								// zobraz trvale
	}
	
	pos++;							// prepnout dalsi cislici
	if (pos > 3) pos = 0;	
}
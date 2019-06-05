/*Aufgabe 2:Wägeverfahren Suksessive Approximation
* Mit Externen DAC wird ein Digitalvoltmeter eingerichtet
*/

// Autor: Nikolas Rieder

#include <stdio.h>
#include <stdint.h>
#include "tm4c1294ncpdt.h"

void wait(unsigned long time)
{
   unsigned long j;

   for (j = 0; j < time; j++);   // wait-loop
}

void ConfigPorts(void)
{
    SYSCTL_RCGCGPIO_R |= ((1 << 3) | (1 << 10) | (1 << 11) | (1 << 9));            // clock enable port D,L,M
    while(!(SYSCTL_PRGPIO_R & ((1 << 3) | (1 << 10) | (1 << 11) | (1 << 9))));     // wait for port D,L,M clock

    GPIO_PORTD_AHB_DEN_R |= 0x03;                                                  // PD(1:0) enable
    GPIO_PORTL_DEN_R |= 0x07;                                                      // PD(2:0) enable
    GPIO_PORTM_DEN_R |= 0xFF;                                                      // PM(7:0) enable
    GPIO_PORTK_DEN_R |= 0xFF;                                                      // PM(7:0) enable

    GPIO_PORTD_AHB_DIR_R |= 0x00;                                                  // PD(1:0) define input
    GPIO_PORTM_DIR_R |= 0xFF;                                                      // PM(7:0) define output
    GPIO_PORTL_DIR_R |= 0x07;                                                      // PM(2:0) define output
    GPIO_PORTK_DIR_R |= 0xFF;                                                      // PM(7:0) define output
}

/**
 * Erfassung der Spannung
 */

unsigned char DACEXTERN(void)
{

	unsigned char VOLT = 0x80;
	unsigned char BIT = 0x80;

    while(BIT >= 0x01)                                                                            // PORTK wird zum Start auf MSB gesetzt. Bei jeden durchlauf um 1 nach Rechts geshifted
    {
        GPIO_PORTK_DATA_R |= VOLT;

        wait(300);                                                                                 // SObald am Eingang des DAC das Signal geändert wird braucht es 30 mikrosekunden bis das Signal an PD(0) stabil anliegt.

        if ((GPIO_PORTD_AHB_DATA_R & 0x1) == 0x01 )                                               // Sobald PD(1) und PD(0) auf high gesetzt sind kann der Wert auf die Variable VOLT gespeichert werden.
        {
        	BIT = BIT/2;
        	VOLT |= BIT;                                                              // Wert wird gespeichert
        }
        else if (GPIO_PORTD_AHB_DATA_R == 0x00)                                                   // Sollte PD(0) auf Low gesetzt sein wird der Wert nicht gespeichert und mit dem nächsten geshifteten werd gearbeitet
        {
        	 VOLT &=(~BIT);
         	 GPIO_PORTK_DATA_R&=(~BIT);
        	 BIT = BIT/2;
        	 VOLT |= BIT;
        	 continue;                                                                            // Zurück zur for schleife
        }
    }
    GPIO_PORTK_DATA_R &= 0x00;
    return VOLT;
}

/**
 * main.c
 */

void main(void)
{
    float voltageStep  = 0.19;
    unsigned short measuredVoltageDigit = 0;
    int measuredVoltage = 0;
    int firstDigit = 0;
    int changeDigit = 0;
    int secondDigit = 0;

    ConfigPorts();                                                                                   // Konfiguration der Ports

    while (1)
    {
    	while(!(GPIO_PORTD_AHB_DATA_R & 0x02))
    	{
    		measuredVoltageDigit = (DACEXTERN() / 8);
    		measuredVoltage = (measuredVoltageDigit * voltageStep) * 100;
    		printf("%d\n", measuredVoltage);
    	}
    }
}



// Laborversuch 3 Aufgabe 2

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
    SYSCTL_RCGCGPIO_R |= ((1 << 3) | (1 << 10) | (1 << 11) | (1 << 9));            // clock enable port D,K,L,M
    while(!(SYSCTL_PRGPIO_R & ((1 << 3) | (1 << 10) | (1 << 11) | (1 << 9))));     // wait for port D,K,L,M clock

    GPIO_PORTD_AHB_DEN_R |= 0x03;   // PD(1:0) enable
    GPIO_PORTL_DEN_R |= 0x03;       // PL(1:0) enable
    GPIO_PORTM_DEN_R |= 0xFF;       // PM(7:0) enable
    GPIO_PORTK_DEN_R |= 0xFF;       // PK(7:0) enable

    GPIO_PORTD_AHB_DIR_R |= 0x00;   // PD(1:0) define input
    GPIO_PORTM_DIR_R |= 0xFF;       // PM(7:0) define output
    GPIO_PORTL_DIR_R |= 0x03;       // PL(1:0) define output
    GPIO_PORTK_DIR_R |= 0xFF;       // PK(7:0) define output

    GPIO_PORTL_DATA_R |= 0x03;      // PL(1:0) output HIGH
}

unsigned char DACEXTERN(void)
{

	unsigned char VOLT, BIT = 0x80;

    while(BIT >= 0x01)                                  // compare Uin with Uout until bit = 0x01
    {
        GPIO_PORTK_DATA_R |= VOLT;                      // PK(7:0) with Uout
        wait(300);                                                                               

        if ((GPIO_PORTD_AHB_DATA_R & 0x1) == 0x01)      // if Uout < Uin
        {
        	BIT = BIT / 2;
        	VOLT |= BIT;
        }
        else if (GPIO_PORTD_AHB_DATA_R == 0x00)         // if Uout > Uin
        {
            VOLT  &= (~BIT);                            // delete bit in Uout
            GPIO_PORTK_DATA_R &= (~BIT);                // delete bit in SAR
            BIT = BIT/2;
            VOLT |= BIT;
            continue;
        }
    }
    GPIO_PORTK_DATA_R &= 0x00;                          // clear PK(7:0)
    return VOLT;                                        // returns voltage result
}

void main(void)
{
    float voltageStep = 0.19;
    unsigned short measuredVoltageDigit = 0;
    int measuredVoltage = 0;
    int firstDigit, secondDigit, thirdDigit, forthDigit = 0;

    ConfigPorts();

    while (1)
    {
    	while(!(GPIO_PORTD_AHB_DATA_R & 0x02))                              // measure voltage unless STOP-button is pressed
    	{
    		measuredVoltageDigit = (DACEXTERN() / 8);
    		measuredVoltage = (measuredVoltageDigit * voltageStep) * 100;   // convert voltage in mV
    		printf("%d\n", measuredVoltage);

            firstDigit = measuredVoltage / 1000;
            secondDigit = (measuredVoltage / 100) - (firstDigit * 10);
            thirdDigit = (measuredVoltage / 10) - (secondDigit * 10) - (firstDigit * 100);
            forthDigit = measuredVoltage - (thirdDigit * 10) - (secondDigit * 100) - (firstDigit * 1000);
    	}
        GPIO_PORTL_DATA_R = (1 << 0);
        GPIO_PORTM_DATA_R |= (forthDigit | (thirdDigit << 4));  // display third and forth digit
        GPIO_PORTL_DATA_R = (1 << 1);
        GPIO_PORTM_DATA_R |= (secondDigit | (firstDigit << 4)); // display first and second digit
    }
}



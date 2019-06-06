// Laborversuch 3 Aufgabe 3

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
   SYSCTL_RCGCGPIO_R |= ((1 << 4) | (1 << 11) | (1 << 3));            // clock enable port D,E,M
   while(!(SYSCTL_PRGPIO_R & ((1 << 4) | (1 << 11) | (1 << 3))));     // wait for port D,E,M clock
   SYSCTL_RCGCADC_R |= (1 << 0);							                   // ADC0 active
   while(!(SYSCTL_PRADC_R & (1 << 0)));					                // wait for ADC0 clock

   GPIO_PORTE_AHB_DEN_R &= ~(1 << 0);						// GPIO-Function deactivate for PE(0)
   GPIO_PORTE_AHB_AFSEL_R |= (1 << 0);						// PE(0) for alternative function
   GPIO_PORTE_AHB_AMSEL_R |= (1 << 0);						// connect ADC0 with PE(0)
   GPIO_PORTM_DEN_R |= (1 << 1);			               // GPIO-Function activate for PM(1)
   GPIO_PORTM_DEN_R &= ~(1 << 0);                     // GPIO-Function deactivate for PM(0)
   GPIO_PORTM_DIR_R |= 0x00;						         // PM(1) Input-signal
   GPIO_PORTM_AFSEL_R |= (1 << 0);                    // PM(0) for alternate function
   GPIO_PORTM_PCTL_R = 0x03;                          // connect TIMER2A with PM(0)
}

void ConfigSampleSequencer(void)
{
	ADC0_ACTSS_R &= ~0x0F;				   // disable all 4 sequencers of ADC0

	SYSCTL_PLLFREQ0_R |= (1 << 23);     //**********************************
	while(!(SYSCTL_PLLSTAT_R & 0x01));  //    Magic Code
	ADC0_CC_R |= 0x01;                  //
	SYSCTL_PLLFREQ0_R &= ~(1 << 23);    //**********************************
   
	ADC0_SSMUX0_R |= 0x00000003;   		// sequencer 0, channel AIN3
	ADC0_SSCTL0_R |= 0x00000002;		   // define sequence length
	ADC0_ACTSS_R |= 0x01;				   // enable sequencer 0 ADC 0
}

void ConfigTimer(void)
{
   SYSCTL_RCGCTIMER_R |= (1 << 2);                    // clock enable TIMER 2
   while(!(SYSCTL_PRTIMER_R & (1 << 2)));             // wait for TIMER2 clock
   TIMER2_CTL_R &= ~0x01;                             // disable TIMER2
   TIMER2_CFG_R = 0x04;                               // 2 x 16-bit mode

   TIMER2_TAMR_R |= ((1 << 3) | 0x02);                // PWM - mode, periodic
   TIMER2_CTL_R |= (1 << 6);                          // inverting
   TIMER2_TAILR_R = 48000 - 1;                        // interval-load-value
   TIMER2_TAMATCHR_R = 32000 - 1;                     // match-value
   TIMER2_CTL_R |= 0x01;                              // TIMER2A enable
}

unsigned long adcIntern(void)
{
	unsigned long data;

	ADC0_PSSI_R |= 0x01;					         // start converting ADC0
	while(ADC0_SSFSTAT0_R & (1 << 8));		   // wait for FIFO "FULL"
	data = (unsigned long) ADC0_SSFIFO0_R;    // read FIFO-Data
	return data * (5000.0/4096) + 0.5;        // return Voltage in mV
}

void main(void)
{
   ConfigPorts();                                                                            
   ConfigSampleSequencer();
   ConfigTimer();

   unsigned long adcValue = 0;
   int analogStickStatus = 0;

   while(1)
   {
      adcValue = adcIntern();                                                             // voltage-value

      if(((adcValue < 1900) || (adcValue > 2100)) && !(GPIO_PORTM_DATA_R & 0x02))         // analog-stick to left or right without sw
      {
         analogStickStatus = 0;
         if((adcValue < 1900) && (analogStickStatus == 0))                                // analog-stick to left
            TIMER2_TAMATCHR_R -= 500;
         else if((adcValue > 2100) && (analogStickStatus == 0))                           // analog-stick to right
            TIMER2_TAMATCHR_R += 500;
      }
      else if(((adcValue >= 1900) && (adcValue <= 2100)) && (GPIO_PORTM_DATA_R & 0x02))   // sw without moving analog-stick
      {
         analogStickStatus = 1;
         if((GPIO_PORTM_DATA_R & 0x02) && (analogStickStatus == 1))                       // sw pressed
            TIMER2_TAMATCHR_R = 48000 - 1;
         else if(!(GPIO_PORTM_DATA_R & 0x02) && (analogStickStatus))                      // sw not pressed
            TIMER2_TAMATCHR_R = 0;
      }
      else
         continue;                 

      printf("%d\n", adcValue);     // voltage-value on console
   }
}

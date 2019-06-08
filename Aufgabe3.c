// Laborversuch 3 Aufgabe 3

#include <stdio.h>
#include <stdint.h>
#include "tm4c1294ncpdt.h"

void configPorts(void)
{
   	SYSCTL_RCGCGPIO_R |= ((1 << 4) | (1 << 11) | (1 << 3));           	// clock enable port D,E,M
   	while(!(SYSCTL_PRGPIO_R & ((1 << 4) | (1 << 11) | (1 << 3))));    	// wait for port D,E,M clock
   	SYSCTL_RCGCADC_R |= (1 << 0);				     					// ADC0 active
   	while(!(SYSCTL_PRADC_R & (1 << 0)));			 					// wait for ADC0 clock

   	GPIO_PORTD_AHB_DEN_R |= 0x01;										// PD(0) enable
   	GPIO_PORTE_AHB_DEN_R &= ~(1 << 0);									// GPIO-Function deactivate for PE(0)
   	GPIO_PORTE_AHB_AFSEL_R |= (1 << 0);									// PE(0) for alternative function
   	GPIO_PORTE_AHB_AMSEL_R |= (1 << 0);									// connect ADC0 with PE(0)

   	GPIO_PORTM_DEN_R |= ((1 << 1) | (1 << 0));							// GPIO-Function enable for PM(1:0)
   	GPIO_PORTM_AFSEL_R |= (1 << 0);                   					// PM(0) for alternate function
   	GPIO_PORTM_PCTL_R = 0x03;                          					// connect TIMER2A with PM(0)
   	GPIO_PORTD_AHB_DIR_R = 0x00;										// PD(0) for sw-input
}

void configSampleSequencer(void)
{
	ADC0_ACTSS_R &= ~0x0F;				                				// disable all 4 sequencers of ADC0

	// magic code
	SYSCTL_PLLFREQ0_R |= (1 << 23);                    					// PLL Power
	while(!(SYSCTL_PLLSTAT_R & 0x01));                 					// until PLL has locked
	ADC0_CC_R |= 0x01;                                 					// PIOSC for analog block
	SYSCTL_PLLFREQ0_R &= ~(1 << 23);                   					// PLL Power off

	// ADC0_SS0 configuration
	ADC0_SSMUX0_R |= 0x00000003;   		               					// sequencer 0, channel AIN3
	ADC0_SSCTL0_R |= 0x00000002;		                  				// define sequence length
	ADC0_ACTSS_R |= 0x01;				                				// enable sequencer 0 ADC 0
}

void configTimer(void)
{
   SYSCTL_RCGCTIMER_R |= (1 << 2);                    					// clock enable TIMER2
   while(!(SYSCTL_PRTIMER_R & (1 << 2)));             					// wait for TIMER2 clock
   TIMER2_CTL_R &= ~0x01;                             					// disable TIMER2
   TIMER2_CFG_R = 0x04;                               					// 2 x 16-bit mode

   TIMER2_TAMR_R |= ((1 << 3) | 0x02);                					// PWM-mode, periodic
   TIMER2_CTL_R |= (1 << 6);                          					// inverting
   TIMER2_TAILR_R = 16000 - 1;                        					// ILV for 1ms (1kHz discrete frequency)
   TIMER2_TAMATCHR_R = 8000 - 1;										// MATCH-value for 0.5ms
   TIMER2_CTL_R |= 0x01;												// TIMER2A enable
}

unsigned long adcIntern(void)
{
	unsigned long data;

	ADC0_PSSI_R |= 0x01;					        					// start converting ADC0
	while(ADC0_SSFSTAT0_R & (1 << 8));		            				// wait for FIFO "FULL"
	data = (unsigned long) ADC0_SSFIFO0_R;             					// read FIFO-Data
	return data * (5000.0 / 4096) + 0.5;                 				// return Voltage in mV
}

void main(void)
{
   configPorts();
   configSampleSequencer();
   configTimer();

   unsigned long adcValue = 0;
   unsigned char analogSwitch, analogSwitchOld = 0;

   while(1)
   {
	adcValue = adcIntern();                										// voltage-value in mV

	analogSwitch = GPIO_PORTD_AHB_DATA_R;										// save new input of SW PD(0)

	if((adcValue <= 2000) && (adcValue >= 1800))								// defined analog-value-area without moving analog-stick
	{
		if((0 == analogSwitchOld) && (1 == analogSwitch))						// if analog-stick pressed (positive-edge)
			TIMER2_TAMATCHR_R = (16000 - 1) * 0.95;								// MATCH-Value = 95% of ILV
		else if((1 == analogSwitchOld) && (0 == analogSwitch))					// if analog-stick released (negative-edge)
			TIMER2_TAMATCHR_R = (16000 - 1) * 0.05;								// MATCH-Value = 5% of ILV
	}
	else if((adcValue < 1800) || (adcValue > 2000))								// defined analog-value-area with moving analog-stick
	{
		if((adcValue < 1800) && (TIMER2_TAMATCHR_R >= ((16000 - 1) * 0.05)))	// if analog-stick moves to left (-> 5% of ILV)
			TIMER2_TAMATCHR_R -= 100;											// reduce MATCH-value of TIMER2A
		else if((adcValue > 2000) && (TIMER2_TAMATCHR_R <= ((16000 - 1) * 0.95)))  	// if analog-stick moves to right (-> 95% of ILV)
			TIMER2_TAMATCHR_R += 100;											// increase MATCH-value of TIMER2A
	}

	analogSwitchOld = analogSwitch;												// save old input of SW PD(0)
	printf("%d\n", adcValue);     												// voltage-value on console
   }
}

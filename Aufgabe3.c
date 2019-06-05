/*Aufgabe 2:Wägeverfahren Suksessive Approximation
* Mit Externen DAC wird ein Digitalvoltmeter eingerichtet
*/

// Autor: Nikolas Rieder

#include <stdio.h>
#include <stdint.h>
#include "tm4c1294ncpdt.h"
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>

#define MIN_TIME 48000*0.05
#define MAX_TIME 48000*0.90

void wait(unsigned long time)
{
   unsigned long j;

   for (j = 0; j < time; j++);   // wait-loop
}

void ConfigPorts(void)
{
    SYSCTL_RCGCGPIO_R |= ((1 << 4) | (1 << 11));            // clock enable port D,L,M
    while(!(SYSCTL_PRGPIO_R & ((1 << 4) | (1 << 11))));     // wait for port E,M clock
    SYSCTL_RCGCADC_R |= (1 << 0);							// ADC0 active
    while(!(SYSCTL_PRADC_R & (1 << 0)));					// wait for ADC0 clock

    GPIO_PORTE_AHB_DEN_R &= ~(1 << 0);						// GPIO-Function deactivate for PE(0)
    GPIO_PORTE_AHB_AFSEL_R |= (1 << 0);						// PE(0) for alternative function
    GPIO_PORTE_AHB_AMSEL_R |= (1 << 0);						// connect ADC0 with PE(0)
    GPIO_PORTM_DEN_R |= ((1 << 0) | (1 << 1));				// GPIO-Function activate for PM(1:0)
    GPIO_PORTM_DIR_R |= (1 << 0);							// PM(0) output-signal
}

void ConfigSampleSequencer(void)
{
	ADC0_ACTSS_R &= ~0x0F;				// disable all 4 sequencers of ADC0
	SYSCTL_PLLFREQ0_R |= (1 << 23);
	while(!(SYSCTL_PLLSTAT_R & 0x01));
	ADC0_CC_R |= 0x01;
	SYSCTL_PLLFREQ0_R &= ~(1 << 23);
	ADC0_SSMUX0_R |= 0x00000003;   		// sequencer 0, channel AIN3
	ADC0_SSCTL0_R |= 0x00000002;		// define sequence length
	ADC0_ACTSS_R |= 0x01;				// enable sequencer 0 ADC 0
}

/**
 * Erfassung der Spannung
 */

unsigned long adcIntern(void)
{
	unsigned long data;

	ADC0_PSSI_R |= 0x01;					// start converting ADC0
	while(ADC0_SSFSTAT0_R & (1 << 8));		// wait for FIFO "FULL"
	data = (unsigned long) ADC0_SSFIFO0_R;
	return data*(5000.0/4096) + 0.5;
}

/**
 * main.c
 */

void main(void)
{
    ConfigPorts();                                                                              // Konfiguration der Ports
    ConfigSampleSequencer();																	// Konfiguration des Sample Sequenzers
    unsigned long adcValue = 0;

    while (1)
    {
    	adcValue = adcIntern();
		printf("%d\n", adcValue);
    }
}

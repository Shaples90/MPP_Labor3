
// Laborversuch 3 Aufgabe 2

#include "tm4c1294ncpdt.h"
#include "stdio.h"
#include "stdint.h"

void wait(unsigned long timevalue)
{
	int tmp = 0;
	for(tmp=0; tmp < timevalue; tmp++);
}

void main(void)
{
	SYSCTL_RCGCGPIO_R |= ((1 << 3) | (1 << 9) | (1 << 10) | (1 << 11));
	while(!(SYSCTL_PRGPIO_R & ((1 << 3) | (1 << 9) | (1 << 10) | (1 << 11))));
	GPIO_PORTD_AHB_DEN_R |= 0x3;
	GPIO_PORTK_DEN_R |= 0xFF;
	GPIO_PORTL_DEN_R |= 0x07;
	GPIO_PORTM_DEN_R |= 0xFF;

	GPIO_PORTD_AHB_DIR_R |= 0x00;
	GPIO_PORTK_DIR_R |= 0xFF;
	GPIO_PORTL_DIR_R |= 0x07;
	GPIO_PORTM_DIR_R |= 0xFF;

	GPIO_PORTK_DATA_R &= ~0xFF;

	while(1)
	{
		unsigned short bit = 0x80;
		unsigned static short Kout = 0;

		while(bit >= 0x01)
		{
			GPIO_PORTK_DATA_R |= bit;
			wait(300);
			if((GPIO_PORTD_AHB_DATA_R & 0x01))
			{
				Kout &= ~bit;
				bit = bit / 2;
				GPIO_PORTK_DATA_R &= ~0xFF;
				wait(300);
			}
			else
				Kout |= bit;
				bit = bit / 2;
				GPIO_PORTK_DATA_R &= ~0xFF;
				wait(300);
				continue;
		}
		float VoltageStep = 0.019;
	    float Ergebnis    = 0.0;
		Ergebnis = Kout * VoltageStep;
		printf("%f\n", Ergebnis);

		/*GPIO_PORTK_DATA_R |= 0xFF;
		wait(500000);
		GPIO_PORTK_DATA_R &= ~0xFF;
		wait(500000);*/
	}
}



/**
 * main.c
 */
#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"

void INIT_TIMER1_REGISTERS(void);
void INIT_SYS_CTRL_REGISTERS(void);

int main(void)
{
    INIT_SYS_CTRL_REGISTERS(); // init system control registers
    INIT_TIMER1_REGISTERS();
	return 0;
}

void INIT_TIMER1_REGISTERS(){
    TIMER1_CTL_R = 0x00; // make sure TIMER1 is disabled before configuring
    TIMER1_CFG_R = 0x04; // configures the timer in 16-bit mode
    TIMER1_TBMR_R = 0x0A; // configure the timer in periodic timer mode, with PWM mode enabled
    TIMER1_TBILR_R= 160; // the reload value to achieve 10us (assuming 16MHz clock)
    TIMER1_TBMATCHR_R = duty; // the compare value for the timer
    TIMER1_CTL_R = 0x0100; // enable Timer B (the timer which we're using)
}

void INIT_SYSTICK(){ // initializing systick
    NVIC_ST_RELOAD_R = 16000*500; // 500 ms
    NVIC_ST_CURRENT_R = 0x00;
    NVIC_ST_CTRL_R = 0x00000007;
}

void SystickInterrupt(){
    SW_State = (GPIO_PORTF_DATA_R & 0x01); // read the state of switch
    if (SW_State == 0x00){ // if it is still pressed, brighten
        duty -= 8;
        if (duty <= 0){ // saturating
            duty = 0;
        }
    }

    else{ // if it is released, stop systick- no need to brighten anymore
        NVIC_ST_CURRENT_R = 0x00;
        NVIC_ST_CTRL_R = 0x00000000;
    }
}

void INIT_SYS_CTRL_REGISTERS(){
    SYSCTL_RCGC2_R |= 0x00000020;       /* enable clock to GPIOF */
    SYSCTL_RCGCTIMER_R = 0x02; // enable clock to General Purpose Timer 1 Module
    SYSCTL_RCGCGPIO_R = 0x20; // enable clock to PORTF GPIO
}



/**
 * main.c
 */
#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"

void INIT_TIMER1_REGISTERS(int);
void INIT_SYS_CTRL_REGISTERS(void);
void PWM_INTERRUPT_HANDLER(void);
void INIT_SYSTICK(void);
void SYSTICK_ISR();
int ring_index = 0;
uint8_t bytestream[48];

int main(void)
{
    INIT_SYS_CTRL_REGISTERS(); // init system control registers
    INIT_SYSTICK();
    INIT_GPIO_PORTF_REGISTERS();
//    int count = 0;
//    int i;
//
//    int index = 15;
//    const int bitstream_length = 384;
    int i;
    const int bytestream_length = 48;

    int byte_index ;
    int bit_index;
    uint8_t bitsel = 0x80;
    for(byte_index = 0; byte_index < bytestream_length; byte_index++){
        bytestream[byte_index] = 0x00;
    }
//    bytestream[7] = 0x5F;
//    bytestream[27] = 0x0F;
//    bytestream[2] = 0x77;
    bytestream[0] = 0xFF;

    while(1){

        for(byte_index = 0; byte_index < bytestream_length; byte_index++){
//            bitsel = 0x01;
            for(bit_index = 0; bit_index < 8; bit_index++){
                if( (bytestream[byte_index] << bit_index) & bitsel ){
                    INIT_TIMER1_REGISTERS(8);
                }

                else{
                    INIT_TIMER1_REGISTERS(12);
                }
            }
        }

        for(i = 0; i < 1000; i++){;} // delay
    }

	return 0;
}

void INIT_TIMER1_REGISTERS(int duty){
    TIMER1_CTL_R = 0x00; // make sure TIMER1 is disabled before configuring
    NVIC_EN0_R |= 0x00400000; // enabling NVIC for TIMER1B
    TIMER1_CFG_R = 0x04; // configures the timer in 16-bit mode
    TIMER1_TBMR_R = 0x209; // configure the timer in periodic timer mode, with PWM mode enabled
    TIMER1_TBILR_R= 20; // the reload value to achieve 1.25us (assuming 16MHz clock)
    TIMER1_TBMATCHR_R = duty; // the compare value for the timer
    TIMER1_IMR_R = 0x800;
    TIMER1_CTL_R = 0x0D00; // enable Timer B (the timer which we're using)
}

void INIT_GPIO_PORTF_REGISTERS(){
    GPIO_PORTF_LOCK_R = 0x4C4F434B;     /* unlock commit register */
    GPIO_PORTF_CR_R = 0x1F;             /* make PORTF configurable */
    GPIO_PORTF_DEN_R = 0x0F;            /* set PORTF pins 4 : 0 pins */
    GPIO_PORTF_DIR_R = 0x0E;            /*  */
    GPIO_PORTF_PUR_R = 0x0E;            /* PORTF0 and PORTF4 are pulled up */
    GPIO_PORTF_AFSEL_R = 0x08; // Select PORTF3 (Green LED) for Alternate Function:
                               // Green LED not driven by GPIO_PORTF_DATA_R but instead by T1CCP1 (PWM Signal from GPT1)
    GPIO_PORTF_PCTL_R = 0x00007000; // connects the PWM output of GPTM1 to PORTF3

    NVIC_EN0_R |= 0x40000000; // 30th bit controls PORTF
    GPIO_PORTF_IS_R = 0x00; // interrupt sensitivity - edge
    GPIO_PORTF_IEV_R = 0x00; // GPIO Interrupt triggered at negative edge from Pulled-Up Switch
    GPIO_PORTF_IM_R = 0x01; // unmasking one switch (SW2)
}

void INIT_SYS_CTRL_REGISTERS(){
    SYSCTL_RCGC2_R |= 0x00000020;       /* enable clock to GPIOF */
    SYSCTL_RCGCTIMER_R = 0x02; // enable clock to General Purpose Timer 1 Module
    SYSCTL_RCGCGPIO_R = 0x20; // enable clock to PORTF GPIO
}

void PWM_INTERRUPT_HANDLER(){
    TIMER1_CTL_R = 0x00;
    TIMER1_ICR_R = 0x800;
    GPIO_PORTF_DATA_R |= 0x02;
}

void INIT_SYSTICK(){
//    NVIC_ST_RELOAD_R = 16000*10; // 10 ms
    NVIC_ST_RELOAD_R = 235294; // actual value : 235294.1176
    NVIC_ST_CURRENT_R = 0x00;
    NVIC_ST_CTRL_R = 0x00000007;
}

void SYSTICK_ISR(){
    bytestream[ring_index] = bytestream[ring_index] -1;
    bytestream[(ring_index + 3)%48] = bytestream[(ring_index + 3)%48] + 1;

//    if (ring_index > 48){
//        ring_index = 0;
//    }
//    if (bytestream[ring_index] < 1){
//        bytestream[ring_index] = 1;
//
//    }
    if (bytestream[(ring_index + 3)%48] > 254){
//        bytestream[(ring_index + 3)%48] = 254;
        ring_index = (ring_index + 3)%48;
    }
}

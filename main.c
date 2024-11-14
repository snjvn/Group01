

/**
 * main.c
 */
#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"

#define CLOCK_RUN 0
#define CLOCK_SET_SEC 1
#define CLOCK_SET_MIN 2
#define CLOCK_SET_HR 3
#define CLOCK_PAUSE 4


void INIT_TIMER1_REGISTERS(int);
void INIT_SYS_CTRL_REGISTERS(void);
void PWM_INTERRUPT_HANDLER(void);
void INIT_SYSTICK(void);
void SYSTICK_ISR();
void GPIO_ISR();
int green_ring_index = 0;
int red_ring_index = 1;
int blue_ring_index = 2;

int state = CLOCK_RUN;

float minute_factor = 1.0/60.0;
float minute_increment_detector = 0.0;
float hour_factor = 1.0/720.0;
float hour_increment_detector = 0.0;
uint8_t bytestream[48];

int update = 0;
int hours = 0;
int mins = 0;
int Systick_Ticks = 0;
int main(void)
{
    INIT_SYS_CTRL_REGISTERS(); // init system control registers
    INIT_SYSTICK();
    INIT_GPIO_PORTF_REGISTERS();

    int i;
    const int bytestream_length = 48;

    int byte_index ;
    int bit_index;
    uint8_t bitsel = 0x80;
    for(byte_index = 0; byte_index < bytestream_length; byte_index++){
        bytestream[byte_index] = 0x00;
    }

    bytestream[green_ring_index] = 0xFF;
    bytestream[red_ring_index] = 0xFF;
    bytestream[blue_ring_index] = 0xFF;
    while(1){
        if (update == 1){
            for(byte_index = 0; byte_index < bytestream_length; byte_index++){
                for(bit_index = 0; bit_index < 8; bit_index++){
                    if( (bytestream[byte_index] << bit_index) & bitsel ){
                        INIT_TIMER1_REGISTERS(8);
                    }

                    else{
                        INIT_TIMER1_REGISTERS(12);
                    }
                }
            }
            update = 0;
        }
        if ( state != CLOCK_RUN){
            for(i = 0; i < 1000; i++){;} // delay
        }
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
    GPIO_PORTF_PUR_R = 0x1F;            /* PORTF0 and PORTF4 are pulled up */
    GPIO_PORTF_AFSEL_R = 0x08; // Select PORTF3 (Green LED) for Alternate Function:
                               // Green LED not driven by GPIO_PORTF_DATA_R but instead by T1CCP1 (PWM Signal from GPT1)
    GPIO_PORTF_PCTL_R = 0x00007000; // connects the PWM output of GPTM1 to PORTF3

    NVIC_EN0_R |= 0x40000000; // 30th bit controls PORTF
    GPIO_PORTF_IS_R = 0x10; // interrupt sensitivity - level, edge
    GPIO_PORTF_IEV_R = 0x10; // GPIO Interrupt triggered at negative edge from Pulled-Up Switch
    GPIO_PORTF_IM_R = 0x11; // unmasking both switches
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
    NVIC_ST_RELOAD_R = 234375; // actual value : 234375
    NVIC_ST_CURRENT_R = 0x00;
    NVIC_ST_CTRL_R = 0x00000007;
}

void GPIO_ISR(){
//
//    if ( (state == CLOCK_RUN) && !(GPIO_PORTF_DATA_R & 0x01) ){
//        GPIO_PORTF_DATA_R ^= 0x02;
//        state = CLOCK_SET;
//        GPIO_PORTF_ICR_R = 0x11;
//
//    }
//    else if ((state == CLOCK_SET) && !(GPIO_PORTF_DATA_R & 0x01)){
////        state = CLOCK_RUN;
//        GPIO_PORTF_DATA_R ^= 0x04;
//        GPIO_PORTF_ICR_R = 0x11;
//    }

    switch (state){
    case CLOCK_RUN:
        if (GPIO_PORTF_DATA_R & 0x01){
            GPIO_PORTF_DATA_R ^= 0x02;
            state = CLOCK_SET_HR;
            GPIO_PORTF_ICR_R = 0x01;
            NVIC_ST_RELOAD_R = 250;
        }
        break;
    case CLOCK_SET_HR:
        if (GPIO_PORTF_DATA_R & 0x01){
//            GPIO_PORTF_DATA_R ^= 0x04;
            state = CLOCK_SET_MIN;
            GPIO_PORTF_ICR_R = 0x01;
            NVIC_ST_RELOAD_R = 1000;
        }
        break;


    case CLOCK_SET_MIN:
        if (GPIO_PORTF_DATA_R & 0x01){
//            GPIO_PORTF_DATA_R ^= 0x02;
            state = CLOCK_SET_SEC;
            GPIO_PORTF_ICR_R = 0x01;
            NVIC_ST_RELOAD_R = 2500;
        }
        break;

    case CLOCK_SET_SEC:
        if (GPIO_PORTF_DATA_R & 0x01){
            GPIO_PORTF_DATA_R ^= 0x04;
            state = CLOCK_PAUSE;
            GPIO_PORTF_ICR_R = 0x01;
        }
        break;
    case CLOCK_PAUSE:
        if (GPIO_PORTF_DATA_R & 0x01){
            GPIO_PORTF_DATA_R ^= 0x04;
            state = CLOCK_RUN;
            GPIO_PORTF_ICR_R = 0x01;
            NVIC_ST_RELOAD_R = 234375;
        }
        break;
    }
}

void SYSTICK_ISR(){
    if (state != CLOCK_PAUSE){
        minute_increment_detector = minute_increment_detector + minute_factor;
        hour_increment_detector = hour_increment_detector + hour_factor;
        bytestream[green_ring_index] = bytestream[green_ring_index] -1;
        bytestream[(green_ring_index + 3)%48] = bytestream[(green_ring_index + 3)%48] + 1;

        if (minute_increment_detector >= 1.0){
            bytestream[red_ring_index] = bytestream[red_ring_index] -1;
            bytestream[(red_ring_index + 3)%48] = bytestream[(red_ring_index + 3)%48] + 1;
            minute_increment_detector = 0.0;
        }
        if (hour_increment_detector >= 1.0){
            bytestream[blue_ring_index] = bytestream[blue_ring_index] -1;
            bytestream[(blue_ring_index + 3)%48] = bytestream[(blue_ring_index + 3)%48] + 1;
            hour_increment_detector = 0.0;
        }

        if (bytestream[(green_ring_index + 3)%48] > 254){
            green_ring_index = (green_ring_index + 3)%48;
        }
        if (bytestream[(red_ring_index + 3)%48] > 254){
            red_ring_index = (red_ring_index + 3)%48;
        }
        if (bytestream[(blue_ring_index + 3)%48] > 254){
            blue_ring_index = (blue_ring_index + 3)%48;
        }
        update = 1;
        Systick_Ticks = Systick_Ticks + 1;
        if (Systick_Ticks == 4096){
            mins = mins + 1;
            Systick_Ticks = 0;
        }
        if (mins == 60){
            hrs += 1;
            mins = 0;
        }
    }

//    else if (state == CLOCK_SET_SEC){
//        if (!(GPIO_PORTF_DATA_R & 0x10)){
//            minute_increment_detector = minute_increment_detector + minute_factor;
//            if (minute_increment_detector >= 1.0){
//                bytestream[green_ring_index] = bytestream[green_ring_index] -1;
//                bytestream[(green_ring_index + 3)%48] = bytestream[(green_ring_index + 3)%48] + 1;
//                minute_increment_detector = 0.0;
//            }
//
//            if (bytestream[(green_ring_index + 3)%48] > 254){
//                green_ring_index = (green_ring_index + 3)%48;
//            }
//        }
//    }
//
//    else if (state == CLOCK_SET_MIN){
//        if (!(GPIO_PORTF_DATA_R & 0x10)){
//            minute_increment_detector = minute_increment_detector + minute_factor;
//            if (minute_increment_detector >= 1.0){
//                bytestream[red_ring_index] = bytestream[red_ring_index] -1;
//                bytestream[(red_ring_index + 3)%48] = bytestream[(red_ring_index + 3)%48] + 1;
//                minute_increment_detector = 0.0;
//            }
//
//            if (bytestream[(red_ring_index + 3)%48] > 254){
//                red_ring_index = (red_ring_index + 3)%48;
//            }
//        }
//    }
//
//    else if (state == CLOCK_SET_HR){
//        if (!(GPIO_PORTF_DATA_R & 0x10)){
//            minute_increment_detector = minute_increment_detector + minute_factor;
//            if (minute_increment_detector >= 1.0){
//                bytestream[blue_ring_index] = bytestream[blue_ring_index] -1;
//                bytestream[(blue_ring_index + 3)%48] = bytestream[(blue_ring_index + 3)%48] + 1;
//                minute_increment_detector = 0.0;
//            }
//
//            if (bytestream[(blue_ring_index + 3)%48] > 254){
//                blue_ring_index = (blue_ring_index + 3)%48;
//            }
//        }
//    }
}

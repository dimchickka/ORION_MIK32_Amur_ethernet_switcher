#include "yagpio.h"
#include "gpio.h"
#include "mik32_memory_map.h"
#include "types.h"
#include "led.h"

void gpio_init(void){
    GPIO_0->DIRECTION_IN = BIT(5);
    GPIO_0->DIRECTION_OUT = BIT(10);
    GPIO_1->DIRECTION_OUT = BIT(13) - 1; //Т.е. все 12 портов на выход
    GPIO_2->DIRECTION_OUT = (BIT(8) - 1) & ~BIT(0); // на выход с 7-го по 1-й включительно
}

void gpio_errorIndicator(retv resOfOperation){
    if(resOfOperation != retv::Ok){
        if(!(GPIO_0->STATE & BIT(SUCCESS_DIOD_PIN))) GPIO_0->SET = BIT(SUCCESS_DIOD_PIN);
        if(GPIO_0->STATE & BIT(ERROR_DIOD_PIN)) GPIO_0->SET = BIT(ERROR_DIOD_PIN);
    }
    else{
        if(!(GPIO_0->STATE & BIT(SUCCESS_DIOD_PIN))) GPIO_0->SET = BIT(SUCCESS_DIOD_PIN);
    }
}
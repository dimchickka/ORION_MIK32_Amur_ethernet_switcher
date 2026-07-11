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
        // Если есть ошибка, то просто зажжём светодиод
        GPIO_0->CLEAR = BIT(SUCCESS_DIOD_PIN);  // включить (активный 0) 
    }
    else{
        //Если нет ошибок, то помигаем 5 раз
        //Dыключить ERROR? включить SUCCESS
        GPIO_0->SET = BIT(ERROR_DIOD_PIN); 
        
        for(int i = 0; i < 5; i++){  // выключить
            GPIO_0->CLEAR = BIT(SUCCESS_DIOD_PIN);  // включить (активный 0) 
            for(volatile int i = 0; i < 1000000; i++);
            GPIO_0->SET   = BIT(SUCCESS_DIOD_PIN);
            for(volatile int i = 0; i < 1000000; i++);
        }
    }
}


retv makeConnectionsBetweenRelay(char* out_buf, uint16_t* out_size){
    if(out_buf[0] != ':') return retv::Fail;
    return retv::Ok;
}
#include "main.h"
#include "config.h"
#include "yagpio.h"
#include "W5100.h"
#include <stdint.h> 

volatile bool itFromW5200 = false;

int main(){
    #pragma region // ==== Init ====
    //Если инициализация успешно завершена, то светодиод помигает MISTAKE_BLINK раз, если ошибка - светодиод просто загорится
    retv result = MCU_Init();
    gpio_errorIndicator(result);
    #pragma endregion

     while(1) {      
        uint8_t s0_ir = INITIAL_VALUE_OF_INTERRUPT_REGISTER;
        w5100_read(S0_IR, &s0_ir);
        if(s0_ir != INITIAL_VALUE_OF_INTERRUPT_REGISTER){
            w5100_itHandler();
        }
    }
}
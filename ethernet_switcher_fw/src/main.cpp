#include "main.h"
#include "config.h"
#include "yagpio.h"

int main(){
    #pragma region // ==== Init ====
    //for(volatile int i = 0; i < 3000000; i++);
    gpio_errorIndicator(MCU_Init());
    #pragma endregion

//приходит прерывание от модуля WS100
//Обрабатываем, что пришло

//Выполняем команды, которы чип нам сказал делать

    while(1) {
            __asm volatile("wfi");
        }
   // __asm volatile("wfi"); // засыпаем до следующего прерывания
}
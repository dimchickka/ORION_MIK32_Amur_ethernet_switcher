#include "main.h"
#include "config.h"

int main(){
    #pragma region // ==== Init ====
    if(!MCU_Init()){

    }
    #pragma endregion

//приходит прерывание от модуля WS100
//Обрабатываем, что пришло

//Выполняем команды, которы чип нам сказал делать


    __asm volatile("wfi"); // засыпаем до следующего прерывания
}
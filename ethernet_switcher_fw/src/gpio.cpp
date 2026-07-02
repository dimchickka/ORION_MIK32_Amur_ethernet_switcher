#include "gpio.h"
#include "gpio.h"
#include "mik32_memory_map.h"

void gpio_init(void){
    GPIO_0->DIRECTION_IN = BIT(5);
    GPIO_1->DIRECTION_OUT = BIT(13) - 1; //Т.е. все 12 портов на выход
    GPIO_2->DIRECTION_OUT = (BIT(8) - 1) & ~BIT(0); // на выход с 7-го по 1-й включительно
}

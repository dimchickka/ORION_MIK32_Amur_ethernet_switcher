#include "mik32_it.h"
#include "gpio_irq.h"
#include "epic.h"
#include "mik32_memory_map.h"

void mik32_it_init(void){

    //На линию прерываний 0 - подключаем 5-й порт
    GPIO_IRQ->LINE_MUX = GPIO_IRQ_LINE_MUX(5, 0);

    //Реагирование на прерывания по фронту
    GPIO_IRQ->EDGE = BIT(0); //записываем единицу в первый бит

    //Спадающий фронт
    GPIO_IRQ->LEVEL = BIT(0);

    //Прерывание не будет формироваться по любому каналу
    GPIO_IRQ->LEVEL = BIT(0);

    //Разрешаем прерывание от настроенного канала
    GPIO_IRQ->ENABLE_SET = BIT(0);

}

//epic - общий контроллер прерываний
void epic_init(void) {
    // Разрешаем линию GPIO_IRQ в EPIC
    // Из epic.h: EPIC_LINE_GPIO_IRQ_S = 5
    EPIC->MASK_EDGE_SET = EPIC_LINE_M(EPIC_LINE_GPIO_IRQ_S);
}


extern "C" void trap_handler(void) {
    // 1. Проверяем источник прерывания в EPIC
    if (EPIC->STATUS & EPIC_LINE_M(EPIC_LINE_GPIO_IRQ_S)) {
        
        // 2. Проверяем линию GPIO_IRQ
        if (GPIO_IRQ->INTERRUPT & GPIO_IRQ_LINE_M(0)) {
            
            // 3. Читаем данные из W5100 по SPI
            // 4. Парсим команду (номер реле + состояние)
            // 5. Дёргаем нужный GPIO → MOSFET → реле
            
            // Сбрасываем флаги
            GPIO_IRQ->CLEAR = GPIO_IRQ_LINE_M(0);
        }
        EPIC->CLEAR = EPIC_LINE_M(EPIC_LINE_GPIO_IRQ_S);
    }
}

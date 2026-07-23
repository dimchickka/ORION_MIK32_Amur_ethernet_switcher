#include "mik32_it.h"
#include "gpio_irq.h"
#include "epic.h"
#include "mik32_memory_map.h"

extern volatile bool itFromW5200;

#define GPIO0_5_IRQ_LINE  5 //If physical interrupts are used

void mik32_it_init(void){

    //На линию прерываний 0 - подключаем 5-й порт
    GPIO_IRQ->LINE_MUX = GPIO_IRQ_LINE_MUX(0, GPIO0_5_IRQ_LINE);

    //Реагирование на прерывания по фронту
    GPIO_IRQ->EDGE = BIT(GPIO0_5_IRQ_LINE); //записываем единицу в первый бит

    //Спадающий фронт
    GPIO_IRQ->LEVEL = 0;
    GPIO_IRQ->LEVEL_SET = 0;
    GPIO_IRQ->LEVEL_CLEAR = BIT(GPIO0_5_IRQ_LINE);

    //Прерывание не будет формироваться по любому каналу
    GPIO_IRQ->ANY_EDGE_CLEAR = BIT(GPIO0_5_IRQ_LINE);
    GPIO_IRQ->CLEAR = BIT(GPIO0_5_IRQ_LINE);          // сброс флага
    //Разрешаем прерывание от настроенного канала
    GPIO_IRQ->ENABLE_SET = BIT(GPIO0_5_IRQ_LINE);

}

//epic - общий контроллер прерываний
void epic_init(void) {
    // Разрешаем линию GPIO_IRQ в EPIC
    // Из epic.h: EPIC_LINE_GPIO_IRQ_S = 5
    EPIC->MASK_EDGE_SET = EPIC_LINE_M(5);
}

volatile uint32_t debug_mcause = 0;
volatile uint32_t debug_mepc = 0;
volatile uint32_t debug_epic_status = 0;
extern "C" void trap_handler(void) {
    // Читаем системные регистры ядра RISC-V перед вашими проверками
    __asm volatile("csrr %0, mcause" : "=r"(debug_mcause));
    __asm volatile("csrr %0, mepc"   : "=r"(debug_mepc));
    debug_epic_status = EPIC->STATUS;
    // 1. Проверяем источник прерывания в EPIC
    if (EPIC->STATUS & EPIC_LINE_M(EPIC_LINE_GPIO_IRQ_S)) {
        
        // 2. Проверяем линию GPIO_IRQ
        if (GPIO_IRQ->INTERRUPT & GPIO_IRQ_LINE_M(GPIO0_5_IRQ_LINE)) {
            itFromW5200 = true;
            // 3. Читаем данные из W5100 по SPI
            // 4. Парсим команду (номер реле + состояние)
            // 5. Дёргаем нужный GPIO → MOSFET → реле
            
            // Сбрасываем флаги
            GPIO_IRQ->CLEAR = GPIO_IRQ_LINE_M(GPIO0_5_IRQ_LINE);
        }
    }
    volatile uint32_t debug_interrupt, debug_edge, debug_level, debug_enable;
// в trap_handler:
debug_interrupt = GPIO_IRQ->INTERRUPT;
debug_edge = GPIO_IRQ->EDGE;
debug_level = GPIO_IRQ->LEVEL;
debug_enable = GPIO_IRQ->ENABLE_SET;
    EPIC->CLEAR = EPIC_LINE_M(EPIC_LINE_GPIO_IRQ_S);
}




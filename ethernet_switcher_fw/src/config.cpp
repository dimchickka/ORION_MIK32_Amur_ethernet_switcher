#include "config.h"
#include "mik32_memory_map.h"
#include "power_manager.h"
#include "pad_config.h"
#include "spi_functions.h"
#include "W5100.h"
#include "mik32_it.h"
#include "gpio.h"
#include "yagpio.h"
#include "types.h"
#include "timer32.h"
#include "wakeup.h"
#include "timers.h"

static void clock_init(void);
static void pad_init(void);
//void timer32_1_init(void);


retv MCU_Init(void){
    clock_init();
    pad_init();
    gpio_init();
    spi_init();
    //timer32_1_init();
    timer32_2_init();
    CHECK(w5100_init());

   // mik32_it_init();
    //epic_init();

    // глобальное разрешение прерываний
    //Выполняем ассемблерную инструкцию, чтобы выставить 3-й бит в регистре, который находится внутри ядра (у него нет адреса в карте памяти) 
    //__asm volatile("csrsi mstatus, 0x8");
    // РАЗРЕШЕНИЕ ВНЕШНИХ ПРЕРЫВАНИЙ ОТ EPIC в mie (включаем MEIE, бит 11 -> 0x800)
    // Заставляем компилятор выделить под маску регистр ("r") и применить инструкцию csrs
   // uint32_t mie_mask = 0x800;
    //__asm volatile("csrs mie, %0" : : "r"(mie_mask));

    return retv::Ok;
}

static void clock_init(void){
    // 1. Включаем внешний кварцевый генератор OSC32M (32 МГц)
    // Инвертированная логика: 0 = включено
    WU->CLOCKS_SYS &= ~(WU_CLOCKS_SYS_OSC32M_EN_M);

    //включаем внутренний генератор HSI32M
    //WU->CLOCKS_SYS &= ~(WU_CLOCKS_SYS_HSI32M_EN_M);


    // 2. Ждём стабилизации кварца (большой запас по времени)
    for (volatile uint32_t i = 0; i < 200000; i++) {
        __asm volatile("nop");
    }

    // 3. Делители шин AHB/APB_M/APB_P в 0 — деление на 1,
    //    частота шин = частоте источника (32 МГц)
    PM->DIV_AHB   = 0;
    PM->DIV_APB_P = 0;
    PM->DIV_APB_M = 0;

    // 4. Выбираем OSC32M (внешний кварц) как источник тактирования AHB
    PM->AHB_CLK_MUX = PM_AHB_CLK_MUX_OSC32M_M;
    //Внтуренний генератор
    //PM->AHB_CLK_MUX = PM_AHB_CLK_MUX_HSI32M_M; 
    // 5. Обязательный NOP после переключения источника тактирования
    __asm volatile("nop");

    // Включаем тактирование периферии: SPI0, GPIO0/1/2, прерывания GPIO, Timer32_2
    PM->CLK_APB_P_SET = PM_CLOCK_APB_P_SPI_0_M
                       | PM_CLOCK_APB_P_GPIO_0_M
                       | PM_CLOCK_APB_P_GPIO_1_M
                       | PM_CLOCK_APB_P_GPIO_2_M
                       | PM_CLOCK_APB_P_GPIO_IRQ_M
                       | PM_CLOCK_APB_P_TIMER32_2_M;

    // Включаем тактирование EPIC, контроллера выводов (PAD_CONFIG) и самого PM
    PM->CLK_APB_M_SET = PM_CLOCK_APB_M_EPIC_M
                       | PM_CLOCK_APB_M_PAD_CONFIG_M
                       | PM_CLOCK_APB_M_PM_M;
}     

static void pad_init(void){
    // ==== SPI ====
    //У порта может быть 4 состояния: 0, 1, 2, 3.
    //Для хранения нужно 2 бита 
    //Нам нужно выставить все порты в состояние 1, чтобы они работали, как SPI0 (стр.300 datasheet)
    PAD_CONFIG->PORT_0_CFG = PAD_CONFIG_PIN(0, 1)
                            |PAD_CONFIG_PIN(1, 1)
                            |PAD_CONFIG_PIN(2, 1)
                            |PAD_CONFIG_PIN(3, 1) //Этот порт нам вообще не нужен
                            |PAD_CONFIG_PIN(4, 1)
                            |PAD_CONFIG_PIN(5, 0)   //INT (настраиваем 1 вход для INT от W5100)
                            |PAD_CONFIG_PIN(9, 0)   //Светодиод для индикации ошибки
                            |PAD_CONFIG_PIN(10, 0) //Отладочный светодиод
                            |PAD_CONFIG_PIN(11, 1) //Порты используются для отладки и прошивки (JTAG порты)
                            |PAD_CONFIG_PIN(12, 1)
                            |PAD_CONFIG_PIN(13, 1)
                            |PAD_CONFIG_PIN(14, 1)
                            |PAD_CONFIG_PIN(15, 1);

PAD_CONFIG->PORT_0_PUPD |= (0b01 << (3 * 2)); // pull-up на пине 3
PAD_CONFIG->PORT_0_PUPD |= (0b01 << (5 * 2)); // pull-up на пине 5 (задаем режим 0b01)

    // ==== GPIO (настраиваем 20 выходов для 20 полевых транзисторов) ====
    PAD_CONFIG->PORT_1_CFG = PAD_CONFIG_PIN(12, 0)
                            |PAD_CONFIG_PIN(11, 0)
                            |PAD_CONFIG_PIN(10, 0)
                            |PAD_CONFIG_PIN(9, 0)
                            |PAD_CONFIG_PIN(8, 0)
                            |PAD_CONFIG_PIN(7, 0)
                            |PAD_CONFIG_PIN(6, 0)
                            |PAD_CONFIG_PIN(5, 0)
                            |PAD_CONFIG_PIN(4, 0) //GPIO1 настраиваем 9 портов на выход
                            |PAD_CONFIG_PIN(3, 2) //Далее пины настраиваются как каналы таймера
                            |PAD_CONFIG_PIN(2, 2) //Timer32_2
                            |PAD_CONFIG_PIN(1, 2)
                            |PAD_CONFIG_PIN(0, 2); 

    PAD_CONFIG->PORT_2_CFG = PAD_CONFIG_PIN(7, 0)
                            |PAD_CONFIG_PIN(6, 0)
                            |PAD_CONFIG_PIN(5, 0)
                            |PAD_CONFIG_PIN(4, 0)
                            |PAD_CONFIG_PIN(3, 0)
                            |PAD_CONFIG_PIN(2, 0)
                            |PAD_CONFIG_PIN(1, 0)
                            |PAD_CONFIG_PIN(0, 0);


}


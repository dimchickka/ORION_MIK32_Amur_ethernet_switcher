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

static void clock_init(void);
static void pad_init(void);
void timer32_1_init(void);


retv MCU_Init(void){
    clock_init();
    pad_init();
    gpio_init();
    spi_init();
    timer32_1_init();
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
    // 1. Включаем внутренний высокоскоростной осциллятор HSI32M
    WU->CLOCKS_SYS &= ~(WU_CLOCKS_SYS_OSC32M_EN_M);

    // 2. Ждем стабилизации (большой запас)
    for (volatile uint32_t i = 0; i < 200000; i++) {
        __asm volatile("nop");
    }

    // 3. Устанавливаем делители в 0 (деление на 1, частота = частоте источника)
    PM->DIV_AHB = 0;
    PM->DIV_APB_P = 0;
    PM->DIV_APB_M = 0;

    // 4. ПРИНУДИТЕЛЬНОЕ ПЕРЕКЛЮЧЕНИЕ НА HSI32M (значение 0)
    // Очищаем регистр и записываем 0
    PM->AHB_CLK_MUX = 0; 
    
    // 5. Обязательный NOP после переключения
    __asm volatile("nop");

    




    //Включаем тактирование для SPI0, GPIO0. GPIO1, GPIO2 и для прерываний
    PM->CLK_APB_P_SET = PM_CLOCK_APB_P_SPI_0_M
                        |PM_CLOCK_APB_P_GPIO_0_M
                        |PM_CLOCK_APB_P_GPIO_1_M
                        |PM_CLOCK_APB_P_GPIO_2_M
                        |PM_CLOCK_APB_P_GPIO_IRQ_M
                        |PM_CLOCK_APB_P_TIMER32_1_M; //Включаем тактирование таймера

    PM->CLK_APB_M_SET = PM_CLOCK_APB_M_EPIC_M
                        |PM_CLOCK_APB_M_PAD_CONFIG_M
                        |PM_CLOCK_APB_M_PM_M;

    // ==== DEBUG: проверяем реальное состояние ====
    volatile uint32_t dbg_clocks_sys = WU->CLOCKS_SYS;    // какие генераторы включены
    volatile uint32_t dbg_ahb_mux    = PM->AHB_CLK_MUX;   // какой источник выбран
    volatile uint32_t dbg_freq_stat  = PM->FREQ_STATUS;    // какие генераторы реально работают
    volatile uint32_t dbg_div_ahb    = PM->DIV_AHB;        // делитель AHB
    volatile uint32_t dbg_div_apb_p  = PM->DIV_APB_P;      // делитель APB_P
    __asm volatile("nop");  // ← breakpoint сюда

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
                            |PAD_CONFIG_PIN(3, 0) //Далее пины настраиваются как каналы таймера
                            |PAD_CONFIG_PIN(2, 0) //Timer32_2
                            |PAD_CONFIG_PIN(1, 0)
                            |PAD_CONFIG_PIN(0, 0); 

    PAD_CONFIG->PORT_2_CFG = PAD_CONFIG_PIN(7, 0)
                            |PAD_CONFIG_PIN(6, 0)
                            |PAD_CONFIG_PIN(5, 0)
                            |PAD_CONFIG_PIN(4, 0)
                            |PAD_CONFIG_PIN(3, 0)
                            |PAD_CONFIG_PIN(2, 0)
                            |PAD_CONFIG_PIN(1, 0)
                            |PAD_CONFIG_PIN(0, 0);


}

void timer32_1_init(void){
    // Выбираем источник тактирования — системная частота (SYS_CLK, 32 МГц)
    PM->TIMER_CFG = (PM->TIMER_CFG & ~PM_TIMER_CFG_MUX_TIMER_M(PM_TIMER_CFG_MUX_TIMER32_1_S))
                   | PM_TIMER_CFG_MUX_TIMER32_TIM1_SYS_CLK_M(PM_TIMER_CFG_MUX_TIMER32_1_S);

    TIMER32_1->PRESCALER = 31
                      | TIMER32_PRESCALER_ENABLE_M; 

    TIMER32_1->CONTROL   = TIMER32_CONTROL_CLOCK_PRESCALER_M
                          | TIMER32_CONTROL_MODE_UP_M;
    TIMER32_1->TOP       = 0xFFFFFFFF;
    TIMER32_1->ENABLE    = TIMER32_ENABLE_TIM_EN_M;


}

void delay_us(uint32_t us){
    uint32_t start = TIMER32_1->VALUE;
    volatile uint32_t currentValueToCheck = 0;
    while((TIMER32_1->VALUE - start) < us){
        currentValueToCheck = TIMER32_1->VALUE;
        uint8_t toDoSmth = 5;
        toDoSmth ++;
    }
}

void delay_ms(uint32_t ms){
    delay_us(ms * 1000);
}
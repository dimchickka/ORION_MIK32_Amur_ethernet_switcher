#include "timers.h"
#include "mik32_memory_map.h"
#include "timer32.h"
#include "power_manager.h"

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

void timer32_2_init(void) {
    PM->TIMER_CFG = (PM->TIMER_CFG & ~PM_TIMER_CFG_MUX_TIMER_M(PM_TIMER_CFG_MUX_TIMER32_2_S))
                   | PM_TIMER_CFG_MUX_TIMER32_TIM1_SYS_CLK_M(PM_TIMER_CFG_MUX_TIMER32_2_S);

    TIMER32_2->ENABLE    = 0;
    TIMER32_2->INT_MASK  = 0;
    TIMER32_2->INT_CLEAR = 0xFFFFFFFF;
    TIMER32_2->PRESCALER = 31;
    TIMER32_2->CONTROL   = TIMER32_CONTROL_MODE_UP_M;
    TIMER32_2->TOP       = RELAY_PERIOD_TICKS;

    for (int ch = 0; ch < CHANNELS_GENERAL_NUMBER; ch++) {
        TIMER32_2->CHANNELS[ch].OCR   = RELAY_PULSE_TICKS;
        TIMER32_2->CHANNELS[ch].CNTRL = TIMER32_CH_CNTRL_MODE_PWM_M | TIMER32_CH_CNTRL_INVERTED_PWM_M; // канал выключен
    }
    TIMER32_2->ENABLE = TIMER32_ENABLE_TIM_EN_M; //Включаем таймер
}

void delay_us(uint32_t us){
    uint32_t start = TIMER32_1->VALUE;
    volatile uint32_t currentValueToCheck = 0;
    while((TIMER32_1->VALUE - start) < us){
        currentValueToCheck = TIMER32_1->VALUE;
    }
}

void delay_ms(uint32_t ms){
    delay_us(ms * 1000);
}

void relay_pulse(uint8_t channel) {
    //Очищаем таймер
    TIMER32_2->ENABLE |= TIMER32_ENABLE_TIM_CLR_M;

    //Включаем канал
    TIMER32_2->CHANNELS[channel].CNTRL |= TIMER32_CH_CNTRL_ENABLE_M;
    // Ждём окончания импульса
    while (TIMER32_2->VALUE < RELAY_PULSE_TICKS);

    // Выключаем канал
    TIMER32_2->CHANNELS[channel].CNTRL &= ~TIMER32_CH_CNTRL_ENABLE_M;
}
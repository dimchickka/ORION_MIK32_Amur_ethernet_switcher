#include "main.h"
#include "config.h"
#include "yagpio.h"
#include <stdint.h> 
#include "gpio.h"
#include "mik32_memory_map.h"
#include "epic.h"
#include "gpio_irq.h"
#include "W5100.h"
#include "pad_config.h"
#include "spi_functions.h"

volatile bool itFromW5200 = false;
static uint8_t variable = 0;

int main(){
    #pragma region // ==== Init ====
    //Если инициализация успешно завершена, то светодиод P0.10 помигает 5 раз, если ошибка - светодиод просто загорится
    retv result = MCU_Init();
    gpio_errorIndicator(result);
#pragma endregion


    volatile uint8_t debug_s0_sr_live = 0xFF;
    volatile uint16_t debug_rx_size = 0;    
     while(1) {
        w5100_read(S0_SR, (uint8_t*)&debug_s0_sr_live);
    
        uint8_t rsr_hi = 0, rsr_lo = 0;
        w5100_read(S0_RX_RSR0, &rsr_hi);
        w5100_read(S0_RX_RSR1, &rsr_lo);
        debug_rx_size = ((uint16_t)rsr_hi << 8) | rsr_lo;
        
        uint8_t s0_ir = 0;
        w5100_read(S0_IR, &s0_ir);
        if(s0_ir != 0x00){
            w5100_itHandler();
    }
    }

        __asm volatile("wfi"); // засыпаем до следующего прерывания
}
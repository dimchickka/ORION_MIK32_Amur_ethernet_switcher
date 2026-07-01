#include "config.h"
#include "mik32_memory_map.h"
#include "power_manager.h"

static void clock_init(void);
static void gpio_init(void);
static void spi_init(void);


retv MCU_Init(void){
    clock_init();
}

static void clock_init(void){
    //Включаем тактирование для SPI0, GPIO0 и для прерываний
    PM->CLK_APB_P_SET = PM_CLOCK_APB_P_SPI_0_M
                        |PM_CLOCK_APB_P_GPIO_0_M
                        |PM_CLOCK_APB_P_GPIO_IRQ_M
                        |PM_CLOCK_APB_P_GPIO_1_M
                        |PM_CLOCK_APB_P_GPIO_2_M;

}

static void gpio_init(void){

}

static void spi_init(void){

}
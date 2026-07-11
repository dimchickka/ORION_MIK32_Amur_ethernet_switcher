#include "spi_functions.h"
#include "spi.h"
#include "mik32_memory_map.h"
#include "config.h"

void spi_0_cs_low(void);
void spi_0_cs_high(void);

void spi_init(void){
    // 1. Выключаем SPI
    SPI_0->ENABLE &= ~SPI_ENABLE_M;
    
    // 2. Чистим буферы
    SPI_0->ENABLE |= SPI_ENABLE_CLEAR_RX_FIFO_M;
    SPI_0->ENABLE |= SPI_ENABLE_CLEAR_TX_FIFO_M;
    
    // 3. Очищаем флаги ошибок чтением INT_STATUS
    volatile uint32_t unused = SPI_0->INT_STATUS;
    (void)unused;
    
    // 4. Настраиваем CONFIG
    SPI_0->CONFIG = SPI_CONFIG_MASTER_M
                  | SPI_CONFIG_BAUD_RATE_DIV_4_M
                  | SPI_CONFIG_CS_0_M
                  | SPI_CONFIG_MANUAL_CS_M;
    
    // 5. Задержка BTWN = 1 (как в HAL)
    SPI_0->DELAY = SPI_DELAY_BTWN(1);
    
    // 6. Включаем SPI
    SPI_0->ENABLE |= SPI_ENABLE_M;
}

void spi_0_cs_low(void){
    SPI_0->CONFIG = (SPI_0->CONFIG & ~SPI_CONFIG_CS_M) | SPI_CONFIG_CS_0_M;
}

void spi_0_cs_high(void){
    SPI_0->CONFIG = (SPI_0->CONFIG & ~SPI_CONFIG_CS_M) | SPI_CONFIG_CS_NONE_M;
}

retv spi_0_transfer(uint8_t tx_data, uint8_t* rx_data){
    SPI_0->TXDATA = tx_data;

    uint32_t timeout = TIMEOUT_SPI_ITERATIONS;
    while(!(SPI_0->INT_STATUS & SPI_INT_STATUS_RX_FIFO_NOT_EMPTY_M)){ //флаг сбрасывается при чтении
        if(--timeout == 0) return retv::Timeout;
    }

    if(rx_data != nullptr){
        *rx_data = (uint8_t)SPI_0->RXDATA;
    }
    else{
        (void)SPI_0->RXDATA;
    }

    return retv::Ok;
}

#include "spi_functions.h"
#include "spi.h"
#include "mik32_memory_map.h"
#include "config.h"

void spi_0_cs_low(void);
void spi_0_cs_high(void);
retv spi_0_transfer(uint8_t tx_data, uint8_t* rx_data);

void spi_init(void){
    //Master, 8МГц, CS = 0, ручное управление CS
    SPI_0->CONFIG = SPI_CONFIG_MASTER_M
                    |SPI_CONFIG_BAUD_RATE_DIV_4_M // 32/4 = 8МГц
                    |SPI_CONFIG_CS_0_M  //Записываем в биты 13-10 в регистр CONFIG значение 1110 (означает, что мы будем работать с CS_0 (с каналом 0))
                    |SPI_CONFIG_MANUAL_CS_M;

    SPI_0->ENABLE = SPI_ENABLE_CLEAR_RX_FIFO_M
                    |SPI_ENABLE_CLEAR_TX_FIFO_M;

    SPI_0->ENABLE = SPI_ENABLE_M;

}

void spi_0_cs_low(void){
    SPI_0->CONFIG &= ~SPI_CONFIG_CS_NONE_M;
}

void spi_0_cs_high(void){
    SPI_0->CONFIG |= SPI_CONFIG_CS_NONE_M;
}

retv spi_0_transfer(uint8_t tx_data, uint8_t* rx_data){
    SPI_0->TXDATA = tx_data;

    uint32_t timeout = TIMEOUT_SPI_ITERATIONS;
    while(!(SPI_0->INT_STATUS & SPI_INT_STATUS_RX_FIFO_NOT_EMPTY_M)){ //флаг сбрасывается при чтении
        timeout--;
    }

    if(!(SPI_0->INT_STATUS & SPI_INT_STATUS_RX_FIFO_NOT_EMPTY_M)) return retv::Timeout;

    *rx_data = (uint8_t)SPI_0->RXDATA;
    return retv::Timeout;
}

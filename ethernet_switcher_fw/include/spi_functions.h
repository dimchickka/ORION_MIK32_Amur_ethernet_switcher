#ifndef SPI_FUNCTIONS_H
#define SPI_FUNCTIONS_H

#include <stdint.h> 
#include "types.h"

#define TIMEOUT_SPI_ITERATIONS 1000

void spi_init(void);
void spi_0_cs_low(void);
void spi_0_cs_high(void);
retv spi_0_transfer(uint8_t tx_data, uint8_t* rx_data);


#endif

#include "W5100.h"
#include "spi_functions.h"

retv w5100_write(uint16_t addr, uint8_t data){
    spi_0_cs_low();
    spi_0_transfer(0xF0, nullptr);
    spi_0_transfer(addr >> 8, nullptr);
    spi_0_transfer(addr & 0xFF, nullptr);
    retv result = spi_0_transfer(data, nullptr);
    spi_0_cs_high();
    return result;
}

retv w5100_read(uint16_t addr, uint8_t* rx){
    spi_0_cs_low();
    spi_0_transfer(0x0F, nullptr);
    spi_0_transfer(addr >> 8, nullptr);
    spi_0_transfer(addr & 0xFF, nullptr);
    retv result = spi_0_transfer(0x00, rx);
    spi_0_cs_high();
    return result;
}

retv w5100_init(void){
    w5100_write(W5100_MR, W5100_MR_RST);

    //Регистр RMSR (RX Memory Size Register)
    //и TMSR - (TX Memory Size Register)
    w5100_write(W5100_RMSR, W5100_RMSR_S0_8KB);
    w5100_write(W5100_TMSR, W5100_TMSR_S0_8KB);

    //Регистр SHAR (Source Hardware Address Register) хранит MAC-адресс устройства
    w5100_write(W5100_SHAR0 + 0, MAC_ADDR_0);
    w5100_write(W5100_SHAR0 + 1, MAC_ADDR_1);
    w5100_write(W5100_SHAR0 + 2, MAC_ADDR_2);
    w5100_write(W5100_SHAR0 + 3, MAC_ADDR_3);
    w5100_write(W5100_SHAR0 + 4, MAC_ADDR_4);
    w5100_write(W5100_SHAR0 + 5, MAC_ADDR_5);

    // IP МК
    w5100_write(W5100_SIPR0 + 0, IP_ADDR_0); // 192
    w5100_write(W5100_SIPR0 + 1, IP_ADDR_1); // 168
    w5100_write(W5100_SIPR0 + 2, IP_ADDR_2); // 1
    w5100_write(W5100_SIPR0 + 3, IP_ADDR_3); // 100

    // Шлюз
    w5100_write(W5100_GAR0 + 0, GW_ADDR_0);  // 192
    w5100_write(W5100_GAR0 + 1, GW_ADDR_1);  // 168
    w5100_write(W5100_GAR0 + 2, GW_ADDR_2);  // 1
    w5100_write(W5100_GAR0 + 3, GW_ADDR_3);  // 1

    //Разрешаем прерывание от сокета 0
    w5100_write(W5100_IMR, W5100_IMR_S0_INT_EN);
    
    return retv::Ok;
}
#ifndef W5100_H
#define W5100_H

#include "types.h"

// ==== useful macroses ====
#define BIT(n) (1 << (n))


// Common registers
#define W5100_MR        0x0000  // Mode
#define W5100_GAR0      0x0001  // Gateway Address
#define W5100_SUBR0     0x0005  // Subnet Mask
#define W5100_SHAR0     0x0009  // MAC Address
#define W5100_SIPR0     0x000F  // Source IP Address
#define W5100_IR        0x0015  // Interrupt
#define W5100_IMR       0x0016  // Interrupt Mask
#define W5100_RMSR      0x001A  // RX Memory Size
#define W5100_TMSR      0x001B  // TX Memory Size

#define W5100_MR_RST    BIT(7)  // Software Reset
#define W5100_RMSR_S0_8KB   0x03  // сокет 0 → 8KB (остальные сокеты работать не будут, т.к. вся память будет предоставлена
#define W5100_TMSR_S0_8KB   0x03  // сокет 0 → 8KB   
#define W5100_IMR_S0_INT_EN 0x01  // Разрешаем прерывания от сокета 0                                                             сокету 1)

//Рандомный MAC-адресс устройтсва
#define MAC_ADDR_0  0x00
#define MAC_ADDR_1  0x08
#define MAC_ADDR_2  0xDC
#define MAC_ADDR_3  0x01
#define MAC_ADDR_4  0x02
#define MAC_ADDR_5  0x03

// IP МК
#define IP_ADDR_0    192
#define IP_ADDR_1    168
#define IP_ADDR_2    1
#define IP_ADDR_3    100

// Маска
#define SUB_ADDR_0   255
#define SUB_ADDR_1   255
#define SUB_ADDR_2   255
#define SUB_ADDR_3   0

// Шлюз = IP ПК
#define GW_ADDR_0    192
#define GW_ADDR_1    168
#define GW_ADDR_2    1
#define GW_ADDR_3    1


retv w5100_init(void);

#endif

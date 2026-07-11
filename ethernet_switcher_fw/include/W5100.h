#ifndef W5100_H
#define W5100_H

#include "types.h"
#include <stdint.h> 

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

// ==== Socket 0 registers ====
#define S0_MR       0x0400  // Socket 0 Mode Register
#define S0_CR       0x0401  // Socket 0 Command Register
#define S0_IR       0x0402  // Socket 0 Interrupt Register
#define S0_SR       0x0403  // Socket 0 Status Register
#define S0_PORT0    0x0404  // Socket 0 Source Port (старший байт)
#define S0_PORT1    0x0405  // Socket 0 Source Port (младший байт)

// ==== Socket Mode Register (Sn_MR) protocol values ====
#define S0_MR_TCP   0x01

// ==== Socket Command Register (Sn_CR) values ====
#define S0_CR_OPEN      0x01
#define S0_CR_LISTEN    0x02
#define S0_CR_CONNECT   0x04
#define S0_CR_DISCON    0x08
#define S0_CR_CLOSE     0x10
#define S0_CR_SEND      0x20
#define S0_CR_RECV      0x40

// ==== Socket Status Register (Sn_SR) values ====
#define SOCK_CLOSED        0x00
#define SOCK_INIT          0x13
#define SOCK_LISTEN        0x14
#define SOCK_ESTABLISHED   0x17
#define SOCK_CLOSE_WAIT    0x1C

// ==== Socket Interrupt Register (Sn_IR) bits ====
#define S0_IR_CON_M      BIT(0)
#define S0_IR_DISCON_M   BIT(1)
#define S0_IR_RECV_M     BIT(2)
#define S0_IR_TIMEOUT_M  BIT(3)
#define S0_IR_SEND_OK_M  BIT(4)

// ==== Local listening port ====
#define LOCAL_TCP_PORT   5000

// ==== Socket 0 RX регистры ====
#define S0_RX_RSR0  0x0426  // RX Received Size (старший байт)
#define S0_RX_RSR1  0x0427  // RX Received Size (младший байт)
#define S0_RX_RD0   0x0428  // RX Read Pointer (старший байт)
#define S0_RX_RD1   0x0429  // RX Read Pointer (младший байт)

// ==== Socket 0 RX буфер ====
#define gS0_RX_BASE  0x6000  // начало RX-памяти сокета 0
#define gS0_RX_MASK  0x1FFF  // маска для 8КБ


// Максимальный размер принимаемой строки
#define RX_BUF_SIZE 40  //Размер выбран под команду :SWIT5 1;SWIT11 5;*OPC?


retv w5100_init(void);
retv w5100_itHandler();
retv w5100_read(uint16_t addr, uint8_t* rx);

#endif

#ifndef YAGPIO_H
#define YAGPIO_H

#include "types.h"
#include <stdint.h>

void gpio_init(void);
void gpio_errorIndicator(retv resOfOperation);
retv parseTheCommand(char* out_buf, uint16_t* out_size);


// telemetry.h

// --- Пины телеметрии ---
#define TM_CLK_PIN      BIT(7)   // GPIO0[7] — CLK 74HC165
#define TM_PL_PIN       BIT(5)   // GPIO1[5] — /PL (защёлкивание данных, параллельная загрузка)
#define TM_CE_PIN       BIT(6)   // GPIO1[6] — /CE (разрешение тактирования)
#define TM_DATA_PIN     BIT(4)   // GPIO1[4] — Q_H (вход данных с регистра)
#define TM_DATA_PIN_S   4

// --- Пины управляющих буферов---
#define CTRL_NENABLE_BUFFER1_PART1  BIT(8)   // GPIO0[8] — SN74LVCZ244APWR
#define CTRL_NENABLE_BUFFER1_PART2  BIT(9)   // GPIO0[9] — SN74LVCZ244APWR
#define CTRL_NENABLE_BUFFER2_PART1  BIT(10)   // GPIO0[10] — SN74LVCZ244APWR
#define CTRL_NENABLE_BUFFER2_PART2  BIT(7)   // GPIO1[7] — SN74LVCZ244APWR
#define CTRL_NENABLE_BUFFER3_PART1  BIT(8)   // GPIO1[8] — SN74LVCZ244APWR
#define CTRL_NENABLE_BUFFER3_PART2  BIT(9)   // GPIO1[9] — SN74LVCZ244APWR


#define NUMBER_OF_RELAY 3 
#define RELAY_0 0
#define RELAY_1 1
#define RELAY_2 2

#define W5100_SEND_STR(s) w5100_send((s), sizeof(s) - 1)
#define STR_MATCH(buf, offset, literal) \
    (strncmp((buf) + (offset), (literal), sizeof(literal) - 1) == 0)

#define RELAY_POSITION_1 1
#define RELAY_POSITION_2 2
#define TELEMETRY_BUF_SIZE 2
#define TELEMETRU_BUF_POSITON_1 0
#define TELEMETRU_BUF_POSITON_2 1


#define MSG_MISTAKE     "MISTAKE\r\n"
#define MSG_OK          "OK\r\n"
#define MSG_POSITION_1  "POSITION 1\r\n"
#define MSG_POSITION_2  "POSITION 2\r\n"

// --- Parts of CMD ----

//----TOGGLE COMMAND ----
#define CMD_START_SYMBOL ':'
#define CMD_START_SYMBOL_OFFSET 1

#define CMD_TOGGLE "SWIT"
#define CMD_TOGGLE_OFFSET 1

#define CMD_FIRST_RELE_NUMBER_OFFSET 5

#define CMD_SPACE_SYMBOL ' '
#define CMD_SPACE_SYMBOL_OFFSET 6

#define CMD_FIRST_RELE_POSITION_OFFSET 7

#define CMD_SECOND_TOGGLE ";SWIT"
#define CMD_SECOND_TOGGLE_OFFSET 8

#define CMD_SECOND_RELE_NUMBER_OFFSET 13

#define CMD_SECOND_RELE_POSITION_OFFSET 15

//----CHECK COMMAND ----
#define CMD_CHECK "CHECK"
#define CMD_CHECK_OFFSET 1

#define CMD_RELAY_0 "0"
#define CMD_RELAY_1 "1"
#define CMD_RELAY_2 "2"
#define CMD_CHECK_ALL "ALL"

#define CMD_RELAY_NUMBER_OFFSET 7



#define CHAR_ZERO '0'
#define MISTAKE_BLINK 5

#endif
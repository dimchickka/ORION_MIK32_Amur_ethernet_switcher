#ifndef CONFIG_H
#define CONFIG_H

#include "types.h"
#include <stdint.h> 

//retv MCU_Init(void);
retv MCU_Init(void);
void delay_us(uint32_t us);
void delay_ms(uint32_t ms);
void timer32_1_init(void);


#endif

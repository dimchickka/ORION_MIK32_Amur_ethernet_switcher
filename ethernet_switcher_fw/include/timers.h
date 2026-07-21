#ifndef TIMERS_H
#define TIMERS_H

#include <stdint.h> 

#define RELAY_PULSE_TICKS   10000   // 10 мс при 1 тик = 1 мкс
#define RELAY_PERIOD_TICKS  20000   // TOP — с запасом (вдвое больше импульса)

void relay_pulse(uint8_t channel);
void timer32_2_init(void);

#endif
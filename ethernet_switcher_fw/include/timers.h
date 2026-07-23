#ifndef TIMERS_H
#define TIMERS_H

#include <stdint.h> 

#define RELAY_PULSE_TICKS   10000   // 10 мс при 1 тик = 1 мкс
#define RELAY_PERIOD_TICKS  20000   // TOP — с запасом (вдвое больше импульса)

#define CHANNEL_FOR_RELAY_PULSE_0 0
#define CHANNEL_FOR_RELAY_PULSE_1 1
#define CHANNEL_FOR_RELAY_PULSE_2 2
#define CHANNEL_FOR_RELAY_PULSE_3 3
#define CHANNELS_GENERAL_NUMBER 4

void relay_pulse(uint8_t channel);
void timer32_2_init(void);

#endif
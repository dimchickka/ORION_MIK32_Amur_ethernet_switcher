#ifndef YAGPIO_H
#define YAGPIO_H

#include "types.h"
#include <stdint.h>

void gpio_init(void);
void gpio_errorIndicator(retv resOfOperation);
retv makeConnectionsBetweenRelay(char* out_buf, uint16_t* out_size);

#endif
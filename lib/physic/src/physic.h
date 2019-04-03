#ifndef __S5_APP6_PHYSIC_H
#define __S5_APP6_PHYSIC_H

#include <mbed.h>
#include <rtos.h>

void start_listener(Mail<uint8_t, 100> *);

Thread * get_listener(void);

#endif

#ifndef __S5_APP6_LINKDATA_H
#define __S5_APP6_LINKDATA_H

#include <mbed.h>
#include <rtos.h>

#include "crc/crc.h"

#define PREAMBULE 0x55
#define START 0x7E
#define END 0x7E

extern Mail<uint8_t, 100> linkdata_bytes_pool;

void start_linkdata(void);

#endif

#ifndef __S5_APP4_HARDWARE_H
#define __S5_APP4_HARDWARE_H

#include <mbed.h>
#include <rtos.h>

#include "frame/frame.h"
#include "manchester/manchester.h"

/**
 * Read Section
 * */

// This synchronize the thread with the ticker.
extern Semaphore physic_read_event;

/**
 * Start the physic's reader thread
 * */
void start_physic_read(Thread *);

/**
 * Return the current period in which the system is sampling.
 *
 * @return us_timestamp_t A 64-bits representation in nano-second.
 * */
us_timestamp_t physic_read_get_current_period(void);

/**
 * Write Section
 * */

#define BIT_PERIOD 1000

/**
 * Start the physic's writer thread
 * */
void start_physic_write(void);

void send_message_to_phy(frame_t &frame);

#endif

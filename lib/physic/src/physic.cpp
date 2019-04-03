#include "physic.h"

static Thread listener(osPriorityAboveNormal, 1024);

static uint8_t current_byte;
static uint8_t position;

static Mail<uint8_t, 100> *link_pool;

/**
 * Routine called by the sampler thread.
 * */
static void
listener_th(void)
{
  while (1) {
    ThisThread::flags_wait_any_for(0xFF, osWaitForever, false);
    uint32_t flags = ThisThread::flags_get();
    switch (flags) {
      case 1:
        if (position == 0) {
          //printf("0x%02x\r\n", current_byte);
          uint8_t *value = (uint8_t*)link_pool->alloc();
          if (value != NULL) {
            *value = current_byte;
            link_pool->put(value);

            current_byte = 0x00;
            position = 7;
          }
        } else position--;
        break;

      case 2:
        if (position == 0) {
          current_byte |= 0x01;
          //printf("0x%02x\r\n", current_byte);
          uint8_t *value = (uint8_t*)link_pool->alloc();
          if (value != NULL) {
            *value = current_byte;
            link_pool->put(value);

            current_byte = 0x00;
            position = 7;
          }
        } else {
          current_byte |= (0x01 << position);
          position--;
        }
        break;

      case 4:
        // We have the "010" sequence at top MSB of the byte
        current_byte = 0x40;
        position = 4;
        break;

      case 8:   // passthrough
      default:
        printf("Got an error\r\n");
        current_byte = 0x00;
        position = 7;
        break;
    }

    ThisThread::flags_clear(flags);
  }
}

void
start_listener(Mail<uint8_t, 100> * _link_pool)
{
  link_pool = _link_pool;
  current_byte = 0x00;
  position = 7;
  listener.start(listener_th);
  printf("Listener id: %x\r\n", listener.get_id());
  listener.set_priority(osPriorityAboveNormal);
}

Thread *
get_listener(void)
{
  return &listener;
}

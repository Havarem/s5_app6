#include "linkdata.h"

Mail<uint8_t, 100> linkdata_bytes_pool;

typedef enum {
  APP_WAITING = 0,
  APP_PREAMBULE = 1,
  APP_START = 2,
  FLAG = 3,
  LENGTH = 4,
  DATA = 5,
  CRC_1 = 6,
  CRC_2 = 7,
  APP_ERROR = 8
} LinkdataState;

static Thread linkdata(osPriorityNormal, 4096);
static LinkdataState state;
static char current_message[74];
static uint8_t message_length;
static uint8_t cursor;
static unsigned short current_crc;

static inline int
check_crc(void)
{
  unsigned short crc = crc16(current_message, message_length);
  return crc == current_crc ? 1 : 0;
}

static inline void
linkdata_state_machine(uint8_t current)
{
  switch (state) {
  case APP_WAITING:
    if (message_length > 0) {
      printf("CRC OK - %s\r\n", current_message);
      message_length = 0;
      cursor = 0;
      current_crc = 0x0000;
    }
    if (current == 0x55) state = APP_PREAMBULE;
    break;

  case APP_PREAMBULE:
    if (current == 0x7E) state = APP_START;
    else state = APP_WAITING;
    break;

  case APP_START:
    state = FLAG;
    break;

  case FLAG:
    message_length = current;
    cursor = 0;
    state = LENGTH;
    break;

  case LENGTH:
    if (message_length == 0) {
      current_crc = current_crc | current;
      current_message[0] = '\0';
      state = CRC_1;
    } else {
      current_message[0] = current;
      cursor++;
      state = DATA;
    }
    break;

  case DATA:
    if (cursor == message_length) {
      current_crc = current_crc | current;
      current_message[cursor] = '\0';
      state = CRC_1;
    } else {
      current_message[cursor] = current;
      cursor++;
    }
    break;

  case CRC_1:
    current_crc = current_crc | (current << 8);
    state = CRC_2;
    break;

  case CRC_2:
    if (check_crc() && current == 0x7E) state = APP_WAITING;
    else state = APP_ERROR;
    break;

  case APP_ERROR:
    printf("Bad CRC - %s\r\n", current_message);
    state = APP_WAITING;
    break;
  }
}

static void
linkdata_th(void)
{
  uint8_t current_byte = 0x00;
  while (1) {
    osEvent event = linkdata_bytes_pool.get();
    if (event.status == osEventMail) {
      uint8_t *byte = (uint8_t *)event.value.p;
      current_byte = *byte;
      linkdata_bytes_pool.free(byte);

      linkdata_state_machine(current_byte);
    }
  }
}

void
start_linkdata(void)
{
  state = APP_WAITING;
  cursor = 0;
  message_length = 0;
  linkdata.start(linkdata_th);
}

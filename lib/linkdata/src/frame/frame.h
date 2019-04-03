#ifndef __S5_APP6_FRAME_H
#define __S5_APP6_FRAME_H

#define MAX_DATA_LENGTH 73

typedef struct frame_t {
  // First byte is the preambule (0x55)
  // Second byte is the start flag (0x7E)
  // Third byte is the flags (for future use)
  // Forth byte is the payload size (0 to MAX_DATA_LENGTH)
  // The next payload size bytes the payload
  // Next 2 bytes are the CRC of the payload
  // The last bytes is the end flag (0x7E)
  char message[80];

  // In bytes
  char length;
} frame_t;

frame_t create_frame(char[MAX_DATA_LENGTH], int);

#endif

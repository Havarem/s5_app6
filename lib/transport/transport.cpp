#include "transport.h"

void send_messages(char * message, int length)
{
  int nb_messages = length / MAX_DATA_LENGTH + (length % MAX_DATA_LENGTH != 0);

  for(int i = 0; i < nb_messages-1; i++) {
    char frame_message[MAX_DATA_LENGTH];
    for(int j = 0; j < MAX_DATA_LENGTH; j++) frame_message[j] = message[i*MAX_DATA_LENGTH + j];

    frame_t current_frame = create_frame(frame_message, MAX_DATA_LENGTH);
    send_message_to_phy(current_frame);
  }

  if(length % MAX_DATA_LENGTH != 0) {
    char frame_message[MAX_DATA_LENGTH];
    for(int j = 0; j < length % MAX_DATA_LENGTH; j++) frame_message[j] = message[j];

    frame_t current_frame = create_frame(frame_message, length % MAX_DATA_LENGTH);
    send_message_to_phy(current_frame);
  }
}
#include "frame.h"
#include "crc/crc.h"

frame_t
create_frame(char content[MAX_DATA_LENGTH], int length)
{
    frame_t frame;
    frame.message[0] = 0x55; //preambule
    frame.message[1] = 0x7E; //start
    frame.message[2] = 0x00; //Flags
    frame.message[3] = length; //lenght
    for (int i = 0; i < length; i++)
    {
        frame.message[i + 4] = content[i];
    }
    unsigned short crc = crc16(content, length);

    frame.message[length+4] =  crc & 0xFF; //CRC
    frame.message[length+5] =  (crc >> 8) & 0xFF; //CRC
    frame.message[length+6] = 0x7E; //end

    frame.length = length + 7;

    return frame;
}
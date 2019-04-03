#include "manchester.h"

void
to_manchester(char res[2], char in)
{
  res[0] = 0;
  res[1] = 0;

  for (int i = 0; i < 4; i++) {
    char bit = (in >> i) & 0x01;
    res[0] = res[0] | !bit << 2 * i;
    res[0] = res[0] | bit << (2 * i + 1);
  }

  for(int i = 4; i < 8; i++) {
    char bit = (in >> i) & 0x01;
    res[1] = res[1] | !bit << 2 * (i - 4);
    res[1] = res[1] | bit << (2 * (i - 4) + 1);
  }
}
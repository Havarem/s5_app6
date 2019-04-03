#include <mbed.h>
#include <rtos.h>

#include "hardware.h"
#include "physic.h"
#include "linkdata.h"
#include "transport.h"

int
main()
{
  osThreadId id = ThisThread::get_id();
  osThreadSetPriority(id, osPriorityRealtime1);

  start_linkdata();
  start_listener(&linkdata_bytes_pool);
  start_physic_read(get_listener());

  DigitalOut led(LED1);
  start_physic_write();

  osThreadSetPriority(id, osPriorityNormal);
  while(1)
  {
    send_messages("ABC", 3);
    led = !led;
    ThisThread::sleep_for(160);
  }

  return 0;
}
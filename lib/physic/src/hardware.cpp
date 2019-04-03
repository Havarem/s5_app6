#include "hardware.h"

/**
 * Read Section
 * */

Semaphore physic_read_event;

typedef enum {
  WAITING = 0,
  SYNCHING_HIGH = 1,
  SYNCHING_LOW = 2,
  INIT = 3,
  MID = 4,
  START = 5,
  ERROR = 6
} PhysicReadState;

static char physic_read_state_name[7][10] = {
  { 'W', 'A', 'I', 'T', 'I', 'N', 'G', '\0' },
  { 'S', 'Y', 'N', 'C', '-', 'H', 'I', 'G', 'H', '\0' },
  { 'S', 'Y', 'N', 'C', '-', 'L', 'O', 'W', '\0' },
  { 'I', 'N', 'I', 'T', '\0' },
  { 'M', 'I', 'D', '\0' },
  { 'S', 'T', 'A', 'R', 'T', '\0' },
  { 'E', 'R', 'R', 'O', 'R', '\0' },
};

//  Thread instance
static Thread physic_reader(osPriorityRealtime, 2048);
// Physic-layer thread reference
static Thread *link_data;

// The pin to listen to
static InterruptIn signal(p21);

// Use to measure periods
static Timer period_timer;
// Sampler period in nano-second
static us_timestamp_t double_period;
// Sampler period in nano-second
static us_timestamp_t period;
// Used to measure the synching high state
static us_timestamp_t t_high = 0;
// Used to measure the synching low state
static us_timestamp_t t_low = 0;
// Used to measure interval between rises and falls
static us_timestamp_t interval;

// Physic read state-machine current state
static PhysicReadState state;

/**
 * ISR for the Sampler Ticker.
 * */
static void
physic_read_tick_routine(void)
{
  physic_read_event.release();
}


static inline void
physic_read_state_machine(int value)
{
  us_timestamp_t interval_range;
  us_timestamp_t interval_margin;
  uint32_t multiple = 0;

  //printf("%s", physic_read_state_name[state]);
  if (state == INIT || state == MID || state == START) {
    period_timer.stop();
    interval = period_timer.read_high_resolution_us();
    period_timer.reset();
    period_timer.start();

    interval_range = interval >> 6;
    interval_margin = interval - interval_range;

    if (interval_margin > double_period) {
      if (value == 0) {
        state = SYNCHING_HIGH;
      } else {
        state = WAITING;
      }
    } else if (interval_margin > period) {
      multiple = 2;
      if (state == START) {
        if (value == 1) state = WAITING;
        else state = SYNCHING_HIGH;
      }
    } else {
      if (state == MID) state = START;
      else if (state == START) state = MID;

      multiple = 1;
    }
  }

  //printf(" %s - ", physic_read_state_name[state]);
  switch (state) {
  case WAITING:
    if (value == 1) {
      state = SYNCHING_HIGH;
      period_timer.reset();
      period_timer.start();
    }
    break;

  case SYNCHING_HIGH:
    if (value == 0) {
      period_timer.stop();
      t_high = period_timer.read_high_resolution_us();
      period_timer.reset();
      period_timer.start();
      state = SYNCHING_LOW;
    }
    break;

  case SYNCHING_LOW:
    if (value == 1) {
      period_timer.stop();
      t_low = period_timer.read_high_resolution_us();
      period_timer.reset();
      period_timer.start();

      us_timestamp_t diff = t_high > t_low ? t_high - t_low : t_low - t_high;
      us_timestamp_t shift_high = t_high >> 6;

      if (shift_high >= diff && t_high > 0) {
        double_period = t_high;
        period = t_high >> 1;

        // This means that we have the value "010" in the MSB of the first byte.
        //printf(" signal 4\r\n");
        link_data->flags_set(0x04);

        state = INIT;
      } else state = WAITING;
    }
    break;

  case INIT:
    if (multiple == 2) {
      link_data->flags_set((value == 0) ? 0x02 : 0x01);

      //printf(" signal %d\r\n", value + 1);
      state = MID;
    } else state = ERROR;
    break;

  case MID:
    //printf(" signal %d\r\n", value + 1);
    link_data->flags_set((value == 0) ? 0x02 : 0x01);
    break;

  case START:
    break;

  case ERROR:
    if (value == 0) state = WAITING;
    break;
  }
}

/**
 * Routine called by the sampler thread.
 * */
static void
physic_read_th(void)
{
  int value;

  while(1) {
    physic_read_event.wait(osWaitForever);
    value = signal;
    physic_read_state_machine(value);
  }
}

void
start_physic_read(Thread * _link_data)
{
  state = WAITING;
  link_data = _link_data;
  physic_reader.start(physic_read_th);

  signal.rise(physic_read_tick_routine);
  signal.fall(physic_read_tick_routine);
}

us_timestamp_t
physic_read_get_current_period(void)
{
  return period;
}

/**
 * Write Secton
 * */

static Ticker physic_writer_tick;

static Thread physic_writer(osPriorityHigh, 1024);

static Mail<frame_t, 10> physic_write_messages_to_send;

static DigitalOut tx(p22);

static void
physic_writer_tick_routine(void)
{
  physic_writer.signal_set(0x1);
}

void
send_message_to_phy(frame_t &_frame)
{
  frame_t *frame = physic_write_messages_to_send.alloc();
  if (frame != NULL) {
    frame->length = _frame.length;
    for (int i = 0; i < _frame.length; i++) frame->message[i] = _frame.message[i];

    physic_write_messages_to_send.put(frame);
  }
}

static void
physic_writer_th(void)
{
  while (1) {
    osEvent evt = physic_write_messages_to_send.get();

    if (evt.status == osEventMail) {
      frame_t *tmp = (frame_t *)evt.value.p;
      frame_t message = *tmp;

      char full_message[160];
      for (int i = 0; i < message.length; i++) {
        char res[2];
        to_manchester(res, message.message[i]);
        full_message[i * 2] = res[1];
        full_message[i * 2 + 1] = res[0];
      }

      for (int i = 0; i < message.length * 2; i++) {
        int j = 7;
        while (j >= 0) {
          Thread::signal_wait(0x1);
          tx = (full_message[i] >> j) & 0x01;
          j--;
        }
      }

      //RESET tx to 0 after message
      Thread::signal_wait(0x1);
      tx = 0;

      physic_write_messages_to_send.free(tmp);
    }
  }
}

void
start_physic_write()
{
  physic_writer_tick.attach_us(physic_writer_tick_routine, BIT_PERIOD);
  physic_writer.start(physic_writer_th);
}
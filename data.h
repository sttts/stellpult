#include <Arduino.h>

#define NUM_WEICHEN 16
#define NUM_LEDS 128
#define NUM_SERVOS 16

struct StateWeiche {
  uint8_t stellungen : 2;
} __attribute__((packed));

struct StateLED {
  uint8_t weiche : 6;
  uint8_t position : 2;
} __attribute__((packed));

struct StateServo {
  uint8_t position[3];
} __attribute__((packed));

struct State {
  uint8_t version;
  StateWeiche weichen[NUM_WEICHEN];
  StateLED leds[NUM_LEDS];
  StateServo servos[NUM_SERVOS];
} __attribute__((packed));

extern State state;

void initData(State &state);

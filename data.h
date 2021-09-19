#include <Arduino.h>

#define NUM_WEICHEN 16
#define NUM_LEDS 128
#define NUM_SERVOS 16

struct StateWeiche {
  uint8_t typ : 1; // 0 = Doppelweiche, 1 = Dreifachweiche
  uint8_t anfangsstellung : 2; // 0 = Links, 1 = Rechts, 2 = Mitte
} __attribute__((packed));

struct StateLED {
  uint8_t weiche : 4; // 0-15
  uint8_t richtung : 3; // bit 0 = Links, bit 1 = Rechts, bit 2 = Mitte
} __attribute__((packed));

struct StateServo {
  uint8_t position[3];
  uint8_t weiche : 4; // 0-15
} __attribute__((packed));

struct State {
  uint8_t version;
  StateWeiche weichen[NUM_WEICHEN];
  StateLED leds[NUM_LEDS];
  StateServo servos[NUM_SERVOS];
  uint8_t brightness : 4;
} __attribute__((packed));

extern State state;

void initData(State &state);
void loadData(State &state);
void saveData(State &state);

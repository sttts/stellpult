#include <Arduino.h>

struct StateWeicheV4 {
  uint8_t typ : 1; // 0 = Doppelweiche, 1 = Dreifachweiche
  uint8_t anfangsstellung : 2;
} __attribute__((packed));

struct StateLedV4 {
  uint8_t weiche : 5;
  uint8_t richtung : 3; // bit 0 = Links, bit 1 = Rechts, bit 2 = Mitte
} __attribute__((packed));

struct StateServoV4 {
  uint8_t position[3];
} __attribute__((packed));

struct StateV4 {
  uint8_t version;
  StateWeicheV4 weichen[NUM_WEICHEN];
  StateLedV4 leds[NUM_LEDS];
  StateServoV4 servos[NUM_SERVOS];
  uint8_t brightness : 4;
} __attribute__((packed));

extern void convertV4(StateV4 &in, State &out);
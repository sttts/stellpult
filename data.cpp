#import "data.h"

void initData(State &state) {
  state.version = 1;

  for (int w = 0; w < NUM_WEICHEN; w++) {
    state.weichen[w].stellungen = 2;
  }
  for (int l = 0; l < NUM_LEDS; l++) {
    state.leds[l].weiche = 1;
    state.leds[l].position = 0;
  }
  for (int s = 0; s < NUM_SERVOS; s++) {
    state.servos[s].position[0] = 50;
    state.servos[s].position[1] = 50;
    state.servos[s].position[2] = 50;
  }
}

#include <EEPROM.h>

#import "data.h"

void initData(State &state) {
  state.version = 4;

  for (int w = 0; w < NUM_WEICHEN; w++) {
    state.weichen[w].typ = 0; // Doppelweiche
    state.weichen[w].anfangsstellung = 0;
  }
  for (int l = 0; l < NUM_LEDS; l++) {
    state.leds[l].weiche = 1;
    state.leds[l].richtung = 0; // nie an
  }
  for (int s = 0; s < NUM_SERVOS; s++) {
    state.servos[s].position[0] = 50;
    state.servos[s].position[1] = 50;
    state.servos[s].position[2] = 50;
  }
  state.brightness = 15;
}

void loadData(State &state) {
  EEPROM.get(0, state);
  if (state.version != 4) {
    Serial.println(F("Initialisiere Daten"));
    initData(state);
  } else {
    Serial.print(F("Daten gefunden. Version: "));
    Serial.println(state.version);
  }
}

void saveData(State &state) {
  EEPROM.put(0, state);
}

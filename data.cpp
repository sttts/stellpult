#include <EEPROM.h>

#import "data.h"

// byte data[] PROGMEM = {4,250,248,248,248,249,250,248,248,248,250,248,248,248,250,248,248,65,33,65,33,65,33,65,33,33,65,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,71,70,38,42,39,71,71,74,1,1,1,77,71,71,43,45,75,75,43,72,40,1,1,1,1,72,72,78,46,44,67,76,67,68,72,1,36,1,1,1,1,1,1,1,1,33,65,66,34,34,66,67,67,66,34,1,1,1,1,1,1,1,34,66,66,34,34,67,66,68,35,1,1,1,37,68,36,68,41,73,69,133,37,69,133,1,1,1,1,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,255}

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
    Serial.print("byte restore[] PROGMEM = {");
    byte *bs = (byte*)&state;
    for (unsigned i = 0; i < sizeof(state); i++) {
      if (i > 0) {
        Serial.print(",");
      }
      Serial.print(bs[i]);
    }
    Serial.println("}");
  }
}

void saveData(State &state) {
  EEPROM.put(0, state);
}

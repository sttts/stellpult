// #include <EEPROM.h>
//#include <FlashStorage_SAMD.h>
#include "ds_external_eeprom_i2c.h"


#import "data.h"
#import "data_v4.h"

ds_external_eeprom_i2c eeprom(ds_external_eeprom_i2c::EEPROM_MODEL::MODEL_24_256, 0x50);

byte initialData[] PROGMEM = {5,250,248,248,248,249,250,248,248,248,250,248,248,248,250,248,248,32,16,32,16,32,16,32,16,16,32,16,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,38,37,21,25,22,38,38,41,1,1,1,44,38,38,26,28,42,42,26,39,23,1,1,1,1,39,39,45,29,27,34,43,34,35,39,1,19,1,1,1,1,1,1,1,1,16,32,33,17,17,33,34,34,33,17,1,1,1,1,1,1,1,17,33,33,17,17,34,33,35,18,1,1,1,20,35,19,35,24,40,36,196,20,36,196,1,1,1,1,32,60,50,0,50,50,50,1,50,50,50,2,50,50,50,3,50,50,50,4,50,50,50,5,50,50,50,6,50,50,50,7,50,50,50,8,50,50,50,9,50,50,50,10,50,50,50,11,48,80,50,12,50,50,50,13,50,50,50,5,50,50,50,15,15};
bool readOnly = false;

bool probe() {
  int addr = sizeof(State)/64*64+64;
  int x = rand();
  eeprom.write(addr, (uint8_t*)&x, sizeof(x));
  int y;
  eeprom.read(addr, (uint8_t*)&y, sizeof(y));
  Serial.print("Schreibtest written=");
  Serial.print(x);
  Serial.print(" read=");
  Serial.println(y);
  return x == y;
}

void initData(State &state) {
  state.version = 5;

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
    state.servos[s].weiche = s;
  }
  state.brightness = 15;
}

void loadData(State &state) {
  uint8_t version;  
  eeprom.read(0, (uint8_t*)&version, sizeof(version));

  switch (version) {
  case 4:
    Serial.print(F("v4 Daten gefunden. Konvertiere..."));
    StateV4 statev4;
    eeprom.read(0, (uint8_t*)&statev4, sizeof(statev4));  
    convertV4(statev4, state);      
    break;
  case 5:
    Serial.print(F("v5 Daten gefunden"));
    eeprom.read(0, (uint8_t*)&state, sizeof(state));  
    break;
  default:
    static bool written = false;
    if (readOnly || written) {
      Serial.println(F("Initialisiere Daten mit Nullen"));
      initData(state);
      return;
    }
    if (probe()) {
      Serial.println(F("Initialisiere Daten mit Default"));
      eeprom.write(0, (uint8_t*)initialData, sizeof(initialData));
      written = true;
      loadData(state);
    } else {
      Serial.println(F("Kann nicht schreiben"));
      initData(state);
      readOnly = true;
    }
    
    break;
  }

  // Restore-Daten
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



void saveData(State &state) {
  if (!probe()) {
    Serial.println("Failed to write probe to EEPROM");
    return;
  }
  
  eeprom.write(0, (uint8_t*)&state, sizeof(state));
}

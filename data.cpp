// #include <EEPROM.h>
//#include <FlashStorage_SAMD.h>
#include "ds_external_eeprom_i2c.h"


#import "data.h"
#import "data_v4.h"

ds_external_eeprom_i2c eeprom(ds_external_eeprom_i2c::EEPROM_MODEL::MODEL_24_256, 0x50);

// byte data[] PROGMEM = {4,250,248,248,248,249,250,248,248,248,250,248,248,248,250,248,248,65,33,65,33,65,33,65,33,33,65,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,71,70,38,42,39,71,71,74,1,1,1,77,71,71,43,45,75,75,43,72,40,1,1,1,1,72,72,78,46,44,67,76,67,68,72,1,36,1,1,1,1,1,1,1,1,33,65,66,34,34,66,67,67,66,34,1,1,1,1,1,1,1,34,66,66,34,34,67,66,68,35,1,1,1,37,68,36,68,41,73,69,133,37,69,133,1,1,1,1,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,255}

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
    Serial.println(F("Initialisiere Daten"));

    byte init[] PROGMEM = {5,250,248,248,248,249,250,248,248,248,250,248,248,248,250,248,248,32,16,32,16,32,16,32,16,16,32,16,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,38,37,21,25,22,38,38,41,1,1,1,44,38,38,26,28,42,42,26,39,23,1,1,1,1,39,39,45,29,27,34,43,34,35,39,1,19,1,1,1,1,1,1,1,1,16,32,33,17,17,33,34,34,33,17,1,1,1,1,1,1,1,17,33,33,17,17,34,33,35,18,1,1,1,20,35,19,35,24,40,36,196,20,36,196,1,1,1,1,32,60,50,0,50,50,50,1,50,50,50,2,50,50,50,3,50,50,50,4,50,50,50,5,50,50,50,6,50,50,50,7,50,50,50,8,50,50,50,9,50,50,50,10,50,50,50,11,48,80,50,12,50,50,50,13,50,50,50,5,50,50,50,15,15};
    eeprom.write(0, (uint8_t*)init, sizeof(init));  
    eeprom.read(0, (uint8_t*)&state, sizeof(state));  
    
    //initData(state);
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
  eeprom.write(0, (uint8_t*)&state, sizeof(state));
}

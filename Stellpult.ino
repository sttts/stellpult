#include <PCA9685.h>
#include "ht16k33.h"
#include "menu.h"
#include "data.h"

PCA9685 pwmController;
HT16K33 HT;
State state;

uint8_t weichenPositionen[NUM_WEICHEN];

void updateLeds() {
  for (uint8_t led=0; led<NUM_LEDS; led++) {
    uint8_t w = state.leds[led].weiche - 1;
    uint8_t r = state.leds[led].richtung;
    uint8_t mask = 1<<weichenPositionen[w];
    if (state.leds[led].richtung&mask) {
      HT.setLed(led);
    } else {
      HT.clearLed(led);
    }
  }
  HT.sendLed();
}

void updateServos() {
  for (uint8_t s=1; s<=NUM_SERVOS; s++) {
    updateServo(s, false);
  }
}

void updateServo(uint8_t s, bool verbose) {
  uint16_t p = state.servos[s-1].position[weichenPositionen[s-1]] * 512 / 100;
  if (verbose) {
    Serial.print(F("updateServo "));
    Serial.print(s);
    Serial.print(F(" "));
    Serial.println(p);
  }
  pwmController.setChannelPWM(s-1, p);
}

void setup() {
  Serial.begin(57600);
  while (!Serial) {}

  loadData(state);
  
  //Wire.begin();
  HT.begin(0x00); // 0x70 is added in the class

  pwmController.resetDevices();       // Resets all PCA9685 devices on i2c line
  pwmController.init();               // Initializes module using default totem-pole driver mode, and default disabled phase balancer
  pwmController.setPWMFrequency(50); // Set PWM freq to 100Hz (default is 200Hz, supports 24Hz to 1526Hz)
  //pwmController.setChannelPWM(0, 128 << 4); // Set PWM to 128/255, shifted into 4096-land

  for (uint8_t w=0; w<NUM_WEICHEN; w++) {
    weichenPositionen[w] = state.weichen[w].anfangsstellung;
  }
  updateServos();
  updateLeds();
  
  menu_setup();
}

uint8_t ledBlinkenAlt = 0;

uint8_t readWeichenKey() {
    uint8_t key = HT.readKey();
    if (key == 0) {
      return 0;
    }
    Serial.print(F("Key pressed: ")); 
    Serial.println(key);

    key -= 1; // TODO(sttts): Weichenstecker alle eins nach links
    if (key < 1 || key > NUM_WEICHEN) {
      return 0;
    }

    return key;
}

void loop() {  
  menu_loop();

  if (ledBlinken == 0 && ledBlinkenAlt != 0) {
    updateLeds();
  } else if (ledBlinken != 0 && ledBlinkenAlt == 0) {
    for (uint8_t led=0; led<NUM_LEDS; led++) {
      HT.clearLed(led);
    }
    HT.sendLed();
  } else if (ledBlinken != ledBlinkenAlt && ledBlinkenAlt != 0) {
       HT.clearLed(ledBlinkenAlt - 1);
  }
  ledBlinkenAlt = ledBlinken;

  if (ledBlinken != 0) {
    if (millis()%(unsigned long)(200)<(unsigned long)100) {
      HT.clearLed(ledBlinken - 1);
    } else {
      HT.setLed(ledBlinken - 1);
    }
    HT.sendLed();
  } else {
    uint8_t w = readWeichenKey();
    if (w != 0) {
      uint8_t stellungen = 2;
      if (state.weichen[w-1].typ == 1) {
        stellungen += 1;
      }
      weichenPositionen[w-1] = (weichenPositionen[w-1]+1) % stellungen;
  
      updateLeds();
      updateServo(w, true);
    }
  }

  delay(100);

  digitalWrite(LED_BUILTIN, millis()%(unsigned long)(500)<(unsigned long)250);
}

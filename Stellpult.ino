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
  updateLeds();
  
  menu_setup();
}

void loop() {  
  menu_loop();
  
  uint8_t key = HT.readKey();
  if (key != 0) {
    Serial.print(F("Key pressed: ")); 
    Serial.println(key); 

    key -= 1; // TODO(sttts): Weichenstecker alle eins nach links

    if (key >= 1 && key <= NUM_WEICHEN) {
      uint8_t w = key - 1;
      uint8_t stellungen = 2;
      if (state.weichen[w].typ == 1) {
        stellungen += 1;
      }
      weichenPositionen[w] = (weichenPositionen[w]+1) % stellungen;

      updateLeds();

      uint16_t p = state.servos[w].position[weichenPositionen[w]] * 512 / 100;
      pwmController.setChannelPWM(0, p);
    }
  }

  delay(100);

  digitalWrite(LED_BUILTIN, millis()%(unsigned long)(500)<(unsigned long)250);
}

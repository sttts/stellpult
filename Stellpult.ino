#include <PCA9685.h>
#include "ht16k33.h"
#include "menu.h"
#include "data.h"

PCA9685 pwmController;
HT16K33 HT;
State state;

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

  for (uint8_t led=0; led<128; led++) {
      HT.setLed(led);
  }
  HT.setLed(0);
  HT.sendLed();

  menu_setup();
}

bool weiche17 = false;

void loop() {  
  menu_loop();
  
  uint8_t key = HT.readKey();
  if (key != 0) {
    Serial.print(F("Key pressed: ")); 
    Serial.println(key); 

    if (key == 2) {
      weiche17 = !weiche17;

      for (uint8_t led=0; led<128; led++) {
        if (weiche17) {
          HT.setLed(led);
        } else {
          HT.clearLed(led);
        }
      }
      
      if (weiche17) {
        HT.setLed(0);
        HT.clearLed(1);
        pwmController.setChannelPWM(0, 150);
      } else {
        HT.setLed(1);
        HT.clearLed(0);
        pwmController.setChannelPWM(0, 400);
      }
      HT.sendLed();
    }
  }

  delay(100);

  digitalWrite(LED_BUILTIN, millis()%(unsigned long)(500)<(unsigned long)250);
}

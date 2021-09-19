#include <menu.h>
#include <PCA9685.h>
#include <SSD1306Ascii.h>
#include <SSD1306AsciiWire.h>
#include <menuIO/SSD1306AsciiOut.h>
#include <menuIO/encoderIn.h>
#include <menuIO/chainStream.h>
#include <menuIO/keyIn.h>
#include <menuIO/keyIn.h>

#include "ht16k33.h"
#include "data.h"

extern HT16K33 HT;
extern PCA9685 pwmController;
extern void updateServos();

using namespace Menu;

#define MAX_DEPTH 3

byte led = 1;
uint8_t ledBlinken;
byte weiche = 1;

byte typ = 0;
Menu::result selectTypChanged(Menu::eventMask e) {
  Serial.print(F("selectTypChanged "));
  Serial.println(typ);
  
  state.weichen[weiche-1].typ = typ;
  saveData(state);
  return Menu::proceed;
}
SELECT(typ,selectTyp,"Typ: ",selectTypChanged,Menu::exitEvent,Menu::noStyle
  ,VALUE("Doppelweiche",0,Menu::doNothing,Menu::noEvent)
  ,VALUE("Dreifachweiche",1,Menu::doNothing,Menu::noEvent)
);

byte anfangsstellung = 0;
Menu::result selectAnfangsstellungChanged(Menu::eventMask e) {
  Serial.print(F("selectAnfangsStellungChanged "));
  Serial.println(anfangsstellung);
  
  state.weichen[weiche-1].anfangsstellung = anfangsstellung;
  saveData(state);
  return Menu::proceed;
}
SELECT(anfangsstellung,selectAnfangsstellung,"Beim Start: ",selectAnfangsstellungChanged,Menu::exitEvent,Menu::noStyle
  ,VALUE("Links",0,Menu::doNothing,Menu::noEvent)
  ,VALUE("Rechts",1,Menu::doNothing,Menu::noEvent)
  ,VALUE("Mitte",2,Menu::doNothing,Menu::noEvent)
);

Menu::result subWeichenSelected(Menu::eventMask e) {
  Serial.print(F("subWeichenSelected "));
  Serial.println(weiche);
  
  typ = state.weichen[weiche-1].typ;
  anfangsstellung = state.weichen[weiche-1].anfangsstellung;
  return Menu::proceed;
}
MENU(subWeichen, "Weichen einstellen", subWeichenSelected, Menu::enterEvent, Menu::wrapStyle
  ,FIELD(weiche,"Nummer","",1,16,1,0,subWeichenSelected,Menu::enterEvent,Menu::wrapStyle)
  ,SUBMENU(selectTyp)
  ,SUBMENU(selectAnfangsstellung)
  ,EXIT("<Zurueck")
);

byte richtung = 0;
Menu::result subLedsRichtungChanged(Menu::eventMask e) {
  Serial.print(F("subLedsRichtungChanged "));
  Serial.println(richtung);
  
  state.leds[led-1].richtung = richtung;
  saveData(state);
  
  // Richtung aufblinken lassen
  if (richtung == 1 || richtung == 2 || richtung == 4) {
    for (uint8_t led=0; led<NUM_LEDS; led++) {
      uint8_t w = state.leds[led].weiche;
      if (w != weiche) {
        continue;
      }
      uint8_t r = state.leds[led].richtung;
      if (state.leds[led].richtung&richtung) {
        HT.setLed(led);
      } else {
        HT.clearLed(led);
      }
    }
    HT.sendLed();

    delay(500);

    HT.clearAll();
  }

  return Menu::proceed;
}
SELECT(richtung,selectRichtung,"Richtung: ",subLedsRichtungChanged,Menu::exitEvent,Menu::noStyle
  ,VALUE("Links",1,Menu::doNothing,Menu::noEvent)
  ,VALUE("Rechts",2,Menu::doNothing,Menu::noEvent)
  ,VALUE("Mitte",4,Menu::doNothing,Menu::noEvent)
  ,VALUE("immer an",7,Menu::doNothing,Menu::noEvent)
  ,VALUE("nie an",0,Menu::doNothing,Menu::noEvent)
  ,VALUE("Links aus",6,Menu::doNothing,Menu::noEvent)
  ,VALUE("Rechts aus",5,Menu::doNothing,Menu::noEvent)
  ,VALUE("Mitte aus",3,Menu::doNothing,Menu::noEvent)
);

Menu::result subLedsSelected(Menu::eventMask e) {
  if (e == Menu::exitEvent) {
    ledBlinken = 0;
    return Menu::proceed;
  }
  ledBlinken = led;
  
  Serial.print(F("subLedsSelected "));
  Serial.println(led);
  Serial.println(e);

  weiche = state.leds[led-1].weiche;
  richtung = state.leds[led-1].richtung;
  return Menu::proceed;
}
Menu::result subLedsWeicheChanged(Menu::eventMask e) {
  Serial.print(F("subLedsWeicheChanged "));
  Serial.println(weiche);
  
  state.leds[led-1].weiche = weiche;
  state.leds[led-1].richtung = richtung;
  saveData(state);
  return Menu::proceed;
}
MENU(subLeds, "LEDs einstellen", subLedsSelected, Menu::enterEvent | Menu::exitEvent, Menu::wrapStyle
  ,FIELD(led,"Nummer","",1,128,1,0,subLedsSelected,Menu::enterEvent,Menu::wrapStyle)
  ,FIELD(weiche,"Weiche","",1,16,1,0,subLedsWeicheChanged,Menu::enterEvent,Menu::wrapStyle)
  ,SUBMENU(selectRichtung)
  ,EXIT("<Zurueck")
);

byte servo = 1;
byte servoLinks = 50;
byte servoRechts = 50;
byte servoMitte = 50;
Menu::result subServoSelected(Menu::eventMask e) {
  Serial.print(F("subServoSelected "));
  Serial.println(servo);
  
  servoLinks = state.servos[servo-1].position[0];
  servoRechts = state.servos[servo-1].position[1];
  servoMitte = state.servos[servo-1].position[2];
  return Menu::proceed;
}
Menu::result subServoUpdated(Menu::eventMask e, uint8_t r, uint8_t pos) {
  if (e == Menu::exitEvent) {
    updateServos();
    return Menu::proceed;
  }

  Serial.print(F("subServoUpdated "));
  Serial.print(r);
  Serial.print(F(" "));
  Serial.println(pos);
  
  state.servos[servo-1].position[r] = pos;
  saveData(state);

  uint16_t p = uint16_t(pos) * 512 / 100;
  pwmController.setChannelPWM(servo-1, p);

  return Menu::proceed;
}
Menu::result subServoLinksUpdated(Menu::eventMask e) {
  return subServoUpdated(e, 0, servoLinks);
}
Menu::result subServoRechtsUpdated(Menu::eventMask e) {
  return subServoUpdated(e, 1, servoRechts);
}
Menu::result subServoMitteUpdated(Menu::eventMask e) {
  return subServoUpdated(e, 2, servoMitte);
}
MENU(subServos, "Servos einstellen", subServoSelected, Menu::enterEvent, Menu::wrapStyle
  ,FIELD(servo,"Nummer","",1,16,1,0,subServoSelected,Menu::enterEvent,Menu::wrapStyle)
  ,FIELD(servoLinks,"Position Links","%",0,100,1,0,subServoLinksUpdated,Menu::enterEvent | Menu::exitEvent,Menu::wrapStyle)
  ,FIELD(servoRechts,"Position Rechts","%",0,100,1,0,subServoRechtsUpdated,Menu::enterEvent | Menu::exitEvent,Menu::wrapStyle)
  ,FIELD(servoMitte,"Position Mitte","%",0,100,1,0,subServoMitteUpdated,Menu::enterEvent | Menu::exitEvent,Menu::wrapStyle)
  ,EXIT("<Zurueck")
);

extern bool servoTest;
Menu::result servosTesten(Menu::eventMask e) {
  servoTest = !servoTest;
  if (servoTest) {
    return Menu::quit;
  } else {
    updateServos();
  }
  return Menu::proceed;
}
Menu::result mainMenuEnter(Menu::eventMask e) {
  servoTest = false;
    updateServos();
  return Menu::proceed;
}
MENU(mainMenu, "Stellpult", mainMenuEnter, Menu::enterEvent, Menu::noStyle
  ,SUBMENU(subWeichen)
  ,SUBMENU(subLeds)
  ,SUBMENU(subServos)
  ,OP("Servos Testen",servosTesten,Menu::enterEvent)
  ,EXIT("<Zurueck")
);

// Display
#define I2C_ADDRESS 0x3C
#define menuFont font5x7
#define fontW 5
#define fontH 7
SSD1306AsciiWire oled;
const panel panels[] MEMMODE = {{0, 0, 128 / fontW, 64 / fontH}};
navNode* nodes[sizeof(panels) / sizeof(panel)]; // navNodes to store navigation status
panelsList pList(panels, nodes, 1); // a list of panels and nodes
idx_t tops[MAX_DEPTH] = {0, 0};
SSD1306AsciiOut outOLED(&oled, tops, pList, 8, 1+((fontH-1)>>3) ); // oled output device menu driver
menuOut* constMEM outputs[] MEMMODE = {&outOLED}; // list of output devices
outputsList out(outputs, sizeof(outputs) / sizeof(menuOut*)); // outputs list

// Input
#define encA    A8 // A1
#define encB    A9 // A0
#define encBtn  A2
encoderIn<encA,encB> encoder; // simple quad encoder driver
encoderInStream<encA,encB> encStream(encoder,4); // simple quad encoder fake Stream
keyMap encBtn_map[]={{-encBtn,defaultNavCodes[enterCmd].ch}}; // negative pin numbers use internal pull-up, this is on when low
keyIn<1> encButton(encBtn_map); // 1 is the number of keys
MENU_INPUTS(in,&encStream,&encButton);

NAVROOT(nav,mainMenu,MAX_DEPTH,in,out);

void menu_setup() {
  pinMode(encA, INPUT);
  pinMode(encB, INPUT);
  pinMode(encBtn, INPUT);  
  pinMode(LED_BUILTIN, OUTPUT);

  nav.timeOut = 180;
  
  oled.begin(&Adafruit128x64, I2C_ADDRESS);
  oled.setFont(menuFont);
  oled.clear();
  oled.setCursor(0, 0);
  oled.print(F("Stellpult"));

  delay(2000);
  
  oled.clear();
  
  encButton.begin();
  encoder.begin();
}

extern uint8_t readWeichenKey();

void menu_loop() {
  nav.poll();

  if (ledBlinken) {
    uint8_t w = readWeichenKey();
    if (w > 0 && w == weiche) {
      bool dreier = state.weichen[w-1].typ == 1;
      if (richtung == 1) {
        richtung = 2;
      } else if (richtung == 2) {
        if (dreier) {
          richtung = 4;
        } else {
          richtung = 1;
        }
      } else {
        richtung = 1;
      }
      subLedsRichtungChanged(Menu::enterEvent);
    } else if (w > 0) {
      weiche = w;
      subLedsWeicheChanged(Menu::enterEvent);

      if (state.leds[led-1].richtung == 0) {
        richtung = 1;
      }
      subLedsRichtungChanged(Menu::enterEvent);
    }
  }
}

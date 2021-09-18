#include <menu.h>
#include <SSD1306Ascii.h>
#include <SSD1306AsciiWire.h>
#include <menuIO/SSD1306AsciiOut.h>
#include <menuIO/encoderIn.h>
#include <menuIO/chainStream.h>
#include <menuIO/keyIn.h>
#include <menuIO/keyIn.h>

#include "data.h"

using namespace Menu;

#define MAX_DEPTH 2

byte led = 1;
byte weiche = 1;

byte typ = 0;
Menu::result selectTypChanged(Menu::eventMask e) {
  Serial.print(F("selectTypChanged "));
  Serial.println(typ);
  
  state.weichen[weiche-1].typ = typ;
  saveData(state);
  return Menu::proceed;
}
SELECT(typ,selectTyp,"Typ: ",selectTypChanged,Menu::updateEvent,Menu::noStyle
  ,VALUE("Doppelweiche",0,Menu::doNothing,Menu::noEvent)
  ,VALUE("Dreifachweiche",1,Menu::doNothing,Menu::noEvent)
);

Menu::result subWeichenSelected(Menu::eventMask e) {
  Serial.print(F("subWeichenSelected "));
  Serial.println(weiche);
  
  typ = state.weichen[weiche-1].typ;
  return Menu::proceed;
}
MENU(subWeichen, "Weichen einstellen", subWeichenSelected, Menu::enterEvent, Menu::wrapStyle
  ,FIELD(weiche,"Nummer","",1,16,1,1,subWeichenSelected,Menu::updateEvent,Menu::wrapStyle)
  ,SUBMENU(selectTyp)
  ,EXIT("<Zurueck")
);

byte richtung = 0;
Menu::result subLedsRichtungChanged(Menu::eventMask e) {
  Serial.print(F("subLedsRichtungChanged "));
  Serial.println(richtung);
  
  state.leds[led-1].richtung = richtung;
  saveData(state);
  return Menu::proceed;
}
SELECT(richtung,selectRichtung,"Richtung: ",subLedsRichtungChanged,Menu::updateEvent,Menu::noStyle
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
  Serial.print(F("subLedsSelected "));
  Serial.println(led);
  
  weiche = state.leds[led-1].weiche;
  richtung = state.leds[led-1].richtung;
  return Menu::proceed;
}
Menu::result subLedsWeicheChanged(Menu::eventMask e) {
  Serial.print(F("subLedsWeicheChanged "));
  Serial.println(weiche);
  
  state.leds[led-1].weiche = weiche;
  saveData(state);
  return Menu::proceed;
}
MENU(subLeds, "LEDs einstellen", subLedsSelected, Menu::enterEvent, Menu::wrapStyle
  ,FIELD(led,"Nummer","",1,128,1,1,subLedsSelected,Menu::updateEvent,Menu::wrapStyle)
  ,FIELD(weiche,"Weiche","",1,16,1,1,subLedsWeicheChanged,Menu::updateEvent,Menu::wrapStyle)
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
Menu::result subServoUpdated(Menu::eventMask e) {
  Serial.print(F("subServoUpdated "));
  Serial.print(servoLinks);
  Serial.print(F(","));
  Serial.print(servoRechts);
  Serial.print(F(","));
  Serial.println(servoMitte);
  
  state.servos[servo-1].position[0] = servoLinks;
  state.servos[servo-1].position[1] = servoRechts;
  state.servos[servo-1].position[2] = servoMitte;
  saveData(state);
  return Menu::proceed;
}
MENU(subServos, "Servos einstellen", subServoSelected, Menu::enterEvent, Menu::wrapStyle
  ,FIELD(servo,"Nummer","",1,16,1,1,subServoSelected,Menu::updateEvent,Menu::wrapStyle)
  ,FIELD(servoLinks,"Position Links","%",0,100,1,1,subServoUpdated,Menu::updateEvent,Menu::wrapStyle)
  ,FIELD(servoRechts,"Position Rechts","%",0,100,1,1,subServoUpdated,Menu::updateEvent,Menu::wrapStyle)
  ,FIELD(servoMitte,"Position Mitte","%",0,100,1,1,subServoUpdated,Menu::updateEvent,Menu::wrapStyle)
  ,EXIT("<Zurueck")
);

MENU(mainMenu, "Stellpult", Menu::doNothing, Menu::noEvent, Menu::wrapStyle
  ,SUBMENU(subWeichen)
  ,SUBMENU(subLeds)
  ,SUBMENU(subServos)
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
#define encA    A1
#define encB    A0
#define encBtn  A2
encoderIn<encA,encB> encoder;//simple quad encoder driver
encoderInStream<encA,encB> encStream(encoder,4); // simple quad encoder fake Stream
keyMap encBtn_map[]={{-encBtn,defaultNavCodes[enterCmd].ch}}; // negative pin numbers use internal pull-up, this is on when low
keyIn<1> encButton(encBtn_map); // 1 is the number of keys
MENU_INPUTS(in,&encStream,&encButton);

NAVROOT(nav,mainMenu,MAX_DEPTH,in,out);

void menu_setup() {
  oled.begin(&Adafruit128x64, I2C_ADDRESS);
  oled.setFont(menuFont);
  oled.clear();
  oled.setCursor(0, 0);
  oled.print(F("Stellpult"));

  delay(2000);
  
  oled.clear();
  
  encButton.begin();
  encoder.begin();

  pinMode(LED_BUILTIN,OUTPUT);
}

void menu_loop() {
  nav.poll();
}

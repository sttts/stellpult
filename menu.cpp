#include <menu.h>
#include <Adafruit_SSD1327.h>
#include <Adafruit_GFX.h>
//#include <Fonts/FreeSans12pt7b.h>
#include <PCA9685.h>
#include <menuIO/adafruitGfxOut.h>
#include <menuIO/chainStream.h>
#include <menuIO/keyIn.h>
#include "ht16k33.h"
#include "data.h"
#include "encoderIn.h"

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

  state.weichen[weiche - 1].typ = typ;
  saveData(state);
  return Menu::proceed;
}
SELECT(typ, selectTyp, "Typ: ", selectTypChanged, Menu::exitEvent, Menu::noStyle
       , VALUE("Doppelweiche", 0, Menu::doNothing, Menu::noEvent)
       , VALUE("Dreifachweiche", 1, Menu::doNothing, Menu::noEvent)
      );

byte anfangsstellung = 0;
Menu::result selectAnfangsstellungChanged(Menu::eventMask e) {
  Serial.print(F("selectAnfangsStellungChanged "));
  Serial.println(anfangsstellung);

  state.weichen[weiche - 1].anfangsstellung = anfangsstellung;
  saveData(state);
  return Menu::proceed;
}
SELECT(anfangsstellung, selectAnfangsstellung, "Beim Start: ", selectAnfangsstellungChanged, Menu::exitEvent, Menu::noStyle
       , VALUE("Links", 0, Menu::doNothing, Menu::noEvent)
       , VALUE("Rechts", 1, Menu::doNothing, Menu::noEvent)
       , VALUE("Mitte", 2, Menu::doNothing, Menu::noEvent)
      );

Menu::result subWeichenSelected(Menu::eventMask e) {
  Serial.print(F("subWeichenSelected "));
  Serial.println(weiche);

  typ = state.weichen[weiche - 1].typ;
  anfangsstellung = state.weichen[weiche - 1].anfangsstellung;
  return Menu::proceed;
}
MENU(subWeichen, "Weichen einstellen", subWeichenSelected, Menu::enterEvent, Menu::noStyle
     , FIELD(weiche, "Nummer", "", 1, 16, 1, 0, subWeichenSelected, Menu::enterEvent, Menu::wrapStyle)
     , SUBMENU(selectTyp)
     , SUBMENU(selectAnfangsstellung)
     , EXIT("<Zurueck")
    );

byte richtung = 0;
Menu::result subLedsRichtungChanged(Menu::eventMask e) {
  Serial.print(F("subLedsRichtungChanged "));
  Serial.println(richtung);

  state.leds[led - 1].richtung = richtung;
  saveData(state);

  // Richtung aufblinken lassen
  if (richtung == 1 || richtung == 2 || richtung == 4) {
    for (uint8_t led = 0; led < NUM_LEDS; led++) {
      uint8_t w = state.leds[led].weiche + 1;
      if (w != weiche) {
        continue;
      }
      uint8_t r = state.leds[led].richtung;
      if (state.leds[led].richtung & richtung) {
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
SELECT(richtung, selectRichtung, "Richtung: ", subLedsRichtungChanged, Menu::exitEvent, Menu::noStyle
       , VALUE("Links", 1, Menu::doNothing, Menu::noEvent)
       , VALUE("Rechts", 2, Menu::doNothing, Menu::noEvent)
       , VALUE("Mitte", 4, Menu::doNothing, Menu::noEvent)
       , VALUE("immer an", 7, Menu::doNothing, Menu::noEvent)
       , VALUE("nie an", 0, Menu::doNothing, Menu::noEvent)
       , VALUE("Links aus", 6, Menu::doNothing, Menu::noEvent)
       , VALUE("Rechts aus", 5, Menu::doNothing, Menu::noEvent)
       , VALUE("Mitte aus", 3, Menu::doNothing, Menu::noEvent)
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

  weiche = state.leds[led - 1].weiche + 1;
  richtung = state.leds[led - 1].richtung;
  return Menu::proceed;
}
Menu::result subLedsWeicheChanged(Menu::eventMask e) {
  Serial.print(F("subLedsWeicheChanged "));
  Serial.println(weiche);

  state.leds[led - 1].weiche = weiche - 1;
  state.leds[led - 1].richtung = richtung;
  saveData(state);
  return Menu::proceed;
}
MENU(subLeds, "LEDs einstellen", subLedsSelected, static_cast<Menu::eventMask>(Menu::enterEvent | Menu::exitEvent), Menu::noStyle
     , FIELD(led, "Nummer", "", 1, 128, 1, 0, subLedsSelected, Menu::enterEvent, Menu::wrapStyle)
     , FIELD(weiche, "Weiche", "", 1, 16, 1, 0, subLedsWeicheChanged, Menu::enterEvent, Menu::wrapStyle)
     , SUBMENU(selectRichtung)
     , EXIT("<Zurueck")
    );

byte servo = 1;
byte servoLinks = 50;
byte servoRechts = 50;
byte servoMitte = 50;
Menu::result subServoSelected(Menu::eventMask e) {
  Serial.print(F("subServoSelected "));
  Serial.println(servo);

  servoLinks = state.servos[servo - 1].position[0];
  servoRechts = state.servos[servo - 1].position[1];
  servoMitte = state.servos[servo - 1].position[2];
  weiche = state.servos[servo - 1].weiche + 1;
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

  state.servos[servo - 1].position[r] = pos;
  saveData(state);

  // Servos der gleichen Weiche auf gleiche Stellung bringen
  for (uint8_t peer = 0; peer < NUM_SERVOS; peer++) {
    if (state.servos[peer].weiche != state.servos[servo - 1].weiche || peer == servo - 1) {
      continue;
    }
    uint16_t p = uint16_t(state.servos[peer].position[r]) * 512 / 100;
    pwmController.setChannelPWM(peer, p);
  }

  uint16_t p = uint16_t(pos) * 512 / 100;
  pwmController.setChannelPWM(servo - 1, p);

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
Menu::result subServoWeicheSelected(Menu::eventMask e) {
  Serial.print(F("subServoWeicheSelected "));
  Serial.println(weiche);

  state.servos[servo - 1].weiche = weiche;
  saveData(state);

  return Menu::proceed;
}
MENU(subServos, "Servos einstellen", subServoSelected, Menu::enterEvent, Menu::noStyle
     , FIELD(servo, "Nummer", "", 1, 16, 1, 0, subServoSelected, Menu::enterEvent, Menu::wrapStyle)
     , FIELD(weiche, "Weiche", "", 1, 16, 1, 0, subServoWeicheSelected, Menu::enterEvent, Menu::wrapStyle)
     , FIELD(servoLinks, "Position Links", "%", 0, 100, 1, 0, subServoLinksUpdated, static_cast<Menu::eventMask>(Menu::enterEvent | Menu::exitEvent), Menu::noStyle)
     , FIELD(servoRechts, "Position Rechts", "%", 0, 100, 1, 0, subServoRechtsUpdated, static_cast<Menu::eventMask>(Menu::enterEvent | Menu::exitEvent), Menu::noStyle)
     , FIELD(servoMitte, "Position Mitte", "%", 0, 100, 1, 0, subServoMitteUpdated, static_cast<Menu::eventMask>(Menu::enterEvent | Menu::exitEvent), Menu::noStyle)
     , EXIT("<Zurueck")
    );

extern bool servoTest;
uint8_t brightness = 15;
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
  brightness = state.brightness;

  servoTest = false;
  updateServos();

  return Menu::proceed;
}
Menu::result subServoBrightnessUpdated(Menu::eventMask e) {
  state.brightness = brightness;
  saveData(state);

  HT.setBrightness(state.brightness);

  return Menu::proceed;
}
MENU(mainMenu, "Stellpult", mainMenuEnter, Menu::enterEvent, Menu::noStyle
     , SUBMENU(subWeichen)
     , SUBMENU(subLeds)
     , SUBMENU(subServos)
     , OP("Servos Testen", servosTesten, Menu::enterEvent)
     , FIELD(brightness, "Helligkeit", "", 0, 15, 1, 0, subServoBrightnessUpdated, Menu::enterEvent, Menu::noStyle)
     , EXIT("<Zurueck")
    );

// Display
//each color is in the format:
//  {{disabled normal,disabled selected},{enabled normal,enabled selected, enabled editing}}
// this is a monochromatic color table
#define BLACK 0
#define DARK 3
#define GRAY 7
#define LIGHT 11
#define WHITE 15
const colorDef<uint16_t> colors[6] MEMMODE={
  /*
  {{0,0},{0,1,1}},//bgColor
  {{1,1},{1,0,0}},//fgColor
  {{1,1},{1,0,0}},//valColor
  {{1,1},{1,0,0}},//unitColor
  {{0,1},{0,0,1}},//cursorColor
  {{1,1},{1,0,0}},//titleColor
  */
  {{BLACK,GRAY},{BLACK,LIGHT,WHITE}},//bgColor
  {{GRAY,DARK},{LIGHT,DARK,BLACK}},//fgColor
  {{GRAY,LIGHT},{WHITE,BLACK,BLACK}},//valColor
  {{GRAY,WHITE},{WHITE,BLACK,BLACK}},//unitColor
  {{BLACK,WHITE},{BLACK,BLACK,WHITE}},//cursorColor
  {{GRAY,GRAY},{WHITE,BLACK,BLACK}},//titleColor
};
#define fontX 6
#define fontY 11
#define gfxWidth 128
#define gfxHeight 128
#define I2C_ADDRESS 0x3c
#define OLED_RESET -1
Adafruit_SSD1327 gfx(128, 128, &Wire, OLED_RESET, 1000000);
MENU_OUTPUTS(out,MAX_DEPTH
  ,ADAGFX_OUT(gfx,colors,fontX,fontY,{0,0,gfxWidth/fontX,gfxHeight/fontY})
  ,NONE
);

// Input
#define encA    3 // A8 (mega)
#define encB    2 // A9 (mega)
#define encBtn  9
Encoder encoder(encA, encB); // simple quad encoder driver
encoderInStream<encA,encB> encStream(encoder, 4); // simple quad encoder fake Stream
keyMap encBtn_map[]={{-encBtn,defaultNavCodes[enterCmd].ch}}; // negative pin numbers use internal pull-up, this is on when low
keyIn<1> encButton(encBtn_map); // 1 is the number of keys
MENU_INPUTS(in,&encStream,&encButton);

NAVROOT(nav, mainMenu, MAX_DEPTH, in, out);

void menu_setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  nav.timeOut = 180;

  Wire.begin();
  gfx.begin(I2C_ADDRESS);
  gfx.clearDisplay();
  //gfx.setFont(&FreeMono9pt7b);
  gfx.setCursor(0, 0);
  gfx.print(F("Stellpult"));
  gfx.setContrast(127);
  gfx.display();

  delay(1000);

  gfx.clearDisplay();
  gfx.display();
  
  encButton.begin();
}

extern uint8_t readWeichenKey();

void menu_loop() {
  nav.doInput();
  if (nav.changed(0)) {//only draw if changed
    nav.doOutput();
    gfx.display();
  }

  if (ledBlinken) {
    uint8_t w = readWeichenKey();
    if (w > 0 && w == weiche) {
      bool dreier = state.weichen[w - 1].typ == 1;
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

      if (state.leds[led - 1].richtung == 0) {
        richtung = 1;
      }
      subLedsRichtungChanged(Menu::enterEvent);
    }
  }
}

#include <menu.h>
#include <SSD1306Ascii.h>
#include <SSD1306AsciiWire.h>
#include <menuIO/SSD1306AsciiOut.h>
#include <menuIO/encoderIn.h>
#include <menuIO/chainStream.h>
#include <menuIO/keyIn.h>
#include <menuIO/keyIn.h>

using namespace Menu;

#define MAX_DEPTH 2

int weiche = 1;
int stellungen = 2;
MENU(subWeichen, "Weichen einstellen", Menu::doNothing, Menu::noEvent, Menu::wrapStyle
  ,FIELD(weiche,"Nummer","",1,16,1,1,Menu::doNothing,Menu::noEvent,Menu::wrapStyle)
  ,FIELD(stellungen,"Stellungen","",2,3,1,1,Menu::doNothing,Menu::noEvent,Menu::wrapStyle)
  ,EXIT("<Zurueck")
);

int richtung=0;
SELECT(richtung,selRichtung,"Richtung",Menu::doNothing,Menu::noEvent,Menu::noStyle
  ,VALUE("Links",0,doNothing,noEvent)
  ,VALUE("Rechts",1,doNothing,noEvent)
  ,VALUE("Mitte",2,doNothing,noEvent)
);

int led = 1;
Menu::result ledSelected(Menu::eventMask e) {
  Serial.print("event: ");
  Serial.println(e);
  Serial.flush();
  return Menu::proceed;
}
MENU(subLeds, "LEDs einstellen", Menu::doNothing, Menu::noEvent, Menu::wrapStyle
  ,FIELD(led,"Nummer","",1,128,1,1,ledSelected,Menu::anyEvent,Menu::wrapStyle)
  ,FIELD(weiche,"Weiche","",1,16,1,1,Menu::doNothing,Menu::noEvent,Menu::wrapStyle)
  ,SUBMENU(selRichtung)
  ,EXIT("<Zurueck")
);

int servo = 1;
int servoPos = 50;
MENU(subServos, "Servos einstellen", Menu::doNothing, Menu::noEvent, Menu::wrapStyle
  ,FIELD(servo,"Nummer","",1,16,1,1,Menu::doNothing,Menu::noEvent,Menu::wrapStyle)
  ,SUBMENU(selRichtung)
  ,FIELD(stellungen,"Position","%",0,100,1,1,Menu::doNothing,Menu::noEvent,Menu::wrapStyle)
  ,EXIT("<Zurueck")
);

MENU(mainMenu, "Stellpult einstellen", Menu::doNothing, Menu::noEvent, Menu::wrapStyle
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
SSD1306AsciiOut outOLED(&oled, tops, pList, 8, 1+((fontH-1)>>3) ); //oled output device menu driver
menuOut* constMEM outputs[] MEMMODE = {&outOLED}; //list of output devices
outputsList out(outputs, sizeof(outputs) / sizeof(menuOut*)); //outputs list

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
  oled.print("Stellpult");

  delay(2000);
  
  oled.clear();
  
  encButton.begin();
  encoder.begin();

  pinMode(LED_BUILTIN,OUTPUT);
}

void menu_loop() {
  nav.poll();
}

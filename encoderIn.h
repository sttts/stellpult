/* -*- C++ -*- */
/********************
Sept. 2014 Rui Azevedo - ruihfazevedo(@rrob@)gmail.com

quadrature encoder driver (PCINT)
quadrature encoder stream (fake, not using buffers)

*/

#ifndef RSITE_ARDUINO_MENU_ENCODER
  #define RSITE_ARDUINO_MENU_ENCODER

  #include <menuDefs.h>
  
  namespace Menu {
    
    //emulate a stream based on encoderIn movement returning +/- for every 'sensivity' steps
    //buffer not needer because we have an accumulator
    class encoderInStream:public menuIn {
    public:
      int (*readFn)();
      int sensivity;
      int oldPos=0;
      encoderInStream(int (*readFn)(),int sensivity):readFn(readFn), sensivity(sensivity) {}
      inline void setSensivity(int s) {sensivity=s;}
      int available(void) {return abs(readFn()-oldPos)/sensivity;}
      int peek(void) override {
        int d=readFn()-oldPos;
        if (d<=-sensivity)return options->navCodes[downCmd].ch;
        if (d>=sensivity) return options->navCodes[upCmd].ch;
        return -1;
      }
      int read() override {
        int d=readFn()-oldPos;
        if(d>0 && oldPos<0) d+=sensivity;
        else if(d<0 && oldPos>0) d-=sensivity;
        if (d<=-sensivity) {
          oldPos-=sensivity;
          return options->navCodes[downCmd].ch;
        }
        if (d>=sensivity) {
          oldPos+=sensivity;
          return options->navCodes[upCmd].ch;
        }
        return -1;
      }
      void flush() {oldPos=readFn();}
      size_t write(uint8_t v) {oldPos=v;return 1;}
    };

    #include <menuIO/keyMapDef.h>
    #include <Adafruit_seesaw.h>

    //if you hold/repeat a key for this ammount of time we will consider it an escape
    #ifndef ESCAPE_TIME
    #define ESCAPE_TIME 1500
    #endif
    //emulate a stream keyboard, this is not using interrupts as a good driver should do
    // AND is not using a buffer either!
    template <int N>
    class seesawKeyIn:public menuIn {
    public:
      keyMap* keys;
      Adafruit_seesaw &ss;
      int lastkey;
      unsigned long pressMills=0;
      seesawKeyIn<N>(Adafruit_seesaw &ss, keyMap k[]):ss(ss), keys(k),lastkey(-1) {}
      void begin() {
        for(int n=0;n<N;n++)
          if (keys[n].pin<0) ss.pinMode(-keys[n].pin,INPUT_PULLUP);
          else ss.pinMode(keys[n].pin,INPUT);
      }
      int available(void) {
        //MENU_DEBUG_OUT<<"available"<<endl;
        int ch=peek();
        if (lastkey==-1) {
          lastkey=ch;
          pressMills=millis();
        } else if (ESCAPE_TIME&&millis()-pressMills>ESCAPE_TIME) return 1;
        if (ch==lastkey) return 0;
        return 1;
        /*int cnt=0;
        for(int n=0;n<N;n++) {
          int8_t pin=keys[n].pin;
          if (digitalRead(pin<0?-pin:pin)!=(pin<0) ) cnt++;
        }
        return cnt;*/
      }
      int peek(void) {
        //MENU_DEBUG_OUT<<"peek"<<endl;
        for(int n=0;n<N;n++) {
          int8_t pin=keys[n].pin;
          if (ss.digitalRead(pin<0?-pin:pin)!=(pin<0) ) return keys[n].code;
        }
        return -1;
      }
      int read() {
        //MENU_DEBUG_OUT<<"read"<<endl;
        int ch=peek();
        if (ch==lastkey) return -1;
        int tmp=lastkey;
        bool longPress=ESCAPE_TIME&&millis()-pressMills>ESCAPE_TIME;
        //MENU_DEBUG_OUT<<"read lastkey="<<lastkey<<" ch="<<ch<<endl;
        //MENU_DEBUG_OUT<<"down time:"<<millis()-pressMills<<endl;
        pressMills=millis();
        lastkey=ch;
        return longPress?options->getCmdChar(escCmd):tmp;//long press will result in escape
      }
      void flush() {}
      size_t write(uint8_t v) {return 0;}
    };

  }//namespace Menu
#endif

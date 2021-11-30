/* -*- C++ -*- */
/********************
Sept. 2014 Rui Azevedo - ruihfazevedo(@rrob@)gmail.com

quadrature encoder driver (PCINT)
quadrature encoder stream (fake, not using buffers)

*/

#ifndef RSITE_ARDUINO_MENU_ENCODER
  #define RSITE_ARDUINO_MENU_ENCODER

  #include <menuDefs.h>
  #include <Encoder.h>

  namespace Menu {
    //emulate a stream based on encoderIn movement returning +/- for every 'sensivity' steps
    //buffer not needer because we have an accumulator
    template<uint8_t pinA,uint8_t pinB>
    class encoderInStream:public menuIn {
    public:
      Encoder &enc;
      int sensivity;
      int oldPos=0;
      encoderInStream(Encoder &enc,int sensivity):enc(enc), sensivity(sensivity) {}
      inline void setSensivity(int s) {sensivity=s;}
      int available(void) {return abs(enc.read()-oldPos)/sensivity;}
      int peek(void) override {
        int d=enc.read()-oldPos;
        if (d<=-sensivity)return options->navCodes[downCmd].ch;
        if (d>=sensivity) return options->navCodes[upCmd].ch;
        return -1;
      }
      int read() override {
        int d=enc.read()-oldPos;
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
      void flush() {oldPos=enc.read();}
      size_t write(uint8_t v) {oldPos=v;return 1;}

    };

  }//namespace Menu
#endif

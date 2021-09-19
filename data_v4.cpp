#import "data.h"
#import "data_v4.h"

void convertV4(StateV4 &in, State &out) {
  out.version = 5;
  memcpy(out.weichen, in.weichen, sizeof(in.weichen));
  memcpy(out.leds, in.leds, sizeof(in.leds));
  for (int s = 0; s < NUM_SERVOS; s++) {
    memcpy(&out.servos[s], &in.servos[s], sizeof(in.servos[s]));
    out.servos[s].weiche = s;
  }
  out.brightness = in.brightness;
}

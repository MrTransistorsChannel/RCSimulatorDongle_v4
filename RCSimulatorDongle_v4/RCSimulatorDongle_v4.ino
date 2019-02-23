#include <SoftwareSerial.h>
#include <FlySkyIBus.h>
#include <PPMIn.h>
#include <Timer1.h>

#define NUM_CHANNELS 6

uint16_t ppm_chans[NUM_CHANNELS];
uint8_t  internals[PPMIN_WORK_SIZE(NUM_CHANNELS)];

SoftwareSerial ibus(2, 3);
FlySkyIBus IBus;
rc::PPMIn PPM(ppm_chans, internals, NUM_CHANNELS);

byte values[NUM_CHANNELS + 1];

boolean isIBus;

uint64_t time;

void setup() {
  pinMode(13, OUTPUT);
  Serial.begin(19200);
  values[0] = 0xFF;

  ibus.begin(115200);
  delay(10);

  if (ibus.available()) {
    IBus.begin(ibus);
    delay(100);
    IBus.loop();
    if (IBus.readChannel(0) > 0) isIBus = true;
    else {
      ibus.end();
      rc::Timer1::init();
      pinMode(2, INPUT);
      EICRA = (1 << ISC00);
      EIMSK = (1 << INT0);
      PPM.setTimeout(1000);
      PPM.start();
      isIBus = false;
    }
  }
  else {
    ibus.end();
    rc::Timer1::init();
    pinMode(2, INPUT);
    EICRA = (1 << ISC00);
    EIMSK = (1 << INT0);
    PPM.setTimeout(1000);
    PPM.start();
    isIBus = false;
  }

}

void loop() {
  if (isIBus) {
    IBus.loop();
    for (int i = 0; i < NUM_CHANNELS; i++) values[i + 1] = map(IBus.readChannel(i), 1000, 2000, 0, 254);
  }
  else {
    PPM.update();
    if (PPM.isStable()) {
      for (int i = 0; i < NUM_CHANNELS; i++) values[i + 1] = map(ppm_chans[i], 1000, 2000, 0, 254);
    }
  }

  if (millis() - time > 10) {
    Serial.write(values, 7);
    time = millis();
  }

}

// ОБРАБОТЧИК ПРЕРЫВАНИЯ

ISR(INT0_vect) {
  PPM.pinChanged(PIND & (1 << 2));
}

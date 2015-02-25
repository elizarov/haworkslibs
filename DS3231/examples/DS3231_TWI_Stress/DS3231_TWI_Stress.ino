/*
  Sample sketch for DS3231 library that stress tests TWIMaster by using watchdog to 
  reset MCU at random points in time and checking if TWI can still continue operation
  despite a slave DS3231 device that is pontentially holding down data line.
  
  This tests shows that this indeed happens, but ATmega TWI hardware correctly handles
  this case on its own when asked to send bus start condition.
  
  Author: Roman Elizarov
*/

#include <TWIMaster.h>
#include <Timeout.h>
#include <FixNum.h>
#include <DS3231.h>

#include <avr/wdt.h>
#include <util/delay.h>

const uint8_t RANDOM_PIN = A0; // must not be connected
const uint8_t LED_PIN = 13;

const uint8_t SDA_PIN = A4;
const uint8_t SCL_PIN = A5;

DS3231 rtc;
Timeout reportTimeout(1000);
int ok;
int fail;
bool wdtEnabled;

void printPinState() {
  Serial.print("SCL = "); Serial.println(digitalRead(SCL_PIN) ? "HIGH" : "LOW");
  Serial.print("SDA = "); Serial.println(digitalRead(SDA_PIN) ? "HIGH" : "LOW");
}

void setup () {
  Serial.begin(57600);
  Serial.println("=== DS3231 TWI Stress ===");
  printPinState(); // print state before 
  TWIMaster.begin(TWI_10K); // set low speed to increase chance to catch TWI in bad state
  pinMode(LED_PIN, OUTPUT);
}

void loop () {
  DateTime dt = rtc.now();
  if (dt) {
    ok++;
    if (wdtEnabled && (ok & 0x07) == 0)
      digitalWrite(LED_PIN, (ok >> 3) & 1);
  } else
    fail++;
  if (reportTimeout.check()) {
    reportTimeout.reset(1000);
    Serial.print("Now "); 
    Serial.print(dt.format());
    Serial.print(" ok="); Serial.print(ok);
    Serial.print(" / fail="); Serial.println(fail);
    printPinState();
    if (ok > 0 && fail == 0) {
      // gather some randomness
      int rnd = 0;
      for (int i = 0; i < 10; i++) {
        rnd *= 2339;
        rnd += analogRead(RANDOM_PIN);
      }
      // tuncate to 15 bits
      rnd &= 0x7fff;
      Serial.print("Rnd delay = "); 
      Serial.print(rnd);
      Serial.println(" x10 us");
      // initialize watchdot to reset in 500ms
      wdtEnabled = true;
      wdt_enable(WDTO_500MS);
      // wait random time
      for (int i = 0; i < rnd; i++)
        _delay_us(10);
    }
    ok = 0;
    fail = 0;
  }  
}

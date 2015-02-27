#include <FixNum.h>
#include <FmtRef.h>
#include <string.h>

char* buf = "[TEST:? a???? b??.?? c+??.? ??% d00+00.000](v?x?)z-?";

FmtRef test(buf);
FmtRef a(buf, 'a');
FmtRef b(buf, 'b');
FmtRef c(buf, 'c');
FmtRef c1(c);
FmtRef d(buf, 'd');
FmtRef d1(d);
FmtRef v(buf, 'v');
FmtRef x(buf, 'x');
FmtRef z(buf, 'z');

void setup() {
  Serial.begin(57600);
  long start = millis();
  // do 1000 times to measure time in micros
  for (int i = 0; i < 1000; i++) {
    test = 1; 
    a = 1234;
    b = fixnum16_2(1234);
    c = 5;
    c1 = fixnum16_1(843);
    d = 3;
    d1 = fixnum16_3(-3145);
    v = 6;
    x = 154; 
    z = -4;
  }
  long stop = millis();
  Serial.println(buf);
  if (strcmp(buf, "[TEST:1 a1234 b12.34 c+5.0  84% d03-03.145](v6x9)z-4") == 0)
    Serial.println(F("TEST PASSED"));
  else
    Serial.println(F("TEST FAILED"));
  Serial.print("Time = ");
  Serial.print(stop - start, DEC);
  Serial.println(" us");  
}

void loop() {
  // nothing here
}

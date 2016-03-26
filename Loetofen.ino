#include "max6675.h"

int thermoDO = 4;
int thermoCS = 5;
int thermoCLK = 6;

MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);

byte incoming;

int out = 2;
int phase = 1;
uint32_t haltezeit, reflowZeit;

bool reflow = false;

void setup() {
  Serial.begin(9600);
  // use Arduino pins
  pinMode(out, OUTPUT); digitalWrite(out, LOW);
  // wait for MAX chip to stabilize
  delay(500);
}

void loop() {

  if (Serial.available() > 0) {     //PWM-Wert einlesen
    // read the incoming byte:
    incoming = Serial.parseInt();

    // say what you got:
    Serial.print("I received: ");
    Serial.println(incoming, DEC);
  }

  if (incoming >= 42) {
    reflow = true;
    incoming = 0;
  }

  Serial.print("C = ");       //Sensor auslesen
  Serial.println(thermocouple.readCelsius());
  delay(200);

  if (reflow == true) {
    routine(225, 160, 100, 240, 255, 215, 50.0);
  }
}

void routine(byte pwm1, int tmp1, byte pwm2, int time2, byte pwm3, int tmp3, float tmpcool) {
  while (reflow == true) {

    Serial.print("Phase = ");
    Serial.print(phase);
    Serial.print(" C = ");       //Sensor auslesen
    Serial.println(thermocouple.readCelsius());

    switch (phase) {
      case 1:

        digitalWrite(out, HIGH);    //PWM1 Aufheizen
        delay(pwm1);
        digitalWrite(out, LOW);
        delay(255 - pwm1);

        if (thermocouple.readCelsius() >= tmp1 - 10) { //Weiterschalten -10C wegen ueberschwingern
          phase = 2;
          haltezeit = millis() / 1000 + time2;
          Serial.print(millis() / 1000);
          Serial.print(" ");
          Serial.println(haltezeit);
        }

        break;
      case 2:

        digitalWrite(out, HIGH);    //PWM2 halten
        delay(pwm2);
        digitalWrite(out, LOW);
        delay(255 - pwm2);

        if (millis() / 1000 >= haltezeit) {
          phase = 3;
        }

        break;
      case 3:
        digitalWrite(out, HIGH);    //PWM3 heizen
        delay(pwm3);
        digitalWrite(out, LOW);
        delay(255 - pwm3);
        if (thermocouple.readCelsius() > tmp3) {   //Hochheizen!
          phase = 4;
        }
        break;
      case 4:
        digitalWrite(out, LOW);
        delay(200);
        if (thermocouple.readCelsius() <= tmpcool) { //abkühlen
          reflow = false;
        }
        break;
    }
  }
  phase = 1;
}

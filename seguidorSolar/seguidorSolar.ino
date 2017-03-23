#include "SeguidorSolar.h"

#define LDR1 A0
#define LDR2 A1
#define POT1 A4
#define FDC1 2
#define FDC2 3
#define ENA 6
#define IN1 9
#define IN2 10
#define ENB 13
#define IN3 11
#define IN4 12
const long interval = 250;


//Inicia controlador do seguidor solar
SeguidorSolar seguidor;

void setup () {
  Serial.begin(9600);
  PonteH ponte;
  ponte.left1 = IN1;
  ponte.left2 = IN2;
  ponte.right1 = IN3;
  ponte.right2 = IN4;
  ponte.left_en = ENA;
  ponte.right_en = ENB;

  seguidor.init(ponte, POT1, LDR1, LDR2, FDC1, FDC2);
  seguidor.autoVerificacao();
}

void loop () {
  seguidor.autoVerificacao();
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();


  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    //      if (sentido == 1) {
    //        sentido = 2;
    //        motor.control(sentido, 255, 0, 0);
    //        Serial.println(lerLdr(LDR1));
    //      } else {
    //        sentido = 1;
    //        motor.control(sentido, 255, 0, 0);
    //        Serial.println(lerLdr(LDR2));
    //      }
    Serial.println(lerLdr(LDR1));
  }
}

unsigned char lerLdr(unsigned char ldr) {
  return map(analogRead(ldr), 0, 900, 0, 100);
}


void autoVerificacao() {

}


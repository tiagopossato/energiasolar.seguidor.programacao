/**
   TODO:
   1: Criar método para procurar o ponto com maior incidência solar.
      Percorrer todo o trajeto, após a autoVerificação e armazenar a
      posição do eixo onde existir a menor diferença entre os sensores
      e ao mesmo tempo o maior índice de incidência de luz.
*/
#define DEBUG
#include "SeguidorSolar.h"
#define LDR1 A2
#define LDR2 A3
#define POT1 A4
#define FDC1 5
#define FDC2 4
#define FDC3 3
#define FDC4 2
#define IN1 13
#define IN2 12
#define IN3 11
#define IN4 10
#define ENA 9
#define ENB 6

const long interval = 10;

/**********CRIAÇÃO DOS OBJETOS E DO EIXO**********/
//cria objeto do motor do eixo que acompanha o sol diariamente
Motor motorDiario;
//cria objeto para o potenciometro do eixo que acompanha o sol diariamente
Potenciometro potDiario;
//cria os sensores do eixo diario
Sensores senDiario;
//cria o eixo que acompanha o sol diariamente
Eixo eixoDiario;
/***************************************************/

/**********CONTROLADOR DE PLACA SOLAR**********/
SeguidorSolar seguidor;

void setup () {
  //inicia
#if defined(DEBUG)
  Serial.begin(9600);
#endif
  /**********INSERE OS PARÂMETROS DOS OBEJTOS**********/
  motorDiario.direita = IN1;
  motorDiario.esquerda = IN2;
  motorDiario.habilita = ENA;
  potDiario.pino = POT1;
  senDiario.fdc1 = FDC1;
  senDiario.fdc2 = FDC2;
  senDiario.ldr1 = LDR1;
  senDiario.ldr2 = LDR2;
  eixoDiario.motor = &motorDiario;
  eixoDiario.pot = &potDiario;
  eixoDiario.pot->minimo = 0;
  eixoDiario.pot->maximo = 1023;
  eixoDiario.sensores = &senDiario;
  /***************************************************/

  /**********INICIA O EIXO**********/
  seguidor.iniciaEixo(&eixoDiario);
  //seguidor.autoVerificacao(&eixoDiario);
}

void loop () {
  static int pos;
  static unsigned long previousMillis = 0;
  static unsigned long previousMillis2 = 0;
  unsigned long currentMillis = millis();
#if defined(DEBUG)
  if (Serial.available()) {
    // set the brightness of the LED:
    pos = Serial.parseInt();
    seguidor.moveParaPosicao(&eixoDiario, pos);
  }
#endif

  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    seguidor.segueLuz(&eixoDiario);
    //Serial.println(analogRead(LDR1)-analogRead(LDR2));

  }
  
#if defined(DEBUG)
  if (currentMillis - previousMillis2 >= 1000) {
    // save the last time you blinked the LED
    previousMillis2 = currentMillis;
    Serial.print("LDR1: ");
    Serial.println(analogRead(LDR1));
    Serial.print("LDR2: ");
    Serial.println(analogRead(LDR2));
    seguidor.mostraPotenciometro(&eixoDiario);
  }
#endif
}

unsigned char lerLdr(unsigned char ldr) {
  return map(analogRead(ldr), 0, 900, 0, 100);
}

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

/**********CRIAÇÃO DOS OBJETOS**********/
//cria o eixo que acompanha o sol diariamente
Eixo eixoDiario;
/***************************************************/
/**********CONTROLADOR DE PLACA SOLAR**********/
SeguidorSolar seguidor;

void setup () {
  //inicia
  Serial.begin(9600);
  /**********INSERE OS PARÂMETROS DOS OBEJTOS**********/
  eixoDiario.motor.direita = IN1;
  eixoDiario.motor.esquerda = IN2;
  eixoDiario.motor.habilita = ENA;
  eixoDiario.pot.pino = POT1;
  eixoDiario.pot.minimo = 100;
  eixoDiario.pot.maximo = 687;
  eixoDiario.sensores.fdc1 = FDC1;
  eixoDiario.sensores.fdc2 = FDC2;
  eixoDiario.sensores.ldr1 = LDR1;
  eixoDiario.sensores.ldr2 = LDR2;
  /***************************************************/

  /**********INICIA O EIXO**********/
  seguidor.iniciaEixo(&eixoDiario);
  //seguidor.autoVerificacao(&eixoDiario);
}

void loop () {
  static uint8_t pos;
  static uint8_t lastPos;
  static unsigned long previousMillis = 0;
  static unsigned long previousMillis2 = 0;
  unsigned long currentMillis = millis();

  if (Serial.available()) {
    pos = Serial.parseInt();
    if (abs(lastPos - pos) >= 5) {
      seguidor.moveParaPosicao(&eixoDiario, pos);
      eixoDiario.posicao.minima = eixoDiario.posicao.atual - 5;
      eixoDiario.posicao.maxima = eixoDiario.posicao.atual + 5;
      lastPos = pos;
    }
  }

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    seguidor.segueLuz(&eixoDiario);
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

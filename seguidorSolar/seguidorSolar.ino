/**
   TODO:
   1: Criar método para procurar o ponto com maior incidência solar.
      Percorrer todo o trajeto, após a autoVerificação e armazenar a
      posição do eixo onde existir a menor diferença entre os sensores
      e ao mesmo tempo o maior índice de incidência de luz.
*/

#define DEBUG
#include "util.h"
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
uint8_t ultimaPos;
boolean seguirLuz = false;

/**********CRIAÇÃO DOS OBJETOS**********/
//cria o eixo que acompanha o sol diariamente
Eixo eixoDiario;
/***************************************************/
/**********CONTROLADOR DE PLACA SOLAR**********/
SeguidorSolar seguidor;

char input[64];

void setup () {
  input[0] = '\0';
  //inicia
  Serial.begin(9600);
  /**********INSERE OS PARÂMETROS DOS OBEJTOS**********/
  eixoDiario.motor.direita = IN1;
  eixoDiario.motor.esquerda = IN2;
  eixoDiario.motor.habilita = ENA;
  eixoDiario.pot.pino = POT1;
  eixoDiario.pot.minimo = 110;//110
  eixoDiario.pot.maximo = 697;//697
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
  static unsigned long previousMillis = 0;
  static unsigned long previousMillis2 = 0;
  unsigned long currentMillis = millis();

  if (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    if (inChar != '\n') {
      sprintf(input, "%s%c", input, inChar);
    }
    if (inChar == '\n' || strlen(input) == 63) {
      if (strlen(input) >= 1) {
        if (sanitizaEntrada(input)) {
          trataComando(input);
        }
      }
      for (uint8_t i = 0; i < 64; i++) {
        input[i] = '\0';
      }
    }
  }

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    if (seguirLuz) {
      seguidor.segueLuz(&eixoDiario);
    }
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

/**
   Trata os comandos recebidos via porta serial
   LISTA DE COMANDOS
   1/[posicao] -> Mova o painel para a posicao (em porcentagem). Ex.: 1:50
   2/[dia]/[segundos] -> informa para a placa:
                                            O dia atual, a de 1 a 365, a partir de 1º de janeiro.
                                            O segundo atual a partir da meia-noite
*/
void trataComando(char *comando) {
  uint16_t dia;
  uint32_t segundo;
  uint8_t posicao;
  int8_t tmp;
  switch (extraiCodigo(comando)) {
    case 1: {
        posicao = (uint8_t)extraiCodigo(comando);
        //arduino-1.8.2/reference/www.arduino.cc/en/Reference/Abs.html
        tmp = ultimaPos - posicao;
        if (abs(tmp) >= 3) {
          seguidor.moveParaPosicao(&eixoDiario, posicao);
          eixoDiario.posicao.minima = eixoDiario.posicao.atual - 5;
          eixoDiario.posicao.maxima = eixoDiario.posicao.atual + 5;
          ultimaPos = posicao;
        }
        break;
      }
    case 2: {
        dia = extraiCodigo(comando);
        segundo = extraiCodigo(comando);
        /*
           Não segue o sol entre as 20:00(72000) e as 6:00(21600)
        */
        if (segundo < 21600 || segundo > 72000) {
          seguirLuz = false;
          return;
        }
        seguirLuz = true;
        posicao = (uint8_t)calculaPosicao(dia, segundo);
        tmp = ultimaPos - posicao;
        if (posicao >= 0) {
          if (abs(tmp) >= 3) {
            seguidor.moveParaPosicao(&eixoDiario, posicao);
            eixoDiario.posicao.minima = eixoDiario.posicao.atual - 5;
            eixoDiario.posicao.maxima = eixoDiario.posicao.atual + 5;
            ultimaPos = posicao;
          }
        }
        break;
      }
    default:
      break;
  }
}

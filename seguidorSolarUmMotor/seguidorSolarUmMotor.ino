/**
   TODO:
   1: Criar método para procurar o ponto com maior incidência solar.
      Percorrer todo o trajeto, após a autoVerificação e armazenar a
      posição do eixo onde existir a menor diferença entre os sensores
      e ao mesmo tempo o maior índice de incidência de luz.
*/
#include <Wire.h>
#include <Time.h>
#ifdef  _WIN32
#include <TimeLib.h>
#endif
#include <DS1307RTC.h>

#define DEBUG

#include "util.h"
#include "./SeguidorSolar/SeguidorSolar.h"
#define LDR1 A1
#define LDR2 A0
#define POT1 A2
#define FDC1 7
#define FDC2 8
#define IN1 4
#define IN2 5
#define ENA 6

const long interval = 1000;
uint8_t ultimaPos;
boolean seguirLuz = false;
String saida;

/**********CRIAÇÃO DOS OBJETOS**********/
//cria o eixo que acompanha o sol diariamente
Eixo eixo;
/***************************************************/
/**********CONTROLADOR DE PLACA SOLAR**********/
SeguidorSolar seguidor;

char input[64];

void setup () {
  input[0] = '\0';
  saida.reserve(64);

  //inicia
  Serial.begin(9600);
  /**********INSERE OS PARÂMETROS DOS OBEJTOS**********/
  eixo.motor.direita = IN1;
  eixo.motor.esquerda = IN2;
  eixo.motor.habilita = ENA;
  eixo.pot.pino = POT1;
  eixo.pot.minimo = 200;//200
  eixo.pot.maximo = 670;//670
  eixo.sensores.fdc1 = FDC1;
  eixo.sensores.fdc2 = FDC2;
  eixo.sensores.ldr1 = LDR1;
  eixo.sensores.ldr2 = LDR2;
  eixo.posicao.minima = 0;
  eixo.posicao.maxima = 100;
  /***************************************************/

  /**********INICIA O EIXO**********/
  seguidor.iniciaEixo(&eixo);
  //seguidor.autoVerificacao(&eixo);
}

void loop () {
  static unsigned long previousMillis = 0;
  static unsigned long previousMillis2 = 0;
  unsigned long currentMillis = millis();
  tmElements_t tm;
  uint32_t segundo = 0;
  uint16_t dia = 0;
  uint8_t posicao;
  int8_t tmp;

  if (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    if (inChar != '\n') {
      sprintf(input, "%s%c", input, inChar);
    }
    if (inChar == '\n' || strlen(input) == 63) {
      if (strlen(input) >= 1) {
        if (sanitizaEntrada(input)) {
          Serial.println(input);
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

    //Leitura da hora
    if (RTC.read(tm)) {
      saida = tm.Day;
      saida += '/';
      saida += tm.Month;
      saida += '/';
      saida += tmYearToCalendar(tm.Year);
      saida += ' ';
      saida += tm.Hour;
      saida += ':';
      saida += tm.Minute;
      saida += ':';
      saida += tm.Second;

      dia = diaDoAno(&tm);
      segundo = segundoAtual(&tm);

      //      String posicao = "[2/";
      //      posicao += dia;
      //      posicao += "/";
      //      posicao += segundo;
      //      posicao += "]";
      Serial.println(saida);
    }

    //Movimenta o eixo para a posicao conforme data e hora
    /*
       Não segue o sol entre as 20:00(72000) e as 6:00(21600)
    */
    //    if (segundo < 21600 || segundo > 72000) {
    //      seguirLuz = false;
    //      return;
    //    }
    //    seguirLuz = true;
    //    posicao = (uint8_t)calculaPosicao(dia, segundo);
    //    tmp = ultimaPos - posicao;
    //    if (posicao >= 0 && posicao <= 100) {
    //      if (abs(tmp) >= 3) {
    //        seguidor.moveParaPosicao(&eixo, posicao);
    //        eixo.posicao.minima = eixo.posicao.atual - 5;
    //        eixo.posicao.maxima = eixo.posicao.atual + 5;
    //        ultimaPos = posicao;
    //      }
    //    }
  }
  //Segue luz pelos sensores
  //    if (seguirLuz) {
  seguidor.segueLuz(&eixo);
  //    }
  //Fim da movimentacao do eixo

#if defined(DEBUG)
  if (currentMillis - previousMillis2 >= 1000) {
    // save the last time you blinked the LED
    previousMillis2 = currentMillis;
    uint16_t ldr1 = 0;
    for (char i = 0; i < 10; i++) {
      ldr1 += analogRead(LDR1);
      delay(1);
    }
    uint16_t ldr2 = 0;
    for (char i = 0; i < 10; i++) {
      ldr2 += analogRead(LDR2);
      delay(1);
    }
    Serial.print("LDR1: ");
    Serial.println(ldr1 / 10);
    Serial.print("LDR2: ");
    Serial.println(ldr2 / 10);
    seguidor.mostraPotenciometro(&eixo);
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
    // Move para posição desejada
    case 1: {
        posicao = (uint8_t)extraiCodigo(comando);
        //arduino-1.8.2/reference/www.arduino.cc/en/Reference/Abs.html
        tmp = ultimaPos - posicao;
        //if (abs(tmp) >= 3) {
        seguidor.moveParaPosicao(&eixo, posicao);
        // eixo.posicao.minima = eixo.posicao.atual - 5;
        //eixo.posicao.maxima = eixo.posicao.atual + 5;
        ultimaPos = posicao;
        //}
        break;
      }
    //Recebe data e hora
    //    case 2: {
    //        break;
    //      }
    default:
      break;
  }
}

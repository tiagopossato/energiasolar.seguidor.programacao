/**
   TODO:
   1: Criar método para procurar o ponto com maior incidência solar.
      Percorrer todo o trajeto, após a autoVerificação e armazenar a
      posição do eixo onde existir a menor diferença entre os sensores
      e ao mesmo tempo o maior índice de incidência de luz.
*/
#include <EEPROM.h>
#include "interface.h"
#include "posicionamento.h"
#include "Eixo.h"

//-------DEFINIÇÕES DOS PINOS---------//
#define LDR1 A2
#define LDR2 A3
#define POT1 A4
#define FDC1 5
#define FDC2 4
#define IN1 13
#define IN2 12
#define ENA 9

const long controlInterval = 10;

/**
 * DIA E HORA PARA O CONTROLE POR TEMPO
 * Essas variáveis devem ser preenchidas 
 * pela leitura do relógio ou através da porta Serial
 */
uint16_t dia;
uint32_t segundo;

/**********CRIAÇÃO DOS OBJETOS**********/
//cria o eixo que acompanha o sol diariamente
Eixo eixoDiario(0x00);
/***************************************************/
char input[24];

void setup()
{
  input[0] = '\0';
  //inicia
  Serial.begin(9600);
  /**********INSERE OS PARÂMETROS DOS OBJETOS**********/
  eixoDiario.motor.rightPin = IN1;
  eixoDiario.motor.leftPin = IN2;
  eixoDiario.motor.enablePin = ENA;
  eixoDiario.position.pin = POT1;
  eixoDiario.position.potMin = 110;
  eixoDiario.position.potMax = 697;
  eixoDiario.safetySensors.sunrise.pin = FDC1;
  eixoDiario.safetySensors.sunset.pin = FDC2;
  eixoDiario.lightSensors.sunrise.pin = LDR1;
  eixoDiario.lightSensors.sunset.pin = LDR2;
  /***************************************************/
  /**********INICIA O EIXO**********/
  eixoDiario.begin();
  //eixoDiario.calibratePosition();
}

void loop()
{
  static unsigned long previousControlTime = 0;
  static unsigned long previousPrintTime = 0;

  unsigned long currentMillis = millis();
  int8_t posicao;

  if (currentMillis - previousControlTime >= controlInterval)
  {
    previousControlTime = currentMillis;
    eixoDiario.readSensors();
    switch (eixoDiario.getControlMode())
    {
    case TIMECONTROL:
      posicao = calculaPosicaoDiaria(dia, segundo);
      if (posicao >= 0)
      {
        //movimenta em passos de 3%
        if (abs(eixoDiario.setPoint - posicao) >= 3)
        {
          eixoDiario.setPoint = posicao;
          eixoDiario.moveTo(posicao);
          eixoDiario.positionMin = eixoDiario.position.value - 5;
          eixoDiario.positionMax = eixoDiario.position.value + 5;
        }
      }
      break;
    case SOLARTRACKER:
      eixoDiario.solarTracker();
      break;
    case HYBRID:
      //faz alguma coisa
      break;
    default:
      break;
    }
  }

  if (currentMillis - previousPrintTime >= 1000)
  {
    previousPrintTime = currentMillis;
    eixoDiario.printStatus();
  }
}

/**
  SerialEvent occurs whenever a new data comes in the hardware serial RX. This
  routine is run between each time loop() runs, so using delay inside loop can
  delay response. Multiple bytes of data may be available.
*/
void serialEvent()
{
  while (Serial.available())
  {
    char inChar = (char)Serial.read();
    if (inChar != '\n')
    {
      sprintf(input, "%s%c", input, inChar);
    }
    if (inChar == '\n' || strlen(input) == 23)
    {
      if (strlen(input) >= 1)
      {
        trataComando(input);
      }
      for (uint8_t i = 0; i < 24; i++)
      {
        input[i] = '\0';
      }
    }
  }
}

/**
   Trata os comandos recebidos via porta serial
   LISTA DE COMANDOS
   {1/[eixo]/[posicao]} -> Mova o painel para a posicao (em porcentagem). 
                    Ex.: {1:50}
   {2/[dia]/[segundos]} -> informa para a placa:
                          O dia atual, a de 1 a 365, a partir de 1º de janeiro.
                          O segundo atual a partir da meia-noite
                          Ex.: {2/20/36000}
                          (10 horas da manhã do dia 20 de junho)
   {3/[eixo]/[controlMode]} -> Informa para a aplicação o modo de controle:
                        1: MANUAL
                        2: SOLARTRACKER
                        3: TIMECONTROL
                        Ex.: {3/2}
                        (Eixo passa a rastrear o sol)
*/
void trataComando(char *comando)
{
  int8_t posicao;
  uint8_t eixo;

  if (!sanitizaEntrada(comando))
    return;

  switch (extraiCodigo(comando))
  {
  case 1:
    eixo = extraiCodigo(comando);
    posicao = extraiCodigo(comando);
    if (eixo == 1 && eixoDiario.getControlMode() == MANUALMODE)
    {
      if (abs(eixoDiario.setPoint - posicao) >= 3)
      {
        eixoDiario.setPoint = posicao;
        eixoDiario.moveTo(posicao);
        eixoDiario.positionMin = eixoDiario.position.value - 5;
        eixoDiario.positionMax = eixoDiario.position.value + 5;
      }
    }
    if (eixo == 2 /*&& ????????.getControlMode() == MANUALMODE*/)
    {
      //um possível segundo eixo
    }
    break;
  case 2:
    dia = extraiCodigo(comando);
    segundo = extraiCodigo(comando);
    break;
  case 3:
    eixo = extraiCodigo(comando);
    if (eixo == 1)
    {
      eixoDiario.setControlMode(extraiCodigo(comando));
    }
    if (eixo == 2)
    {
      //um possível segundo eixo
    }
    break;
  default:
    break;
  }
}

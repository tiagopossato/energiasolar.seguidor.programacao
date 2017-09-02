#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include "SeguidorSolar.h"
#include <PID_v1.h>
#define DEBUG
/**
   Inicia um eixo, configurando os pinos de entrada e saída
*/
void SeguidorSolar::iniciaEixo(Eixo *eixo) {
  eixo->motor.direcao = true;
  eixo->motor.velocidade = 0;
  pinMode(eixo->motor.direita, OUTPUT);
  pinMode(eixo->motor.esquerda, OUTPUT);
  pinMode(eixo->motor.habilita, OUTPUT);

  pinMode(eixo->sensores.fdc1, INPUT);
  pinMode(eixo->sensores.fdc2, INPUT);

}

/**
   Método que executa uma auto verificação no exito, com os seguintes passos:
   Vira o painel para o oeste até chegar no fim de curso
   Verifica o valor no potenciômetro e armazena como valor máximo
   Vira o painel para leste até chegar no fim de curso
   Verifica o valor no potenciômetro e armazena como valor mínimo
*/
void SeguidorSolar::autoVerificacao(Eixo *eixo) {
#if defined(DEBUG)
  Serial.println("autoVerificacao");
#endif
  while (digitalRead(eixo->sensores.fdc1)) {
    this->controlaMotor(eixo, 255);
  }
  eixo->pot.maximo = analogRead(eixo->pot.pino);

  while (digitalRead(eixo->sensores.fdc2)) {
    this->controlaMotor(eixo, -255);
  }
  eixo->pot.minimo = analogRead(eixo->pot.pino);
  this->paraMotor(&eixo->motor);

  this->mostraPotenciometro(eixo);
}

/**
   Move o motor do eixo para a posicao desejada
   Recebe como parâmetro um eixo e a nova posicao em %
*/
void SeguidorSolar::moveParaPosicao(Eixo *eixo, uint8_t novaPosicao) {
  if (novaPosicao > 100)novaPosicao = 100;
  uint8_t lastMin = eixo->posicao.minima;
  uint8_t lastMax = eixo->posicao.maxima;

  Serial.print("Move de: ");
  Serial.print(eixo->posicao.atual);
  Serial.print("% Para: ");
  Serial.print(novaPosicao);
  Serial.println("%");

  if (novaPosicao > eixo->posicao.atual) {
    eixo->posicao.maxima = novaPosicao;
    eixo->posicao.minima = eixo->posicao.atual;
  } else {
    eixo->posicao.minima = novaPosicao;
    eixo->posicao.maxima = eixo->posicao.atual;
  }

  unsigned long previousMillis = millis();
  //  unsigned long currentMillis = millis();
  static uint16_t timeOut = 20000;
  double mySetpoint = 0;
  double myInput = 0;
  double myOutput = 0;
  PID myPID(&myInput, &myOutput, &mySetpoint, 2, 5, 2, DIRECT);
  myPID.SetOutputLimits(-255, 255);
  //turn the PID on
  myPID.SetMode(AUTOMATIC);

  this->lePotenciometro(eixo);
  myInput = eixo->posicao.atual;
  mySetpoint = novaPosicao;

  while (eixo->posicao.atual != novaPosicao) {
    if (millis() - previousMillis >= timeOut) {
#if defined(DEBUG)
      Serial.println("timeOut");
#endif
      break;
    }
    this->lePotenciometro(eixo);
    myInput = eixo->posicao.atual;
    myPID.Compute();
    if (myOutput < 0 && myOutput > -75)myOutput = -75;
    if (myOutput > 0 && myOutput < 75 )myOutput = 75;
    this->controlaMotor(eixo, myOutput);
  }

  this->paraMotor(&eixo->motor);
  this->mostraPotenciometro(eixo);
  eixo->posicao.minima = lastMin;
  eixo->posicao.maxima = lastMax;
}

/*
   Realiza a leitura e o tratamento do sinal do potenciometro e
   salva o valor na posicao do eixo recebido como parâmetro
*/
void SeguidorSolar::lePotenciometro(Eixo *eixo) {
  int soma = 0;
  for (char i = 0; i < 10; i++) {
    soma += analogRead(eixo->pot.pino);
    delay(1);
  }
  eixo->posicao.atual = map(soma / 10, eixo->pot.minimo, eixo->pot.maximo, 0, 100);
}

void SeguidorSolar::mostraPotenciometro(Eixo *eixo) {
  this->lePotenciometro(eixo);
#if defined(DEBUG)
  Serial.print("Posicao atual: ");
  Serial.print(eixo->posicao.atual);
  Serial.println("%");
#endif
  Serial.print("[1/");
  Serial.print(eixo->posicao.atual);
  Serial.println("]");
}

void SeguidorSolar::segueLuz(Eixo *eixo) {
  static unsigned long lastStop;
  int16_t velocidade = analogRead(eixo->sensores.ldr1) - analogRead(eixo->sensores.ldr2);
  if (abs(velocidade) < 10) {
    this->paraMotor(&eixo->motor);
    lastStop = millis();
    return;
  }
  //Verifica se passou cinco segundos apos a ultima vez que parou o motor
  if (millis() - lastStop < 5000) {
    return;
  }

  if (velocidade > 0) {
    velocidade = velocidade + 120;
    if (velocidade > 150) {
      velocidade = 150;
    }
    this->controlaMotor(eixo, velocidade);
    return;
  }
  if (velocidade < 0) {
    velocidade = velocidade - 130;
    if (velocidade < -150) {
      velocidade = -150;
    }
    this->controlaMotor(eixo, velocidade);
    return;
  }
}


/**
   Gira o motor do eixo recebido por parâmetro
   A velocidade é um valor que varia de -255 a 255, sendo que
   valores abaixo de 0 invertem o sentido de giro do motor
*/
void SeguidorSolar::controlaMotor(Eixo *eixo, int16_t velocidade) {
//#if defined(DEBUG)
//  Serial.print("Velocidade: ");
//  Serial.println(velocidade);
//#endif
  this->lePotenciometro(eixo);
  if (velocidade < 0) {
    if (velocidade < -255) velocidade = -255;
    //if (digitalRead(eixo->sensores.fdc2) == false || eixo->posicao.atual < eixo->posicao.minima || eixo->posicao.atual < 1) {
    if (eixo->posicao.atual < eixo->posicao.minima || eixo->posicao.atual < 0) {
      this->paraMotor(&eixo->motor);
      return;
    }
    velocidade = -velocidade;
    if (eixo->motor.direcao == true) {
      eixo->motor.direcao = false;
      eixo->motor.velocidade = 0;
    }
    digitalWrite (eixo->motor.direita, HIGH);
    digitalWrite (eixo->motor.esquerda, LOW);
  } else {
    if (velocidade > 255) velocidade = 255;
    //if (digitalRead(eixo->sensores.fdc1) == false || eixo->posicao.atual > eixo->posicao.maxima || eixo->posicao.atual > 99) {
    if (eixo->posicao.atual > eixo->posicao.maxima || eixo->posicao.atual > 100) {
      this->paraMotor(&eixo->motor);
      return;
    }
    if (eixo->motor.direcao == false) {
      eixo->motor.direcao = true;
      eixo->motor.velocidade = 0;
    }
    digitalWrite (eixo->motor.direita, LOW);
    digitalWrite (eixo->motor.esquerda, HIGH);
  }

  //partida suave
  int16_t i = 0;
  if (eixo->motor.velocidade != velocidade) {
    for (i = 0; i < velocidade; i++) {
      analogWrite(eixo->motor.habilita, i);
      delayMicroseconds(500);
    }
  }

  eixo->motor.velocidade = velocidade;
}

void SeguidorSolar::paraMotor (Motor *motor) {
  digitalWrite (motor->direita, LOW);
  digitalWrite (motor->esquerda, LOW);
  motor->velocidade = 0;
  analogWrite (motor->habilita, motor->velocidade);
}

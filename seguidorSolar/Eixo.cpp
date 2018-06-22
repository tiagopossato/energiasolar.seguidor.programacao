#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include <EEPROM.h>
#include "Eixo.h"
#include <PID_v1.h>

#define DEBUG

Eixo::Eixo(uint8_t _address)
{
  address = _address;
}

/**
   Inicia um eixo, configurando os pinos de entrada e saída
*/
void Eixo::begin()
{
  controlMode = getControlMode();
  this->motor.direction = true;
  this->motor.speed = 0;
  pinMode(this->motor.rightPin, OUTPUT);
  pinMode(this->motor.leftPin, OUTPUT);
  pinMode(this->motor.enablePin, OUTPUT);

  pinMode(this->safetySensors.sunrise.pin, INPUT);
  pinMode(this->safetySensors.sunset.pin, INPUT);
}

uint8_t Eixo::getControlMode()
{
  EEPROM.get(address, controlMode);
  return controlMode;
}

void Eixo::setControlMode(uint8_t _controlMode)
{
  if (_controlMode != MANUALMODE &&
      _controlMode != SOLARTRACKER &&
      _controlMode != TIMECONTROL &&
      _controlMode != HYBRID)
  {
    _controlMode = MANUALMODE;
  }
  EEPROM.update(address, _controlMode);
  controlMode = _controlMode;
}

void Eixo::readSensors()
{
  readPositionSensor();
  readLightSensors();
  readSafetySensors();
}

uint8_t Eixo::readPositionSensor()
{
  int soma = 0;
  for (char i = 0; i < 10; i++)
  {
    soma += analogRead(this->position.pin);
    delayMicroseconds(250);
  }
  this->position.potValue = soma / 10;
  this->position.value = constrain(map(this->position.potValue, this->position.potMin, this->position.potMax, 0, 100), 0, 100);
  return this->position.value;
}

void Eixo::readLightSensors()
{
  //Le sensores de luz
  this->lightSensors.sunrise.value = analogRead(this->lightSensors.sunrise.pin);
  this->lightSensors.sunset.value = analogRead(this->lightSensors.sunset.pin);
}
void Eixo::readSafetySensors()
{
  //le sensores de segurança
  this->safetySensors.sunrise.value = digitalRead(this->safetySensors.sunrise.pin);
  this->safetySensors.sunset.value = digitalRead(this->safetySensors.sunset.pin);
}

/**
   Método que executa uma auto verificação no exio, com os seguintes passos:
   Vira o painel para o oeste até chegar no fim de curso
   Verifica o valor no potenciômetro e armazena como valor máximo
   Vira o painel para leste até chegar no fim de curso
   Verifica o valor no potenciômetro e armazena como valor mínimo
*/
bool Eixo::calibratePosition()
{
#if defined(DEBUG)
  Serial.println("autoVerificacao");
#endif
  uint8_t max;
  uint8_t min;
  while (this->safetySensors.sunrise.value == false)
  {
    readSafetySensors();
    //está girando ao contrário!
    if (this->safetySensors.sunset.value)
    {
      this->stopMotor();
      return false;
    }
    this->controlMotor(255);
  }
  this->stopMotor();
  max = readPositionSensor();

  while (this->safetySensors.sunset.value == false)
  {
    readSafetySensors();
    if (this->safetySensors.sunrise.value)
    {
      this->stopMotor();
      return false;
    }
    this->controlMotor(-255);
  }
  this->stopMotor();

  //se sucesso, armazena os valores
  this->position.potMin = readPositionSensor();
  this->position.potMax = max;
  return true;
  this->printStatus();
}

/**
   Move o motor do eixo para a posicao desejada
   Recebe como parâmetro nova posicao em %
*/
void Eixo::moveTo(uint8_t newPosition)
{
  if (newPosition > 100)
    newPosition = 100;

  uint8_t lastMin = this->positionMin;
  uint8_t lastMax = this->positionMax;

#if defined(DEBUG)
  Serial.print("Move de: ");
  Serial.print(this->position.value);
  Serial.print("% Para: ");
  Serial.print(newPosition);
  Serial.println("%");
#endif

  if (newPosition > this->position.value)
  {
    this->positionMax = newPosition;
    this->positionMin = this->position.value;
  }
  else
  {
    this->positionMin = newPosition;
    this->positionMax = this->position.value;
  }

  unsigned long previousMillis = millis();
  static uint16_t timeOut = 20000;
  double setpoint;
  double input;
  double output;
  PID myPID(&input, &output, &setpoint, 2, 5, 2, DIRECT);
  myPID.SetOutputLimits(-255, 255);
  //turn the PID on
  myPID.SetMode(AUTOMATIC);

  input = this->readPositionSensor();
  setpoint = newPosition;

  while (this->position.value != newPosition)
  {
    if (millis() - previousMillis >= timeOut)
    {
#if defined(DEBUG)
      Serial.println("timeOut");
#endif
      break;
    }

    this->readPositionSensor();
    input = this->position.value;
    myPID.Compute();
    //limita os valores do PID
    if (output < 0 && output > -75)
      output = -75;
    if (output > 0 && output < 75)
      output = 75;
    this->controlMotor(output);
  }

  this->stopMotor();
  this->positionMax = lastMin;
  this->positionMin = lastMax;
}

void Eixo::solarTracker()
{
  static unsigned long lastStop;
  this->readSensors();
  int16_t velocidade = this->lightSensors.sunset.value - this->lightSensors.sunrise.value;
  if (abs(velocidade) < 5)
  {
    this->stopMotor();
    lastStop = millis();
    return;
  }
  //Verifica se passou cinco segundos apos a ultima vez que parou o motor
  if (millis() - lastStop < 5000)
  {
    return;
  }

  if (velocidade > 0)
  {
    velocidade = velocidade + 120;
    if (velocidade > 150)
    {
      velocidade = 150;
    }
    this->controlMotor(velocidade);
    return;
  }

  if (velocidade < 0)
  {
    velocidade = velocidade - 130;
    if (velocidade < -150)
    {
      velocidade = -150;
    }
    this->controlMotor(velocidade);
    return;
  }

  //  this->controlMotor(map(velocidade, -30, 30, 255, -255));

  //
  //  static double setpoint;
  //  static double input;
  //  static  double output;
  //  static PID myPID(&input, &output, &setpoint, 1, 0.05, 0.25, DIRECT);//modo conservador
  //  //static PID myPID(&input, &output, &setpoint, 4, 0.2, 1, DIRECT); // modo agressivo
  //  myPID.SetOutputLimits(-255, 255);
  //  //turn the PID on
  //  myPID.SetMode(AUTOMATIC);
  //
  //  input = velocidade;
  //  setpoint = 0;
  //  myPID.Compute();
  //  this->controlMotor(output);
}

/**
   Gira o motor do eixo recebido por parâmetro
   A velocidade é um valor que varia de -255 a 255, sendo que
   valores abaixo de 0 invertem o sentido de giro do motor
*/
void Eixo::controlMotor(int16_t speed)
{
  this->readSensors();

  // Restringe os valores da velocidade
  // Verifica os sensores
  // Marca o sentido de giro
  // Altera os pinos da ponte H para configurar o sentido de giro
  if (speed < 0)
  {
    if (speed < -255)
    {
      speed = -255;
    }
    if (this->safetySensors.sunrise.value == true ||
        this->position.value < this->positionMin ||
        this->position.value < 0)
    {
      this->stopMotor();
      return;
    }
    //a saída PWM usa a velocidade sempre positiva
    speed = -speed;
    //verifica se estava indo para o outro lado
    if (this->motor.direction == true)
    {
      this->motor.direction = false;
      this->motor.speed = 0;
    }
    digitalWrite(this->motor.rightPin, LOW);
    digitalWrite(this->motor.leftPin, HIGH);
  }
  else
  {
    if (speed > 255)
    {
      speed = 255;
    }
    if (this->safetySensors.sunset.value == true ||
        this->position.value > this->positionMax ||
        this->position.value > 100)
    {
      this->stopMotor();
      return;
    }

    if (this->motor.direction == false)
    {
      this->motor.direction = true;
      this->motor.speed = 0;
    }
    digitalWrite(this->motor.rightPin, HIGH);
    digitalWrite(this->motor.leftPin, LOW);
  }

  //partida suave
  int16_t i = 0;
  if (this->motor.speed != speed)
  {
    for (i = 0; i < speed; i++)
    {
      analogWrite(this->motor.enablePin, i);
      delayMicroseconds(500);
    }
  }

  this->motor.speed = speed;
}

void Eixo::stopMotor()
{
  this->motor.speed = 0;
  analogWrite(this->motor.enablePin, this->motor.speed);
  digitalWrite(this->motor.leftPin, LOW);
  digitalWrite(this->motor.rightPin, LOW);
}

void espacador()
{
  for (char i = 0; i < 30; i++)
  {
    Serial.print(F("-"));
  }
  Serial.println();
}

void Eixo::printStatus()
{
  espacador();
  Serial.print(F("Position: "));
  Serial.println(this->position.value);
  Serial.print(F("Light on sunrise side: "));
  Serial.println(this->lightSensors.sunrise.value);
  Serial.print(F("Light on sunset side: "));
  Serial.println(this->lightSensors.sunset.value);
  Serial.print(F("Safety sensors: sunrise -> "));
  Serial.print(this->safetySensors.sunrise.value);
  Serial.print(F(", sunset -> "));
  Serial.println(this->safetySensors.sunset.value);
  Serial.print(F("Control Mode -> "));
  Serial.println(this->getControlMode());
  espacador();
}

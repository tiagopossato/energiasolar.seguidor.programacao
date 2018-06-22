#ifndef Eixo_h
#define Eixo_h

//----------TIPO DE CONTROLE------------//
#define MANUALMODE 1
#define SOLARTRACKER 2
#define TIMECONTROL 3
#define HYBRID 4

class Eixo
{
  //Define a estrutura com os parâmetros necessários para controlar um motor
  struct Motor
  {
    uint8_t rightPin;
    uint8_t leftPin;
    uint8_t enablePin;
    int16_t speed;
    bool direction;
  };

  //define um sensor de posição
  struct PositionSensor
  {
    uint8_t pin; //Pino físico
    /**
   * Min e Max reprensentam os limites que o eixo pode atingir, 
   * podem ser definidos manualmente ou através do método calibratePosition
   */
    uint16_t potMax; //valor máximo que o potenciometro assume em 100% do curso
    uint16_t potMin;
    uint16_t potValue; // 0...1024
    int16_t value; //Valor atual lido em % (0..100) percentual de deslocamento
  };

  struct Sensor
  {
    uint8_t pin;
    uint16_t value;
  };

  struct LightSensors
  {
    Sensor sunrise; //
    Sensor sunset;  //
  };

  struct SafetySensors
  {
    Sensor sunrise; //
    Sensor sunset;  //
  };

public:
  Motor motor;
  PositionSensor position;
  LightSensors lightSensors;
  SafetySensors safetySensors;
  uint8_t setPoint;
  
  //usados para definir uma faixa maxima que o painel pode se deslocar
  uint8_t positionMax; 
  uint8_t positionMin;

  Eixo(uint8_t _address);

  void begin(); //iniciar eixo
  void readSensors(); //ler todos os sensores

  bool calibratePosition(); //
  void moveTo(uint8_t newPosition);
  void printStatus();
  void solarTracker();
  uint8_t getControlMode();
  void setControlMode(uint8_t _controlMode);

private:
  uint8_t controlMode;
  uint8_t address;
  uint8_t readPositionSensor();
  void readLightSensors();
  void readSafetySensors();
  void controlMotor(int16_t speed);
  void stopMotor();
};

#endif

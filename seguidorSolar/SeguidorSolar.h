#ifndef SeguidorSolar_h
#define SeguidorSolar_h
#include <PID_v1.h>

struct Motor {
  uint8_t direita;
  uint8_t esquerda;
  uint8_t habilita;
  int16_t velocidade;
  bool direcao;
};

struct Potenciometro {
  uint8_t pino;
  uint16_t maximo;
  uint16_t minimo;
};

struct Sensores {
  uint8_t ldr1;
  uint8_t ldr2;
  uint8_t fdc1;
  uint8_t fdc2;
};

struct Posicao {
  uint8_t maxima;
  uint8_t minima;
  uint8_t atual;
};

struct Eixo {
  Motor motor;
  Potenciometro pot;
  Sensores sensores;
  Posicao posicao;
};

class SeguidorSolar
{
  public:
    void iniciaEixo(Eixo *eixo);
    void autoVerificacao(Eixo *eixo);
    void moveParaPosicao(Eixo *eixo, uint8_t posicao);
    void mostraPotenciometro(Eixo *eixo);
    void segueLuz(Eixo *eixo);

  private:
    void lePotenciometro(Eixo *eixo);
    void controlaMotor (Eixo *eixo, int16_t velocidade);
    void paraMotor(Motor *motor);


};

#endif

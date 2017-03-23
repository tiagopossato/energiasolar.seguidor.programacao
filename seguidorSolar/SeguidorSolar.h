#ifndef SeguidorSolar_h
#define SeguidorSolar_h
#include <LispMotor.h>

struct PonteH{
	uint8_t left1;
	uint8_t left2;
	uint8_t right1;
	uint8_t right2;
	uint8_t left_en;
	uint8_t right_en;
	int16_t left;
	int16_t right;
	bool left_dir;
	bool right_dir;
};

class SeguidorSolar
{		
	public:	
		void init(
			PonteH _ponte,
			uint8_t _pinoPotenciometro,
			uint8_t _pinoLdr1,
			uint8_t _pinoLdr2,
			uint8_t _pinoFdc1,
			uint8_t _pinoFdc2
		);
		
		void autoVerificacao();
		
	protected:
		PonteH ponte;
		uint8_t ldr1;
		uint8_t ldr2;
		uint8_t fdc1;
		uint8_t fdc2;
		
		struct Potenciometro {
			uint8_t pino;
			uint8_t maximo;
			uint8_t minimo;
		};
		
		Potenciometro pot1;
		void controlaMotor (int16_t left, int16_t right);
		void paraMotor();
};

#endif

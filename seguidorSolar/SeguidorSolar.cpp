#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include "SeguidorSolar.h"

 void SeguidorSolar::init(PonteH _ponte, uint8_t _pinoPotenciometro, uint8_t _pinoLdr1, uint8_t _pinoLdr2, uint8_t _pinoFdc1, uint8_t _pinoFdc2){
	this->ponte = _ponte;

	this->ponte.right_dir = true;
	this->ponte.left_dir = true;
	pinMode(this->ponte.left1, OUTPUT);
	pinMode(this->ponte.left2, OUTPUT);
	pinMode(this->ponte.right1, OUTPUT);
	pinMode(this->ponte.right2, OUTPUT);
	pinMode(this->ponte.left_en, OUTPUT);
	pinMode(this->ponte.right_en, OUTPUT);

	this->pot1.pino = _pinoPotenciometro;
	this->ldr1 = _pinoLdr1;
	this->ldr2 = _pinoLdr2;
	this->fdc1 = _pinoFdc1;
	pinMode(this->fdc1, INPUT);
	this->fdc2 = _pinoFdc2;
	pinMode(this->fdc2, INPUT);
}

/**
   Método que executa uma auto verificação no sistema, com os seguintes passos:
   Vira o painel para o oeste até chegar no fim de curso
   Verifica o valor no potenciômetro e armazena como valor máximo
   Vira o painel para leste até chegar no fim de curso
   Verifica o valor no potenciômetro e armazena como valor mínimo
*/
 void SeguidorSolar::autoVerificacao(){	 
	 while(digitalRead(this->fdc1)){
		 this->controlaMotor(128,0);
	 }
	 this->pot1.maximo = analogRead(this->pot1.pino);
	 
	while(digitalRead(this->fdc2)){
		this->controlaMotor(-128,0);
	 }
	 this->pot1.minimo = analogRead(this->pot1.pino);
	 this->paraMotor();
 }

void SeguidorSolar::controlaMotor (int16_t left, int16_t right) {
	if (left < 0) {
		left = -left;
		if (this->ponte.left_dir == true) {
			this->ponte.left_dir = false;
			this->ponte.left = 0;
		}
		digitalWrite (this->ponte.left1, HIGH);
		digitalWrite (this->ponte.left2, LOW);
	} else {
		if (this->ponte.left_dir == false) {
			this->ponte.left_dir = true;
			this->ponte.left = 0;
		}
		digitalWrite (this->ponte.left1, LOW);
		digitalWrite (this->ponte.left2, HIGH);
	}

	if (right < 0) {
		right = -right;
		if (this->ponte.right_dir == true) {
			this->ponte.right_dir = false;
			this->ponte.right = 0;
		}
		digitalWrite (this->ponte.right1, HIGH);
		digitalWrite (this->ponte.right2, LOW);
	} else {
		if (this->ponte.right_dir == false) {
			this->ponte.right_dir = true;
			this->ponte.right = 0;
		}
		digitalWrite (this->ponte.right1, LOW);
		digitalWrite (this->ponte.right2, HIGH);
	}

	//partida suave
	int16_t i=0;
	if(this->ponte.left != left){
		for(i=0;i<left;i++){
			analogWrite (this->ponte.left_en, i);
			delay(5);
		}
	}
	if(this->ponte.right != right){
		for(i=0;i<right;i++){
			analogWrite (this->ponte.right_en, i);
			delay(5);
		}
	}
	
	this->ponte.left = left;
	this->ponte.right = right;
}

void SeguidorSolar::paraMotor () {
	digitalWrite (this->ponte.left1, LOW);
	digitalWrite (this->ponte.left2, LOW);
	digitalWrite (this->ponte.right1, LOW);
	digitalWrite (this->ponte.right2, LOW);
	this->ponte.left = 0;
	this->ponte.right = 0;
	analogWrite (this->ponte.left_en, this->ponte.left);
	analogWrite (this->ponte.right_en, this->ponte.right);
}

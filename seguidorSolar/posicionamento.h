#include <Time.h>
#ifdef _WIN32
#include <TimeLib.h>
#endif

/**
   Calcula o segundo em que o sol nasce no dia
*/
uint32_t nascerDoSol(uint16_t dia)
{
  return (0.0000000302070774172166 * pow(dia, 5)) - (0.0000198076304492236 * pow(dia, 4)) + (0.0035950755 * pow(dia, 3)) - (0.2694919508 * pow(dia, 2)) + (48.27020716989 * dia) + 19901.4743825752;
}

/**
   Calcula o segundo em que o sol se põe no dia
*/
uint32_t porDoSol(uint16_t dia)
{
  return (0.0000000415881756309789 * pow(dia, 5)) - (0.000042747 * pow(dia, 4)) + (0.0155753852 * pow(dia, 3)) - (2.1677644896 * pow(dia, 2)) + (58.67243756 * dia) + 69346.2344757363;
}

/**
   Calcula posicao do painel pela hora do dia
*/
int8_t calculaPosicaoDiaria(uint16_t dia, uint32_t segundo)
{
  uint32_t nascerSol = nascerDoSol(dia);
  uint32_t porSol = porDoSol(dia);
  //  Serial.print("Nascer do sol: ");
  //  Serial.println(nascerSol);
  //  Serial.print("Por do sol: ");
  //  Serial.println(porSol);
  if (segundo < nascerSol || segundo > porSol)
  {
    return -1;
  }

  uint32_t segundosPorDia = porSol - nascerSol;
  uint32_t segundosRelativos = (segundo - nascerSol) * 100 / segundosPorDia;
  //Serial.print(segundosRelativos);
  //Serial.println("% do dia.");

  if (segundosRelativos >= 0 && segundosRelativos <= 100)
  {
    //Serial.print(segundosRelativos);
    //Serial.println("% do dia.");
    return segundosRelativos;
  }
}

/**
   Calcula posição do painel em relação à época do ano
*/

int8_t calculaPosicaoAnual(uint16_t dia)
{
  //TODO: Implementar, retornando valor fixo
  return 50;
}

/**
   Retorna o dia do ano a partir de 1º de Janeiro
*/
uint16_t diaDoAno(tmElements_t *tm) {
  uint8_t tmp = 1;
  uint16_t dias = tm->Day;
  for (; tmp < tm->Month; tmp++) {
    switch (tmp) {
      case 1: dias += 31; break;
      case 2: dias += 28; break;
      case 3: dias += 31; break;
      case 4: dias += 30; break;
      case 5: dias += 31; break;
      case 6: dias += 30; break;
      case 7: dias += 31; break;
      case 8: dias += 31; break;
      case 9: dias += 30; break;
      case 10: dias += 31; break;
      case 11: dias += 30; break;
    }
  }
  return dias;
}

/**
   Calcula o segundo atual a partir da zero horas
*/
uint32_t segundoAtual(tmElements_t *tm) {
  return ((uint32_t)tm->Hour * 60 * 60 ) + ((uint32_t)tm->Minute * 60 ) + (uint32_t)tm->Second;
}

void calculaAngulo(tmElements_t *tm) {
  // Get julian date.
  uint16_t iJulianDate = diaDoAno(tm);
  float fLatitude = -27.1234;
  float fLongitude = -51.2345;
  float fTimeZone = -3;

  ////////////////////////////////////////////////////////////
  // CALCULATE SOLAR VALUES
  ////////////////////////////////////////////////////////////
  // Calculate solar declination as per Carruthers et al.
  float t = 2 * 3.1415 * ((iJulianDate - 1) / 365.0);
  float fDeclination = (0.322003
                  - 22.984 * cos(t)
                  - 0.357898 * cos(2 * t)
                  - 0.14398 * cos(3 * t)
                  + 3.94638 * sin(t)
                  + 0.019334 * sin(2 * t)
                  + 0.05928 * sin(3 * t)
                 );

}



//Função para resetar o programa
void(* resetFunc) (void) = 0;


uint32_t extraiCodigo(char *entrada) {
  char i = 0;
  String tmp;
  tmp.reserve(10);
  for (i = 0;; i++) {
    //aceita valores até 0 to 4,294,967,295
    if (i > 10) break;
    if (entrada[i] == '/') break;
    tmp += entrada[i];
  }

  sprintf(entrada, entrada + i + 1);
  //Serial.print("Numero: ");
  //Serial.println(tmp.toInt());
  return tmp.toInt();
}



/**
   Calcula o segundo em que o sol nasce no dia
*/
uint32_t nascerDoSol(uint16_t dia) {
  return (0.0000000302070774172166 * pow(dia, 5))
         - (0.0000198076304492236 * pow(dia, 4))
         + (0.0035950755 * pow(dia, 3))
         - (0.2694919508 * pow(dia, 2))
         + (48.27020716989 * dia)
         + 19901.4743825752;
}

/**
   Calcula o segundo em que o sol se põe no dia
*/
uint32_t porDoSol(uint16_t dia) {
  return (0.0000000415881756309789 * pow(dia, 5))
         - (0.000042747 * pow(dia, 4))
         + (0.0155753852 * pow(dia, 3))
         - (2.1677644896 * pow(dia, 2))
         + (58.67243756 * dia)
         + 69346.2344757363;
}


/**
   Calcula posicao do painel pela hora do dia
*/
int8_t calculaPosicao(uint16_t dia, uint32_t segundo) {
  uint32_t nascerSol = nascerDoSol(dia);
  uint32_t porSol = porDoSol(dia);
  //  Serial.print("Nascer do sol: ");
  //  Serial.println(nascerSol);
  //  Serial.print("Por do sol: ");
  //  Serial.println(porSol);
  if (segundo < nascerSol || segundo > porSol) {
    return -1;
  }

  uint32_t segundosPorDia =  porSol - nascerSol;
  uint32_t segundosRelativos = (segundo - nascerSol) * 100 / segundosPorDia;
  //Serial.print(segundosRelativos);
  //Serial.println("% do dia.");

  if (segundosRelativos >= 0 && segundosRelativos <= 100) {
    //Serial.print(segundosRelativos);
    //Serial.println("% do dia.");
    return segundosRelativos;
  }
}



#include <string.h>
char auxiliar[24] = {};

short buscaCaracter(char *entrada, char caracter)
{
  unsigned char pos = 0;
  for (; pos <= strlen(entrada); pos++)
  {
    if (caracter == entrada[pos])
    {
      return pos;
    }
  }
  return -1;
}

bool sanitizaEntrada(char *entrada)
{
  short inicio;
  short fim;

  inicio = buscaCaracter(entrada, '{');
  if (inicio == -1)
  {
    return false;
  }
  fim = buscaCaracter(entrada, '}');
  if (fim == -1)
  {
    return false;
  }

  memcpy(auxiliar, &entrada[inicio + 1], fim - (inicio + 1));
  sprintf(entrada, "%s", auxiliar);
  for (unsigned char i = 0; i < strlen(entrada); i++)
  {
    auxiliar[i] = '\0';
  }
  return true;
}

uint32_t extraiCodigo(char *entrada)
{
  uint8_t i = 0;
  char bufferTmp[6] = {'\0', '\0', '\0', '\0', '\0', '\0'};
  uint8_t len = strlen(entrada) <= 5 ? strlen(entrada) : 5; //limita a variável a 5 ou menos
  for (i = 0; i <= len; i++)
  {
    if (entrada[i] == '/') {
      break;
    }
    bufferTmp[i] = entrada[i];
  }
  sprintf(entrada, entrada + i + 1);
  return strtoul(bufferTmp, NULL, 10);
}

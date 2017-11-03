#include <stdio.h>

#include "../_sdk/nedodefs.h" //C only

#include "../_sdk/emit.c" //����� ��� compile � asm (����� �� ������� ������)

#include "../_sdk/read.c" //����� ��� compile � token

#include "../_sdk/fmttg.h" //����� ��� token, asm, export

#ifdef TARGET_THUMB
#include "tokenarm.c"
#else
#include "tokenz80.c"
#endif

#include "token.c"

int main(int argc,char* argv[])
{
  int i;
  char *filename;

  tokinit();
  for (i=1; i<argc; i++) {
    filename = argv[i];
    tokenize(filename); //("asm.f");
  }; //for
  tokenize_end();

  return 0;
}

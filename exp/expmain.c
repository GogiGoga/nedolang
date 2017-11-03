#include <stdio.h>

#include "../_sdk/nedodefs.h" //C only

#include "../_sdk/emit.c" //����� ��� compile � asm (����� �� ������� ������)

#include "../_sdk/read.c" //����� ��� compile � token

#include "../_sdk/fmttg.h" //����� ��� token, asm, export

#include "exporttg.c"
#include "export.c"

int main(int argc,char* argv[])
{
  int i;
  char *filename;

  for (i=1; i<argc; i++) {
    filename = argv[i];
    asmexport(filename);
  }; //for

  return 0;
}

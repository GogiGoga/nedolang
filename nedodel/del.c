#include "../_sdk/io.h"
#include "../_sdk/str.h"

VAR BYTE psystrk[0x900];

FUNC BOOL comparedesc(PCHAR filename, PBYTE desc)
{
VAR CHAR c;
VAR BOOL res;
VAR BYTE i = 0x08;
  filename = findlastslash(filename);
  loop:
    c = *(PCHAR)filename;
    INC filename;
    IF (c == '.') { //todo test '\0', descbuf all spaces
      dot:
      //desc = (PBYTE)((UINT)desc + (UINT)i);
      res = (*(PCHAR)((UINT)desc + (UINT)i) == *(PCHAR)filename);
      goto quit;
    };
    IF (c != *(PCHAR)desc) {
      res = +FALSE;
      goto quit;
    };
    INC desc;
    DEC i;
    IF (i == 0x00) {
      c = *(PCHAR)filename;
      INC filename;
      //IF (c == '.') 
              goto dot;
      //res = (*(PCHAR)desc == ' '); //filenam8 (without ext) //TODO
      //goto quit;
    };
    goto loop;
  quit:
RETURN res;
}

PROC main(PCHAR fn)
{
VAR BYTE nerased;
//VAR BYTE nfiles;
VAR PBYTE curfiledesc;
  //������ ��������� �������
  readsectors((PBYTE)psystrk, 0x0000, 0x09);
  //nfiles = psystrk[0x8e4];
  nerased = psystrk[0x8f4];
  
  //������� �������� ���������� = &psystrk[0x0000]
  curfiledesc = (PBYTE)psystrk;
  loop:
    //���� ���������� ����
    IF (*(PBYTE)curfiledesc == 0x00) goto quit;
    IF (comparedesc(fn, curfiledesc)) {
      //������������ ����������
      curfiledesc[0] = 0x01; //erased
      //DEC nfiles;
      INC nerased;
    };
    //��������� � ���������� �����������
    curfiledesc = &curfiledesc[16];
    //��������� �� 0
  goto loop;
  quit:

  //������������ ��������� ������
  //psystrk[0x8e4] = nfiles; //number of files
  psystrk[0x8f4] = nerased; //number of erased files
  //����� ��������� �������
  writesectors((PBYTE)psystrk, 0x0000, 0x09);
}

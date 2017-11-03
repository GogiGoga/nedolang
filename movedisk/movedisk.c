#include "../_sdk/io.h"
#include "../_sdk/str.h"

VAR BYTE psystrk[0x900];
#define BUFSECTORS 0x10
#define BUFSIZE (UINT)(BUFSECTORS*0x100)
VAR BYTE buf[BUFSIZE];

FUNC UINT copybody(UINT from, UINT to, BYTE count) // TODO ����������� �� �� �� ����� �� ������, �� ��� ����� ���� ��������� ��������� ������!
{
VAR UINT nextrdsector;
VAR UINT nextwrsector;
VAR BYTE wrsectors;
  nextrdsector = from;
  nextwrsector = to;
  WHILE (count > 0x00) {
    IF (count >= BUFSECTORS) {
      wrsectors = BUFSECTORS;
    }ELSE {
      wrsectors = count;
    };
    nextrdsector = readsectors((PBYTE)buf, nextrdsector, wrsectors);
    nextwrsector = writesectors((PBYTE)buf, nextwrsector, wrsectors);
    count = count - wrsectors;
  };
RETURN nextwrsector;
}

PROC movedisk()
{
VAR UINT wasfreeplace; //
VAR UINT freeplace; //���� ����� ���� �����
VAR PBYTE curfiledesc; //������ ������ ����������
VAR PBYTE freefiledesc; //���� ����� ����������
VAR UINT nfreesectors;
VAR BYTE nfiles;
VAR BYTE count;
  //������ ��������� �������
  readsectors((PBYTE)psystrk, 0x0000, 0x09);
  nfiles = 0x00; //psystrk[0x8e4];
  nfreesectors = *(PUINT)(&psystrk[0x8e5]);
  //������ ���������� ����� = 0x0100
  freeplace = 0x0100;
  freefiledesc = (PBYTE)psystrk;
  //������� �������� ���������� = &psystrk[0x0000]
  curfiledesc = (PBYTE)psystrk;
  loop:
    //���� ���������� ����
    IF (*(PBYTE)curfiledesc == 0x00) goto quit;
    count = curfiledesc[0x0d]; //������ � ��������
    IF (*(PBYTE)curfiledesc == 0x01) {
      nfreesectors = nfreesectors + (UINT)count;
      goto next;
    };
    //������������� ���� ����� � ������ ������� �����
    wasfreeplace = freeplace;
    freeplace = copybody(*(PUINT)(&curfiledesc[0x0e]), freeplace, count);
    //������������ ����������
    memcopy(curfiledesc, 14, freefiledesc);
    POKE *(PUINT)(&curfiledesc[0x0e]) = wasfreeplace;
    freefiledesc = &freefiledesc[16];
    INC nfiles;
    next:
    //��������� � ���������� �����������
    curfiledesc = &curfiledesc[16];
    //��������� �� 0
  goto loop;
  quit:
  
  WHILE (freefiledesc[0] != 0x00) {
    freefiledesc[0] = 0x00; //end of directory
    freefiledesc = &freefiledesc[16];
  };
  
  //������������ ��������� ������
  POKE *(PUINT)(&psystrk[0x8e1]) = freeplace;
  psystrk[0x8e4] = nfiles; //number of files
  POKE *(PUINT)(&psystrk[0x8e5]) = nfreesectors;
  psystrk[0x8f4] = 0x00; //number of erased files
  //����� ��������� �������
  writesectors((PBYTE)psystrk, 0x0000, 0x09);
}

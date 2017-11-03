#include "../_sdk/str.h"

FUNC UINT strjoin(PCHAR to, UINT tolen, PCHAR s2/**, UINT s2len*/) //����� ��� �����������!
{ //to = to + s2;
VAR UINT len;
VAR CHAR c;
  to = &to[tolen/**+_FIRST*/];
  len = tolen; //����� ��� �����������!
  loop: //REPEAT {
    c = *(PCHAR)s2;
    IF ((c == '\0') || (len>=_STRMAX)) goto endloop; //BREAK; //���������� �� ����������
    POKE *(PCHAR)(to) = c;
    INC s2;
    INC to;
    INC len;
  goto loop; //}UNTIL ( (c=='\0') || (len>=_STRMAX) ); //���������� �� ����������
  endloop:
  RETURN len; //����� ��� �����������!
}

FUNC UINT strjoineol(PCHAR to, UINT tolen, PCHAR s2/**, UINT s2len*/, CHAR eol) //����� ��� �����������!
{ //to = to + s2;
VAR UINT len;
VAR CHAR c;
  to = &to[tolen/**+_FIRST*/];
  len = tolen; //����� ��� �����������!
  loop: //REPEAT {
    c = *(PCHAR)s2;
    IF ((c == eol) || (len>=_STRMAX)) goto endloop; //BREAK; //���������� �� ����������
    POKE *(PCHAR)(to) = c;
    INC s2;
    INC to;
    INC len;
  goto loop; //}UNTIL ( (c==eol) || (len>=_STRMAX) ); //���������� �� ����������
  endloop:
  RETURN len; //����� ��� �����������!
}

FUNC UINT strcopy(PCHAR from, UINT len, PCHAR to) //����� ��� �����������!
{
VAR UINT i = len+1;
VAR CHAR c;
  REPEAT {
    c = *(PCHAR)from;
    POKE *(PCHAR)(to) = c;
    INC from;
    INC to;
    DEC i;
  }UNTIL (i==0); //(c=='\0'); //���������� ����������
  RETURN len; //����� ��� �����������!
}

PROC memcopy(PBYTE from, UINT len, PBYTE to)
{
VAR CHAR c;
  REPEAT {
    c = *(PCHAR)from;
    POKE *(PCHAR)(to) = c;
    INC from;
    INC to;
    DEC len;
  }UNTIL (len==0);
}

PROC memcopyback(PBYTE from, UINT len, PBYTE to)
{
VAR CHAR c;
  REPEAT {
    c = *(PCHAR)from;
    POKE *(PCHAR)(to) = c;
    DEC from;
    DEC to;
    DEC len;
  }UNTIL (len==0);
}

FUNC BOOL strcp(PCHAR s1, PCHAR s2)
{
VAR UINT i;
VAR CHAR c1;
VAR BOOL ok;
  i = 0;
  ok = +TRUE; //�� ��������� ������ �����
  REPEAT { //��������� ��� ������� ������ ������, ����� �����������
    //IF (s1[i] != s2[i]) { //������ ������ ���� ��������� ��� ������ �� ������
    //  result = +FALSE;
    //  BREAK;
    //};
    c1 = s1[i];
    IF (c1 != s2[i]) {ok = +FALSE; BREAK; }; //������ �� ������ ��� ���� �� ����� ���������
    //IF (c1 == '\0') BREAK; //��� ������ ���������
    INC i;
  }UNTIL (c1 == '\0'); //��� ������ ���������
  //IF (s2[i] != '\0') result = +FALSE; //������ ������ �� ���������
  RETURN ok;
}

/**
FUNC UINT strclear(PCHAR s)
{ //s = "";
  //s[_FIRST]='\0'; //���������� (�� ����� - ���������� ����� strclose)
  RETURN 0; //����� ��� �����������!
}
*/
FUNC UINT stradd(PCHAR s, UINT len, CHAR c) //����� ��� �����������!
{ //s = s + c;
  IF (len < _STRMAX) {
    //POKE *(PCHAR)(+(POINTER)(s) + +(POINTER)(len) + +(POINTER)(_FIRST)) = c;
    s[len/**+_FIRST*/] = c;
    INC len;
    //s[len/**+_FIRST*/]='\0'; //���������� (�� ����� - ���������� ����� strclose)
  };
  RETURN len;
}

//PROC strclose(PCHAR s, UINT len) //�������� ����� strclear ��� ������ stradd
//{
//  s[len/**+_FIRST*/] = '\0'; //����������
//}

//FUNC UINT strcopy(PCHAR from, UINT len, PCHAR to) //����� ��� �����������!
//{
//  RETURN strjoin(to, 0/**strclear(to)*/, from); //tolen;
//}

/**
PROC stringdecapitalize(PCHAR s1, UINT s1len)
{
VAR UINT i;
VAR CHAR c;
  i = _FIRST;
  WHILE (+TRUE) {
    c = s1[i];
    IF (c == '\0') BREAK;
    IF ((c>='A') && (c<='Z')) {
      //POKE *(PCHAR)(+(POINTER)(s1) + +(POINTER)(i)) = c | +(CHAR)0x20;
      s1[i] = +(CHAR)(c | +(CHAR)0x20);
    };
    i = +(UINT)(i + 1);
  };
}
*/

FUNC BOOL strcplow(PCHAR s1/**, UINT s1len*/, PCHAR s2)
{
VAR UINT i;
VAR CHAR c1;
VAR CHAR c2;
VAR BOOL ok;
  i = 0;
  ok = +TRUE; //�� ��������� ������ �����
  REPEAT { //��������� ��� ������� ������ ������, ����� �����������
    c1 = s1[i/**+_FIRST*/];
    c2 = s2[i/**+_FIRST*/];
    IF ((+(BYTE)c1>=+(BYTE)'A') && (+(BYTE)c1<=+(BYTE)'Z')) c1 = +(CHAR)(+(BYTE)c1 | 0x20);
    IF ((+(BYTE)c2>=+(BYTE)'A') && (+(BYTE)c2<=+(BYTE)'Z')) c2 = +(CHAR)(+(BYTE)c2 | 0x20);
    IF (c1 != c2) {ok = +FALSE; BREAK; }; //������ �� ������ ��� ���� �� ����� ���������
    //IF (c1 == '\0') BREAK; //��� ������ ���������
    //IF (c2 == '\0') {result = +FALSE; BREAK; }; //������ ������ ���� ���������
    INC i;
  }UNTIL (c1 == '\0'); //��� ������ ���������
  //IF (s2[i/**+_FIRST*/] != '\0') result = +FALSE; //������ ������ �� ���������
  RETURN ok;
}

FUNC UINT hash(PBYTE pstr)
{
VAR UINT hash;
VAR BYTE c;
  hash = 0x0000;
  c = 0x00;
  REPEAT {
    c = c^(+(BYTE)hash);
    hash = hash + hash;
    c = c+(+(BYTE)hash);
    hash = (hash&0xff00) + +(UINT)c;
    c = *(PBYTE)(pstr); //����� � ������ ������������� 0x00
    INC pstr;
  }UNTIL (c == 0x00);
  RETURN hash;
};

//// imported
#include "../_sdk/io.h"
#include "../_sdk/str.h" //stradd, strjoineol
#include "../_sdk/emit.h"

EXTERN BYTE _token; //������� ��������� �����
EXTERN BYTE _prefixedtoken; //�������������� ����� � ������ \n � �.�.
EXTERN BYTE _curdir; //����� ������� �������������� ��������� ���������� (����� ��� ���������� ��������� �������)
EXTERN BOOL _labelchanged; //���� "�������� �����" - ����� ��� ������ �� LABEL (�� �� �� REEQU)
EXTERN LONG _value[_MAXVALS];
EXTERN PBYTE _inclfile[_MAXINCLUDES];

EXTERN UINT _curlnbeg; //����� ������ �� ������ ������ ������

EXTERN BYTE _reg; //��������� �������
EXTERN BYTE _oldreg; //���������� �������
EXTERN BYTE _base; //���� ���� �������
EXTERN BYTE _base2; //����2 ���� ������� (��� �������� ���������)

EXTERN BYTE _nvalues; //����� �������� � �����
EXTERN BYTE _ninclfiles; //����� �������� ������

EXTERN UINT _curaddr; //�����, ���� �����
EXTERN UINT _curshift; //$=(_curaddr+curshift), curshift=(disp-_curaddr)
EXTERN UINT _curbegin; //��������� ����� �����, ���� �����
//EXTERN BYTE _curpage0;
//EXTERN BYTE _curpage1;
//EXTERN BYTE _curpage2;
//EXTERN BYTE _curpage3;

EXTERN PBYTE _pstr; //����� � ������ ������������� TOK_ENDTEXT
EXTERN PBYTE _curlabeltext;
EXTERN PBYTE _evallabeltext;
EXTERN PCHAR _fn;
EXTERN UINT _lenfn;

PROC asmpushvalue FORWARD(LONG value);
PROC asmpushbool FORWARD(BOOL b);
FUNC LONG asmpopvalue FORWARD();
//PROC asmwritestate FORWARD();
//PROC asmreadstate FORWARD();
PROC readlabel FORWARD();
PROC findlabel FORWARD(PBYTE labeltext);
FUNC LONG getlabel FORWARD(); //�������� ��������������� ����� findlabel!!!
PROC errwrongreg FORWARD();
PROC errwrongpar FORWARD();
PROC asmerrtext FORWARD();
PROC asmbyte FORWARD(BYTE _token);
PROC asmemitblock FORWARD(); //�������� ������ ����� org
PROC asmdir_label FORWARD();
PROC asmfmt_reequ FORWARD();

PROC asmreadprefixed FORWARD();

EXTERN PBYTE _fincb;
EXTERN BOOL _asms;

PROC decltoken FORWARD(BYTE bb);
PROC decldig FORWARD(UINT d);

EXTERN PBYTE _forg;
EXTERN PBYTE _fdecl;

EXTERN BYTE _isaddr; //����� "� ��������� ������������� �����"

#ifdef TARGET_THUMB
#include "asmf_arm.c" //// ���������������� ��������� � ����������
#else
#include "asmf_z80.c" //// ���������������� ��������� � ����������
#endif

PROC asmloop()
{
VAR UINT scale; //���������� ������� ���������
VAR LONG tempvalue; //��������, ��������� �� popvalue � ������� ����� �� pushvalue
VAR UINT i;
  loop:
    _token = readfin();
    switch (_token) {

#include "asmj.c" //// ����������� �����

#ifdef TARGET_THUMB
#include "asmj_arm.c" //// ���������������� �����
#else
#include "asmj_z80.c" //// ���������������� �����
#endif

      default : {err(+(CHAR)_token); enderr(); goto loop;}
    };
  endloop:;
}


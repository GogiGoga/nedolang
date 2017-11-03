//// imported
#include "../_sdk/emit.h"

#include "sizesarm.h"

//�����, ��� ����� ��������� ��������, ������������ _joined
//(����� call - ��� _callee, � ����� loadrg/b - ��� _const)
//� ����� ������ ���������� (put, call, jpiffalse) ����� _jpflag = 0x00 (����� ���������� "����������������" ��������� ����� ��������� ����� �������� �������)
//������� ������ � jpiffalse, �.�. ��������� �� ����� ���� "�����������������"
EXPORT VAR PCHAR _callee; //�������� ���������� ��������� (� ������ ������)
EXPORT VAR UINT  _lencallee;
EXTERN PCHAR _joined; //���������
EXTERN UINT  _lenjoined;
EXTERN PCHAR _const; //������ ���������
EXTERN UINT  _lenconst;

EXPORT VAR BYTE _exprlvl; //������� ��������� (������� ������� == 1)

//������� ��������� (�� rgs) - todo � codegen �� ������������ rgs
EXTERN BYTE _rnew;
EXTERN BYTE _rold;
EXTERN BYTE _rold2;
EXTERN BYTE _rold3;

PROC getrfree FORWARD(); //������ ��������� ������� � ������� ��� ���
PROC freernew FORWARD(); //���������� ������� � ������� �����

//rg pool
PROC getnothing FORWARD(); //��������� �������� � ����� � ����� � ����������
//PROC getmainrg FORWARD(); //����� RMAIN=new � ���������� ��������� ��������
PROC getmain2rgs FORWARD(); //����� RMAIN=old, RMAIN2=new � ���������� ��������� �������� //��� call2rgs
//PROC getmain3rgs FORWARD(); //��� call3rgs
PROC getmain4rgs FORWARD(); //��� call4rgs
PROC setmainrg FORWARD(); //���������� ��������, ��� ����� �� �������� ��������� � �������� //��� call2rgs
PROC setmain2rgs FORWARD(); //���������� ��������, ��� ����� �� �������� ��������� � ��������� //��� call4rgs

PROC rgs_initrgs FORWARD();

////
#define _RGBUFSZ (BYTE)(_NRGS+0x01)

  //���������� ���������:
CONST BYTE _RMAIN = 0x01; /**HL*/ /**������� ���������� � ������� ��������� ����������� �������*/
CONST BYTE _RMAIN2= 0x02; /**DE*/ /**������� ������� ����� ���������� � ������� ��������� ����������� �������*/
CONST BYTE _RMAIN3= 0x03;
CONST BYTE _RMAIN4= 0x04;

CONST PCHAR _RNAME [_RGBUFSZ] = {
  "R0", //0 ������, rtemp
  "R1",
  "R2",
  "R3",
  "R4",
//  "R5",
//  "R6"
};

VAR BOOL _fused;

VAR INT _funcstkdepth;

VAR BYTE _jpflag; //0=OR A:JZ, 1=JZ, 2=JNZ, 3=JNC, 4=JC

EXPORT CONST BYTE _typesz[32] = { //������ ���� � ������ ��� �������
  _SZ_BYTE/**T_BYTE */, //��� ���� ��������
  _SZ_REG/**T_UINT */, //��� ���� ��������
  _SZ_REG/**T_INT  */, //��� ���� ��������
  _SZ_BOOL/**T_BOOL */,
  _SZ_LONG/**T_LONG */,
  _SZ_BYTE/**T_CHAR */,
  _SZ_LONG/**T_FLOAT*/,
  0x00/**unknown*/,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  _SZ_REG/**T_PBYTE */,
  _SZ_REG/**T_PUINT */,
  _SZ_REG/**T_PINT  */,
  _SZ_REG/**T_PBOOL */,
  _SZ_REG/**T_PLONG */,
  _SZ_REG/**T_PCHAR */,
  _SZ_REG/**T_PFLOAT*/,
  _SZ_REG/**        */,
  _SZ_REG, _SZ_REG, _SZ_REG, _SZ_REG, _SZ_REG, _SZ_REG, _SZ_REG, _SZ_REG //pointer to...
};

CONST BYTE _typeshift[32] = { //log ������ ���� (n ��� 2^n ����) ��� �������
  _RL_BYTE/**T_BYTE */, //��� ���� ��������
  _RL_REG/**T_UINT */, //��� ���� ��������
  _RL_REG/**T_INT  */, //��� ���� ��������
  _RL_BOOL/**T_BOOL */,
  _RL_LONG/**T_LONG */,
  _RL_BYTE/**T_CHAR */,
  _RL_LONG/**T_FLOAT*/,
  0x00/**unknown*/,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  _RL_REG/**T_BYTE  */,
  _RL_REG/**T_UINT  */,
  _RL_REG/**T_INT   */,
  _RL_REG/**T_BOOL  */,
  _RL_REG/**T_LONG  */,
  _RL_REG/**T_CHAR  */,
  _RL_REG/**T_PFLOAT*/,
  _RL_REG/**        */,
  _RL_REG, _RL_REG, _RL_REG, _RL_REG, _RL_REG, _RL_REG, _RL_REG, _RL_REG //pointer to...
};

//VAR CHAR  _swass[_STRLEN]; //todo ������?
//VAR PCHAR _wass; //todo ������
//VAR UINT  _lenwass; //todo ������

VAR UINT constindex;
//VAR UINT wasconstindex; //todo ������

VAR UINT asmcmds; //������� ������ ������ � ������� ��������� �������� �����

PROC initrgs FORWARD(); //�������� ��������� ��������� � ������ (������������ � cemitfunc)

VAR CHAR asmlblbuf[65536];
VAR UINT lenasmlblbuf;

PROC asm_dw FORWARD();
PROC asm_b FORWARD();

FUNC UINT mystrlen(PCHAR s) //����� ��� �����������!
{
VAR UINT len;
VAR CHAR c;
  len = 0; //����� ��� �����������!
  loop:
    c = *(PCHAR)s;
    IF (c == '\0') goto endloop; //BREAK;
    INC s;
    INC len;
  goto loop;
  endloop:
RETURN len; //����� ��� �����������!
}

FUNC UINT mystrcopy(PCHAR from, PCHAR to)
{
VAR UINT len;
VAR CHAR c;
  len = 0; //����� ��� �����������!
  loop:
    c = from[len];
    to[len] = c;
    IF (c == '\0') goto endloop; //BREAK;
    INC len;
  goto loop;
  endloop:
RETURN len; //����� ��� �����������!
}

PROC varmangledstr(PCHAR s)
{
VAR CHAR c;
  IF (*(PCHAR)s == '\'') { //'c'
    varstr(s);
  }ELSE {
    loop:
      c = *(PCHAR)s;
      IF (c == '\0') goto quit;
      IF ((c == '.')||(c == '/')) {
        varstr("__");
      }ELSE varc(c);
      INC s;
    goto loop;
    quit:
  }
}

PROC asmmangledstr(PCHAR s)
{
VAR CHAR c;
  IF (*(PCHAR)s == '\'') { //'c'
    asmstr(s);
  }ELSE {
    loop:
      c = *(PCHAR)s;
      IF (c == '\0') goto quit;
      IF ((c == '.')||(c == '/')) {
        asmstr("__");
      }ELSE asmc(c);
      INC s;
    goto loop;
    quit:
  }
}

PROC asm_pc4_eol()
{
  asmstr("__"); asmuint(constindex); asmmangledstr(_fn); endasm(); //fn �����, ����� ��������� ��� ������ ������� �� �������
}

PROC asm_endpc4()
{
  asmstr("__"); asmuint(constindex); asmmangledstr(_fn); endasm(); //fn �����, ����� ��������� ��� ������ ������� �� �������
  INC constindex;
}

PROC asm_align4b()
{
  asmstr("\tALIGN 4"); endasm();
}

EXPORT PROC emitasmlabel(PCHAR s)
{
  asmmangledstr(s); /**asmc( ':' );*/ endasm();
}

EXPORT PROC emitfunclabel(PCHAR s)
{
  asmmangledstr(s); asmstr("\tEQU $+1"); endasm();
}

EXPORT PROC emitvarlabel(PCHAR s)
{
  varmangledstr(s); /**varc( ':' );*/ endvar();
}

PROC asmaddlabel(PCHAR s, UINT constindex)
{
VAR UINT len;
  POKE *(PUINT)(&asmlblbuf[lenasmlblbuf]) = constindex;
  lenasmlblbuf = lenasmlblbuf + +sizeof(UINT);
  len = mystrcopy(s, &asmlblbuf[lenasmlblbuf]);
  lenasmlblbuf = lenasmlblbuf + len + 1;
  asmstr("__"); asmuint(constindex); asmmangledstr(_fn); //fn �����, ����� ��������� ��� ������ ������� �� �������
  asmstr("__"); asmmangledstr(s); //��� �����������
//  IF (high) {asmstr( ">>32"/**WORDBITS*/ );  }; //todo long
}

PROC asmflushlabels()
{
VAR UINT i;
VAR UINT len;
  IF (lenasmlblbuf != 0) asm_align4b();
  i = 0;
  WHILE (i != lenasmlblbuf) {
    asmstr("__"); asmuint(*(PUINT)(&asmlblbuf[i])); asmmangledstr(_fn); //fn �����, ����� ��������� ��� ������ ������� �� �������
    i = i + +sizeof(UINT);
    asmstr("__"); emitasmlabel(&asmlblbuf[i]); //��� �����������
    //len = mystrcopy(&asmlblbuf[i], _wass);
    asm_dw(); asmmangledstr(&asmlblbuf[i]); endasm();
    i = i + mystrlen(&asmlblbuf[i]) + 1;
  };
  lenasmlblbuf = 0;
  asmcmds = 0;
}

PROC asmjpflushlabels()
{
  asm_b();
  asm_pc4_eol();
  asmflushlabels(); //�� ������� constindex
  asm_endpc4();
}

//////////// ������ ��������� ��� ���������� ����� ��������

PROC endasmcmd()
{
  endasm();
  INC asmcmds;
  IF (asmcmds >= 250) asmjpflushlabels();
}

PROC asm_direct()
{
  asmc('#');
}

PROC asm_comma()
{
  asmc(',');
}

PROC asm_open()
{
  asmc('[');
}

PROC asm_close()
{
  asmc(']');
}

PROC asm_opcurl()
{
  asmc('{');
}

PROC asm_clcurl()
{
  asmc('}');
}

PROC asm_rname(BYTE r)
{
  asmstr( _RNAME[+(UINT)r] );
}

PROC asm_mrname(BYTE r)
{
  asm_open(); asm_rname(r); asm_close();
}

PROC asm_rtemp()
{
  asmstr("R0");
}

PROC asm_mrtemp()
{
  asmstr("[R0]");
}

PROC asm_rzero()
{
  asmstr("R8");
}

PROC asm_rff()
{
  asmstr("R7");
}

PROC asm_rmain()
{
  asm_rname(_RMAIN);
}

PROC asm_close_eol()
{
  asm_close(); endasmcmd();
}

PROC var_alignwsz()
{
  varstr("\tALIGN 4"); endvar();
}

EXPORT PROC var_db() //�������� �� compile!
{
  varstr( "\tDCB " );
}

EXPORT PROC asm_db() //������� ��� ����������� �������� ����� TODO
{
  asmstr( "\tDCB " );
}

EXPORT PROC var_dw() //�������� �� compile!
{
//  var_align4b();
  varstr( "\tDCD " );
}

EXPORT PROC asm_dw() //������� ��� ���������������� ��������
{
//  asm_align4b();
  asmstr( "\tDCD " );
}

PROC var_dl()
{
//  var_align4b();
  varstr( "\tDCQ " );
}

EXPORT PROC var_ds() //�������� �� compile!
{
  varstr( "\tSPACE " );
}

PROC asm_ands() //� ���������� ������
{
  asmstr( "\tANDS " );
}

PROC asm_ors() //� ���������� ������
{
  asmstr( "\tORRS " );
}

PROC asm_eors() //� ���������� ������
{
  asmstr( "\tEORS " );
}

PROC asm_rsbs() //� ���������� ������
{
  asmstr( "\tRSBS " );
}

PROC asm_subs() //� ���������� ������
{
  asmstr( "\tSUBS " );
}

PROC asm_sbcs() //� ���������� ������
{
  asmstr( "\tSBCS " );
}

PROC asm_adds() //� ���������� ������
{
  asmstr( "\tADDS " );
}

PROC asm_adcs() //� ���������� ������
{
  asmstr( "\tADCS " );
}

PROC asm_ldr()
{
  asmstr( "\tLDR " );
}

PROC asm_ldrb()
{
  asmstr( "\tLDRB " );
}

PROC asm_str()
{
  asmstr( "\tSTR " );
}

PROC asm_strb()
{
  asmstr( "\tSTRB " );
}

PROC asm_mov() //��� ��������� ������
{
  asmstr( "\tMOV " );
}

PROC asm_movs() //� ���������� ������
{
  asmstr( "\tMOVS " );
}

PROC asm_mvns() //� ���������� ������
{
  asmstr( "\tMVNS " );
}

PROC asm_b() //jr
{
  asmstr( "\tB " );
}

PROC asm_bl() //jp/call
{
  asmstr( "\tBL " );
  INC asmcmds; //long cmd
}

PROC asm_beq() //jp z
{
  asmstr( "\tBEQ " );
}

PROC asm_bne() //jp nz
{
  asmstr( "\tBNE " );
}

PROC asm_bcs() //jp c
{
  asmstr( "\tBCS " );
}

PROC asm_bcc() //jp nc
{
  asmstr( "\tBCC " );
}

PROC asm_bpl() //jp p
{
  asmstr( "\tBPL " );
}

PROC asm_bvc() //jp pe? (no overflow)
{
  asmstr( "\tBVC " );
}

PROC asm_bvs() //jp po? (overflow)
{
  asmstr( "\tBVS " );
}

PROC asm_blt() //< (�������� ���������)
{
  asmstr( "\tBLT " );
}

PROC asm_bgt() //> (�������� ���������)
{
  asmstr( "\tBGT " );
}

PROC asm_bx() //jp reg
{
  asmstr( "\tBX " );
}

PROC asm_blx() //call reg???
{
  asmstr( "\tBLX " );
}

PROC asm_nop_eol()
{
  asmstr("\tNOP"); endasmcmd();
}

PROC asm_push()
{
  asmstr( "\tPUSH " );
}

PROC asm_pop()
{
  asmstr( "\tPOP " );
}

PROC emit0(BYTE reg) //��� ��������� ������
{
  asm_mov(); asm_rname(reg); asm_comma(); asm_rzero(); endasmcmd();
  //asm_mov(); asm_rname(reg); asm_comma(); asm_direct(); asmc('0'); endasmcmd();
  //asm_sub(); asm_rname(reg); asm_comma(); asm_rname(reg); endasmcmd();
}

PROC emitmaskb(BYTE reg)
{
  asm_ands(); asm_rname(reg); asm_comma(); asm_rff(); endasmcmd();
}

PROC asm_call_eol(PCHAR s)
{
  asm_bl(); asmmangledstr(s); endasmcmd();
}

///////////////////////////////////
//�������� �� commands
PROC unproxy()
{
}

PROC proxy(BYTE r)
{
}

///////////////////////////////////////////////////////////
//��������� � �������� ����� ��� rgs

PROC emitpushrg(BYTE rnew)
{
  asm_push(); asm_opcurl(); asm_rname(rnew); asm_clcurl(); endasmcmd();
  INC _funcstkdepth;
}

PROC emitpoprg(BYTE rnew) //������� ��� ������� � getrfree/getrg
{
  asm_pop(); asm_opcurl(); asm_rname(rnew); asm_clcurl(); endasmcmd();
  DEC _funcstkdepth;
}

PROC emitmovrg(BYTE rsrc, BYTE rdest) //�� ���������� � �� ����������� (��. emitmoverg)
{
  asm_movs(); asm_rname(rdest); asm_comma(); asm_rname(rsrc); endasmcmd(); //�� mov, ������ ��� ��� �������� r0..r7
}

///////////////////////////////////////////////////////////////////////////////////////
//��� ��������� ���������� ���
//���������� �� �����������, �������� �� commands

EXPORT PROC emitexport(PCHAR s)
{
  asmstr("\tEXPORT "); asmmangledstr(s); endasmcmd();
}

EXPORT PROC emitvarpreequ(PCHAR s)
{
//  varstr("\tIF :LNOT: :DEF: "); varmangledstr(s); endvar(); //todo ������ (Keil)
}

EXPORT PROC emitvarpostequ()
{
//  varstr("\tENDIF"); endvar(); //todo ������ (Keil)
}

EXPORT PROC varequ(PCHAR s)
{
  varmangledstr(s); varstr("\tEQU ");
}

PROC emitjpmainrg() //"jp (hl)"
{
  //�� ���� ������ ��� ��������������� ��� �������� ������ ���� ��������
  //(������ ��� ��������� �������������, �� ��� storergs � � ��������������� ��������� ���� ����������� �������)
  getnothing(); //getnothingword();
  asm_bx(); asm_rmain(); endasmcmd();
}

PROC emitcallmainrg() //"call (hl)"
{
  //�� ���� ������ ��� ��������������� ��� �������� ������ ���� ��������
  //(������ ��� ��������� �������������, �� ��� storergs � � ��������������� ��������� ���� ����������� �������)
  getnothing(); //getnothingword();
/**  //asm_blx(); asm_rmain(); endasmcmd();
  asm_add(); asm_rtemp(); asm_comma(); asmstr("PC"); asm_comma(); asmstr("#8"); endasmcmd();
  asm_mov(); asmstr("LR"); asm_comma(); asm_rtemp(); endasmcmd();
  asm_bx(); asm_rmain(); endasmcmd();
  asm_nop_eol();
*/
  asm_call_eol("_JPHL.");
  _fused = +FALSE; //����� ����������
}

PROC emitjp()
{
  //�� ���� ������ ��� ��������������� ��� �������� ������ ���� ��������
  //(������ ��� ��������� �������������, �� ��� storergs � � ��������������� ��������� ���� ����������� �������)
  unproxy();
  getnothing(); //getnothingword();
  asm_b(); asmmangledstr(_joined); endasmcmd();
}

PROC emitbtoz() //����� jp!
{
  //����� ����� ��������� �� ����, �� ����� �� ������ BOOL
  //IF (_jpflag == 0x00) {
  IF (!_fused) { //���������� ��� �� ������
    asm_ors(); asm_rname(_rnew); asm_comma(); asm_rname(_rnew); endasmcmd();
  };
}

PROC emitjpiffalse()
{
  //�� ���� ������ ��� ��������������� ��� �������� ������ ���� ��������
  //(������ ��� ��������� �������������, �� ��� storergs � � ��������������� ��������� ���� ����������� �������)
  getnothing(); //getnothingword();
  IF       (_jpflag == 0x02) {asm_beq(); //asm_bne();
  }ELSE IF (_jpflag == 0x03) {asm_bcc(); //asm_bcs();
  }ELSE IF (_jpflag == 0x04) {asm_bcs(); //asm_bcc();
  }ELSE                      {asm_bne(); //asm_beq();
  };
  asm_pc4_eol();

  asm_bl(); //long jp
  asmmangledstr(_joined);
  endasmcmd();

  asm_endpc4();
  _fused = +FALSE;
  _jpflag = 0x00;
}

PROC emitret()
{
  asm_pop(); asmstr( "{PC}" ); endasmcmd();
  asmflushlabels();
}

PROC emitcall2rgs(PCHAR s)
{
  unproxy();
  getmain2rgs();
  initrgs();
  asm_call_eol(s);
  setmainrg(); //��������� � RMAIN
}
/**
PROC emitcall3rgs(PCHAR s) //todo ���������
{
  unproxy();
  getmain3rgs();
  initrgs();
  asm_call_eol(s);
  //������ ��� �������� �������� ��� ���������
  //setwordcontext();
  setmain2rgs(); //��������� � RMAIN,RMAIN2
}
*/
PROC emitcall4rgs(PCHAR s) //todo ���������
{
  unproxy();
  getmain4rgs();
  initrgs();
  asm_call_eol(s);
  //������ ��� �������� �������� ��� ���������
  //setwordcontext();
  setmain2rgs(); //��������� � RMAIN,RMAIN2
}

PROC emitcallproc()
{
  //_jpflag = 0x00;
  asm_call_eol(_callee);
}

PROC emitloadrtemp(BYTE r, PCHAR s)
{
  asm_ldr(); asm_rname(r); asm_comma();
  //asmc('L'); asmstr(s); //todo ������������ � ���������� ���������, � ����������� ���������� ������������ (rtemp ������������� ������ ����� � � inc/dec)
  asmaddlabel(s, constindex);
  INC constindex;
  endasmcmd();
}

PROC emitloadrg(BOOL high) //������� ��� ����� ����� getrfree
{
  emitloadrtemp(_rnew,_const);
//  asm_ldr(); asm_rname(_rnew); asm_comma(); asm_mrtemp(); endasmcmd(); //todo long
  _fused = +FALSE; //����� ����������
}

PROC emitloadrg0() //������� ��� ����� ����� getrfree
{
  emit0(_rnew); //����� �� �����
}

PROC emitloadb() //����������� ��� ����� ����� getfreea
{
  asm_movs(); asm_rname(_rnew); asm_comma(); asm_direct(); asmmangledstr(_const); endasmcmd();
}

PROC emitgetrg(BOOL high) //������� ��� ����� ����� getrfree
{
  emitloadrtemp(0x00,_joined);
  asm_ldr(); asm_rname(_rnew); asm_comma(); asm_mrtemp(); endasmcmd(); //todo long
}

PROC emitgetb() //����������� ��� ����� ����� getfreea
{
  emitloadrtemp(0x00,_joined);
  asm_ldrb(); asm_rname(_rnew); asm_comma(); asm_mrtemp(); endasmcmd();
}

PROC emitputrg(BOOL high) //ld [],new
{
  //_jpflag = 0x00;
  emitloadrtemp(0x00,_joined);
  asm_str(); asm_rname(_rnew); asm_comma(); asm_mrtemp(); endasmcmd(); //todo long
  _fused = +FALSE; //����� ����������
}

PROC emitputb()
{
  //_jpflag = 0x00;
  emitloadrtemp(0x00,_joined);
  asm_strb(); asm_rname(_rnew); asm_comma(); asm_mrtemp(); endasmcmd();
  _fused = +FALSE; //����� ����������
}

PROC emitshl1rg()
{
  asm_adds(); asm_rname(_rnew); asm_comma(); asm_rname(_rnew); endasmcmd();
}

PROC emitshl1b()
{
  asm_adds(); asm_rname(_rnew); asm_comma(); asm_rname(_rnew); endasmcmd();
  emitmaskb(_rnew);
}
/**
PROC emitshr1rg(BYTE rnew)
{
  asmstr( "\tSRL " ); asm_rhigh(rnew); endasmcmd();
  asmstr( "\tRR " ); asm_rlow(rnew); endasmcmd();
}

PROC emitshr1signedrg(BYTE rnew)
{
  asmstr( "\tSRA " ); asm_rhigh(rnew); endasmcmd();
  asmstr( "\tRR " ); asm_rlow(rnew); endasmcmd();
}
*/
//PROC emitshr1b(BYTE anew)
//{
//  asmstr( "\tSRL " ); /**rganame*/asm_rlow(anew] ); endasmcmd();
//}

PROC emitinvb() //~A -> A
{
  asm_eors(); asm_rname(_rnew); asm_comma(); asm_rff(); endasmcmd();
}

PROC emitinvrg()
{
  asm_mvns(); asm_rname(_rnew); asm_comma(); asm_rname(_rnew); endasmcmd();
}

PROC emitnegrg()
{
  asm_rsbs(); asm_rname(_rnew); asm_comma(); asm_rname(_rnew); asm_comma(); asm_direct(); asmc('0'); endasmcmd();
}

PROC emitSxorVtob() //����� subflags (less), �������������� A
{ //todo ���������
  emit0(_rnew); //��� ��������� ������
  asm_bgt(); asm_pc4_eol();
  asm_eors(); asm_rname(_rnew); asm_comma(); asm_rff(); endasmcmd();
  asm_endpc4();
}

PROC emitinvSxorVtob() //����� subflags (moreeq), �������������� A
{ //todo ���������
  emit0(_rnew); //��� ��������� ������
  asm_blt(); asm_pc4_eol();
  asm_eors(); asm_rname(_rnew); asm_comma(); asm_rff(); endasmcmd();
  asm_endpc4();
}

PROC emitxorrg() //old^new => old
{
  asm_eors(); asm_rname(_rold); asm_comma(); asm_rname(_rnew); endasmcmd();
  _fused = +TRUE; //^^
}

PROC getxorb() //RGA^RGA2 -> RGA
{
  asm_eors(); asm_rname(_rold); asm_comma(); asm_rname(_rnew); endasmcmd();
  _fused = +TRUE; //^^
}

PROC emitorrg() //old|new => old
{
  asm_ors(); asm_rname(_rold); asm_comma(); asm_rname(_rnew); endasmcmd();
  _fused = +TRUE; //||
}

PROC getorb() //RGA|RGA2 -> RGA
{
  asm_ors(); asm_rname(_rold); asm_comma(); asm_rname(_rnew); endasmcmd();
  _fused = +TRUE; //||
}

PROC emitandrg() //old&new => old
{
  asm_ands(); asm_rname(_rold); asm_comma(); asm_rname(_rnew); endasmcmd();
  _fused = +TRUE; //&&
}

PROC getandb() //RGA&RGA2 -> RGA
{
  asm_ands(); asm_rname(_rold); asm_comma(); asm_rname(_rnew); endasmcmd();
  _fused = +TRUE; //&&
}

PROC emitaddrg() //old+new => old
{
  asm_adds(); asm_rname(_rold); asm_comma(); asm_rname(_rnew); endasmcmd();
}

PROC emitadcrg() //old+new => old
{
  asm_adcs(); asm_rname(_rold); asm_comma(); asm_rname(_rnew); endasmcmd();
}

PROC emitaddb() //old+new
{
  asm_adds(); asm_rname(_rold); asm_comma(); asm_rname(_rnew); endasmcmd();
  emitmaskb(_rold);
}

PROC emitaddbconst() //new8+<const>
{
  asm_adds(); asm_rname(_rnew); asm_comma(); asm_direct(); asmmangledstr(_const); endasmcmd();
  emitmaskb(_rnew);
}

PROC emitsubrg() //old-new => old
{
  asm_subs(); asm_rname(_rold); asm_comma(); asm_rname(_rnew); endasmcmd();
}

PROC emitsbcrg() //old-new => old
{
  asm_sbcs(); asm_rname(_rold); asm_comma(); asm_rname(_rnew); endasmcmd();
}

PROC emitsubb() //old-new
{
  asm_subs(); asm_rname(_rold); asm_comma(); asm_rname(_rnew); endasmcmd();
  emitmaskb(_rold);
}

PROC emitsubbconst() //new8-<const>
{
  asm_subs(); asm_rname(_rnew); asm_comma(); asm_direct(); asmmangledstr(_const); endasmcmd();
  emitmaskb(_rnew);
}

PROC emitsubflags(BYTE rnew, BYTE rold) //r2-r1 => CY,sign,overflow
{
  asm_subs(); asm_rname(rold); asm_comma(); asm_rname(rnew); endasmcmd();
  _fused = +TRUE;
}

PROC emitsubbflags(BYTE anew, BYTE aold) //a2-a1 => CY
{ //sign,overflow �� �����!
  asm_subs(); asm_rname(aold); asm_comma(); asm_rname(anew); endasmcmd();
  _fused = +TRUE;
}

PROC emitsubz() //old-new => Z
{
  asm_subs(); asm_rname(_rold); asm_comma(); asm_rname(_rnew); endasmcmd();
  _fused = +TRUE;
}

PROC emitsubbz() //old-new => Z
{
  asm_subs(); asm_rname(_rold); asm_comma(); asm_rname(_rnew); endasmcmd();
  _fused = +TRUE;
}

PROC emitsubbzconst() //new-<const> => Z
{
  asm_subs(); asm_rname(_rnew); asm_comma(); asm_direct(); asmmangledstr(_const); endasmcmd();
  _fused = +TRUE;
}

PROC emitsublongz() //old2-new, old3-old => Z
{ //todo
  _fused = +TRUE;
}

PROC emitpokerg() //����� ���������� � ������ ������� ������
{
  asm_str(); asm_rname(_rnew); asm_comma(); asm_mrname(_rold); endasmcmd();
  _fused = +FALSE; //����� ����������
}

PROC emitpokeb() //����� ���������� � ������ ������� ������
{
  asm_strb(); asm_rname(_rnew); asm_comma(); asm_mrname(_rold); endasmcmd();
  _fused = +FALSE; //����� ����������
}

PROC emitpokelong() //old2(addr), old(high), new(low)
{
  asm_str(); asm_rname(_rnew); asm_comma(); asm_mrname(_rold2); endasmcmd();
  asm_adds(); asm_rname(_rold2); asm_comma(); asmstr("#4"); endasmcmd();
  asm_str(); asm_rname(_rold); asm_comma(); asm_mrname(_rold2); endasmcmd();
  _fused = +FALSE; //����� ����������
}

PROC emitpeekrg() //[new] => new
{
  asm_ldr(); asm_rname(_rnew); asm_comma(); asm_mrname(_rnew); endasmcmd();
}

PROC emitpeekb()
{
  asm_ldrb(); asm_rname(_rnew); asm_comma(); asm_mrname(_rnew); endasmcmd();
}

PROC emitpeeklong() //[old] => old(high),new(low)
{
  asm_ldr(); asm_rname(_rnew); asm_comma(); asm_mrname(_rold); endasmcmd();
  asm_adds(); asm_rname(_rold); asm_comma(); asmstr("#4"); endasmcmd();
  asm_ldr(); asm_rname(_rold); asm_comma(); asm_mrname(_rold); endasmcmd();
}

PROC emitztob()
{
  //asmstr(";emitztoa exprlvl="); asmuint(_exprlvl); endasmcmd();
  IF (_exprlvl != 0x01) { //if (a == b)
    emit0(_rnew); //��� ��������� ������
    asm_bne(); asm_pc4_eol();
    emitinvb();
    asm_endpc4();
    _fused = +FALSE; //����� ���� ��� if ((a==b))
    //_jpflag = 0x00;
  }ELSE {
    //_fused = +TRUE;
    _jpflag = 0x02;
  };
}

PROC emitinvztob()
{
  //asmstr(";emitinvztoa exprlvl="); asmuint(_exprlvl); endasmcmd();
  IF (_exprlvl != 0x01) { //if (a != b)
    emit0(_rnew); //��� ��������� ������
    asm_beq(); asm_pc4_eol();
    emitinvb();
    asm_endpc4();
    _fused = +FALSE; //����� ���� ��� if ((a!=b))? todo test //�������� ���� ifnot ((a!=b))
    //_jpflag = 0x00;
  }ELSE {
    //_fused = +TRUE;
    _jpflag = 0x01;
  };
}

PROC emitcytob()
{
  //asmstr(";emitcytoa exprlvl="); asmuint(_exprlvl); endasmcmd();
  IF (_exprlvl != 0x01) { //if (a < b)
    asm_sbcs(); asm_rname(_rnew); asm_comma(); asm_rname(_rnew); endasmcmd();
    //_fused = +FALSE;
    //_jpflag = 0x00;
  }ELSE {
    //_fused = +TRUE;
    _jpflag = 0x03;
  };
}

PROC emitinvcytob()
{
  //asmstr(";emitinvcytoa exprlvl="); asmuint(_exprlvl); endasmcmd();
  IF (_exprlvl != 0x01) { //if (a >= b)
    asm_sbcs(); asm_rname(_rnew); asm_comma(); asm_rname(_rnew); endasmcmd();
    asm_eors(); asm_rname(_rnew); asm_comma(); asm_rff(); endasmcmd();
    //_fused = +FALSE;
    //_jpflag = 0x00;
  }ELSE {
    //_fused = +TRUE;
    _jpflag = 0x04;
  };
}

PROC emitrgtob() //������ ������� - ����������
{
  emitmaskb(_rnew); //todo ��������, ������������ ��.����� �����, ����� ���������
}

PROC emitbtorg()
{
}

PROC emitincrg_byname()
{
  emitloadrtemp(_rnew,_joined);
  asm_ldr(); asm_rtemp(); asm_comma(); asm_mrname(_rnew); endasmcmd();
  asm_adds(); asm_rtemp(); asm_comma(); asmstr("#1"); endasmcmd();
  asm_str(); asm_rtemp(); asm_comma(); asm_mrname(_rnew); endasmcmd();
  //asm_adds(); asm_rname(_rnew); asm_comma(); asm_direct(); asmc('1'); endasmcmd();
  _fused = +FALSE; //����� ����������
}

PROC emitincb_bypoi()
{
  asm_ldrb(); asm_rtemp(); asm_comma(); asm_mrname(_rnew); endasmcmd();
  asm_adds(); asm_rtemp(); asm_comma(); asmstr("#1"); endasmcmd();
  asm_strb(); asm_rtemp(); asm_comma(); asm_mrname(_rnew); endasmcmd();
  _fused = +FALSE; //����� ����������
}
/**
PROC emitinclong() //todo
{
  asm_inc(); asm_open(); asm_rname(_rnew); asm_close(); endasmcmd();
  _fused = +FALSE; //����� ����������
}
*/
PROC emitdecrg_byname()
{
  emitloadrtemp(_rnew,_joined);
  asm_ldr(); asm_rtemp(); asm_comma(); asm_mrname(_rnew); endasmcmd();
  asm_subs(); asm_rtemp(); asm_comma(); asmstr("#1"); endasmcmd();
  asm_str(); asm_rtemp(); asm_comma(); asm_mrname(_rnew); endasmcmd();
  //asm_subs(); asm_rname(_rnew); asm_comma(); asmstr("#1"); endasmcmd();
  _fused = +FALSE; //����� ����������
}

PROC emitdecb_bypoi()
{
  asm_ldrb(); asm_rtemp(); asm_comma(); asm_mrname(_rnew); endasmcmd();
  asm_subs(); asm_rtemp(); asm_comma(); asmstr("#1"); endasmcmd();
  asm_strb(); asm_rtemp(); asm_comma(); asm_mrname(_rnew); endasmcmd();
  _fused = +FALSE; //����� ����������
}
/**
PROC emitdeclong() //todo
{
  asm_dec(); asm_open(); asm_rname(_rnew); asm_close(); endasmcmd();
  _fused = +FALSE; //����� ����������
}
*/

PROC emitincrg_bypoi() //[old], new free
{
  asm_ldr(); asm_rtemp(); asm_comma(); asm_mrname(_rnew); endasmcmd();
  asm_adds(); asm_rtemp(); asm_comma(); asmstr("#1"); endasmcmd();
  asm_str(); asm_rtemp(); asm_comma(); asm_mrname(_rnew); endasmcmd();
  _fused = +FALSE; //����� ����������
}

PROC emitdecrg_bypoi() //[old], new free
{
  asm_ldr(); asm_rtemp(); asm_comma(); asm_mrname(_rnew); endasmcmd();
  asm_subs(); asm_rtemp(); asm_comma(); asmstr("#1"); endasmcmd();
  asm_str(); asm_rtemp(); asm_comma(); asm_mrname(_rnew); endasmcmd();
  _fused = +FALSE; //����� ����������
}

/////////////
EXPORT PROC initcode()
{
//  _wass = (PCHAR)_swass;
  _jpflag = 0x00;
  constindex = 0;
  asmcmds = 0;
  asmstr("\tALIGN 2"); endasm();
}

EXPORT PROC endcode()
{
  asmflushlabels();
  asmstr("\tEND"); endasmcmd();
  varstr("\tEND"); endvar();
}

PROC initrgs()
{
  rgs_initrgs();
  _fused = +FALSE; //����� ������� ���� ��������� initif ��� ����� (�������� � ������ if, while, until)
}

PROC emitfunchead()
{
  asm_push(); asmstr( "{LR}" ); endasmcmd();
  initrgs();
}

PROC setmainb()
{
  setmainrg(); //��������� � RMAIN
}

PROC prefernoregs()
{
}


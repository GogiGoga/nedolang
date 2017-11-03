//// imported
//�����, ��� ����� ��������� ��������, ������������ _joined (����� call - ��� _callee)
#include "../_sdk/typecode.h"
#include "../_sdk/str.h"
#include "../_sdk/emit.h"

#ifdef TARGET_THUMB
#include "sizesarm.h"
#else
#include "sizesz80.h"
#endif

CONST BYTE _typeshift[32]; //log ������ ���� (n ��� 2^n ����)
CONST BYTE _typesz[32];

CONST BYTE _RMAIN; //������� ���������� � 1-�� ��������� ����������� �������
//#endif

EXPORT VAR TYPE _t; //������� ��� (��� cmds)
EXPORT VAR BOOL _isloc; //��������� �� ����������� ����������

//EXTERN PCHAR _tword; //������� �����
//EXTERN UINT _lentword;
EXPORT VAR PCHAR _name; //����� ��� �������� (��� ������� �����)
EXPORT VAR UINT  _lenname;
EXPORT VAR PCHAR _joined; //���������
EXPORT VAR UINT  _lenjoined;

#ifdef TARGET_THUMB
#include "codearm.c"
#else
#include "codez80.c"
#endif

#include "regs.c"

////

VAR PCHAR _const; //������ ���������
VAR UINT  _lenconst;
//VAR PCHAR _wastword; //������ ���������
//VAR UINT  _lenwastword;
VAR BOOL _wasconst;
VAR CHAR  _sc[_STRLEN]; //������ ���������
//VAR CHAR  _sw[_STRLEN]; //������ tword
VAR TYPE _wast;

VAR BYTE _sz; //�������� ������ ���� - ������������ � ������ ����������

#define _STRSTKMAX 256
VAR CHAR _strstk[_STRSTKMAX]; //str1+<len>+str2+<len>...+strN+<len> //���� ' '+str1+' '+str2+...+' '+strN
EXPORT VAR UINT _lenstrstk;

#ifdef BIGMEM
#define _LBLBUFSZ 0xffff
#else
#define _LBLBUFSZ 0x2300 /**0x2100*/
#endif
VAR UINT _lblbuffreeidx; //[1];
VAR UINT _oldlblbuffreeidx; //[1];
CONST UINT _LBLBUFEOF = 0xffff;
#define _LBLBUFMAXSHIFT (UINT)(_LBLBUFSZ-_STRLEN-20)
//todo ����������� ��������
VAR UINT _lblshift[0x100];
VAR UINT _oldlblshift[0x100];
VAR BYTE _lbls[_LBLBUFSZ];
VAR BYTE _lblhash;

VAR UINT _typeaddr;
EXPORT VAR UINT _varszaddr;
EXPORT VAR UINT _varsz;

PROC cmdshl1 FORWARD();

PROC errtype(PCHAR msg, TYPE t)
{
  errstr("operation "); errstr(msg); errstr(" bad type="); erruint(+(UINT)t); enderr();
}

EXPORT PROC strpush(PCHAR s, UINT len) //joined ��� callee
{ //str1+<len>+str2+<len>...+strN+<len> (� �������������)
  IF ((_lenstrstk+len+2) > _STRSTKMAX) {
    errstr("STRING STACK OVERFLOW"); enderr();
  }ELSE {
    strcopy(s, len, &_strstk[_lenstrstk]);
    _lenstrstk = _lenstrstk + len + 1;
    _strstk[_lenstrstk] = +(CHAR)(+(BYTE)len);
    INC _lenstrstk;
  };
}

EXPORT FUNC UINT strpop(PCHAR s)
{ //str1+<len>+str2+<len>...+strN+<len> (� �������������)
VAR UINT len;
  DEC _lenstrstk;
  len = +(UINT)(+(BYTE)(_strstk[_lenstrstk]));
  _lenstrstk = _lenstrstk - len - 1;
  strcopy((PCHAR)(&_strstk[_lenstrstk]), len, s);
RETURN len;
}

EXPORT PROC initlblbuf()
{
  _lblhash = 0x00;
  REPEAT {
    _lblshift[_lblhash] = _LBLBUFEOF;
    INC _lblhash;
  }UNTIL (_lblhash == 0x00);
  _lblbuffreeidx = 0;
};

EXPORT FUNC UINT gettypename(PCHAR s) //����� �������� ���� ��������� � joined (����� ����� lbltype)
{
RETURN strcopy((PCHAR)&_lbls[_typeaddr], (UINT)*(PBYTE)&_lbls[_typeaddr-1], s);
}

EXPORT PROC setvarsz(UINT addr, UINT shift)
{
  POKE *(PUINT)(&_lbls[addr]) = shift;
}

EXPORT FUNC TYPE lbltype() //������� ��� ����� _name
{
VAR PBYTE plbl; //����� � ������� ������������� ����
VAR TYPE t;
VAR UINT plbl_idx;
  t = _T_UNKNOWN;
  _lblhash = +(BYTE)hash((PBYTE)_name);
  plbl_idx = _lblshift[_lblhash];
  WHILE (plbl_idx != _LBLBUFEOF) { //���� ������� ����� �� �����������
    plbl = &_lbls[plbl_idx];
    plbl_idx = *(PUINT)(plbl);
    plbl = &plbl[+sizeof(UINT)+1]; //skip string size
    IF (strcp((PCHAR)_name, (PCHAR)plbl)) { //����� �������
      _typeaddr = (UINT)(plbl - _lbls); //��� ����������� ���� � ������� ����������
      plbl = &plbl[_lenname+1];
      t = *(TYPE*)(plbl);
      plbl = &plbl[+sizeof(TYPE)]; //INC plbl;
      _isloc = (BOOL)*(PBYTE)(plbl);
      INC plbl;
      IF ((t&_T_TYPE)==(TYPE)0x00) _typeaddr = *(PUINT)(plbl); //���������� ����� ����, ���� ��� ����������, � �� ���������� ����
      plbl = &plbl[+sizeof(UINT)];
      _varsz = *(PUINT)(plbl);
      break;
    };
  };
RETURN t;
}

EXPORT PROC dellbl() //��� undef
{
VAR PBYTE plbl; //����� � ������� ������������� ����
VAR UINT plbl_idx;
  _lblhash = +(BYTE)hash((PBYTE)_name);
  plbl_idx = _lblshift[_lblhash];
  WHILE (plbl_idx != _LBLBUFEOF) { //���� ������� ����� �� �����������
    plbl = &_lbls[plbl_idx];
    plbl_idx = *(PUINT)(plbl);
    plbl = &plbl[+sizeof(UINT)+1]; //skip string size
    IF (strcp((PCHAR)_name, (PCHAR)plbl)) { //����� �������
      POKE *(PCHAR)(plbl) = '\0';
      break;
    };
  };
}

EXPORT PROC addlbl(TYPE t, BOOL isloc, UINT varsz/**, PCHAR size, UINT lensize*/) //(_name)
{
VAR PBYTE plbl;
VAR UINT freeidx;
VAR TYPE oldt;
  oldt = lbltype(); //(_name) //������������� _isloc (���� �������) � _typeaddr (�����, ���� �������)
  IF ((oldt == _T_UNKNOWN)||isloc/**(isloc&&!_isloc)*/) { //���� �� ���� ����� ��� ��������� ������ ���������� ��� ������ ��������� (���������)
    //����� ���: ����� � ������ ������� ����� ����� �������� � ������ ����� ��� �� ������� �� ������ ������ �������
    freeidx = _lblbuffreeidx; //[0] //������ ���������� �����
    IF (freeidx < _LBLBUFMAXSHIFT) { //���� ����� ��� �����
      plbl = &_lbls[freeidx]; //��������� �� ������ ����������� �����
      //����� �����
      POKE *(PUINT)(plbl) = _lblshift[_lblhash]; //������ ��������� �� ������ �������
      plbl = &plbl[+sizeof(UINT)];
      POKE *(PBYTE)(plbl) = (BYTE)_lenname;
      INC plbl;
      strcopy(_name, _lenname, (PCHAR)plbl);
      plbl = &plbl[_lenname+1];
      POKE *(TYPE*)(plbl) = t;
      plbl = &plbl[+sizeof(TYPE)]; //INC plbl;
      POKE *(PBYTE)(plbl) = (BYTE)isloc;
      INC plbl;
      POKE *(PUINT)(plbl) = _typeaddr; //������ �� �������� ���� (��� ���������)
      plbl = &plbl[+sizeof(UINT)];
      _varszaddr = (UINT)(plbl - _lbls); //����� ����� ����� ���� ������
      POKE *(PUINT)(plbl) = varsz;
      _lblbuffreeidx = +(UINT)(plbl - _lbls) + +sizeof(UINT); //��������� ����� ����������� �����
      _lblshift[_lblhash] = freeidx; //����� ��������� �� ������ �������
    }ELSE {errstr("nomem"); enderr();
    };
  }ELSE IF (oldt != t) {
    errstr("addvar type doesn't match previous declaration:"); errstr(_name); enderr();
  };
}

//#define _LBLBUFSZ (UINT)(+sizeof(UINT)*0x100) /**������ sizeof � ����*/
EXPORT PROC keepvars() //����� ������� ��������� �����
{
  memcopy((PBYTE)_lblshift, +sizeof(UINT)*0x100 /**todo const*/, (PBYTE)_oldlblshift);
  _oldlblbuffreeidx = _lblbuffreeidx;
}

EXPORT PROC undovars() //����� ��������� ����� (������ ��)
{
  memcopy((PBYTE)_oldlblshift, +sizeof(UINT)*0x100 /**todo const*/, (PBYTE)_lblshift);
  _lblbuffreeidx = _oldlblbuffreeidx;
}

//////////////////////////////// MATH ///////////////////////////////////////
//��������� �� ����� � �� ������� ������ ���������

EXPORT PROC var_alignwsz_label(TYPE t)
{
  IF (_typesz[t&_TYPEMASK]!=_SZ_BYTE) {
    var_alignwsz();
  };
  emitvarlabel(_joined);
}

EXPORT PROC var_def(TYPE t, PCHAR s) //�������� �� compile!
{
VAR BYTE sz = _typesz[t];
  IF (sz==_SZ_BYTE/**(tmasked==_T_BYTE)||(tmasked==_T_CHAR)||(tmasked==_T_BOOL)*/) {
    var_db(); varstr(s); endvar();
  }ELSE IF (sz==_SZ_REG/**(tmasked==_T_INT)||(tmasked==_T_UINT)*/) {
    var_dw(); varstr(s); endvar(); //������ DW ����� uint
  }ELSE IF (sz==_SZ_LONG/**tmasked==_T_LONG*/) {
    var_dl(); varstr(s); endvar();
  }ELSE { errstr( "const bad type " ); erruint(+(UINT)t); enderr(); };
}

PROC pushconst() //��������� �����, ������� �� ��������� ��� pushnum
{ //������ �������� joined
#ifdef USE_COMMENTS
;;  cmtstr( ";PUSHCONST " ); cmtstr( _const ); endcmt();
#endif
  //_lenwastword = strcopy(_joined, _lenjoined, _wastword);
  //_lenjoined = strcopy(_const, _lenconst, _joined);
  _sz = _typesz[_wast];
  IF (_sz==_SZ_BYTE) { //(_wast==_T_BYTE)||(_wast==_T_CHAR)||(_wast==_T_BOOL)
    getrfree(); //_rnew
    emitloadb();
  }ELSE IF (_sz==_SZ_REG) { //(_wast==_T_INT)||(_wast==_T_UINT)||(_wast>=_T_POI)
    getrfree(); //_rnew
    emitloadrg(+FALSE);
  }ELSE IF (_sz==_SZ_LONG) { //_wast==_T_LONG
    getrfree(); //_rnew
    emitloadrg(+TRUE); //high
    getrfree(); //_rnew
    emitloadrg(+FALSE); //low
  }ELSE errtype("pushconst",_wast);
  //_lenjoined = strcopy(_wastword, _lenwastword, _joined);
  _wasconst = +FALSE;
}

EXPORT PROC cmdpushnum()
{
#ifdef USE_COMMENTS
;;  cmtstr(";PUSHNUM "); cmtstr(_joined); endcmt();
#endif
  IF (_wasconst) pushconst();
  _lenconst = strcopy(_joined, _lenjoined, _const);
  _wast = _t;
  _wasconst = +TRUE;
}

EXPORT PROC cmdpushvar()
{
#ifdef USE_COMMENTS
;;  cmtstr(";PUSHVAR "); cmtstr(_joined); endcmt();
#endif
  IF (_wasconst) pushconst();
  _sz = _typesz[_t];
  IF (_sz==_SZ_BYTE/**(_t==_T_BYTE)||(_t==_T_CHAR)||(_t==_T_BOOL)*/) {
    getrfree(); //_rnew
    emitgetb();
  }ELSE IF (_sz==_SZ_REG/**(_t==_T_INT)||(_t==_T_UINT)||(_t>=_T_POI)*/) {
    getrfree(); //_rnew
    emitgetrg(+FALSE);
  }ELSE IF (_sz==_SZ_LONG/**_t==_T_LONG*/) {
    getrfree(); //_rnew
    emitgetrg(+TRUE); //high
    getrfree(); //_rnew
    emitgetrg(+FALSE); //low
  }ELSE errtype("pushvar",_t);
}

EXPORT PROC cmdpopvar()
{
#ifdef USE_COMMENTS
;;  cmtstr(";POPVAR "); cmtstr(_joined); endcmt();
#endif
  IF (_wasconst) pushconst();
  _sz = _typesz[_t];
  IF (_sz==_SZ_BYTE/**(_t==_T_BYTE)||(_t==_T_CHAR)||(_t==_T_BOOL)*/) {
    getrnew();
    emitputb();
    freernew();
  }ELSE IF (_sz==_SZ_REG/**(_t==_T_INT)||(_t==_T_UINT)||(_t>=_T_POI)*/) {
    getrnew();
    emitputrg(+FALSE);
    freernew();
  }ELSE IF (_sz==_SZ_LONG/**_t==_T_LONG*/) {
    getrnew();
    emitputrg(+FALSE); //low
    freernew();
    getrnew();
    emitputrg(+TRUE); //high
    freernew();
  }ELSE errtype("popvar",_t);
}

EXPORT PROC cmdcastto(TYPE t2)
{
VAR BYTE tsz;
VAR BYTE t2sz;
#ifdef USE_COMMENTS
;;  cmtstr(";OPERATION cast "); cmtuint(+(UINT)_t); cmt('>'); cmtuint(+(UINT)t2); endcmt();
#endif
  tsz = _typesz[_t];
  t2sz = _typesz[t2];
  IF (tsz==t2sz/**_t == t2*/) { //���� ������ ������� - �� ������ ���������
    //todo badcast LONG<=>FLOAT
  }ELSE {
    IF (_wasconst) pushconst(); //�� ������ ��������� ��� BYTE<=>CHAR
    IF (tsz==_SZ_BYTE/**_t==_T_BYTE*/) { //BYTE =>
      IF (t2sz==_SZ_REG/**(t2==_T_INT)||(t2==_T_UINT)*/) { //BYTE => INT|UINT
        getrnew();
        emitbtorg();
      }ELSE /**IF (t2sz==_SZ_LONG)*/ { //BYTE => LONG
        getrnew();
        emitbtorg();
        getrfree(); //_rnew
        emitloadrg0(); //������ old = rg, new = 0
        swaptop(); //������ old = 0, new = rg
      //}ELSE IF (t2==_T_CHAR) { //BYTE => CHAR
      //}ELSE IF (t2==_T_BOOL) { //BYTE => BOOL (for serialization)
      };//ELSE goto bad;
    }ELSE IF (tsz==_SZ_REG/**(_t==_T_INT)||(_t==_T_UINT)*/) {
      IF (t2sz==_SZ_BYTE/**t2==_T_BYTE*/) { //INT|UINT => BYTE
        getrnew();
        emitrgtob();
      }ELSE /**IF (t2sz==_SZ_LONG)*/ { //INT|UINT => LONG
        getrfree(); //_rnew
        //IF (_t==_T_UINT) { //UINT => LONG
          emitloadrg0(); //������ old = rg, new = 0
        //}ELSE IF (_t==_T_UINT) { //INT => LONG
        //};
        swaptop(); //������ old = 0, new = rg
      //}ELSE IF (t2>=_T_POI) { //UINT => POINTER (INT possible but not desirable)
      };//ELSE goto bad;
    }ELSE IF (tsz==_SZ_LONG) { //LONG =>
      IF (t2sz==_SZ_REG/**(t2==_T_INT)||(t2==_T_UINT)*/) { //LONG => INT|UINT
        swaptop(); //������ old = low, new = high
        freernew(); //high
      }ELSE /**IF (t2sz==_SZ_BYTE)*/ { //LONG => BYTE
        swaptop(); //������ old = low, new = high
        freernew(); //high
        getrnew();
        emitrgtob();
/**      }ELSE { //LONG<=>FLOAT
        bad:
        errstr("bad cast: "); erruint(+(UINT)_t); err('>'); erruint(+(UINT)t2); enderr();*/
      };
    //}ELSE IF ( (_t==_T_CHAR) && (t2==_T_BYTE) ) { //CHAR => BYTE
    //}ELSE IF ( (_t==_T_BOOL) && (t2==_T_BYTE) ) { //BOOL => BYTE (for serialization)
    //}ELSE IF ( (_t>=_T_POI) && ((t2==_T_UINT)||(t2>=_T_POI)) ) { //POINTER => UINT|POINTER (conversion between pointers)
    //}ELSE IF ((t2&_T_POI)!=0x00) { // anything (i.e. array) => POINTER
//    }ELSE {goto bad; //�� ���� ������� �� ���������
    };
  };
  _t = t2;
}

EXPORT PROC cmdadd()
{
#ifdef USE_COMMENTS
;;  cmtstr(";OPERATION +"); endcmt();
#endif
  IF (_t==_T_BYTE) {
    IF (_wasconst) {
      getrnew();
      emitaddbconst();
      _wasconst = +FALSE;
    }ELSE {
      getrnew();
      getrold();
      emitaddb();
      freernew();
    };
  }ELSE {
    IF (_wasconst) pushconst();
    IF (_typesz[_t]==_SZ_REG/**(_t==_T_INT)||(_t==_T_UINT)||(_t>=_T_POI)*/) {
      getrold(); //���� ������, ����� �������� ������� �������
      getrnew();
      emitaddrg(); //rold+rnew => rold
      freernew();
    }ELSE IF (_t==_T_LONG) { //�� float
      getrold2(); //oldlow //���� ������, ����� �������� ������� �������
      getrnew(); //newlow
      emitaddrg(); //old2+new => old2
      freernew();
      //������ new=newhigh, old=(oldlow+newlow)
      getrold2(); //oldhigh �� ����� //���� ������, ����� �������� ������� �������
      getrnew(); //newhigh
      emitadcrg(); //old2+new+CY => old2
      freernew();
      //������ new=(oldlow+newlow), old=(oldhigh+newhigh+CY)
    }ELSE errtype("+",_t);
  };
}

EXPORT PROC cmdsub() //�� ������� ������� �����!
{
#ifdef USE_COMMENTS
;;  cmtstr( ";OPERATION -" ); endcmt();
#endif
  IF (_t==_T_BYTE) {
    IF (_wasconst) {
      getrnew();
      emitsubbconst();
      _wasconst = +FALSE;
    }ELSE {
      getrnew();
      getrold();
      emitsubb();
      freernew();
    };
  }ELSE {
    IF (_wasconst) pushconst();
    IF (_typesz[_t]==_SZ_REG/**(_t==_T_INT)||(_t==_T_UINT)||(_t>=_T_POI)*/) {
      getrold(); //���� ������, ����� �������� ������� �������
      getrnew();
      emitsubrg(); //rold-rnew => rold
      freernew();
      //IF(_t>=_T_POI) {t = _T_INT;};
    }ELSE IF (_t==_T_LONG) { //�� float
      getrold2(); //oldlow //���� ������, ����� �������� ������� �������
      getrnew(); //newlow
      emitsubrg(); //old2-new => old2
      freernew();
      //������ new=newhigh, old=(oldlow-newlow)
      getrold2(); //oldhigh �� ����� //���� ������, ����� �������� ������� �������
      getrnew(); //newhigh
      emitsbcrg(); //old2-new-CY => old2
      freernew();
      //������ new=(oldlow-newlow), old=(oldhigh-newhigh-CY)
    }ELSE errtype("-",_t);
  };
}

EXPORT PROC cmdaddpoi() //_t �������� ��� ������ �������
{
VAR BYTE i;
#ifdef USE_COMMENTS
;;  cmtstr(";OPERATION +poi"); endcmt();
#endif
  IF (_wasconst) pushconst();
/**  i = _typeshift[_t&_TYPEMASK];
  WHILE (i != 0x00) {
    _t = _T_INT;
    cmdshl1();
    DEC i;
  };*/
  getrold(); //���� ������, ����� �������� ������� �������
  getrnew();
  i = _typesz[_t&_TYPEMASK];
  WHILE (i != 0x00) {
    emitaddrg(); //rold+rnew => rold
    DEC i;
  };
  freernew();
}

EXPORT PROC cmdmul()
{
#ifdef USE_COMMENTS
;;  cmtstr( ";OPERATION *" ); endcmt();
#endif
  IF (_wasconst) pushconst();
  IF (_t==_T_BYTE) {emitcall2rgs("_MULB.");
  }ELSE IF (_t==_T_UINT) {emitcall2rgs("_MUL.");
  }ELSE IF (_t==_T_INT) {emitcall2rgs("_MULSIGNED.");
  }ELSE IF (_t==_T_LONG) {emitcall4rgs("_MULLONG.");
  }ELSE errtype("*",_t);
}

EXPORT PROC cmddiv() //������ ��������� �� �����!
{
#ifdef USE_COMMENTS
;;  cmtstr(";OPERATION /"); endcmt();
#endif
  IF (_wasconst) pushconst();
  IF (_t==_T_BYTE) {emitcall2rgs("_DIVB.");
  }ELSE IF (_t==_T_UINT) {emitcall2rgs("_DIV.");
  }ELSE IF (_t==_T_INT) {emitcall2rgs("_DIVSIGNED.");
  }ELSE IF (_t==_T_LONG) {emitcall4rgs("_DIVLONG.");
  }ELSE errtype("/",_t);
}

EXPORT PROC cmdshl() //������ �������� ������� ���, ������� ������ �����!
{
#ifdef USE_COMMENTS
;;  cmtstr( ";OPERATION shl" ); endcmt();
#endif
  IF (_wasconst) pushconst();
  IF (_t==_T_BYTE) {emitcall2rgs("_SHLB.");
  }ELSE IF ( (_t==_T_INT)||(_t==_T_UINT) ) {emitcall2rgs("_SHL.");
  }ELSE IF (_t==_T_LONG) {emitcall4rgs("_SHLLONG.");
  }ELSE errtype("<<",_t);
}

EXPORT PROC cmdshr() //������ �������� ������� ���, ������� ������ �����!
{
#ifdef USE_COMMENTS
;;  cmtstr( ";OPERATION shr" ); endcmt();
#endif
  IF (_wasconst) pushconst();
  IF (_t==_T_BYTE) {emitcall2rgs("_SHRB.");
  }ELSE IF (_t==_T_UINT) {emitcall2rgs("_SHR.");
  }ELSE IF (_t==_T_INT) {emitcall2rgs("_SHRSIGNED.");
  }ELSE IF (_t==_T_LONG) {emitcall4rgs("_SHRLONG.");
  }ELSE errtype(">>",_t);
}

EXPORT PROC cmdshl1()
{
#ifdef USE_COMMENTS
;;  cmtstr( ";SHL1" ); endcmt();
#endif
  IF (_wasconst) pushconst();
  IF (_t==_T_BYTE) {
    getrnew();
    emitshl1b();
  }ELSE IF ( (_t==_T_INT)||(_t==_T_UINT) ) {
    getrnew();
    emitshl1rg();
  //}ELSE IF (_t==_T_LONG) { //todo
  }ELSE errtype("shl1",_t);
}
/**
EXPORT PROC cmdshr1()
{
#ifdef USE_COMMENTS
;;  cmtstr( ";SHR1" ); endcmt();
#endif
  IF (_wasconst) pushconst();
  IF ( (_t==_T_BYTE)||(_t==_T_CHAR) ) {emitshr1rgb(getrnew());
  }ELSE IF (_t==_T_INT) {emitshr1signedrg(getrnew());
  }ELSE IF (_t==_T_UINT) {emitshr1rg(getrnew());
  }ELSE errtype("shr1",_t);
}
*/
EXPORT PROC cmdand()
{
#ifdef USE_COMMENTS
;;  cmtstr( ";OPERATION &" ); endcmt();
#endif
  IF (_wasconst) pushconst();
  _sz = _typesz[_t];
  IF (_sz==_SZ_BYTE) {
    getrnew();
    getrold();
    getandb();
    freernew();
  }ELSE IF (_sz==_SZ_REG) {
    getrnew();
    getrold();
    emitandrg();
    freernew();
  }ELSE /**IF (_sz==_SZ_LONG)*/ {
    getrnew();
    getrold2();
    emitandrg(); //low
    freernew();
    getrnew();
    getrold2();
    emitandrg(); //high
    freernew();
  };//ELSE errtype("&",_t);
}

EXPORT PROC cmdor()
{
#ifdef USE_COMMENTS
;;  cmtstr( ";OPERATION |" ); endcmt();
#endif
  IF (_wasconst) pushconst();
  _sz = _typesz[_t];
  IF (_sz==_SZ_BYTE) {
    getrnew();
    getrold();
    getorb();
    freernew();
  }ELSE IF (_sz==_SZ_REG) {
    getrnew();
    getrold();
    emitorrg();
    freernew();
  }ELSE /**IF (_sz==_SZ_LONG)*/ {
    getrnew();
    getrold2();
    emitorrg(); //low
    freernew();
    getrnew();
    getrold2();
    emitorrg(); //high
    freernew();
  };//ELSE errtype("|",_t);
}

EXPORT PROC cmdxor()
{
#ifdef USE_COMMENTS
;;  cmtstr( ";OPERATION ^" ); endcmt();
#endif
  IF (_wasconst) pushconst();
  _sz = _typesz[_t];
  IF (_sz==_SZ_BYTE) {
    getrnew();
    getrold();
    getxorb();
    freernew();
  }ELSE IF (_sz==_SZ_REG) {
    getrnew();
    getrold();
    emitxorrg();
    freernew();
  }ELSE /**IF (_sz==_SZ_LONG)*/ {
    getrnew();
    getrold2();
    emitxorrg(); //low
    freernew();
    getrnew();
    getrold2();
    emitxorrg(); //high
    freernew();
  };//ELSE errtype("^",_t);
}

EXPORT PROC cmdpoke() //����� ���������� � ������ ������� ������
{ //_t = ��� ������ ��� ����������
#ifdef USE_COMMENTS
;;  cmtstr(";POKE"); endcmt();
#endif
  IF (_wasconst) pushconst();
  _sz = _typesz[_t];
  IF (_sz==_SZ_BYTE/**(_t==_T_BYTE)||(_t==_T_CHAR)||(_t==_T_BOOL)*/) {
    getrnew();
    getrold();
    emitpokeb();
    freernew();
    freernew();
  }ELSE IF (_sz==_SZ_REG/**(_t==_T_INT)||(_t==_T_UINT)||(_t>=_T_POI)*/) {
    getrnew();
    getrold();
    emitpokerg();
    freernew();
    freernew();
  }ELSE /**IF (_sz==_SZ_LONG)*/ {
    getrnew();
    getrold2(); _rold2 = _rold;
    getrold();
    emitpokelong();
    freernew();
    freernew();
    freernew();
  };//ELSE errtype("poke",_t); //�� ���� �������
}

EXPORT PROC cmdpeek() //_t = ��� ������ ��� ����������, �� const
{
#ifdef USE_COMMENTS
;;  cmtstr( ";PEEK" ); endcmt();
#endif
  IF (_wasconst) pushconst();
  _sz = _typesz[_t];
  IF (_sz==_SZ_BYTE/**(_t==_T_BYTE)||(_t==_T_CHAR)||(_t==_T_BOOL)*/) {
    getrnew();
    emitpeekb();
  }ELSE IF (_sz==_SZ_REG/**(_t==_T_INT)||(_t==_T_UINT)||(_t>=_T_POI)*/) {
    getrnew();
    emitpeekrg();
  }ELSE /**IF (_sz==_SZ_LONG)*/ {
    getrfree(); //_rnew
    getrnew();
    getrold();
    emitpeeklong();
  };//ELSE errtype("peek",_t); //�� ���� �������
}

EXPORT PROC cmdneg()
{
#ifdef USE_COMMENTS
;;  cmtstr( ";NEG" ); endcmt();
#endif
  IF (_wasconst) pushconst();
  IF ( (_t==_T_INT)||(_t==_T_UINT) ) {
    getrnew();
    emitnegrg();
  }ELSE errtype("neg",_t);
}

EXPORT PROC cmdinv()
{
#ifdef USE_COMMENTS
;;  cmtstr( ";INV" ); endcmt();
#endif
  IF (_wasconst) pushconst();
  _sz = _typesz[_t];
  IF (_sz==_SZ_BYTE) {
    getrnew();
    emitinvb();
  }ELSE IF (_sz == _SZ_REG) {
    getrnew();
    emitinvrg();
  }ELSE /**IF (_t==_T_LONG)*/ {
    getrnew();
    emitinvrg();
    getrold(); _rnew = _rold;
    emitinvrg();
  };//ELSE errtype("inv",_t);
}

EXPORT PROC cmdinc()
{
#ifdef USE_COMMENTS
;;  cmtstr(";OPERATION ++"); endcmt();
#endif
  IF (_wasconst) pushconst();
  IF (_t==_T_BYTE) {
    prefernoregs(); //������ [hl]
    _t = _T_POI|_T_BYTE; cmdpushnum();
    IF (_wasconst) pushconst();
    getrnew();
    emitincb_bypoi();
    freernew();
  }ELSE IF (_typesz[_t]==_SZ_REG/**(_t==_T_INT)||(_t==_T_UINT)||(_t>=_T_POI)*/) {
    getrfree(); //_rnew
    //emitgetrg(+FALSE);
    emitincrg_byname();
    //emitputrg(+FALSE);
    freernew();
/**  }ELSE IF (_t==_T_LONG) {
    emitinclong();
*/
  }ELSE errtype("inc",_t);
}

EXPORT PROC cmddec()
{
#ifdef USE_COMMENTS
;;  cmtstr( ";OPERATION --" ); endcmt();
#endif
  IF (_wasconst) pushconst();
  IF (_t==_T_BYTE) {
    prefernoregs(); //������ [hl]
    _t = _T_POI|_T_BYTE; cmdpushnum();
    IF (_wasconst) pushconst();
    getrnew();
    emitdecb_bypoi();
    freernew();
  }ELSE IF (_typesz[_t]==_SZ_REG/**(_t==_T_INT)||(_t==_T_UINT)||(_t>=_T_POI)*/) {
    getrfree(); //_rnew
    //emitgetrg(+FALSE);
    emitdecrg_byname();
    //emitputrg(+FALSE);
    freernew();
/**  }ELSE IF (_t==_T_LONG) {
    emitdeclong();
*/
  }ELSE errtype("dec",_t);
}

EXPORT PROC cmdincbyaddr()
{
#ifdef USE_COMMENTS
;;  cmtstr(";OPERATION ++byaddr"); endcmt();
#endif
  IF (_wasconst) pushconst();
  getrnew();
  IF (_t==_T_BYTE) {
    emitincb_bypoi(); //[new]
  }ELSE IF (_typesz[_t]==_SZ_REG/**(_t==_T_INT)||(_t==_T_UINT)||(_t>=_T_POI)*/) {
    getrfree();
    getrold();
    emitincrg_bypoi(); //[old]
    freernew();
/**  }ELSE IF (_t==_T_LONG) {
    emitinclong();
*/
  }ELSE errtype("incbypoi",_t);
  freernew();
}

EXPORT PROC cmddecbyaddr()
{
#ifdef USE_COMMENTS
;;  cmtstr(";OPERATION --byaddr"); endcmt();
#endif
  IF (_wasconst) pushconst();
  getrnew();
  IF (_t==_T_BYTE) {
    emitdecb_bypoi(); //[new]
  }ELSE IF (_typesz[_t]==_SZ_REG/**(_t==_T_INT)||(_t==_T_UINT)||(_t>=_T_POI)*/) {
    getrfree();
    getrold();
    emitdecrg_bypoi(); //[old]
    freernew();
/**  }ELSE IF (_t==_T_LONG) {
    emitdeclong();
*/
  }ELSE errtype("decbypoi",_t);
  freernew();
}

//////////////////////////////////
//���������

EXPORT PROC cmdless() //������ ������ ������
{
#ifdef USE_COMMENTS
;;  cmtstr( ";OPERATION <" ); endcmt();
#endif
  IF (_wasconst) pushconst();
  IF (_t==_T_BYTE) {
    getrnew();
    getrold();
    emitsubbflags(_rnew, _rold); //old-new
    freernew();
    freernew();
    getrfree(); //_rnew
    emitcytob();
  }ELSE IF (_t==_T_UINT) {
    getrnew();
    getrold();
    emitsubflags(_rnew, _rold); //old-new
    freernew();
    freernew();
    getrfree(); //_rnew
    emitcytob();
  }ELSE IF (_t==_T_INT) {
    getrnew();
    getrold();
    emitsubflags(_rnew, _rold); //old-new
    freernew();
    freernew();
    getrfree(); //_rnew
    emitSxorVtob(); //S xor overflow
  //}ELSE IF (_t==_T_LONG) {
  }ELSE errtype("<",_t);
  _t = _T_BOOL;
}

EXPORT PROC cmdmore() //������ ������ ������
{
#ifdef USE_COMMENTS
;;  cmtstr( ";OPERATION >" ); endcmt();
#endif
  IF (_wasconst) pushconst();
  IF (_t==_T_BYTE) {
    getrold();
    getrnew();
    emitsubbflags(_rold, _rnew); //new-old
    freernew();
    freernew();
    getrfree(); //_rnew
    emitcytob();
  }ELSE IF (_t==_T_UINT) {
    getrold();
    getrnew();
    emitsubflags(_rold, _rnew); //new-old
    freernew();
    freernew();
    getrfree(); //_rnew
    emitcytob();
  }ELSE IF (_t==_T_INT) {
    getrold();
    getrnew();
    emitsubflags(_rold, _rnew); //new-old
    freernew();
    freernew();
    getrfree(); //_rnew
    emitSxorVtob(); //S xor overflow
  //}ELSE IF (_t==_T_LONG) {
  }ELSE errtype(">",_t);
  _t = _T_BOOL;
}

EXPORT PROC cmdlesseq() //������ <= ������
{
#ifdef USE_COMMENTS
;;  cmtstr( ";OPERATION <=" ); endcmt();
#endif
  IF (_wasconst) pushconst();
  IF (_t==_T_BYTE) {
    getrold();
    getrnew();
    emitsubbflags(_rold, _rnew); //new-old
    freernew();
    freernew();
    getrfree(); //_rnew
    emitinvcytob();
  }ELSE IF (_t==_T_UINT) {
    getrold();
    getrnew();
    emitsubflags(_rold, _rnew); //new-old
    freernew();
    freernew();
    getrfree(); //_rnew
    emitinvcytob();
  }ELSE IF (_t==_T_INT) {
    getrold();
    getrnew();
    emitsubflags(_rold, _rnew); //new-old
    freernew();
    freernew();
    getrfree(); //_rnew
    emitinvSxorVtob(); //S xor overflow + �������� ����� ����������
  //}ELSE IF (_t==_T_LONG) {
  }ELSE errtype("<=",_t);
  _t = _T_BOOL;
}

EXPORT PROC cmdmoreeq() //������ >= ������
{
#ifdef USE_COMMENTS
;;  cmtstr( ";OPERATION >=" ); endcmt();
#endif
  IF (_wasconst) pushconst();
  IF (_t==_T_BYTE) {
    getrnew();
    getrold();
    emitsubbflags(_rnew, _rold); //old-new
    freernew();
    freernew();
    getrfree(); //_rnew
    emitinvcytob();
  }ELSE IF (_t==_T_UINT) {
    getrnew();
    getrold();
    emitsubflags(_rnew, _rold); //old-new
    freernew();
    freernew();
    getrfree(); //_rnew
    emitinvcytob();
  }ELSE IF (_t==_T_INT) {
    getrnew();
    getrold();
    emitsubflags(_rnew, _rold); //old-new
    freernew();
    freernew();
    getrfree(); //_rnew
    emitinvSxorVtob(); //S xor overflow + �������� ����� ����������
  //}ELSE IF (_t==_T_LONG) {
  }ELSE errtype(">=",_t);
  _t = _T_BOOL;
}

EXPORT PROC cmdeq()
{
#ifdef USE_COMMENTS
;;  cmtstr( ";OPERATION ==" ); endcmt();
#endif
  _sz = _typesz[_t];
  IF (_sz==_SZ_BYTE) {
    IF (_wasconst) {
      getrnew();
      emitsubbzconst(); //new-<const>
      _wasconst = +FALSE;
      freernew();
    }ELSE {
      getrnew();
      getrold();
      emitsubbz(); //old-new
      freernew();
      freernew();
    };
    getrfree(); //_rnew
    emitztob();
  }ELSE {
    IF (_wasconst) pushconst();
    IF (_sz==_SZ_REG) {
      getrnew();
      getrold();
      emitsubz(); //old-new
      freernew();
      freernew();
      getrfree(); //_rnew
      emitztob();
    }ELSE /**IF (_sz==_SZ_LONG)*/ {
      getrold2(); _rold2 = _rold; //oldlow //���� ������, ����� �������� ������� �������
      getrold3(); _rold3 = _rold; //oldhigh �� ����� //���� ������, ����� �������� ������� �������
      getrnew(); //newlow
      getrold(); //newhigh
      emitsublongz(); //old2-new, old3-old
      freernew();
      freernew();
      freernew();
      freernew();
      getrfree(); //_rnew
      emitztob();
    };//ELSE errtype("==",_t); //�� ���� �������
  };
  _t = _T_BOOL;
}

EXPORT PROC cmdnoteq()
{
#ifdef USE_COMMENTS
;;  cmtstr( ";OPERATION !=" ); endcmt();
#endif
  _sz = _typesz[_t];
  IF (_sz==_SZ_BYTE) {
    IF (_wasconst) {
      getrnew();
      emitsubbzconst(); //new-<const>
      _wasconst = +FALSE;
      freernew();
    }ELSE {
      getrnew();
      getrold();
      emitsubbz(); //old-new
      freernew();
      freernew();
    };
    getrfree(); //_rnew
    emitinvztob();
  }ELSE {
    IF (_wasconst) pushconst();
    IF (_sz==_SZ_REG) {
      getrnew();
      getrold();
      emitsubz(); //old-new
      freernew();
      freernew();
      getrfree(); //_rnew
      emitinvztob();
    }ELSE /**IF (_sz==_SZ_LONG)*/ {
      getrold2(); _rold2 = _rold; //oldlow //���� ������, ����� �������� ������� �������
      getrold3(); _rold3 = _rold; //oldhigh �� ����� //���� ������, ����� �������� ������� �������
      getrnew(); //newlow
      getrold(); //newhigh
      emitsublongz(); //old2-new, old3-old
      freernew();
      freernew();
      freernew();
      freernew();
      getrfree(); //_rnew
      emitinvztob();
    };//ELSE errtype("!=",_t); //�� ���� �������
  };
  _t = _T_BOOL;
}

///////////////////////////////////
//��������� ������� � ���������

EXPORT PROC cmdlabel()
{
  //IF (_wasconst) pushconst(); //���������� ���������
  //�� ���� ������ ��� ��������������� ��� �������� ������ ���� ��������
  //(������ ��� ��������� �������������, �� ��� storergs � � ��������������� ��������� ���� ����������� �������)
  getnothing();
  emitfunclabel(_joined);
}

EXPORT PROC cmdjpval()
{
  //IF (_wasconst) pushconst();
  getmainrg();
  freernew();
  emitjpmainrg(); //"jp (hl)" //������ getnothing
}

EXPORT PROC cmdcallval()
{
  IF (_wasconst) pushconst();
  getmainrg();
  freernew();
  emitcallmainrg(); //"call (hl)" //������ getnothing //todo ������� � ����������
}

EXPORT PROC cmdjp()
{
#ifdef USE_COMMENTS
;;  cmtstr( ";JUMP " ); cmtstr(_joined); endcmt();
#endif
  //IF (_wasconst) pushconst();
  emitjp();
}

EXPORT PROC cmdjpiffalse()
{
#ifdef USE_COMMENTS
;;  cmtstr( ";JUMP IF FALSE " ); cmtstr(_joined); endcmt();
#endif
  IF (_wasconst) pushconst(); //����� ���� �� ��������� � IF (+TRUE)
  getrnew();
  emitbtoz();
  freernew();
  emitjpiffalse();
}

EXPORT PROC cmdcall()
{
#ifdef USE_COMMENTS
;;  cmtstr( ";CALL " ); cmtstr(_callee); endcmt();
#endif
  //IF (_wasconst) pushconst();
  unproxy();
  getnothing(); //��������� �������� � ����� � ����������� ������� � ���������� ��������
  emitcallproc();
  IF (_t!=_T_PROC) {
    _sz = _typesz[_t];
    IF (_sz==_SZ_BYTE/**(_t==_T_BYTE)||(_t==_T_CHAR)||(_t==_T_BOOL)*/) {
      setmainb(); //��������� � RMAIN
    }ELSE IF (_sz==_SZ_REG/**(_t==_T_INT)||(_t==_T_UINT)||(_t>=_T_POI)*/) {
      setmainrg(); //��������� � RMAIN
    }ELSE IF (_sz==_SZ_LONG/**_t==_T_LONG*/) {
      setmain2rgs(); //��������� � RMAIN,RMAIN2
    }ELSE errtype("call",_t); //�� ���� �������
  };
}

EXPORT PROC cmdfunc()
{
#ifdef USE_COMMENTS
;;  cmtstr( ";FUNC" ); endcmt();
#endif
  //IF (_wasconst) pushconst();
  _funcstkdepth = +(INT)0;
  emitfunchead();
}

EXPORT PROC cmdstorergs() //����� ���������� ��������� ���������� ����������� ���������
{
#ifdef USE_COMMENTS
;;  cmtstr(";STOREREGS"); endcmt();
#endif
  //IF (_wasconst) pushconst();
  getnothing(); //����� ������������ hl //todo ������ (����� ����� �������������� ���������) - ����� ��������� ��������� ����� �� � �����, � � ���������
}

EXPORT PROC cmdpushpar() //��� ����������� �������� (���������� ���������� ������ ��� �������)
{
#ifdef USE_COMMENTS
;;  cmtstr(";PUSHPAR "); cmtstr(_joined); endcmt();
#endif
  //IF (_wasconst) pushconst();
  _sz = _typesz[_t];
  IF (_sz==_SZ_BYTE) {
    getnothing(); //����� ������������ A //todo ������ (����� ����� �������������� ���������) - ����� ��������� ��������� ����� �� � �����, � � ���������
    getrfree(); //_rnew
    emitgetb(); //cmdpushvarb(s); //���������, ���� �������� �� ��������������
  }ELSE IF (_sz==_SZ_REG) {
    getnothing(); //����� ������������ hl //todo ������ (����� ����� �������������� ���������) - ����� ��������� ��������� ����� �� � �����, � � ���������
    getrfree(); //_rnew
    emitgetrg(+FALSE);
  }ELSE /**IF (_sz==_SZ_LONG)*/ {
    getnothing(); //����� ������������ hl //todo ������ (����� ����� �������������� ���������) - ����� ��������� ��������� ����� �� � �����, � � ���������
    getrfree(); //_rnew
    emitgetrg(+TRUE); //high
    getnothing(); //����� ������������ hl //todo ������ (����� ����� �������������� ���������) - ����� ��������� ��������� ����� �� � �����, � � ���������
    getrfree(); //_rnew
    emitgetrg(+FALSE); //low
  };//ELSE errtype("pushpar",_t); //�� ���� �������
}

EXPORT PROC cmdpoppar() //��� ����������� �������� (�������������� ���������� ������ ��� �������)
{ //����� ���� ������ 1-2 �������� (���������)
#ifdef USE_COMMENTS
;;  cmtstr(";POPPAR"); endcmt();
#endif
  //IF (_wasconst) pushconst();
  _sz = _typesz[_t];
  IF (_sz==_SZ_BYTE) {
    getrfree(); //_rnew
    emitpoprg(_rnew);
    emitputb();
    freernew();
  }ELSE IF (_sz==_SZ_REG) {
    getrfree(); //_rnew
    emitpoprg(_rnew);
    emitputrg(+FALSE); //low
    freernew();
  }ELSE /**IF (_sz==_SZ_LONG)*/ {
    getrfree(); //_rnew
    emitpoprg(_rnew);
    emitputrg(+FALSE); //low
    freernew();
    getrfree(); //_rnew
    emitpoprg(_rnew);
    emitputrg(+TRUE); //high
    freernew();
  };//ELSE errtype("poppar",_t); //�� ���� �������
} //������ � ��������� ����� ���� �����, �� � ����� (����� ret ���� ������� ��������� �� �����)

EXPORT PROC cmdresult()
{
#ifdef USE_COMMENTS
;;  cmtstr( ";RESULT" ); endcmt();
#endif
  IF (_wasconst) pushconst();
  _sz = _typesz[_t];
  IF (_sz==_SZ_BYTE) {
  }ELSE IF (_sz==_SZ_REG) {
    getmainrg(); //todo ������������� � ������������� ���������
  }ELSE /**IF (_sz==_SZ_LONG)*/ {
    getmain2rgs(); //todo ������������� � ������������� ���������
  };//ELSE errtype("result",_t); //�� ���� �������
}

EXPORT PROC cmdret(BOOL isfunc)
{
#ifdef USE_COMMENTS
;;  cmtstr( ";ENDFUNC" ); endcmt();
#endif
  //IF (_wasconst) pushconst();
  IF ( isfunc ) {
    _sz = _typesz[_t];
    IF (_sz==_SZ_BYTE) {
      getrnew();
      proxy(_rnew);
    }ELSE IF (_sz==_SZ_REG) {
      getmainrg();
    }ELSE /**IF (_sz==_SZ_LONG)*/ {
      getmain2rgs();
    };//ELSE errtype("endfunc",_t); //�� ���� �������
  };
  emitret();
  IF (+(UINT)_funcstkdepth!=0) { errstr("funcstkdepth=" ); erruint(+(UINT)_funcstkdepth); enderr(); };
  initrgs();
}

EXPORT PROC initcmd()
{
  _const  = (PCHAR)_sc;
  //_wastword  = (PCHAR)_sw;
  _wasconst = +FALSE;
}

/// imported
#include "../_sdk/typecode.h"
#include "../_sdk/str.h"
#include "../_sdk/io.h"
#include "../_sdk/emit.h"
//EXTERN UINT _curlnbeg; //����� ������ �� ������ ������ ������ //��� read, emit
//EXTERN BOOL _cmts; //��� read, emit

CONST BYTE _typesz[32];

#ifdef TARGET_THUMB
#include "sizesarm.h"
#else
#include "sizesz80.h"
#endif

CONST BOOL _isalphanum[256];

EXTERN BOOL _doskip; //���������� ������, ����� ������������ � #

EXTERN PCHAR _tword; //������� �����
EXTERN UINT  _lentword;
VAR PCHAR _prefix; //������� �������� ����� (������� - � _tword)
VAR UINT  _lenprefix;
VAR PCHAR _title; //�������� ������� ��������� (� ������ ������)
VAR UINT  _lentitle;
EXTERN PCHAR _callee; //�������� ���������� ��������� (� ������ ������)
EXTERN UINT  _lencallee;
EXTERN PCHAR _name; //����� ��� �������� (��� ������� �����)
EXTERN UINT  _lenname;
EXTERN PCHAR _joined; //���������
EXTERN UINT  _lenjoined;
VAR PCHAR _ncells; //� addlbl ������ ���������� ncells � callee
VAR UINT  _lenncells;
VAR CHAR  _s1[_STRLEN]; //������� �������� ����� (������� - � _tword)
VAR CHAR  _s2[_STRLEN]; //�������� ������� ��������� (� ������ ������)
VAR CHAR  _s3[_STRLEN]; //�������� ���������� ��������� (� ������ ������)
VAR CHAR  _s4[_STRLEN]; //����� ��� �������� (��� ������� �����)
VAR CHAR  _s5[_STRLEN]; //���������
VAR CHAR  _s6[_STRLEN]; //����� ���������

EXTERN CHAR _cnext;
EXTERN UINT _spcsize; //����� �������� ����� ����������� �������

EXTERN UINT _curline; //������� ����� ������

EXTERN UINT _waseols; //������� ���� EOL � �������� ����

PROC rdch FORWARD();
PROC rdchcmt FORWARD();
PROC rdquotes FORWARD(CHAR eol);
PROC rdaddword FORWARD();
PROC rdword FORWARD();
PROC initrd FORWARD();

PROC strpush FORWARD(PCHAR s, UINT len); //joined ��� callee
FUNC UINT strpop FORWARD(PCHAR s);
EXTERN UINT _lenstrstk;

PROC initlblbuf FORWARD();

//math (��� ��������� ����)
PROC cmdneg FORWARD();
PROC cmdinv FORWARD();
PROC cmdpoke FORWARD();
PROC cmdpeek FORWARD();
PROC cmdpushvar FORWARD();
PROC cmdpopvar FORWARD();
PROC cmdpushnum FORWARD();
PROC cmdmul FORWARD();
PROC cmddiv FORWARD();
//PROC cmdshl1 FORWARD();
//PROC cmdshr1 FORWARD();
PROC cmdshl FORWARD();
PROC cmdshr FORWARD();
PROC cmdadd FORWARD();
PROC cmdsub FORWARD();
PROC cmdaddpoi FORWARD(); //�������� �������������� ���� � ����������
PROC cmdand FORWARD();
PROC cmdor FORWARD();
PROC cmdxor FORWARD();
PROC cmdinc FORWARD();
PROC cmddec FORWARD();
PROC cmdincbyaddr FORWARD();
PROC cmddecbyaddr FORWARD();

//��������� (��� ��������� ����)
PROC cmdless FORWARD();
PROC cmdmore FORWARD();
PROC cmdlesseq FORWARD();
PROC cmdmoreeq FORWARD();
PROC cmdeq FORWARD();
PROC cmdnoteq FORWARD();

//��������� ������� � ��������� (��� ��������� ����)
PROC cmdjpval FORWARD();
PROC cmdcallval FORWARD();
PROC cmdjp FORWARD();
PROC cmdjpiffalse FORWARD();
PROC cmdcall FORWARD();
PROC cmdfunc FORWARD();
PROC cmdpushpar FORWARD(); //��� ����������� �������� (���������� ���������� ������ ��� �������)
PROC cmdstorergs FORWARD(); //����� ���������� ��������� ���������� ����������� ���������
PROC cmdpoppar FORWARD(); //��� ����������� �������� (�������������� ���������� ������ ��� �������)
PROC cmdresult FORWARD();
PROC cmdret FORWARD(BOOL isfunc); //�������������� ���������� ����� ������ ������� �� ����� � �����

PROC cmdcastto FORWARD(TYPE t2);
PROC cmdlabel FORWARD();
PROC var_alignwsz_label FORWARD(TYPE t);
PROC var_def FORWARD(TYPE t, PCHAR s);

PROC initcmd FORWARD();

PROC initcode FORWARD();
PROC endcode FORWARD();
PROC emitasmlabel FORWARD(PCHAR s);
PROC emitfunclabel FORWARD(PCHAR s);
PROC emitvarlabel FORWARD(PCHAR s);
PROC emitexport FORWARD(PCHAR s);
PROC emitvarpreequ FORWARD(PCHAR s);
PROC emitvarpostequ FORWARD();
PROC varequ FORWARD(PCHAR s);
PROC asm_db FORWARD(); //������� ��� ����������� �������� ����� TODO
PROC var_db FORWARD();
PROC var_dw FORWARD();
//PROC var_dl FORWARD();
PROC var_ds FORWARD();

FUNC UINT gettypename FORWARD(PCHAR s); //����� �������� ���� ��������� � s (����� ����� lbltype)
PROC setvarsz FORWARD(UINT addr, UINT shift);
FUNC TYPE lbltype FORWARD(); //������� ��� ����� _name
PROC dellbl FORWARD(); //������� ����� _name
PROC addlbl FORWARD(TYPE t, BOOL isloc, UINT varsz/**, PCHAR size, UINT lensize*/); //(_name)
PROC keepvars FORWARD(); //����� ������� ��������� �����
PROC undovars FORWARD(); //����� ��������� ����� (������ ��)
EXTERN UINT _varszaddr;
EXTERN UINT _varsz;

EXTERN BYTE _exprlvl; //������� ��������� (������� ������� == 1)
EXTERN TYPE _t; //������� ���
EXTERN BOOL _isloc; //��������� �� ����������� ����������

////
CONST UINT _MAXPARS = 16; /**������������ ����� ���������� � ������ �������*/
//todo ���������� ������� �������� f(g(h(...()...)

//��������� ����������� (���� ��� ��������� commands � codetg):
VAR UINT _curlbl; //����� ���������
VAR UINT _tmpendlbl; //����� ��������� ��� ������ �� ����� while/repeat

VAR BYTE _namespclvl; //������� ����������� ������������ ��� (����� ����� � ��������)

VAR BOOL _isrecursive; //������� ����������� ��������� �����������
VAR BOOL _wasreturn; //���� ������� return (����� �� ������ ������� � ����������� �������) //����������� � func
VAR TYPE _curfunct; //��� ������� (��� return) //����������� � func
VAR BOOL _isexp; //������� ����������� ����������, ����������� ������/���������, ���������/������� �������������� (�� �� ���������, �.�. ��� �� ����� � �� �����)

VAR CHAR _c0;
VAR CHAR _c2;

VAR UINT _parnum;

VAR UINT _doskipcond;
//_doskipcond � ������� ���� ������, ������� ������� �������� ifdef � ������� ������� ���������� (����� ��������)
//(������ ����������� ����� ���� ������ ����������)
//���� (_doskipcond&1) == 0 (�.�. �� � ���������� �����), �� ������� ifdef ������������ �� ����� ��������� (�.�. else �� ��������)
//������������ ������� ����� ifdef ����� � _doskip
//�� ������� ������ _doskipcond = 1, _doskip = +FALSE

VAR BOOL _morecmd; //���� "���� �� �������" � eatcmd

VAR CHAR _opsym;

VAR BOOL _addrexpr; //��������� � & ��� &(<structname>-><field>) (TODO ��������???)

#define _MAXHINCLUDES 0x08
VAR PBYTE _hinclfile[_MAXHINCLUDES];
VAR UINT _hnline[_MAXHINCLUDES];
VAR BYTE _nhinclfiles; //����� �������� ������
//

#ifdef USE_HINTS
;;PROC hint_tword() {
;;  hintstr("//_tword=\""); hintstr(_tword); hintstr("\", cnext=\""); hint(_cnext); hint('\"'); endhint();
;;}
#endif

PROC err_tword(PCHAR s)
{
  errstr(s); errstr(" expected, but we have \'"); errstr(_tword); err('\''); enderr();
}

PROC doexp() //������ _joined
{
  IF (_isexp) emitexport(_joined);
}

PROC eat(CHAR c)
{
  IF (*(PCHAR)_tword!=c) {
    err(c); errstr(" expected, but we have \'"); errstr(_tword); err('\''); enderr();
  };
  rdword();
}

PROC jdot()
{
  _lenjoined = stradd(_joined, _lenjoined,'.');
  _joined[_lenjoined] = '\0'; //strclose(_joined, _lenjoined);
}

VAR UINT _genn;

PROC gendig(UINT d)
{
VAR BYTE dig;
  dig = +(BYTE)'A';
  WHILE (_genn >= d) {
    _genn = _genn - d;
    INC dig;
    _wasdig = +TRUE;
  };
  IF (_wasdig) {
    _lenjoined = stradd(_joined, _lenjoined, +(CHAR)dig);
  };
}

PROC jautonum(UINT n)
{
  _genn = n;
  _wasdig = +TRUE;
  IF (n != 0) {
    _wasdig = +FALSE;
    gendig(676);
    gendig(26);
  };
  gendig(1);
  jdot();
}

PROC genjplbl(UINT n)
{
  _lenjoined = strcopy(_title, _lentitle, _joined);
  jautonum(n);
}

PROC jtitletword()
{
  _lenjoined = strcopy(_title, _lentitle, _joined); //_lenjoined = strjoin(/**to=*/_joined, 0/**strclear(_joined)*/, _title);
  _lenjoined = strjoin(/**to=*/_joined, _lenjoined, _tword);
  _joined[_lenjoined] = '\0'; //strclose(_joined, _lenjoined);
}

PROC matchstar()
{
  IF (_cnext == '*') {
    _t = _t|_T_POI;
    _varsz = (UINT)_SZ_REG;
    rdword(); //use *
  };
}

PROC eattype()
{
loop:
  _lenname = strcopy(_tword, _lentword, _name);
  _t = lbltype()&~_T_TYPE;
  IF (_t == _T_STRUCTWORD) {
    rdword(); //use STRUCT
    goto loop;
  };
  matchstar();
  rdword();
}

PROC doprefix(BYTE nb) //������� n ���� ���� 'word.' �� title � prefix (name ��� ��������)
{
  _lenprefix = 0; //strclear(_prefix)
  WHILE (nb > 0x00) { //���������� �����
    _lenprefix = strjoineol(/**to=*/_prefix, _lenprefix, &_title[_lenprefix], '.');
    _lenprefix = stradd(_prefix, _lenprefix, '.');
    DEC nb;
  };
  _prefix[_lenprefix] = '\0'; //strclose(_prefix, _lenprefix);
  _lenjoined = strcopy(_prefix, _lenprefix, _joined);
  _lenjoined = strjoin(/**to=*/_joined, _lenjoined, _tword/**, _lentword*/);
  _joined[_lenjoined] = '\0'; //strclose(_joined, _lenjoined);
}

PROC adddots()
{
  WHILE (_cnext == '.') {
    rdaddword(); //��������� �����
    rdaddword(); //��������� ��������� �����
  };
}

PROC joinvarname(BOOL iscall) //���������� _t = ���(_name)
{ //������������� ��� ��������
VAR BYTE lvl;
  _lenname = strcopy(_tword, _lentword, _name);
  _t = lbltype();
  IF ((_t&_T_TYPE)!=(TYPE)0x00) { //todo ��������� ������
    matchstar();
  };
  IF (!_isloc) {
    _lenjoined = strcopy(_tword, _lentword, _joined);
  }ELSE {
    lvl = _namespclvl;
    IF (iscall && (lvl != 0x00)) {
      DEC lvl; //proc()
    };
    doprefix(lvl); //������� n ���� ���� 'word.' �� title � prefix (name ��� ��������)
    //todo ��������� � ���� ��� (��� �����������)?
    //��� todo ���������� stru1->f1.f2 � tstru1.f1+tstru2.f2
  };
}

PROC eatvarname() //��� �������� �����
{
  eattype(); //t = _t; //��� ��� ��� ��������
  adddots();
  _lenname = strcopy(_tword, _lentword, _name);
  doprefix(_namespclvl); //������� n ���� ���� 'word.' �� title � prefix (name ��� ��������)
  rdword(); //'['
  IF (*(PCHAR)_tword == '[') {
    _t = _t|_T_ARRAY;
  };
}

PROC getstructfield() //���������� _t = ��� ����
{ //��������� ��� ��������� � ����������, ��� _t = ����� ���������
  _lenjoined = gettypename(_joined); //����� �������� ���� ��������� � joined
  eat('>'); //rdword(); //use '>'
  jdot();
  _lenjoined = strjoin(/**to=*/_joined, _lenjoined, _tword); //structname.structfield
  _joined[_lenjoined] = '\0'; //strclose(_joined, _lenjoined);
  //_t = _T_POI|_T_BYTE; //��� ��� - ����� ���������
  cmdpushnum(); //structname.structfield
  cmdadd();
  _lenname = strcopy(_joined, _lenjoined, _name); //structname.structfield
  _t = lbltype(); //(_name)
}

//////////////////////////////////////////
// compiler

//call �������� _tword � expr
PROC eatexpr RECURSIVE FORWARD();  //�������� call
FUNC BOOL eatcmd RECURSIVE FORWARD();  //�������� call
FUNC TYPE do_call RECURSIVE FORWARD(BOOL isfunc); //���������� ��� �������
PROC compfile RECURSIVE FORWARD(PCHAR fn);

PROC varstrz()
{
  emitvarlabel(_joined); //varstr(_joined); /**varc( ':' );*/ endvar();
  WHILE (+TRUE) {
    rdquotes('\"');
    rdch(); //��������� ����������� �������
    _tword[_lentword] = '\0'; //strclose(_tword, _lentword);
    var_db(); varstr(_tword); endvar();
    IF (_cnext != '\"') BREAK;
    rdword(); //����������� ������� ����������� ������
  };
  var_db(); varc('0'); endvar();
}

PROC asmstrz() //������� ��� ����������� �������� �����
{
  emitasmlabel(_joined); //asmstr(_joined); /**varc( ':' );*/ endasm();
  WHILE (+TRUE) {
    rdquotes('\"');
    rdch(); //��������� ����������� �������
    _tword[_lentword] = '\0'; //strclose(_tword, _lentword);
    asm_db(); asmstr(_tword); endasm();
    IF (_cnext != '\"') BREAK;
    rdword(); //����������� ������� ����������� ������
  };
  asm_db(); asmc('0'); endasm();
}

PROC eatidx() //��� idxarray � switch
{
  INC _exprlvl; //no jump optimization
  eatexpr(); //��������� ������ ��� ������!!!
  DEC _exprlvl;
  IF (_t==_T_BYTE) cmdcastto(_T_UINT);
  IF (_t!=_T_UINT) {errstr("idx bad type "); erruint(+(UINT)_t); enderr(); };
}

FUNC TYPE idxarray RECURSIVE(TYPE t)
{
  rdword(); //������ ����� expr
  IF ((t&_T_ARRAY) != (TYPE)0x00) { //������
    t = t&_TYPEMASK; //&(~(_T_ARRAY|_T_CONST)); //����� ���� _T_POI (���� ������ �����)
    _t = _T_POI|_T_BYTE; cmdpushnum(); //������������� �������� ������� - ���� ��� �����
  }ELSE IF ((t&_T_POI) != (TYPE)0x00) { //���������
    _t = t; cmdpushvar(); //������������� ��������� � �������� ������� - ������ ��� ��������
    t = t&(~_T_POI);
  }ELSE {errstr("[] not in array "); erruint(+(UINT)t); enderr(); };
  eatidx();
  _t = t; //��� �������� �������
  cmdaddpoi();
RETURN t; //��� �������� �������
}

PROC numtype()
{
  /**IF (_cnext == '.') { //������� ����� (������ �������� � ����� ��� ����������� ������)
    rdaddword(); //��������� �����
    rdaddword(); //��������� ������� �����
    IF ( (_tword[_lentword-1]=='e') && (_cnext=='-') ) {
      rdaddword(); //��������� '-' ������������� ����������
      rdaddword(); //��������� ������������� ����������
    };
    _t = _T_FLOAT;
  }ELSE*/ IF (*(PCHAR)_tword == '-') { //� val ��� ����, ���� ��� define
    _t = _T_INT;
  }ELSE IF (_tword[_lentword-1]=='L') {
    _t = _T_LONG;
  }ELSE IF (+(BYTE)_tword[1] > +(BYTE)'9') { //��������� �������� ��������� �������
    IF ((_lentword<=4)&&(_tword[1]=='x')) {
      _t = _T_BYTE;
    }ELSE IF ((_lentword<=10)&&(_tword[1]=='b')) {
      _t = _T_BYTE;
    }ELSE {
      _t = _T_UINT;
    };
  }ELSE {
    _t = _T_UINT;
  };
}

PROC val RECURSIVE()
{
VAR TYPE t; //��� cast,peek
{
//<val>::=
//(<expr>) //��������� (�����������)
//|<num> //���������� ����� (��������� �������) INT/UINT/LONG
//|<num>.<num>[e-<num>] //float ����� (��������� �������)
//|<var> //����������
//|'CHAR' //���������� ��������� (��������� �������)
//|"str" //��������� ��������� (��������� �������)
//|+(<type>)<val> //��������� val, ����� ������� ������� � <type>
//|+<boolconst>
//|+_<enumconst> BYTE
//|<lbl>([<val>,...]) //call
//|-<val> //��������� val, ����� ������� NEG
//|~<val> //��������� val, ����� ������� INV
//|!<val> //��������� val, ����� ������� INV(BOOL)
////|<<val> //��������� val, ����� ������� SHL1
////|><val> //��������� val, ����� ������� SHR1
//|*(<ptype>)<val> //��������� ������ �� ������ (��������� val, ����� ������� PEEK)
//|&<var> //����� ����������
 //������� ��� ���������
#ifdef USE_HINTS
;;  hintstr("//val: word=\""); hintstr(_tword); hintstr("\", cnext=\""); hint(_cnext); hint('\"'); endhint();
#endif
  //_t = _T_UNKNOWN; //debug (����� ����� �������� �� ������� typesz ��� ������ �����)
  //IF ( !_waseof )
  { //������ �� ������������ � ����� ���������� ����� (������ � ����������� �������)
    _opsym = *(PCHAR)_tword;
    IF (_opsym == '\'') {
      rdquotes('\'');
      rdch(); //��������� ����������� �������
      _tword[_lentword] = '\0'; //strclose(_tword, _lentword);
      _lenjoined = strcopy(_tword, _lentword, _joined);
      _t = _T_CHAR; cmdpushnum();
    }ELSE IF (_opsym == '\"') {
      _lenjoined = strcopy(_title, _lentitle, _joined);
      jautonum(_curlbl);
      INC _curlbl;
      //_joined[_lenjoined] = '\0'; //strclose(_joined, _lenjoined);
      _t = _T_POI|_T_CHAR; cmdpushnum();
      varstrz(); //� ������ joined
    }ELSE IF (_opsym == '+') {
      rdword(); //'(' of type or +TRUE/+FALSE (BOOL) or +_CONSTANT or +__CONSTANT (BYTE)
      _opsym = *(PCHAR)_tword;//|+(CHAR)0x20;
      IF (_opsym=='_') { //+_CONSTANT or +__CONSTANT (BYTE)
        //��������� ����� ����� ��������� ��� ���������
        adddots();
        joinvarname(/**iscall*/+FALSE);
        _lenjoined = strcopy(_name, _lenname, _joined); //����������
        _t = _T_BYTE; cmdpushnum();
      }ELSE IF (+(BYTE)(+(BYTE)_opsym - +(BYTE)'0') < 0x0a) { //+num
        IF (!_waseof) val(); //����������� ����� val
        IF (_t == _T_UINT) cmdcastto(_T_INT); //��� �������� �������� ���� +15
      }ELSE IF (_opsym!='(') { //+TRUE/+FALSE (BOOL) //+sizeof
        IF (_opsym=='s') { //+sizeof //TODO uppercase?
          rdword(); //������������ sizeof
          eat('('/**, "\'(\'"*/);
          eattype();
          //eatexpr(); //�� ������ �� expr ��� ��������� ')', �� ��������� ������ ��� ������� �� ���������
          //eat(')', "\')\'");
//          IF ((_t&_T_TYPE)!=0x00) {
            emitn(_varsz); //gettypesz();
            _lenjoined = strcopy(_nbuf, _lennbuf, _joined); //����������
//          }ELSE { //�� ���
            //- �� ���������� �������� ���� - ����� ������, �.�. ��� �������� ���������� TODO (������� ��������� _issizeof)
            //- �� ���������� ������� - ����� ������, �.�. ��� ������ ������� TODO
            //- �� ���������� ��������� - ����� ������, �.�. ��� ������ ��������� TODO
            //- �� ��������� - ����� ������, �.�. ������������� �������� TODO
//          };
          _t = _T_UINT; cmdpushnum(); //(_joined)
        }ELSE { //+TRUE/+FALSE (BOOL)
          _lenjoined = strcopy(_tword, _lentword, _joined); //_lenjoined = strjoin(/**to=*/_joined, 0/**strclear(_joined)*/, _tword/**, _lentword*/);
          //strclose(_joined, _lenjoined);
          _t = _T_BOOL; cmdpushnum(); //(_joined)
        };
      }ELSE { //'(': ��� typecast
/**        rdword(); //type
        t = eattype();
        eat(')', "\')\'");
        IF (!_waseof) val(); //����������� ����� val
        cmdcastto(t);*/
        //rdword(); //������ ����� expr
        //eatexpr(); //�� ������ �� expr ��� ��������� ')', �� ��������� ������ ��� ������� �� ���������
        IF (!_waseof) val(); //����������� ����� val
        //IF (_t == _T_UINT) cmdcastto(_T_INT); //��� �������� �������� ���� +15 (������ ������ +(type)val)
      };
    }ELSE IF (+(BYTE)(+(BYTE)_opsym - +(BYTE)'0') < 0x0a) { //num
      numtype(); //_t
      _lenjoined = strcopy(_tword, _lentword, _joined);
      cmdpushnum();
    }ELSE IF (_isalphanum[+(BYTE)_opsym] /**|| (_opsym=='.')*/) { //<var> //���� isalpha
      adddots();
     //���� ����� �������, �� do_variable ���� ������ � namespclvl-1!!!
      IF (_cnext == '(') { //call
        _t = do_call(+TRUE); //isfunc
      }ELSE {
        joinvarname(+FALSE); //iscall
        IF (_cnext == '[') { //<varname>[<idx>]
          rdword(); //'['
          _t = idxarray(_t);
          cmdpeek();
        }ELSE { //<varname>
          IF ((_t&_T_TYPE)==(TYPE)0x00) {
            /**IF (_t==_T_STRUCT) {
              _t = _T_POI|_T_BYTE;
              cmdpushnum(); //���� ����� ��������� (��� .)
            }ELSE*/ IF ((_t&_T_ARRAY)!=(TYPE)0x00) { //array without [] as a pointer
              _t = _t&(~(_T_ARRAY|_T_CONST))|_T_POI;
              cmdpushnum(); //������������� �������� ������� - ���� ��� �����
            }ELSE IF ((_t&_T_CONST)==(TYPE)0x00) {
              cmdpushvar(); //��������� (� ��� ����� �� ���������) ��� ������� ����������
            }ELSE { //��������� (equ)
              _t = _t&_TYPEMASK; //&(~_T_CONST);
              cmdpushnum();
            };
          };
        };
      };
    }ELSE IF (_opsym == '(') {
      rdword(); //������ ����� expr
      eatexpr(); //�� ������ �� expr ��� ��������� ')', �� ��������� ������ ��� ������� �� ���������
      IF ((_t&_T_TYPE)!=(TYPE)0x00) { //(type)val //������ � sizeof(expr)
        t = _t&~_T_TYPE;
        _t = t;
        rdword();
        val();
        cmdcastto(t);
      };
    }ELSE IF (_opsym == '-') {
      IF (/**isnum(_cnext)*/+(BYTE)(+(BYTE)_cnext - +(BYTE)'0') < 0x0a) { //-<const>
        rdaddword();
        //todo float
        _t = _T_INT;
        _lenjoined = strcopy(_tword, _lentword, _joined);
        cmdpushnum();
      }ELSE { //-<var>
        rdword(); //������ ����� val
        IF (!_waseof) val(); //����������� ����� val
        cmdneg();
      };
    }ELSE IF (_opsym == '!') {
      rdword(); //������ ����� val
      IF (!_waseof) val(); //����������� ����� val
      cmdinv(); //TODO invBOOL
    }ELSE IF (_opsym == '~') {
      rdword(); //������ ����� val
      IF (!_waseof) val(); //����������� ����� val
      cmdinv();
    }ELSE IF (_opsym == '*') {
      rdword(); //'(' of type
      eat('('/**, "\'(\'"*/); //IF (*(PCHAR)_tword != '(') err_tword("\'(\'");
      eattype(); t = _t/**|_T_POI*/; //tpoi //todo PPOINTER
      eat(')'/**, "\')\'"*/); //IF (*(PCHAR)_tword != ')') err_tword("\')\'");
      IF (!_waseof) val(); //����� ��� ��������� �� ����� //����������� ����� val
      _t = t&~_T_POI; //&~_T_CONST;
      cmdpeek();
    }ELSE IF (_opsym == '&') {
      IF (_cnext == '(') { //&(<structname>-><field>)
        _addrexpr = +TRUE;
        rdword(); //'('
        val();
        _t = _t|_T_POI;
      }ELSE {
        rdword();
        adddots();
        joinvarname(/**iscall*/+FALSE);
        IF (_cnext == '[') { //&<varname>[<idx>]
          rdword(); //'['
          _t = idxarray(_t)/**��� �������� �������*/|_T_POI;
        }ELSE { //&<varname>
          //IF ((_t&_T_ARRAY)!=0x00) { //&<arrayname> - error
          //}ELSE IF ((_t&_T_CONST)!=0x00) { //&<constname> - error
          //}ELSE { //&<varname>
            _t = _t&_TYPEMASK|_T_POI;
            cmdpushnum();
          //};
        };
      };
    }ELSE {
      errstr("WRONG PREFIX "); err(_opsym); enderr();
      _t = _T_UNKNOWN; //debug (����� ����� �������� �� ������� typesz ��� ������ �����)
    };
  };
#ifdef USE_HINTS
;;  hinttype("end val",_t);
#endif
}
}

PROC eatmulval RECURSIVE()
{
VAR CHAR opsym;
VAR TYPE t1;
{
//<val>[<*|/><val>...] => push[push<*|/>...]
//������������� �� ������� ����� ����������� �������� (��������, ')' ��� ';')
 //������� ��� ���������
#ifdef USE_HINTS
;;  hintstr("//mulval"); endhint();
#endif
  val();
  rdword();
#ifdef USE_HINTS
;;  hintstr("//mulval after val"); hint_tword();
#endif
  REPEAT {
    opsym = *(PCHAR)_tword;
    //IF (opsym==')') BREAK; //fast exit
    IF (opsym!='*')
      IF (opsym!='/')
        IF (opsym!='&')
          BREAK;
    t1 = _t;
    rdword();
    IF (opsym=='&')
      IF (*(PCHAR)_tword==opsym)
        rdword(); //use '&' //C compatibility
    val();
    _t = _t&_TYPEMASK; //&(~_T_CONST);
    rdword();
#ifdef USE_HINTS
;;    hintstr("//mulval after val2"); hint_tword();
#endif
    IF (t1 != _t) {errstr("opsym "); err(opsym); errstr(" type "); erruint(+(UINT)t1); errstr("!="); erruint(+(UINT)_t); enderr(); };
    IF       (opsym=='&') {cmdand();
    }ELSE IF (opsym=='*') {cmdmul();
    }ELSE /**IF (opsym=='/')*/ {cmddiv();
    };
  }UNTIL (_waseof);
#ifdef USE_HINTS
;;  hinttype("end mulval",_t);
#endif
}
}

PROC eatsumval RECURSIVE()
{
VAR CHAR opsym;
VAR TYPE t1;
{
//<mulval>[<+|-><mulval>...] => push[push<+|->...]
 //������� ��� ���������
#ifdef USE_HINTS
;;  hintstr("//sumval"); endhint();
#endif
  eatmulval();
  REPEAT {
    opsym = *(PCHAR)_tword;
    //IF (opsym==')') BREAK; //fast exit
    IF (opsym!='+')
      IF (opsym!='-')
        IF (opsym!='|')
          IF (opsym!='^')
            BREAK;
    t1 = _t;
    rdword();
    IF (*(PCHAR)_tword=='>') { //structinstancepointer->structfield
      //��������� ��� ��������� � ����������, ��� _t = ����� ���������
      getstructfield(); //_t = ��� ����
      IF (!_addrexpr) cmdpeek(); //peek
      rdword(); //������������ structfield
    }ELSE {
      IF (opsym=='|') //||(opsym=='^')
        IF (*(PCHAR)_tword==opsym)
          rdword(); //use '|' or '^' //C compatibility
      eatmulval();
      _t = _t&_TYPEMASK; //&(~_T_CONST);
      IF (t1 != _t) /**&& ((t&_T_POI)!=0x00)*/ {errstr("opsym "); err(opsym); errstr(" type "); erruint(+(UINT)t1); errstr("!="); erruint(+(UINT)_t); enderr(); };
      //todo addpointer
      IF       (opsym == '+') {cmdadd();
      }ELSE IF (opsym == '-') {cmdsub(); //�� ������� ������� �����!
      }ELSE IF (opsym == '|') {cmdor();
      }ELSE /**IF (opsym == '^')*/ {cmdxor();
      };
    };
  }UNTIL (_waseof);
#ifdef USE_HINTS
;;  hinttype("end sumval",_t);
#endif
}
}

PROC eatexpr RECURSIVE()
{
VAR CHAR opsym;
VAR TYPE t1;
VAR BOOL modified;
VAR BOOL dbl;
{
//<sumval>[<=><sumval>...]
 //������� ��� ��������� (����� ��� do_call_par)
#ifdef USE_HINTS
;;  hintstr("//expr"); endhint();
#endif
  INC _exprlvl;
  eatsumval();
  REPEAT {
    opsym = *(PCHAR)_tword;
    //IF (opsym==')') BREAK; //fast exit
    IF (opsym!='<')
      IF (opsym!='>')
        IF (opsym!='=')
          IF (opsym!='!')
            BREAK;
    t1 = _t;
    rdword();
    modified = (*(PCHAR)_tword=='=');
    dbl = (*(PCHAR)_tword==opsym);
    IF ( modified||dbl ) rdword(); //use '=' or '>' or '<'
    eatsumval();
    _t = _t&_TYPEMASK; //&(~_T_CONST);
    IF (t1 != _t) {errstr("opsym "); err(opsym); errstr(" type "); erruint(+(UINT)t1); errstr("!="); erruint(+(UINT)_t); enderr(); };
    IF (opsym == '=') {
      IF (!dbl) {errstr( "assign in expr" ); enderr(); };
      cmdeq(); //������ _t = _T_BOOL
    }ELSE IF (opsym == '!') {
      cmdnoteq(); //������ _t = _T_BOOL
    }ELSE IF (opsym == '<') {
      IF (dbl) {
        cmdshl(); //������ �������� ������� ���, ������� ������ �����!
      }ELSE IF (modified) {
        cmdlesseq(); //������ _t = _T_BOOL
      }ELSE cmdless(); //������ _t = _T_BOOL
    }ELSE /**IF (opsym == '>')*/ {
      IF (dbl) {
        cmdshr(); //������ �������� ������� ���, ������� ������ �����!
      }ELSE IF (modified) {
        cmdmoreeq(); //������ _t = _T_BOOL
      }ELSE cmdmore(); //������ _t = _T_BOOL
    };
  }UNTIL (_waseof);
  //��� ��������� (� _tword) ')' ��� ������ ����������������� ������
  DEC _exprlvl;
  _addrexpr = +FALSE; //�� ��������� � &
#ifdef USE_HINTS
;;  hinttype("end expr",_t);
;;  hint_tword();
#endif
}
}

PROC eatpoke()
//poke*(<ptype>)(<pointerexpr>)=<expr>
{
VAR TYPE t;
#ifdef USE_HINTS
;;  hintstr("//poke"); endhint();
#endif
  _exprlvl = 0x01; //no jump optimization
  eat('*');
  eat('(');
  eattype(); t = _t&~_T_POI;
  eat(')');
  eat('('); //'=' ����� ���� � ���������
  eatexpr(); //todo ���-�� ���������� ��������� ��������� ����� ��������� =, ����� ���������� �������
  //todo ��������� pointer
  eat(')'); //'=' ����� ���� � ���������
  eat('=');
  eatexpr();
  IF (t != _t) {errstr("poke variable type="); erruint(+(UINT)t); errstr(", but expr type="); erruint(+(UINT)_t); enderr(); };
  cmdpoke();
#ifdef USE_HINTS
;;  hintstr("//end poke"); endhint();
#endif
}

PROC eatlet()
//<var>[<[><expr><]>]=<expr>
//<var>-><field>=<expr>
{
VAR TYPE t;
VAR BOOL ispoke;
#ifdef USE_HINTS
;;  hintstr("//let"); endhint();
#endif
  _exprlvl = 0x01; //no jump optimization
  joinvarname(/**iscall*/+FALSE); t = _t; //t!!!
  rdword(); //'['
  ispoke = +FALSE;
  IF (*(PCHAR)_tword/**_cnext*/ == '[') {
    t = idxarray(t); //t = t&(~(_T_ARRAY|_T_POI));
    eat(']');
    ispoke = +TRUE;
  }ELSE {
    WHILE (*(PCHAR)_tword/**_cnext*/ == '-') {
      _t = t;
      IF (ispoke) { //�� ������ ->
        cmdpeek();
      }ELSE { //������ ->
        cmdpushvar(); //��������� (� ��� ����� �� ���������) ��� ������� ����������
      };
      eat('-');
      //��������� ��� ��������� � ����������, ��� _t = ����� ���������
      getstructfield(); //_t = ��� ����
      t = _t; //todo
      rdword(); //������������ structfield
      ispoke = +TRUE;
    };
  };
  eat('=');
 strpush(_joined,_lenjoined);
  eatexpr(); //�������� ��� _t
 _lenjoined = strpop(_joined);
  IF (t!=_t) {
    errstr("let variable type="); erruint(+(UINT)t); errstr(", but expr type="); erruint(+(UINT)_t); enderr();
  };
  IF (ispoke) {
    cmdpoke();
  }ELSE {
    cmdpopvar();
  };
#ifdef USE_HINTS
;;  hintstr("//end let"); hint_tword();
#endif
}

PROC eatwhile RECURSIVE()
//while<expr><cmd>[;]
//; ������ ������ "WHILE (expr);cmd"
/**
LOCAL ;���������� ����������_�������������
TEMPWHILE: ;������������ ����������_�������������.TEMPWHILE
 <�������>
 jp cc,TEMPEND ;������������ ����������_�������������.TEMPEND
 <����>
 jp TEMPWHILE ;������������ ����������_�������������.TEMPWHILE
TEMPEND: ;������������ ����������_�������������.TEMPEND ;��� ��� break
ENDL
*/
{
VAR UINT beglbl;
VAR UINT wasendlbl;
{
#ifdef USE_HINTS
;;  hintstr("//while"); endhint();
#endif
  _exprlvl = 0x00; //jump optimization possible
  wasendlbl = _tmpendlbl;
  beglbl = _curlbl; INC _curlbl;
  _tmpendlbl = _curlbl; INC _curlbl;
  genjplbl(beglbl); cmdlabel();
  eat('(');
  eatexpr(); //parentheses not included
  genjplbl(_tmpendlbl); cmdjpiffalse();
  eat(')');
  eatcmd(); //���� while
  genjplbl(beglbl); cmdjp();
  genjplbl(_tmpendlbl); cmdlabel();
  _tmpendlbl = wasendlbl;
  //IF (*(PCHAR)_tword/**_cnext*/ != ';') {errstr("\';\' expected, but we have \'"); err(*(PCHAR)_tword/**_cnext*/); err('\''); enderr();};
#ifdef USE_HINTS
;;  hintstr("//end while"); endhint();
#endif
}
}

PROC eatrepeat RECURSIVE()
//repeat<cmd>until<expr>
/**
LOCAL ;���������� ����������_�������������
TEMPREPEAT: ;������������ ����������_�������������.TEMPREPEAT
 <����>
 <�������>
 jp cc,TEMPREPEAT ;������������ ����������_�������������.TEMPREPEAT
TEMPEND: ;��� break
ENDL
*/
{
VAR UINT beglbl;
VAR UINT wasendlbl;
{
#ifdef USE_HINTS
;;  hintstr("//repeat"); endhint();
#endif
  wasendlbl = _tmpendlbl;
  beglbl = _curlbl; INC _curlbl;
  _tmpendlbl = _curlbl; INC _curlbl;
  genjplbl(beglbl); cmdlabel();
  eatcmd(); //���� repeat
  IF ( +(CHAR)(+(BYTE)(*(PCHAR)_tword)|0x20)!='u'/**"until"*/ ) err_tword("UNTIL");
  rdword();
  eat('(');
  _exprlvl = 0x00; //jump optimization possible
  eatexpr(); //parentheses not included
  eat(')');
  genjplbl(beglbl); cmdjpiffalse();
  genjplbl(_tmpendlbl); cmdlabel();
  _tmpendlbl = wasendlbl;
#ifdef USE_HINTS
;;  hintstr("//end repeat"); endhint();
#endif
}
}

PROC eatbreak() //todo inline
//break
{
  genjplbl(_tmpendlbl);
  cmdjp();
}

PROC eatif RECURSIVE()
//if <expr> <cmd>[else<cmd>];
//(; ������ ������ "IF (expr);cmd" � ������ ���������� if)
/**
<�������>
LOCAL ;���������� ����������_�������������
 jp cc,TEMPELSE;������������ ����������_�������������.TEMPELSE
 <���� then>
[ jp TEMPENDIF ;������������ ����������_�������������.TEMPENDIF]
TEMPELSE: ;������������ ����������_�������������.TEMPELSE
 <���� else>
TEMPENDIF: ;������������ ����������_�������������.TEMPENDIF
ENDL
*/
{
VAR UINT elselbl;
VAR UINT endiflbl;
{
#ifdef USE_HINTS
;;  hintstr("//if"); endhint();
#endif
  _exprlvl = 0x00; //jump optimization possible
  elselbl = _curlbl; INC _curlbl;
  endiflbl = _curlbl; INC _curlbl;
  eat('(');
  eatexpr(); //parentheses not included
  genjplbl(elselbl); cmdjpiffalse();
  eat(')'); //rdword();
  eatcmd(); //���� then
  IF (*(PCHAR)_tword/**_cnext*/ != ';'/**"endif"*/) {
    IF ( +(CHAR)(+(BYTE)(*(PCHAR)_tword)|0x20)!='e'/**"else"*/ ) err_tword("ELSE or \';\'");
    genjplbl(endiflbl); cmdjp();
    genjplbl(elselbl); cmdlabel();
    rdword();
    eatcmd(); //���� else
    genjplbl(endiflbl); cmdlabel();
    IF (*(PCHAR)_tword/**_cnext*/ != ';'/**"endif"*/) { errstr( "\';\' expected, but we have \'"); err(*(PCHAR)_tword/**_cnext*/); err('\''); enderr(); };
    //������ ������� ';', �� ����� ��� ��������� if
  }ELSE { //������� 'endif' (���� IF ��� ELSE)
    genjplbl(elselbl); cmdlabel();
  };
#ifdef USE_HINTS
;;  hintstr("//end if"); endhint();
#endif
}
}
/**
PROC eatmodule RECURSIVE()
//module<lbl><cmd>
{
  _lentitle = strjoin(_title, _lentitle, _tword);
  _lentitle = stradd(_title, _lentitle, '.');
  INC _namespclvl; //��������� ����� � title

  rdword();
  eatcmd();

  DEC _namespclvl;
  doprefix(_namespclvl); //to prefix
  _lentitle = strcopy(_prefix, _lenprefix, _title); //title = prefix //�������� ����������� �����
}
*/
PROC eatreturn() //todo inline
{
//todo ��������� isfunc (��� �� � �������) ��� ������ ������
#ifdef USE_HINTS
;;  hintstr("//return"); endhint();
#endif
  _exprlvl = 0x01; //no jump optimization
  eatexpr(); //��������� ������ ��� ������!!!
  IF ( _t != (_curfunct&(~_T_RECURSIVE)) ) {errstr("return type="); erruint(+(UINT)_curfunct); errstr(", but expr type="); erruint(+(UINT)_t); enderr(); };
  cmdresult();
#ifdef USE_HINTS
;;  hintstr("//end return"); endhint();
#endif
  _wasreturn = +TRUE; //���������� �������� "�������� ����� return"
}

PROC eatinc()
{
#ifdef USE_HINTS
;;  hintstr("//inc"); endhint();
#endif
  IF (*(PCHAR)_tword == '*') {
    rdword(); //'('
    eatexpr();
    _t = _t&~_T_POI;
    cmdincbyaddr();
  }ELSE {
    //��������� ����� ����� ���������� ��� ���������
    adddots(); //�������� ���
    joinvarname(/**iscall*/+FALSE); //doprefix(_namespclvl); //prefix:=title[FIRST to...];
    cmdinc();
    rdword();
  };
}

PROC eatdec()
{
#ifdef USE_HINTS
;;  hintstr("//dec"); endhint();
#endif
  IF (*(PCHAR)_tword == '*') {
    rdword(); //'('
    eatexpr();
    _t = _t&~_T_POI;
    cmddecbyaddr();
  }ELSE {
    //��������� ����� ����� ���������� ��� ���������
    adddots(); //�������� ���
    joinvarname(/**iscall*/+FALSE); //doprefix(_namespclvl); //prefix:=title[FIRST to...];
    cmddec();
    rdword();
  };
}

PROC var_num(TYPE t, PCHAR s)
{
VAR TYPE tmasked = t&(~_T_ARRAY);
/**  IF ( (t&_T_POI)!=(TYPE)0x00 ) { //������������ ��� ����� (������ equ)
    varstr_tword(); //DB "str"
  }ELSE*/ IF ( (t&_T_CONST)!=(TYPE)0x00 ) {
    varequ(_title); /**varstr(_title); varc( '=' );*/ varstr(s); endvar();
  }ELSE {
    IF (t==tmasked/**(t&_T_ARRAY)==(TYPE)0x00*/) {
      var_alignwsz_label(t);
      //emitvarlabel(_joined); //varstr(_joined); /**varc( ':' );*/ endvar();
    };
    var_def(tmasked, s);
  };
}

//TODO ���������(������ ����?) + ������� �������� � �����������?
PROC do_const_num(TYPE t)
{
  IF ((*(PCHAR)_tword == '-') || (*(PCHAR)_tword == '+')) {
    rdaddword(); //��������� �����
    var_num(t, _tword);
  }ELSE IF (*(PCHAR)_tword == '\'') {
    rdquotes('\'');
    rdch(); //��������� ����������� �������
    _tword[_lentword] = '\0'; //strclose(_tword, _lentword);
    var_num(t, _tword);
  }ELSE IF (*(PCHAR)_tword == '&') { //&<var>, &<func>, &<structinstance>
    rdword(); //������������ &, ��������� ������ �����
    adddots(); //�������� ���
    var_num(_T_ARRAY|_T_UINT, _tword); //_T_ARRAY �� ��� ������� �����
  }ELSE IF (*(PCHAR)_tword == '\"') {
    _lenjoined = strcopy(_title, _lentitle, _joined);
    IF ((t&_T_ARRAY)!=(TYPE)0x00) { //������ ������ �������
      jdot();
      jautonum(_curlbl);
      INC _curlbl;
      //_joined[_lenjoined] = '\0'; //strclose(_joined, _lenjoined);
      var_num(_T_ARRAY|_T_UINT, _joined); //_T_ARRAY �� ��� ������� �����
      asmstrz(); //� ������ _joined //������� ������ varstrz //todo ������� ��������� ���������, ����� ������������ ������? (������ ����� &str � const pchar arr[]?)
    }ELSE {
      varstrz(); //� ������ _joined
    };
  }ELSE var_num(t, _tword);
}

PROC eatextern()
//extern<type><variable>[<[><expr><]>]
{
VAR TYPE t;
#ifdef USE_HINTS
;;  hintstr("//extern"); endhint();
#endif
  _exprlvl = 0x01; //no jump optimization
  eatvarname(); t = _t; //��� �������� ������������ �� _ (������ _name)
  IF (*(PCHAR)_tword == '[') {
    //t = t|_T_ARRAY; //��� � eatvarname
    //rdbrackets(); //_tword='[' //TODO evaluate expr (� �� ������ ���������� � ������, �.�. �� ������������ joined?)
    _lentword = 0; //strclear(_tword); //������ � ������ ������
    rdquotes(']');
    rdch(); //���������� ']'
    //_lenncells = strcopy(_tword, _lentword, _ncells); //n = _tword;
  //}ELSE {
    //n ="1";
    //_lenncells = stradd(_ncells, strclear(_ncells), '1'); //n = n + '1';
    //strclose(_ncells, _lenncells);
    rdword();
  };
  addlbl(t, /**isloc*/+FALSE, (UINT)_typesz[t&_TYPEMASK]/**, _ncells, _lenncells*/); //(_name) //TODO ������ ������� (��������� �� ������ extern?)!
#ifdef USE_HINTS
;;  hintstr("//end extern"); endhint();
#endif
}

PROC eatvar RECURSIVE(BOOL ispar, BOOL body) //���� var, �� body==+TRUE, ����� body==!forward
//var<type><variable>[<[><expr><]>][=<expr>]
//��� � ���������� ��� ���������� ������� <type><variable>
{
VAR TYPE t;
{
#ifdef USE_HINTS
;;  hintstr("//var"); endhint();
#endif
  _exprlvl = 0x01; //no jump optimization
  eatvarname(); t = _t; //��� �������� ������������ �� _ (������ _name)
  IF (*(PCHAR)_tword == '[') {
    //t = t|_T_ARRAY; //��� � eatvarname
   //strpush(_joined,_lenjoined);
    //TODO evaluate expr (� �� ������ ���������� � ������, �.�. �� ������������ joined?)
    _lentword = 0; //strclear(_tword); //������ � ������ ������
    rdquotes(']');
   //_lenjoined = strpop(_joined);
    _lenncells = strcopy(_tword, _lentword, _ncells); //n = _tword;
    rdch(); //���������� ']'
    rdword();
  }ELSE { //n = "1"
    _lenncells = stradd(_ncells, 0/**strclear(_ncells)*/, '1'); //n = n + '1';
    _ncells[_lenncells] = '\0'; //strclose(_ncells, _lenncells);
  };
  IF (body) addlbl(t, /**isloc*/(_namespclvl!=0x00), (UINT)_typesz[t&_TYPEMASK]/**, _ncells, _lenncells*/); //(_name) //TODO ������ ������� ��� ���������!
  IF (ispar) { //parameter of func/proc
    IF (body) {
      var_alignwsz_label(t);
      //emitvarlabel(_joined); //varstr(_joined); /**varc( ':' );*/ endvar();
    };
    _lenjoined = strcopy(_prefix, _lenprefix, _joined); //_lenjoined = strjoin(/**to=*/_joined, 0/**strclear(_joined)*/, _prefix/**, _lenprefix*/); //prefix ������� �� doprefix/eatvarname ����
    jautonum(_parnum);
    INC _parnum; //!!! todo �������� ������
    INC _curlbl; //!!! todo �������� ������
    //_joined[_lenjoined] = '\0'; //strclose(_joined, _lenjoined);
    _lenname = strcopy(_joined, _lenjoined, _name);
    addlbl(t, /**isloc*/+FALSE, (UINT)_typesz[t&_TYPEMASK]/**, "0", _lenncells*/); //�������� � �������, ��� �� �������� ������ //(_name) //TODO ������ ������� ��� ���������!
  };
  IF (body) {
    IF ((t&_T_ARRAY)!=(TYPE)0x00) {
      var_alignwsz_label(t);
      //emitvarlabel(_joined); //varstr(_joined); /**varc( ':' );*/ endvar();
      var_ds(); /**varstr( "\tDS " );*/ varuint(+(UINT)_typesz[t&_TYPEMASK]); varc('*'); varstr(_ncells); endvar();
    }ELSE {
      var_num(t, "0");
    };
    doexp(); //_joined
  };
  IF (*(PCHAR)_tword == '=') { //TODO ���������, ��� �� ������ ������� (�������������!)
    rdword(); //'='
   strpush(_joined,_lenjoined);
    eatexpr();
   _lenjoined = strpop(_joined);
    //IF ( (t!=_t) && !( ((t&_T_POI)!=(TYPE)0x00) && (/**(texpr==_T_UINT)||*/((_t&_T_POI)!=(TYPE)0x00)) ) ) {errstr("let variable="); errstr(_joined); errstr(" type="); erruint(+(UINT)t); errstr(", but expr type="); erruint(+(UINT)_t); enderr(); };
    IF (t!=_t) {errstr("let variable="); errstr(_joined); errstr(" type="); erruint(+(UINT)t); errstr(", but expr type="); erruint(+(UINT)_t); enderr(); };
    cmdpopvar();
  };
  IF (_isrecursive && !ispar) { //local variable of recursive func/proc
    _t = t;
    cmdpushpar(); //todo ��� ������ � ���������?
   strpush(_joined,_lenjoined);
    WHILE (*(PCHAR)_tword==';') {
      rdword();
    }; //C compatibility
    eatcmd(); //recursive function body must be in {} after vars!
   _lenjoined = strpop(_joined);
    _t = t;
    cmdpoppar(); //todo ��� ������ � ���������?
  };
#ifdef USE_HINTS
;;  hintstr("//end var"); endhint();
#endif
}
}

//TODO ���������(������ ����?) + ������� ��������?
PROC eatconst()
//<constnum>::=[-]<num>|'<char>'|"<str>"["<str>"...]
//const<type><variable>[=<constnum>]
//|const<type><variable><[><expr><]>[<[><expr><]>...][={<constnum>[,<constnum>...]}] - ��������� {} ��������� (todo ������� ��� ������������ � �������������� ����������� �������� � ����������)
//|const pchar<variable><[><expr><]><[><expr><]>={"<str>"[,"<str>"...]}
{
VAR TYPE t;
VAR UINT i = 0;
#ifdef USE_HINTS
;;  hintstr("//const"); endhint();
#endif
  _exprlvl = 0x01; //no jump optimization
  eattype(); t = _t|_T_CONST; //��� ��� ��� ��������
  adddots();
  doprefix(_namespclvl); //������� n ���� ���� 'word.' �� title � prefix (name ��� ��������)
  rdword(); //'['
  IF (*(PCHAR)_tword == '[') {
    t = t|_T_ARRAY;
  };
  //_joined �������� ��� ���������
  //_name �������� ���
  _lentitle = strcopy(_joined, _lenjoined, _title); //��� ������������ ��� ��������� ��������
  INC _namespclvl; //��������� ����� � title

  //��� ����� �������� ��� ����
  lbltype();
  _lenname = strcopy(_joined, _lenjoined, _name); //��� ���������
  _lencallee = gettypename(_callee); //����� �������� ���� ��������� � callee (����� ����� lbltype)
  //_lencallee = strcopy(_joined, _lenjoined, _callee);

  //title �������� ��� ��������� (����� ��� ������������ ��� ��������� �������� � ������� � do_const_num)
  //_lenjoined = strcopy(_title, _lentitle, _joined);
  //_joined ���� �������� ��� ���������

  addlbl(t, /**isloc*/+FALSE, (UINT)_typesz[t&_TYPEMASK]/**, _ncells, _lenncells*/); //(_name) //TODO ������ ������� ��� ���������!
  WHILE (*(PCHAR)_tword == '[') { //[size]
    //rdbrackets(); //_tword='['
    _lentword = 0; //strclear(_tword); //������ � ������ ������
    rdquotes(']');
    rdch(); //���������� ']'
    rdword();
  };
  IF (*(PCHAR)_tword == '=') {
    rdword(); //num or '{'
    IF (*(PCHAR)_tword == '{') { //array or struct
      var_alignwsz_label(t);
      //emitvarlabel(_title); //varstr(_title/**_joined*/); /**varc( ':' );*/ endvar();
      doexp(); //_joined //���� �� �������������� ���������? ������ ����������� �������/���������
      REPEAT{ //���� ���� �������� _joined, �� ���������� _title
        rdword(); //num

        IF (t == (_T_STRUCT|_T_CONST)) { //��������� (� �� ��� ���������� ����)
          _lenjoined = strcopy(_callee, _lencallee, _joined); //callee �������� ��� ���� ���������
          _lenjoined = stradd(_joined, _lenjoined, '.');
          jautonum(i);
          _lenname = strcopy(_joined, _lenjoined, _name);
          _t = lbltype();
          do_const_num(lbltype()&(~_T_TYPE)|_T_ARRAY); //_T_ARRAY �� ��� ������� �����
          INC i;
        }ELSE { //�� ���������
          do_const_num(t&(~_T_CONST)); //_T_ARRAY �� ��� ������� �����
        };

        rdword(); //',' or '}'
      }UNTIL (*(PCHAR)_tword == '}');
    }ELSE { //not array
      do_const_num(t);
    };
    rdword();
  };

  DEC _namespclvl; doprefix(_namespclvl); _lentitle=strcopy(_prefix,_lenprefix,_title);/**title =prefix;*/ //�������� ����������� �����

  //_isexp = +FALSE; //���� �� �������������� ���������? ������ ����������� �������/���������

#ifdef USE_HINTS
;;  hintstr("//end const"); endhint();
#endif
}

PROC eatfunc(BOOL isfunc, TYPE oldfunct, BOOL oldwasreturn)
//proc<procname>[recursive][forward](<type><par>,...])[<cmd>]
//|func<type><funcname>[recursive][forward]([<type><par>,...])[<cmd>]
{
VAR BOOL isforward;
  _curlbl = 0; //���������� ��������� ���������, �.�. � ��� ������� �������
  IF ( isfunc ) {
    eattype(); _curfunct = _t;
  }ELSE _curfunct = _T_PROC;
#ifdef USE_HINTS
;;    hintstr("//func "); hinttype(_title,_curfunct);
#endif
  _lenname = strcopy(_tword, _lentword, _name);
  jtitletword();
  rdword(); //'(' or "recursive" or "forward"
  IF (+(CHAR)(+(BYTE)(*(PCHAR)_tword)|0x20) == 'r') {
    _curfunct = _curfunct|_T_RECURSIVE;
    _isrecursive = +TRUE;
    rdword(); //'('
  }ELSE _isrecursive = +FALSE;
  IF (+(CHAR)(+(BYTE)(*(PCHAR)_tword)|0x20) == 'f') {
    isforward = +TRUE;
    rdword(); //'('
  }ELSE isforward = +FALSE;
  addlbl(_curfunct, /**isloc*/+FALSE, 0/**, _ncells"0", 1*/); //(_name) //������ if (!isforward), ������ ��� ���� ��������� ��� ������� � forward
  IF (!isforward) {
    cmdlabel(); //(_joined)
    cmdfunc(); //������ initrgs
    doexp(); //_joined
  };
  jdot();
  _lentitle = strcopy(_joined, _lenjoined, _title);
  INC _namespclvl; //��������� ����� � title

  eat('(');
  _parnum = 0;
  WHILE (!_waseof) {
    IF (*(PCHAR)_tword == ')') BREAK;
    eatvar(/**ispar*/+TRUE, /**body*/!isforward);
    IF (*(PCHAR)_tword == ')') BREAK; //����� ','
    rdword(); //type or ')'
  };
  rdword();

  keepvars(); //������ ����� �����������, �.�. �� ���� ������� ����� ���� ������� (TODO ���������� ����� ��� �������� �������� ����� ����������)

  IF (!isforward) {
    eatcmd(); //���� �������
    _t = _curfunct&(~_T_RECURSIVE);
    cmdret(isfunc);
    IF (isfunc && !_wasreturn) {errstr("return expected"); enderr(); };
#ifdef USE_HINTS
;;    hintstr("/////end func"); endhint();
#endif
  };

  undovars();

  DEC _namespclvl; doprefix(_namespclvl); _lentitle=strcopy(_prefix,_lenprefix,_title);/**title =prefix;*/ //�������� ����������� �����
  _curfunct = oldfunct; //���������� ������� ��� �������
  _wasreturn = oldwasreturn; //�������� �������� "�������� ����� return"
  _isexp = +FALSE;
}

PROC do_callpar RECURSIVE(TYPE funct, UINT parnum)
{
VAR TYPE t;
{
#ifdef USE_HINTS
;;  hintstr("//call_par"); endhint();
#endif
  IF ( (*(PCHAR)_tword!=')') && !_waseof ) {
    _lenjoined = strcopy(_callee, _lencallee, _joined);
    jdot();
    jautonum(parnum);
    //INC _curlbl; //�� �����, �.�. ��� ����� (�.�. ������ �������)
    //_joined[_lenjoined] = '\0'; //strclose(_joined, _lenjoined);
;;    cmtstr(";accesspar="); cmtstr(_joined); endcmt();
    _lenname = strcopy(_joined, _lenjoined, _name);
    t = lbltype(); //(_name)
    IF ((funct&_T_RECURSIVE)!=(TYPE)0x00/**isstacked*/) {
      _t = t;
      cmdpushpar(); //(_joined)
    };
    strpush(_joined,_lenjoined);
    INC _exprlvl; //no jump optimization
    eatexpr(); //����� ���������� ������� do_call � �������� callee (���� �� ����������)! //��������� ������ ��� ������!!!
    DEC _exprlvl;
    IF (t != _t) {errstr("callpar type="); erruint(+(UINT)t); errstr(", but expr type="); erruint(+(UINT)_t); enderr(); };
    _lenjoined = strpop(_joined);
    cmdpopvar(); //(_joined)
    IF (*(PCHAR)_tword == ',') rdword(); //parameter or ')'
    IF (parnum < _MAXPARS) {
      strpush(_joined,_lenjoined);
      do_callpar(/**isfunc,*/ funct, /**isstacked,*/ parnum+1); //����������
      _lenjoined = strpop(_joined);
    }/**ELSE {errstr("too many parameters"); enderr(); }*/;
    IF ((funct&_T_RECURSIVE)!=(TYPE)0x00/**isstacked*/) {
      _t = t;
      cmdpoppar(); //(_joined)
    };
  }ELSE {
    _t = funct&(~_T_RECURSIVE);
    cmdcall();
  };
#ifdef USE_HINTS
;;  hintstr("//end call_par"); endhint();
#endif
}
}

FUNC TYPE do_call RECURSIVE(BOOL isfunc)
//<lbl>([recursive][(<type>)<val>,...])
{
VAR TYPE t;
{
  INC _exprlvl; //no jump optimization
  joinvarname(/**iscall*/+TRUE); t = _t; //t!!!
  IF (t == _T_UNKNOWN) {errstr("unknown function "); errstr(_joined); enderr(); };
#ifdef USE_HINTS
;;    hinttype("call",t);
#endif
  IF (!isfunc) t = (t&_T_RECURSIVE)|_T_PROC; //����� ����� ���� �������� ������� ��� ���������
  strpush(_callee,_lencallee); //�� ������ ��������� �������
  _lencallee = strcopy(_joined, _lenjoined, _callee); //��� �����
  jdot();
  rdword(); //'('
  eat('(');
  do_callpar(t, /**parnum*/0); //���������� [call]title, [���������� ����������], ������������, ��������, [�������������� ����������], �������������� [call]title
  _lencallee = strpop(_callee); //�� ������ ��������� �������
  DEC _exprlvl; //no jump optimization
RETURN t&(~_T_RECURSIVE);
}
}

PROC eatcallpoi()
//call(<poi>)
{
 //��������� ����� ����� ���������� ��� ���������
  adddots(); //�������� ���
  joinvarname(/**iscall*/+FALSE); //doprefix(_namespclvl); //prefix:=title[FIRST to...];
  eat('(');
  eatexpr();
  //todo ��������� pointer
  eat(')');
  cmdcallval();
  rdword();
}

PROC eatlbl() //todo inline
//_lbl<lblname><:>
{
  jtitletword();
  cmdlabel();
  rdword(); //skip ':' for C compatibility
  rdword(); //�����!
}

PROC eatgoto() //������� ������ ������ ������� ��������� //todo inline
//goto<lblname>
{
  //rdword(); //lbl
  jtitletword();
  cmdjp();
  rdword();
}

PROC eatasm()
//asm("asmtext")
{
  //rdword(); //'('
  rdword(); //'\"'
  WHILE (!_waseof) {
    _lentword = 0/**strclear(_tword)*/; //������ � ������ ������
    rdquotes('\"'/**, +FALSE*/);
    asmstr(_tword); endasm();
    rdch(); //���������� ����������� �������
    IF (_cnext != '\"') BREAK;
    rdword(); //'\"' ����������� ������� ����������� ������
  };
  rdword(); //')'
  rdword();
}

PROC eatenum()
//enum{<constname0>,<constname1>...}
{
VAR UINT i = 0;
  //rdword(); //'{'
  WHILE (!_waseof) {
    rdword(); //�����
    varequ(_tword); /**varstr(_tword); varc('=');*/ varuint(i); endvar();
    rdword(); //',' ��� '}'
    IF (*(PCHAR)_tword!=',') BREAK;
    INC i;
  };
  rdword();
}

PROC eatstruct()
//struct<name>{<type1><field1>[;]<type2><field2>[;]...}
{
VAR UINT shift = 0;
VAR UINT varszaddr;
VAR UINT i = 0;
  _lentitle = strjoin(/**to=*/_title, _lentitle, _tword/**, _lentword*/);
  _title[_lentitle] = '\0'; //strclose(_title, _lentitle);
  //strpush(_title,_lentitle); //��� �����
  _lenname = strcopy(_title, _lentitle, _name); //��� �����
  addlbl(_T_STRUCT|_T_TYPE, /**isloc*/+FALSE, 0/**, "0", _lenncells*/); //(_name) //�������������� �������, ����� ���������
  varszaddr = _varszaddr;

  _lentitle = stradd(_title, _lentitle, '.');
  _title[_lentitle] = '\0'; //strclose(_title, _lentitle);
  INC _namespclvl; //��������� ����� � title
  rdword(); //������������ ���
  eat('{');

  WHILE (!_waseof) {
    eattype(); //������������ ���
    //rdword(); //�����
    jtitletword();
    //asmstr(_joined); asmc('='); asmuint(shift); endasm();
    varequ(_joined); /**varstr(_joined); varc('=');*/ varuint(shift); endvar();
    _lenname = strcopy(_joined, _lenjoined, _name);
    addlbl(_t, /**isloc*/+FALSE, (UINT)_typesz[_t&_TYPEMASK]/**, "0", _lenncells*/); //(_name)

    genjplbl(i);
    _lenname = strcopy(_joined, _lenjoined, _name);
    addlbl(_t, /**isloc*/+FALSE, (UINT)_typesz[_t&_TYPEMASK]/**, "0", _lenncells*/); //���������������� (_name)
    INC i;

    shift = shift + (UINT)_typesz[_t&_TYPEMASK];

    rdword(); //��� ��� ';' ��� '}' //������������ �����
    IF (*(PCHAR)_tword==';') rdword(); //���
    IF (*(PCHAR)_tword=='}') BREAK;
  };

  //_lenname = strpop(_name);
  //addlbl(_T_STRUCT|_T_TYPE, /**isloc*/+FALSE/**, "0", _lenncells*/); //�������� � �������, ��� �� �������� ������ //(_name)
  //��� �� ��������� sizeof_structname (=shift)
  //��� ���� �� ��� ��������� ��������� �� ��� �� ��������� (�.�. ���������� � ������, � ��������� ������ �����; ������� ������ ������ - ������ �����)
  setvarsz(varszaddr, shift);

  DEC _namespclvl;
  doprefix(_namespclvl); //to prefix
  _lentitle = strcopy(_prefix, _lenprefix, _title); //title = prefix //�������� ����������� �����
  rdword();
}

PROC eatswitch() //��������� ������������ �����������, �� ����������� ����������� switch ���������
//'J' � ������������ ������ ����� ������, �.�. ��������� ������ ��� ���� � �� ����������� (����� ��������� '.' � ������)
//switch (<byteexpr>){...};
//case <byteconst>: //������������ ��������� � ������ (�� ����������� �� � ���)
//default: //������������ ��������� � ������ (�� ����������� �� � ���)
{
VAR BYTE ib;
VAR UINT wastmpendlbl;
  //rdword(); //'('
  wastmpendlbl = _tmpendlbl;
  _tmpendlbl = _curlbl; INC _curlbl;

  //pushvar <title>.J
  _lenjoined = strcopy(_title, _lentitle, _joined);
  _lenjoined = stradd(_joined, _lenjoined, 'J');
  _joined[_lenjoined] = '\0'; //strclose(_joined , _lenjoined);
  _t = _T_UINT|_T_POI;
  cmdpushnum(); //������������� ��������� � �������� ������� - ������ ��� ��������

  eatidx();
  _t = _T_UINT; //��� �������� �������
  cmdaddpoi();
  cmdpeek();

  cmdjpval();

  //������������ ������ ��������� �������� ������������ ����� ��������
  //procname.aab.<num> = procname.aab.default (���� ��� aab TODO)
  //������������ ������� ���������, ����������� ������������� ������� ��������
  //DW procname.aab.1 (���� ��� aab TODO)
  varstr(_title); varc('J'); endvar();
  ib = 0x00;
  REPEAT {
    asmstr(_title); asmuint(+(UINT)ib); asmc('='); asmstr(_title); asmstr("default"); endasm(); //�� ����! ������� asm
    var_dw(); varstr(_title); varuint(+(UINT)ib); endvar(); //TODO "DP", �.�. �� ������ POINTER?
    INC ib;
  }UNTIL (ib == 0x00);

  eatcmd(); //{...}

  genjplbl(_tmpendlbl); cmdlabel();
  _tmpendlbl = wastmpendlbl;
}

PROC eatcase()
//case <byteconst>:
{
  //rdword(); //byteconst

  //��������� ������������ ����� ��������
  //procname.aab.#<_tword> = $ (���� ��� aab TODO)
  asmstr(_title); asmc('#'); asmstr(_tword); asmc('='); asmc('$'); endasm();

  rdword(); //skip ':' for C compatibility
  rdword(); //�����!
}

FUNC BOOL eatcmd RECURSIVE() //���������� +FALSE, ���� ����� �����
{
{
 //��������� ����� ����� ���������� ��� ���������
  adddots(); //�������� ���
  _c0 = *(PCHAR)_tword;
  IF ((_c0=='}') || _waseof) {
    rdword();
    _morecmd = +FALSE;
  }ELSE {
//    IF (_wasreturn) {
//      IF (_c0!=';') {errstr("cmd after return!"); enderr(); };
//    };
    IF (_cnext=='=') { //let
      eatlet();
    }ELSE IF (_cnext=='[') { //let []
      eatlet();
    }ELSE IF (_cnext=='-') { //let ->
      eatlet();
    }ELSE IF ( (_cnext=='(') && (_spcsize==0) ) { //call
      do_call(/**isfunc*/+FALSE); rdword();
    }ELSE IF ( _cnext==':' ) { //lbl
      eatlbl();
    }ELSE {
      IF (_c0==';') {
        rdword(); //C compatibility
      }ELSE IF (_c0=='{') {
        rdword(); WHILE (eatcmd()) {};
      }ELSE {
        _c0 = +(CHAR)(+(BYTE)_c0|0x20);
        _c2 = +(CHAR)(+(BYTE)_tword[2]|0x20);
        IF       (_c0=='v') { //var
          rdword(); eatvar(/**ispar*/+FALSE, /**body*/+TRUE);
          _isexp = +FALSE; //������ ������, ����� �� �������������� ��������� ���������
        }ELSE IF (_c0=='e') { //enum //extern //export
          IF (_c2=='t') { //extern
            rdword(); eatextern();
          }ELSE IF (_c2=='p') { //export
            rdword(); _isexp = +TRUE;
          }ELSE { //enum
            rdword(); eatenum();
          };
        }ELSE IF (_c0=='c') { //const //case //call
          IF (_c2=='n') { //const
            rdword(); eatconst();
          }ELSE IF (_c2=='l') { //call
            rdword(); eatcallpoi();
          }ELSE { //case
            rdword(); eatcase();
          };
        }ELSE IF (_c0=='f') { //func
          rdword(); eatfunc(+TRUE, _curfunct, _wasreturn);
        }ELSE IF (_c0=='p') { //proc //poke
          IF (_c2=='o') { //proc
            rdword(); eatfunc(+FALSE, _curfunct, _wasreturn);
          }ELSE { //poke
            rdword(); eatpoke();
          };
        }ELSE IF (_c0=='r') { //return //repeat
          IF (_c2=='t') { //return
            rdword(); eatreturn();
          }ELSE { //repeat
            rdword(); eatrepeat();
          };
        }ELSE IF ( _c0=='w' ) { //while
          rdword(); eatwhile();
        }ELSE IF ( _c0=='b' ) { //break
          rdword(); eatbreak(); //no parameters (rds nothing)
        }ELSE IF ( _c0=='d' ) { //dec
          rdword(); eatdec();
        }ELSE IF ( _c0=='i' ) { //inc //if
          IF ( _c2=='c' ) { //inc
            rdword(); eatinc();
          }ELSE { //if
            rdword(); eatif();
          };
        }ELSE IF ( _c0=='g' ) { //goto
          rdword(); eatgoto();
        }ELSE IF ( _c0=='a' ) { //asm
          rdword(); eatasm();
        }ELSE IF ( _c0=='s' ) { //struct //switch
          IF (_c2=='r') { //struct
            rdword(); eatstruct();
          }ELSE { //switch
            rdword(); eatswitch();
          };
//        }ELSE IF ( _c0=='m' ) { //module
//          rdword(); eatmodule();
        }ELSE IF ( _c0=='t' ) { //typedef <type> <name>
          rdword();
          eattype();
          _lenname = strcopy(_tword, _lentword, _name);
          addlbl(_T_TYPE + _t, /**isloc*/+FALSE, (UINT)_typesz[_t]);
          rdword(); //������������ ���
        }ELSE IF ( _c0=='#' ) { //define, include... (���� �������� ���� � ���������� ������ �������� ����������)
          rdword(); //define, undef, include, if, else, [elif], ifdef, ifndef, endif, [import], [line], [error], [pragma]
//��������� �������� ����������:
//_doskipcond � ������� ���� ������, ������� ������� �������� ifdef � ������� ������� ���������� (����� ��������)
//(������ ����������� ����� ���� ������ ����������)
//���� (_doskipcond&1) == 0 (�.�. �� � ���������� �����), �� ������� ifdef ������������ �� ����� ��������� (�.�. else �� ��������)
//������������ ������� ����� ifdef ����� � _doskip
//�� ������� ������ _doskipcond = 1, _doskip = +FALSE
//���� ifdef, ��:
  //_doskipcond = _doskipcond+_doskipcond
  //���� !_doskip, ��:
    //INC _doskipcond;
    //���� ifdef �� �����, �� _doskip = +TRUE
//���� else � ((_doskipcond&1) != 0), �� _doskip = !_doskip
//���� endif, �� _doskip = ((_doskipcond&1) == 0); _doskipcond = _doskipcond>>1
          _c2 = _tword[2];
          IF ((_c2 == 'c')&&(!_doskip)) { //include
            rdword(); //"
            _lentword = 0;
            rdquotes('\"'/**, +FALSE*/); //IF (_c0 == '\"') { rdquotes('>'); }ELSE rdquotes('\"');
            _hinclfile[_nhinclfiles] = _fin;
            _hnline[_nhinclfiles] = _curline;
            INC _nhinclfiles;
            compfile(_tword);
            DEC _nhinclfiles;
            _fin = _hinclfile[_nhinclfiles];
            _curline = _hnline[_nhinclfiles];
            _waseof = +FALSE;
            rdch(); //���������� ����������� �������
          }ELSE IF (_c2 == 'd') { //ifdef/endif/undef
            IF (*(PCHAR)_tword == 'e') { //endif
              //_doskip = +FALSE; //todo ����������� (������� ����� ����)
              _doskip = ((_doskipcond&1) == 0);
              _doskipcond = _doskipcond>>1;
            }ELSE IF (*(PCHAR)_tword == 'i') { //ifdef
              _doskipcond = _doskipcond+_doskipcond;
              rdword(); //���
              _lenname = strcopy(_tword, _lentword, _name);
              //_t = lbltype(); //���� ����, �� _T_UNKNOWN
              IF (!_doskip) {
                INC _doskipcond; //�� ���������� ���� ifdef
                //��� ����� - ���������� ����
                _doskip = (lbltype() == _T_UNKNOWN); //�������� ������� �����, ����� ������������ � #, � ����� ������������ ������ ��
              };
            }ELSE { //undef
              rdword(); //���
              _lenname = strcopy(_tword, _lentword, _name);
              dellbl();
            };
          }ELSE IF (_c2 == 'n') { //ifndef
              _doskipcond = _doskipcond+_doskipcond;
              rdword(); //���
              _lenname = strcopy(_tword, _lentword, _name);
              //_t = lbltype(); //���� ����, �� _T_UNKNOWN
              IF (!_doskip) {
                INC _doskipcond; //�� ���������� ���� ifndef
                //���� ����� - ���������� ����
                _doskip = (lbltype() != _T_UNKNOWN); //�������� ������� �����, ����� ������������ � #, � ����� ������������ ������ ��
              };
          }ELSE IF (_c2 == 's') { //else
            //_doskip = !_doskip;
            IF ((_doskipcond&1) != 0) _doskip = !_doskip; //�� ���������� ���� ifdef
          }ELSE IF ((_c2 == 'f')&&(!_doskip)) { //define
            rdword(); //���
            _lenjoined = strcopy(_tword, _lentword, _joined);
            rdword(); //�������� ��� (
            IF (*(PCHAR)_tword == '(') { //TODO ��� ����������� ��������� � � const
              rdword(); //eat('(');
              eattype();
              eat(')');
              rdquotes(')');
              rdch(); //��������� ����������� ������
              _tword[_lentword] = '\0'; //strclose(_tword, _lentword);
            }ELSE {
              numtype(); //_t
            };
            _lenname = strcopy(_joined, _lenjoined, _name);
            addlbl(_t|_T_CONST, /**isloc*/+FALSE, (UINT)_typesz[_t/**&_TYPEMASK*/]/**, _ncells, _lenncells*/); //(_name) //TODO ������ ������� ��� ���������!
            emitvarpreequ(_name);
            varequ(_name); /**varstr(_name); varc( '=' );*/ varstr(_tword); endvar();
            emitvarpostequ();
          };

          //rdchcmt(); //rdaddword(); //���������� ������ ������ ����� �����������, ������ ��������� ������
          WHILE (_waseols==0/** && !_waseof*/ ) {
            rdchcmt(); //���������� ��� ������
          };
          _tword[_lentword] = '\0'; //strclose(_tword, _lentword); //todo �������� �������� clear..close
          IF (+(BYTE)_cnext < +(BYTE)'!') {
            rdch(); //���������� ��������� ������ �����������, ������ ��������� ������ (TODO ������������� ��� /* */)
          };
          rdword();
        }ELSE {
          errstr("WRONG COMMAND "); errstr(_tword); enderr();
          rdword();
        };
      };
    }; //not a headless cmd
    _morecmd = +TRUE;
  }; //not '{'
  RETURN _morecmd;
}
}

PROC compfile RECURSIVE(PCHAR fn)
{
  _fin = nfopen(fn, "rb");
  IF (_fin != (PBYTE)0) {
    strpush(_fn,_lenfn);
    _lenfn = strjoineol(_fn, 0/**_lenfn*/, fn, '\0');
    _fn[_lenfn] = '\0';
    _waseof = +FALSE;

    _doskipcond = 1;
    _curline = 1;
    initrd();
    rdword();

    WHILE (eatcmd()) {};

    fclose(_fin);
    _lenfn = strpop(_fn);
  }ELSE {
    errstr("no file "); errstr(fn); enderr();
  };
}

FUNC UINT strjoineollast(PCHAR to, UINT tolen, PCHAR s2/**, UINT s2len*/, CHAR eol) //����� ��� �����������!
{ //to = to + s2;
VAR UINT len;
VAR UINT last;
VAR CHAR c;
  to = &to[tolen];
  len = tolen; //����� ��� �����������!
  last = 0; //����� ���������� �����������
  loop:
    c = *(PCHAR)s2;
    IF ((c == '\0') || (len>=_STRMAX)) goto endloop; //���������� �� ����������
    POKE *(PCHAR)(to) = c;
    INC s2;
    IF (c == eol) last = len;
    INC to;
    INC len;
  goto loop; //���������� �� ����������
  endloop:
RETURN last; //����� ���������� �����������
}

PROC compile(PCHAR fn)
{
  _prefix = (PCHAR)_s1; //����������� � doprefix: module/func/const/var/extern - ��������, joinvarname
  _title  = (PCHAR)_s2;
  _callee = (PCHAR)_s3;
  _name   = (PCHAR)_s4;
  _joined = (PCHAR)_s5;
  _ncells = (PCHAR)_s6;

  _fn = (PCHAR)_m_fn;
  _lenfn = 0;

  _tmpendlbl = 0; //����� ������� �� ������� (break ��� ������� ������ ������ � ����)
  _lenstrstk = 0;
  _curlbl = 0; //���������� ��������� ��������� (��� �������� �����)

  initlblbuf();

  _lenname = strcopy("INT", 3, _name);
  addlbl(_T_TYPE + _T_INT, +FALSE, (UINT)_typesz[_T_INT]);
  _lenname = strcopy("UINT", 4, _name);
  addlbl(_T_TYPE + _T_UINT, +FALSE, (UINT)_typesz[_T_UINT]);
  _lenname = strcopy("BYTE", 4, _name);
  addlbl(_T_TYPE + _T_BYTE, +FALSE, (UINT)_typesz[_T_BYTE]);
  _lenname = strcopy("BOOL", 4, _name);
  addlbl(_T_TYPE + _T_BOOL, +FALSE, (UINT)_typesz[_T_BOOL]);
  _lenname = strcopy("LONG", 4, _name);
  addlbl(_T_TYPE + _T_LONG, +FALSE, (UINT)_typesz[_T_LONG]);
  _lenname = strcopy("CHAR", 4, _name);
  addlbl(_T_TYPE + _T_CHAR, +FALSE, (UINT)_typesz[_T_CHAR]);
  //_lenname = strcopy("FLOAT", 5, _name);
  //addlbl(_T_TYPE + _T_FLOAT, +FALSE, (UINT)_typesz[_T_FLOAT]);
  _lenname = strcopy("STRUCT", 6, _name);
  addlbl(_T_TYPE + _T_STRUCTWORD, +FALSE, (UINT)_typesz[_T_STRUCT]);
  _lenname = strcopy("PINT", 4, _name);
  addlbl(_T_TYPE + _T_POI + _T_INT, +FALSE, (UINT)_typesz[_T_POI]);
  _lenname = strcopy("PUINT", 5, _name);
  addlbl(_T_TYPE + _T_POI + _T_UINT, +FALSE, (UINT)_typesz[_T_POI]);
  _lenname = strcopy("PBYTE", 5, _name);
  addlbl(_T_TYPE + _T_POI + _T_BYTE, +FALSE, (UINT)_typesz[_T_POI]);
  _lenname = strcopy("PBOOL", 5, _name);
  addlbl(_T_TYPE + _T_POI + _T_BOOL, +FALSE, (UINT)_typesz[_T_POI]);
  _lenname = strcopy("PLONG", 5, _name);
  addlbl(_T_TYPE + _T_POI + _T_LONG, +FALSE, (UINT)_typesz[_T_POI]);
  _lenname = strcopy("PCHAR", 5, _name);
  addlbl(_T_TYPE + _T_POI + _T_CHAR, +FALSE, (UINT)_typesz[_T_POI]);
  //_lenname = strcopy("PFLOAT", 5, _name);
  //addlbl(_T_TYPE + _T_POI + _T_FLOAT, +FALSE, (UINT)_typesz[_T_POI]);
//  _lenname = strcopy("POINTER", 7, _name);
//  addlbl(_T_TYPE + _T_POI, +FALSE);
  //_lenname = strcopy("PPROC", 5, _name);
  //addlbl(_T_TYPE + _T_POI + _T_PROC, +FALSE, (UINT)_typesz[_T_POI]);

  _lentitle = 0/**strclear(_title)*/;
  POKE *(PCHAR)(_title) = '\0'; //strclose(_title, _lentitle);
  _namespclvl = 0x00;
  //_exprlvl = 0x00; //������ ����� ����������� 0 (����� �������������� ���������) ��� 1 (������) ��� inc-dec (��� ��������� ���������� ���� ������)

 _addrexpr = +FALSE;
  _isexp = +FALSE;
  _curfunct = _T_UNKNOWN; //�� ������ ������
  _wasreturn = +FALSE; //�������� �������� "�������� ����� return"

   _lenjoined = strjoineollast(_joined, 0, fn, '.');
   _lenjoined = strjoin(_joined, _lenjoined, ".asm");
   _joined[_lenjoined] = '\0'; //strclose(_joined, _lenjoined);
  _fout = openwrite(_joined);

   _lenjoined = strjoineollast(_joined, 0, fn, '.');
   _lenjoined = strjoin(_joined, _lenjoined, ".var");
   _joined[_lenjoined] = '\0'; //strclose(_joined, _lenjoined);
  _fvar = openwrite(_joined);

  _nhinclfiles = 0x00;

  initcmd();
  initcode(); //�������� emitregs
  compfile(fn);
  endcode();

  fclose(_fvar);
  fclose(_fout);
}

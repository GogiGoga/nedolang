//����������� �� ����� ���� ������
//����������� �� ������������ ����� � �� ���������� �����
//_asmwaseol ������������ � asmrdword_tokspc(), � eol ��������� ����� �������

//// imported
#include "../_sdk/str.h"
#include "../_sdk/io.h"

VAR UINT _curlnbeg; //����� ������ �� ������ ������ ������ //��� read, emit
VAR BOOL _cmts; //��� read, emit

CONST BOOL _isalphanum[256];

EXTERN PCHAR _tword; //������� �����
EXTERN UINT _lentword;
//EXTERN CHAR _s0[_STRLEN]; //������� �����

EXTERN BOOL _waseof;
EXTERN PBYTE _fin;
EXTERN PBYTE _fout;
FUNC BYTE readfin FORWARD();

EXTERN UINT _curline; //������� ����� ������
EXTERN UINT _waseols; //����� EOL � �������� ����
EXTERN UINT _spcsize; //����� �������� ����� ����������� �������
EXTERN CHAR _cnext;

FUNC UINT stradd FORWARD(PCHAR s, UINT len, CHAR c);
FUNC BOOL strcplow FORWARD(PCHAR s1, PCHAR s2);

PROC rdchcmt FORWARD();
PROC rdch FORWARD();
PROC rdquotes FORWARD(CHAR eol);
PROC initrd FORWARD();

CONST BYTE _ASMMAXSPC;

PROC tokpre FORWARD(); //��������� ������� �����

////
VAR CHAR _c1small;
VAR CHAR _c2small;
VAR BOOL _asmwaseof;
VAR UINT _asmwaseols; //����� EOL � �������� ����
VAR UINT _asmspcsize;

VAR CHAR _stokfn[_STRLEN]; //������� �����
VAR PCHAR _tokfn; //������� �����
VAR UINT _lentokfn;

VAR BYTE _temppar;

#define _TOKLBLBUFSIZE 0x1200
VAR UINT _toklblbuffreestart; //[1];
//VAR UINT _tokoldlblbuffreestart; //[1];
CONST UINT _TOKLBLBUFEOF = 0xffff;
//CONST UINT _TOKLBLBUFMAXSHIFT  = 0x1fb0/**(_LBLBUFSIZE-0x0100)*/; //0x3f00
#define _TOKLBLBUFMAXSHIFT (UINT)(_TOKLBLBUFSIZE-_STRLEN-10)
//todo ����������� ��������
VAR UINT _toklblshift[0x100];
//VAR UINT _tokoldlblshift[0x100];
VAR BYTE _toklbls[_TOKLBLBUFSIZE]; //0x4000
VAR BYTE _toklblhash;

PROC rdaddwordall() //��������� ��������� ������� � �������
{
  _spcsize = 0; //����� �������� ����� ����������� �������
  _waseols = 0;
  _curlnbeg = _curline;

  IF (_isalphanum[+(BYTE)_cnext] ) { //����� � ����������
    REPEAT { //��� ������������ (EOF �� ����������)
      IF (_lentword < _STRMAX) { //� ���� �� ����������� ��� ����� ������, �� �� � ������ ����
        _tword[_lentword] = _cnext;
        INC _lentword;
      };
      _cnext = +(CHAR)readfin();
    }UNTIL (!_isalphanum[+(BYTE)_cnext]/** || _waseof*/ );
  }ELSE { //����� �� ����������������� ������� - ������ ���� ������������ (����� �� �� ������)
    //rdch(); //������ ��� ������ ���������� + ������ ��� ���� ������
    IF (_lentword < _STRMAX) { //� ���� �� ����������� ��� ����� ������, �� �� � ������ ����
      _tword[_lentword] = _cnext;
      INC _lentword;
    };
    _cnext = +(CHAR)readfin();
  }; //������ ��������� ��� ������� � �����, �.�. ��� ��� ������������ cnext � ���� ���

  goto loopgo;
  loop: //REPEAT { //��� ���������� ��� EOF
    _cnext = +(CHAR)readfin();
  loopgo:
    IF (+(BYTE)_cnext < +(BYTE)'!') { //��������� ������
      INC _spcsize; //spaces after tword
      IF (+(BYTE)_cnext == 0x0a) {
        INC _curline;
        _spcsize = 0;
        INC _waseols;
      }ELSE IF (+(BYTE)_cnext == 0x09) {
        _spcsize = _spcsize + 7;
      };
      IF (!_waseof) goto loop;
    };
  //}UNTIL (_wasdieresis || _waseof );
  _tword[_lentword] = '\0'; //strclose(_tword, _lentword); //todo �������� �������� clear..close
}
/**
PROC rdwordall()
//������ ����� _tword (�� ������ � ����������� ������ ��������, �������, ���������)
//����� �������������, ����� �������� ��� ���������������� ������ (�� ������ � cnext, � ������ � ����� ����� ����)
{
  _lentword = 0; //strclear(_tword); //todo �������� �������� clear..close
  rdaddwordall();
}
*/
//////////////////

PROC asmtoken(BYTE token)
{
  writebyte(_fout, token);
}

PROC tokspc()
{
//IF (!_waseol) { //���� �� �������� �� ������ �����
  WHILE (_asmspcsize > +(UINT)_ASMMAXSPC) { //����� �������� ����� ����������� �������
    asmtoken(+_TOKSPC0+_ASMMAXSPC);
    _asmspcsize = _asmspcsize - +(UINT)_ASMMAXSPC;
  };
  IF (_asmspcsize!=0) asmtoken(+_TOKSPC0 + +(BYTE)_asmspcsize);
//};
}

PROC asmrdword_tokspc() //������������ ������� ����� ������� ������� � ������ �����
//todo �������� eol ��� �������?
{
  _asmwaseof = _waseof;
  _asmwaseols = _waseols;
  _asmspcsize = _spcsize; //����� �������� ����� ����������� �������
  IF (_waseols==0) tokspc(); //������������ ������� ����� ������� ������� (����� ����������� �� ����� eol)
  //rdwordall();
  _lentword = 0/**strclear(_tword)*/; //todo �������� �������� clear..close
  rdaddwordall();
  _c1small = +(CHAR)(+(BYTE)(*(PCHAR)_tword)|0x20);
  _c2small = +(CHAR)(+(BYTE)_tword[1]|0x20);
}

PROC toktext() //���������� <text>text<endtext>
{
  asmtoken(+_TOKTEXT);
  fputs(_tword, _fout);
  asmtoken(+_TOKENDTEXT);
}

//������������� ����� ������ � �������� ������ �� ����� ����������/������ (������� _tword)
//������� ��������� ������� � ������, ������� � waseol ��� ������� ':'
PROC tokerr(BYTE token)
{
  asmtoken(+_ERR);
  asmtoken(token);
  WHILE (+TRUE) {
    toktext(); //���������� <text>text<endtext>
    IF ((_waseols!=0)||(_waseof)||(*(PCHAR)_tword==':')) BREAK;
    asmrdword_tokspc();
  }; //TODO � ���� ������
  asmtoken(+_TOKENDERR);
  asmrdword_tokspc(); //[������������ �������� ������� ��� EOL,] ���� ��������� ������� (����� ����������)
  //todo asmfmt_err?
}

//������������� ����� ������ � ������� ������ �� ����� ������ (������� _tword)
//��� �� ����� �������???
PROC tokerrcmd()
{
  tokerr(+_ERRCMD); //todo ������� asmcmd_err...asmfmt_err ������?
}

PROC tokinitlblbuf()
{
  _toklblhash = 0x00;
  REPEAT {
    _toklblshift[_toklblhash] = _TOKLBLBUFEOF;
    INC _toklblhash;
  }UNTIL (_toklblhash == 0x00);
  _toklblbuffreestart = 0;
};

PROC tokcalllbl()
{
VAR PBYTE plbl; //����� � ������� ������������� ����
//VAR PBYTE pstr; //����� � ������ ������������� ����
//VAR BYTE cstr; //������ �� ������
//VAR BYTE clbl; //������ �� ������� �����
//VAR BOOL res;
VAR UINT plbl_idx;
VAR PBYTE calladdr;
  //res = +FALSE;
  _toklblhash = +(BYTE)hash((PBYTE)_tword);

  plbl_idx = _toklblshift[_toklblhash];
  WHILE (plbl_idx != _TOKLBLBUFEOF) { //���� ������� ����� �� �����������
    plbl = &_toklbls[plbl_idx];
    plbl_idx = *(PUINT)(plbl);
    plbl = &plbl[+sizeof(UINT)/**+1*/]; //skip string size
    IF (strcp((PCHAR)_tword, (PCHAR)plbl)) { //����� �������
      plbl = &plbl[10/**_lenname+1*/];
      calladdr = (PBYTE)*(PLONG)(plbl); //todo PPOI
      plbl = &plbl[+sizeof(LONG)]; //todo POI
      _temppar = *(PBYTE)(plbl);
      //INC plbl;
      CALL (calladdr);
      //res = +TRUE;
      //BREAK;
      goto e;
    };
  };
  tokerrcmd(); //not found
  e: //RETURN res;
//asmtoken(0xfd);
}

PROC tokaddlbl1(PCHAR txt, PBYTE proc, BYTE data)
{
VAR PBYTE plbl;
//VAR UINT freestart_idx;
//VAR UINT plbl_idx;
  _toklblhash = +(BYTE)hash((PBYTE)txt);

  //����� ���: ����� � ������ ������� ����� ����� �������� � ������ ����� ��� �� ������� �� ������ ������ �������
  //freestart_idx = _toklblbuffreestart; //[0] //������ ���������� �����
  //plbl_idx = _toklblbuffreestart; //[0] //freestart_idx; //��������� �� ������ ������ ����������� ����� (���� ����������� ���������!) //������ ���������� �����
  plbl = &_toklbls[_toklblbuffreestart]; //[0] [plbl_idx]; //��������� �� ������ ����������� ����� //������ ���������� �����
  //����� �����
  POKE *(PUINT)(plbl) = _toklblshift[_toklblhash]; //������ ��������� �� ������ �������
  _toklblshift[_toklblhash] = _toklblbuffreestart; //[0] //freestart_idx; //����� ��������� �� ������ ������� //������ ���������� �����
  plbl = &plbl[+sizeof(UINT)];
  //POKE *(PBYTE)(plbl) = (BYTE)10/**(BYTE)_lenname*/;
  //INC plbl;
  strcopy(txt, 9/**_lenname*/, (PCHAR)plbl);
  plbl = &plbl[10/**_lenname+1*/];
  POKE *(PLONG)(plbl) = (LONG)proc; //todo PPOI
  plbl = &plbl[+sizeof(LONG)]; //todo POI
  POKE *(PBYTE)(plbl) = data;
  //INC plbl;
  //plbl_idx = +(UINT)(plbl - _toklbls); //��������� ����� ����������� �����
  //_toklblshift[_toklblhash] = freestart_idx; //����� ��������� �� ������ �������
  //_toklblbuflen = _toklblbuflen + plbl_idx - freestart_idx;
  _toklblbuffreestart = +(UINT)(plbl - _toklbls) + 1; //��������� ����� ����������� ����� //plbl_idx; /**[0]*/
  //INC _toklblcount;
}

PROC stringdecapitalize(PCHAR s)
{
VAR CHAR c;
  loop:
    c = *(PCHAR)s;
    IF (c == '\0') goto quit;
    IF (((BYTE)c>=(BYTE)'A') && ((BYTE)c<=(BYTE)'Z')) {
      POKE *(PCHAR)(s) = (CHAR)((BYTE)c | 0x20);
    };
    INC s;
  goto loop;
  quit:
}

PROC tokaddlbl(PCHAR txt, PBYTE proc, BYTE data)
{
  tokaddlbl1(txt, proc, data);
  stringdecapitalize(txt);
  tokaddlbl1(txt, proc, data);
}

//////////////////////////////////////
//���� match ��������, �� _tword ��������� (�������� ���������) � ��������� ��������������� �����
//���� �� ��������, �� _tword �� ���������, ������ �� ������� (������ ���� +FALSE)
//���� ������, �� ������ �� ������� (������ ���� +FALSE)

    //reg � rp ����� ����������� � ����� ������� � inc,dec,[ld],[add],[adc],[sbc]
    //����� ���� reg (b,c,d,e,h,l,a,hx,lx,hy,ly, �� �� i,r!), ���� rp (bc,de,hl,sp,ix,iy, �� �� af!)
    //i,r � af ������������ ��������, �.�. ���� ������ ��� ��������� ������, ��������� ������� �� ������ ��������� ��� ������ ��� ����� ����������

FUNC BOOL matchdirect()
{
VAR BOOL ok;
  IF (*(PCHAR)_tword=='#') {
    asmtoken(+_TOKDIRECT);
    asmrdword_tokspc();
    ok = +TRUE;
  }ELSE ok = +FALSE;
RETURN ok;
}

FUNC BOOL matchcomma()
{
VAR BOOL ok;
  IF (*(PCHAR)_tword==',') {
    asmtoken(+_TOKCOMMA);
    asmrdword_tokspc();
    ok = +TRUE;
  }ELSE ok = +FALSE;
RETURN ok;
}

FUNC BOOL matchprime()
{
VAR BOOL ok;
  IF (*(PCHAR)_tword=='\'') {
    asmtoken(+_TOKPRIMESYM);
    asmrdword_tokspc();
    ok = +TRUE;
  }ELSE ok = +FALSE;
RETURN ok;
}

FUNC BOOL matchquote()
{
VAR BOOL ok;
  IF (*(PCHAR)_tword=='\"') {
    asmtoken(+_TOKDBLQUOTESYM);
    //asmrdword_tokspc();
    ok = +TRUE;
  }ELSE ok = +FALSE;
RETURN ok;
}

FUNC BOOL matchreequ()
{
VAR BOOL ok;
  IF ( (*(PCHAR)_tword=='=') || ((_c1small=='e')&&(_c2small=='q')) ) {
    asmtoken(+_TOKEQUAL/**'='*/);
    asmrdword_tokspc();
    ok = +TRUE;
  }ELSE ok = +FALSE;
RETURN ok;
}

FUNC BOOL matchopen()
{
VAR BOOL ok;
  IF       (*(PCHAR)_tword=='(') {asmtoken(+_TOKOPEN); asmrdword_tokspc(); ok = +TRUE;
  }ELSE IF (*(PCHAR)_tword=='[') {asmtoken(+_TOKOPENSQ); asmrdword_tokspc(); ok = +TRUE;
  }ELSE ok = +FALSE;
RETURN ok;
}

FUNC BOOL matchclose()
{
VAR BOOL ok;
  IF       (*(PCHAR)_tword==')') {asmtoken(+_TOKCLOSE); asmrdword_tokspc(); ok = +TRUE;
  }ELSE IF (*(PCHAR)_tword==']') {asmtoken(+_TOKCLOSESQ); asmrdword_tokspc(); ok = +TRUE;
  }ELSE ok = +FALSE;
RETURN ok;
}

//////////////////////////////////////
FUNC BOOL tokexpr RECURSIVE FORWARD();

PROC eatlabel(BYTE token)
{
  WHILE ((_cnext=='.')||(_cnext=='#')) {
    rdaddwordall(); //��������� �����
    IF (_spcsize==0)
      IF (_isalphanum[+(BYTE)_cnext]) //���� isalpha
        IF (_waseols==0)
          //IF (!_waseof)
            rdaddwordall(); //��������� ��������� �����
  };
  asmtoken(token/**_TOKLABEL ��� _CMDLABEL*/);
  asmtoken(+_TOKTEXT);
  fputs(_tword, _fout);
  asmtoken(+_TOKENDTEXT);
  asmrdword_tokspc(); //������������ ������� ����� ������� (������������) ������� � ������ �����
  //���� ����� ���� ��������� ������ � ������, ����� ���� ������������ �� ������ �� ��� ����� (�� ������ � _asmwaseol)
}

PROC eatval RECURSIVE() //������ ������, �� �� ������� ������ ����� ��������� (������, �������, ����� ������)
//<val>::=
//(<expr>) //��������� (�����������)
//|<num> //���������� ����� (��������� �������)
//|<var> //����� ��� �������� ������� (TODO �� ������������� �����: ��� ��� &<var>)
//|$
//|'CHAR' //���������� ���������
//|-<val> //��������� val, ����� ������� NEG
//|+<val> //��������� val
//|~<val> //��������� val, ����� ������� INV
//|!<val> //��������� val, ����� ������� INV(BOOL)
//|*<val> //��������� ������ �� ������ (��������� val, ����� ������� PEEK)
{ //������� ��� ���������
VAR CHAR opsym;
{
  opsym = *(PCHAR)_tword;
  IF (!_asmwaseof) { //������ �� ������������ � ����� ���������� �����
    IF (+(BYTE)(+(BYTE)opsym - +(BYTE)'0') < 0x0a) { //<num> //����� <num><text>digits<endtext>
      asmtoken(+_TOKNUM);
      asmtoken(+_TOKTEXT);
      fputs(_tword, _fout);
      asmtoken(+_TOKENDTEXT);
      asmrdword_tokspc(); //������������ ������� ����� ������� (������������) ������� � ������ �����
      //���� ����� ���� ��������� ������ � ������, ����� ���� ������������ �� ������ �� ��� ����� (�� ������ � _asmwaseol)
    }ELSE IF (_isalphanum[+(BYTE)opsym] || (opsym=='.') ) { //todo ������ '.', ���� ��������� �������� ����� � ����� //���� isalpha
      eatlabel(+_TOKLABEL);
    }ELSE IF ( opsym=='$' ) {
      asmtoken(+_TOKDOLLAR);
      asmrdword_tokspc(); //������������ ������� ����� ������� (������������) ������� � ������ �����
    }ELSE IF ( opsym=='(' ) {
      asmtoken(+_TOKOPEN);
      asmrdword_tokspc(); //������������ ������� ����� ������� (������������) ������� � ������ �����
      tokexpr(); //�������� //�� ������ �� expr ��� ��������� ')', �� ��������� ������ ��� ������� �� ���������
      asmtoken(+_TOKCLOSE);
      asmrdword_tokspc(); //������������ ������� ����� ������� (������������) ������� � ������ �����
    }ELSE IF ( opsym=='\'' ) { //todo �������� TOK_TEXT...TOK_ENDTEXT
      asmtoken(+_TOKPRIME);
      _lentword = 0/**strclear(_tword)*/; //������ � ������ ������
      rdquotes('\''/**, +FALSE*/);
      toktext(); //���������� <text>text<endtext>
      rdch(); //���������� ����������� �������
      asmtoken(+_TOKPRIME);
      asmrdword_tokspc(); //������������ ������� ����� ������� (������������) ������� � ������ �����
    }ELSE IF ( opsym=='-' ) {
      asmtoken(+_OPPUSH0); asmtoken(+_TOKMINUS/**'-'*/);
      asmrdword_tokspc(); //������������ ������� ����� ������� (������������) ������� � ������ �����
      eatval(); //����������� ����� val
      asmtoken(+_OPSUB);
    }ELSE IF ( opsym=='+' ) {
      asmtoken(+_OPPUSH0); asmtoken(+_TOKPLUS/**'+'*/);
      asmrdword_tokspc(); //������������ ������� ����� ������� (������������) ������� � ������ �����
      eatval(); //����������� ����� val
      asmtoken(+_OPADD);
    }ELSE IF ( opsym=='*' ) {
      asmtoken(+_OPPUSH0); asmtoken(+_TOKSTAR/**'*'*/);
      asmrdword_tokspc(); //������������ ������� ����� ������� (������������) ������� � ������ �����
      eatval(); //����������� ����� val
      asmtoken(+_OPPEEK);
/**    }ELSE IF ( opsym=='~' ) {
      asmtoken(+_OPPUSH0); asmtoken(+_TOKTILDE);
      asmrdword_tokspc(); //������������ ������� ����� ������� (������������) ������� � ������ �����
      eatval(); //����������� ����� val
      asmtoken(+_OPINV);
    }ELSE IF ( opsym=='!' ) {
      asmtoken(+_OPPUSH0); asmtoken(+_TOKEXCL);
      asmrdword_tokspc(); //������������ ������� ����� ������� (������������) ������� � ������ �����
      eatval(); //����������� ����� val
      asmtoken(+_OPINV); //todo BOOL
*/    }ELSE { tokerr(+_ERREXPR);/**errstr( ">>>WRONG PREFIX " ); err( opsym ); enderr();*/ };
  };
}
}

PROC eatmul()
{
  /**asmtoken(_ASMOPPUSHSKIP1);*/ asmtoken(+_TOKSTAR/**'*'*/);
  asmrdword_tokspc(); //������������ ������� ����� ������� (������������) ������� � ������ �����
  eatval(); //������ ������, �� �� ������� ������ ����� ���������
  asmtoken(+_OPMUL);
}

PROC eatdiv()
{
  /**asmtoken(_ASMOPPUSHSKIP1);*/ asmtoken(+_TOKSLASH/**'/'*/);
  asmrdword_tokspc(); //������������ ������� ����� ������� (������������) ������� � ������ �����
  eatval(); //������ ������, �� �� ������� ������ ����� ���������
  asmtoken(+_OPDIV);
}

PROC eatand()
{
  /**asmtoken(_ASMOPPUSHSKIP1);*/ asmtoken(+_TOKAND/**'&'*/);
  asmrdword_tokspc(); //������������ ������� ����� ������� (������������) ������� � ������ �����
  eatval(); //������ ������, �� �� ������� ������ ����� ���������
  asmtoken(+_OPAND);
}

PROC eatandbool()
{
  /**asmtoken(_ASMOPPUSHSKIP2);*/ asmtoken(+_TOKAND/**'&'*/); asmtoken(+_TOKAND/**'&'*/);
  asmrdword_tokspc(); //������������ ������� ����� ������� (������������) ������� � ������ �����
  eatval(); //������ ������, �� �� ������� ������ ����� ���������
  asmtoken(+_OPAND);
}

PROC eatmulval RECURSIVE() //������ ������, �� �� ������� ������ ����� ��������� (������, �������, ����� ������)
{ //������� ��� ���������
VAR CHAR opsym;
VAR BOOL dbl;
{
  eatval(); //������ ������, �� �� ������� ������ ����� ���������
  WHILE ((!_asmwaseof)&&(_asmwaseols==0)) { //��� ��������� �������� � �������-������ ����� �� (_asmwaseol - ����� ����� val)
    opsym=*(PCHAR)_tword;
    dbl = (opsym=='&')&&(_cnext==opsym); //C compatibility
    IF (dbl) asmrdword_tokspc(); //use '&' //C compatibility
    IF       (opsym=='*') {eatmul();
    }ELSE IF (opsym=='/') {eatdiv();
    }ELSE IF (opsym=='&') {
      IF (dbl) {eatandbool();
      }ELSE eatand();
    }ELSE BREAK;
  };
}
}

PROC eatadd()
{
  /**asmtoken(_ASMOPPUSHSKIP1);*/ asmtoken(+_TOKPLUS/**'+'*/);
  asmrdword_tokspc(); //������������ ������� ����� ������� (������������) ������� � ������ �����
  eatmulval(); //������ ������, �� �� ������� ������ ����� ���������
  asmtoken(+_OPADD);
}

PROC eatsub()
{
  /**asmtoken(_ASMOPPUSHSKIP1);*/ asmtoken(+_TOKMINUS/**'-'*/);
  asmrdword_tokspc(); //������������ ������� ����� ������� (������������) ������� � ������ �����
  eatmulval(); //������ ������, �� �� ������� ������ ����� ���������
  asmtoken(+_OPSUB);
}

PROC eator()
{
  /**asmtoken(_ASMOPPUSHSKIP1);*/ asmtoken(+_TOKPIPE/**'|'*/);
  asmrdword_tokspc(); //������������ ������� ����� ������� (������������) ������� � ������ �����
  eatmulval(); //������ ������, �� �� ������� ������ ����� ���������
  asmtoken(+_OPOR);
}

PROC eatorbool()
{
  /**asmtoken(_ASMOPPUSHSKIP2);*/ asmtoken(+_TOKPIPE/**'|'*/); asmtoken(+_TOKPIPE/**'|'*/);
  asmrdword_tokspc(); //������������ ������� ����� ������� (������������) ������� � ������ �����
  eatmulval(); //������ ������, �� �� ������� ������ ����� ���������
  asmtoken(+_OPOR);
}

PROC eatxor()
{
  /**asmtoken(_ASMOPPUSHSKIP1);*/ asmtoken(+_TOKCARON/**'^'*/);
  asmrdword_tokspc(); //������������ ������� ����� ������� (������������) ������� � ������ �����
  eatmulval(); //������ ������, �� �� ������� ������ ����� ���������
  asmtoken(+_OPXOR);
}

PROC eatxorbool()
{
  /**asmtoken(_ASMOPPUSHSKIP2);*/ asmtoken(+_TOKCARON/**'^'*/); asmtoken(+_TOKCARON/**'^'*/);
  asmrdword_tokspc(); //������������ ������� ����� ������� (������������) ������� � ������ �����
  eatmulval(); //������ ������, �� �� ������� ������ ����� ���������
  asmtoken(+_OPXOR);
}

PROC eatsumval RECURSIVE() //������ ������, �� �� ������� ������ ����� ��������� (������, �������, ����� ������)
{ //������� ��� ���������
VAR CHAR opsym;
VAR BOOL dbl;
{
  eatmulval(); //������ ������, �� �� ������� ������ ����� ���������
  WHILE ((!_asmwaseof)&&(_asmwaseols==0)) { //��� ��������� �������� � �������-������ ����� �� (_asmwaseol - ����� ����� val)
    opsym=*(PCHAR)_tword;
    dbl = ( (opsym=='|')||(opsym=='^') )&&(_cnext==opsym); //C compatibility
    IF (dbl) asmrdword_tokspc(); //use '|' or '^' //C compatibility
    IF (opsym=='+') {eatadd();
    }ELSE IF (opsym=='-') {eatsub();
    }ELSE IF (opsym=='|') {
      IF (dbl) {eatorbool();
      }ELSE eator();
    }ELSE IF (opsym=='^') {
      IF (dbl) {eatxorbool();
      }ELSE eatxor();
    }ELSE BREAK;
  };
}
}

PROC eatshl()
{
  /**asmtoken(_ASMOPPUSHSKIP2);*/ asmtoken(+_TOKLESS/**'<'*/); asmtoken(+_TOKLESS/**'<'*/);
  asmrdword_tokspc(); //������������ ������� ����� ������� (������������) ������� � ������ �����
  eatsumval(); //������ ������, �� �� ������� ������ ����� ���������
  asmtoken(+_OPSHL);
}

PROC eatshr()
{
  /**asmtoken(_ASMOPPUSHSKIP2);*/ asmtoken(+_TOKMORE/**'>'*/); asmtoken(+_TOKMORE/**'>'*/);
  asmrdword_tokspc(); //������������ ������� ����� ������� (������������) ������� � ������ �����
  eatsumval(); //������ ������, �� �� ������� ������ ����� ���������
  asmtoken(+_OPSHR);
}

PROC eatless()
{
  asmtoken(+_TOKLESS);
  asmrdword_tokspc(); //������������ ������� ����� ������� (������������) ������� � ������ �����
  eatsumval(); //������ ������, �� �� ������� ������ ����� ���������
  asmtoken(+_OPLESS);
}

PROC eatlesseq()
{
  asmtoken(+_TOKLESS); asmtoken(+_TOKEQUAL);
  asmrdword_tokspc(); //������������ ������� ����� ������� (������������) ������� � ������ �����
  eatsumval(); //������ ������, �� �� ������� ������ ����� ���������
  asmtoken(+_OPLESSEQ);
}

PROC eatmore()
{
  asmtoken(+_TOKMORE);
  asmrdword_tokspc(); //������������ ������� ����� ������� (������������) ������� � ������ �����
  eatsumval(); //������ ������, �� �� ������� ������ ����� ���������
  asmtoken(+_OPMORE);
}

PROC eatmoreeq()
{
  asmtoken(+_TOKMORE); asmtoken(+_TOKEQUAL);
  asmrdword_tokspc(); //������������ ������� ����� ������� (������������) ������� � ������ �����
  eatsumval(); //������ ������, �� �� ������� ������ ����� ���������
  asmtoken(+_OPMOREEQ);
}

PROC eateq()
{
  asmtoken(+_TOKEQUAL); asmtoken(+_TOKEQUAL);
  asmrdword_tokspc(); //������������ ������� ����� ������� (������������) ������� � ������ �����
  eatsumval(); //������ ������, �� �� ������� ������ ����� ���������
  asmtoken(+_OPEQ);
}

PROC eatnoteq()
{
  asmtoken(+_TOKEXCL); asmtoken(+_TOKEQUAL);
  asmrdword_tokspc(); //������������ ������� ����� ������� (������������) ������� � ������ �����
  eatsumval(); //������ ������, �� �� ������� ������ ����� ���������
  asmtoken(+_OPNOTEQ);
}

FUNC BOOL tokexpr RECURSIVE() //������ ������, �� �� ������� ������ ����� ��������� (������, �������, ����� ������)
{ //������� ��� ���������
VAR CHAR opsym;
VAR BOOL modified;
VAR BOOL dbl;
//VAR BOOL ok;
{
  //ok = +TRUE;
  matchdirect(); //������� '#'
  eatsumval(); //������ ������, �� �� ������� ������ ����� ���������
  WHILE ((!_asmwaseof)&&(_asmwaseols==0)) { //��� ��������� �������� � �������-������ ����� �� (_asmwaseol - ����� ����� val)
    opsym = *(PCHAR)_tword;
    modified = (_cnext=='=');
    dbl = (_cnext==opsym);
    IF (modified||dbl) asmrdword_tokspc(); //use '=' or '>' or '<'
    IF (opsym=='<') {
      IF (dbl) {eatshl(); //������ �������� ������� ���, ������� ������ �����!
      }ELSE IF (modified) {eatlesseq();
      }ELSE {eatless();
      };
    }ELSE IF (opsym=='>') {
      IF (dbl) {eatshr(); //������ �������� ������� ���, ������� ������ �����!
      }ELSE IF (modified) {eatmoreeq();
      }ELSE {eatmore();
      };
    }ELSE IF (opsym=='=') {eateq();
    }ELSE IF (opsym=='!') {eatnoteq();
    }ELSE BREAK;
  };
  RETURN +TRUE; //ok; //todo err
}
}

FUNC BOOL tokexpr_close() //����� ���� ������ ������ ������ ���������, �.�. ������ ��� ����������
{
VAR BOOL ok;
  ok =             tokexpr();
  IF (ok) ok = matchclose();
  RETURN ok;
}

PROC asm_direct_expr_close_token(BYTE token)
{
  //if(matchdirect()) {
    IF (tokexpr()) {
      IF (matchclose()) {asmtoken(token);
      }ELSE tokerr(+_ERRCLOSE); //no closing bracket
    }ELSE tokerr(+_ERREXPR); //wrong expr
  //}else tokerr(_ERRPAR); //no direct
}

PROC tokcomment()
{
  asmtoken(+_TOKCOMMENT);
  _lentword = 0/**strclear(_tword)*/; //������ � ������ ������
  IF (_waseols==0) { //����� �� ������ ��������� ; ����� ��������
    WHILE (_spcsize != 0) { //�������� ��������� �������
      _lentword = stradd(_tword, _lentword, ' ');
      DEC _spcsize;
    };
    WHILE (_waseols==0/**&&(!_waseof)*//**&&(_cnext!=_DIERESIS)*/) {
      rdchcmt(); //���������� ��� ������
    };
  };
  _tword[_lentword] = '\0'; //strclose(_tword, _lentword); //todo �������� �������� clear..close
  toktext(); //���������� <text>text<endtext>
  asmtoken(+_TOKENDCOMMENT);
  IF (+(BYTE)_cnext < +(BYTE)'!') {
    IF (_cnext == '\t') {
      _spcsize = _spcsize + 8;
    }ELSE {
      INC _spcsize;
    };
    rdch(); //������ ��� ������ ���������� + ������ ��� ���� ������
  };
  asmrdword_tokspc(); //[������������ �������� ������� ��� EOL,] ���� ��������� ������� (����� ����������)
}

PROC tokorg()
{
  asmtoken(+_CMDORG);
  asmrdword_tokspc(); //������� �������
  IF (tokexpr()) {asmtoken(+_FMTCMD);
  }ELSE tokerr(+_ERREXPR); //wrong expr
}

PROC tokalign()
{
  asmtoken(+_CMDALIGN);
  asmrdword_tokspc(); //������� �������
  IF (tokexpr()) {asmtoken(+_FMTCMD);
  }ELSE tokerr(+_ERREXPR); //wrong expr
}
/**
PROC tokpage()
{
  tokerrcmd();
}

PROC tokif()
{
  asmtoken(+_CMDIF);
  asmrdword_tokspc(); //������� �������
  IF (tokexpr()) {asmtoken(+_FMTCMD);
  }ELSE tokerr(+_ERREXPR); //wrong expr
  //tokerrcmd();
}

PROC tokelse()
{
  asmtoken(+_CMDELSE);
  asmrdword_tokspc(); //������� �������
  //tokerrcmd();
}

PROC tokendif()
{
  asmtoken(+_CMDENDIF);
  asmrdword_tokspc(); //������� �������
  //tokerrcmd();
}

PROC tokdup()
{
  tokerrcmd();
}

PROC tokedup()
{
  tokerrcmd();
}

PROC tokmacro()
{
  tokerrcmd();
}

PROC tokendm()
{
  tokerrcmd();
}
*/
PROC tokexport()
{
  asmtoken(+_CMDEXPORT);
  asmrdword_tokspc(); //������� call
  eatlabel(+_TOKLABEL);
  //asmmtoken(+_TOKTEXT);
  //fputs(_tword, _fout);
  //asmtoken(+_TOKENDTEXT);
  //asmrdword_tokspc(); //������������ ������� ����� ������� (������������) ������� � ������ �����
  asmtoken(+_FMTCMD); //����� ������������ label
}
/**
PROC toklocal()
{
  tokerrcmd();
}

PROC tokendl()
{
  tokerrcmd();
}

PROC tokdisp()
{
  asmtoken(+_CMDDISP);
  asmrdword_tokspc(); //������� �������
  IF (tokexpr()) {asmtoken(+_FMTCMD);
  }ELSE tokerr(+_ERREXPR); //wrong expr
}

PROC tokent()
{
  asmtoken(+_CMDENT);
  asmrdword_tokspc(); //������� �������
  asmtoken(+_FMTCMD);
}
*/
PROC tokinclude()
{
  asmtoken(+_CMDINCLUDE);
  asmrdword_tokspc(); //������� �������
  IF (matchquote()) {
    //asmtoken(+_OPWRSTR);
    _lentword = 0/**strclear(_tword)*/; //������ � ������ ������
    rdquotes('\"');
    toktext(); //���������� <text>text<endtext>
    rdch(); //���������� ����������� �������
    asmtoken(+_TOKDBLQUOTESYM);
    asmrdword_tokspc();
  }ELSE tokerr(+_ERREXPR);
  //asmtoken(+_FMTCMD); //todo?
}

PROC tokincbin()
{
  asmtoken(+_CMDINCBIN);
  asmrdword_tokspc(); //������� �������
  IF (matchquote()) {
    //asmtoken(+_OPWRSTR);
    _lentword = 0/**strclear(_tword)*/; //������ � ������ ������
    rdquotes('\"');
    toktext(); //���������� <text>text<endtext>
    rdch(); //���������� ����������� �������
    asmtoken(+_TOKDBLQUOTESYM);
    asmrdword_tokspc();
  }ELSE tokerr(+_ERREXPR);
  //todo ��������� (�������, �����)
  //asmtoken(+_FMTCMD); //todo?
}

PROC tokdb()
{
  asmtoken(+_CMDDB);
  asmrdword_tokspc(); //������� �������
  REPEAT {
    IF (matchquote()) {
      asmtoken(+_OPWRSTR);
      _lentword = 0/**strclear(_tword)*/; //������ � ������ ������
      rdquotes('\"');
      toktext(); //���������� <text>text<endtext>
      rdch(); //���������� ����������� �������
      asmtoken(+_TOKDBLQUOTESYM);
      asmrdword_tokspc();
    }ELSE IF (tokexpr()) {
      asmtoken(+_OPWRVAL);
    }ELSE tokerr(+_ERREXPR);
  }UNTIL (!matchcomma());
  asmtoken(+_FMTCMD);
}

PROC tokdw()
{
  asmtoken(+_CMDDW);
  asmrdword_tokspc(); //������� �������
  REPEAT {
    tokexpr();
    asmtoken(+_OPWRVAL);
  }UNTIL (!matchcomma());
  asmtoken(+_FMTCMD);
}

PROC tokdl()
{
  asmtoken(+_CMDDL);
  asmrdword_tokspc(); //������� �������
  REPEAT {
    tokexpr();
    asmtoken(+_OPWRVAL);
  }UNTIL (!matchcomma());
  asmtoken(+_FMTCMD);
}

PROC tokds()
{
  asmtoken(+_CMDDS);
  asmrdword_tokspc(); //������� �������
  tokexpr();
  WHILE (matchcomma()) {
    tokexpr();
  };
  asmtoken(+_FMTCMD);
}
/**
PROC tokdisplay()
{
  tokerrcmd();
}

PROC tokrepeat()
{
  tokerrcmd();
}

PROC tokuntil()
{
  tokerrcmd();
}

PROC tokstruct()
{
  tokerrcmd();
}

PROC tokendstruct()
{
  tokerrcmd();
}
*/
PROC tokcolon()
{
  asmtoken(+_TOKCOLON);
  asmrdword_tokspc(); //������� ':'
}

///////////////////////

PROC tokcmd()
{
  //_asmwaseof=_waseof;
  //rdword_tokspc(); //��������� � ���������� ����������
  //stringdecapitalize(_tword,_lentword);
  IF (*(PCHAR)_tword==';') {tokcomment();
/**  }ELSE IF (*(PCHAR)_tword==':') {
    asmtoken(+_TOKCOLON);
    asmrdword_tokspc(); //������� ':'
*/
  }ELSE IF (_asmspcsize!=0) {
    //tokasm(); //assembly mnemonic or directive
    tokcalllbl();
/**    IF (!tokcalllbl(_tword)) {
      tokerrcmd(); //wrong _tword
    };*/
  }ELSE { //<label> ��� <label>=<expr>
    //asmtoken(0xff);
    eatlabel(+_CMDLABEL); //������� �����
    IF (matchreequ()) { //<label>=<expr>
      IF (tokexpr()) {
        asmtoken(+_FMTREEQU);
      }ELSE tokerr(+_ERREXPR); //wrong expr
    }ELSE //IF ((_asmwaseols!=0)||(_asmwaseof)||(*(PCHAR)_tword==':')||(*(PCHAR)_tword==';')){ //<label>
      asmtoken(+_FMTCMD); //<label>
    //}ELSE tokerrcmd(); //wrong _tword
  };
  //����� eol � �������, ������� �� �������� ��� ������ ��������� �����
  IF (_asmwaseols!=0) {
    WHILE (_asmwaseols!=0) {
      asmtoken(+_TOKEOL);
      DEC _asmwaseols;
    };
    tokspc(); //������������ ������� �����
  };
}

PROC tokinit()
{
  _tokfn = (PCHAR)_stokfn;
  tokinitlblbuf();
  tokpre(); //��������� ������� �����
}

PROC tokenize(PCHAR fn)
{
VAR UINT i;
  _fin = nfopen(fn, "rb");
  IF (_fin != (PBYTE)0) {
    _waseof = +FALSE;
    _curline = 1;
    initrd();

    _lentokfn = 0;
    i = 0;
    WHILE (fn[_lentokfn] != '\0') {
      _tokfn[_lentokfn] = fn[_lentokfn];
      IF (fn[_lentokfn] == '.') i = _lentokfn;
      INC _lentokfn;
    };
    //_lentokfn = strjoineol(_tokfn, 0, fn, '.'); //terminator is not copied
    //_lentokfn = stradd(_tokfn, _lentokfn, '.');
    _lentokfn = i+1; //after last dot
    _lentokfn = stradd(_tokfn, _lentokfn, (CHAR)((BYTE)fn[_lentokfn]&0xdf));
    _lentokfn = stradd(_tokfn, _lentokfn, '_');
    _tokfn[_lentokfn] = '\0';
    _fout = openwrite(_tokfn);

    _asmwaseols = 0;
    _asmwaseof = +FALSE;
    asmrdword_tokspc(); //������ ������ �������

    WHILE (!_asmwaseof) { //todo ��� endasm ('}')
      tokcmd();
    };

    asmtoken(+_TOKEOF);
    fclose(_fout); //closefout();

    fclose(_fin); //closefin();
  };
}

PROC tokenize_end()
{
}

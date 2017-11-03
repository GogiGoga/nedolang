//// imported
#include "../_sdk/str.h"
#include "../_sdk/io.h"
#include "../_sdk/emit.h"

PROC asmorgword FORWARD(LONG addr);
PROC asmloop FORWARD();
PROC initmemmodel FORWARD();

////
///////����������, ����������� � post (��������� ���������� � �������� ������):

EXTERN UINT _curlnbeg; //����� ������ �� ������ ������ ������ //��� read, emit

VAR BYTE _token; //������� ��������� �����
VAR BYTE _curdir; //����� ������� �������������� ��������� ���������� (����� ��� ���������� ��������� �������)

VAR BYTE _reg; //��������� �������
VAR BYTE _oldreg; //���������� �������
VAR BYTE _base; //���� ���� �������
VAR BYTE _base2; //����2 ���� ������� (��� �������� ���������)

VAR BYTE _nvalues; //����� �������� � �����

VAR UINT _curaddr; //�����, ���� �����
VAR UINT _curshift; //$=(_curaddr+curshift), curshift=(disp-_curaddr)
VAR UINT _curbegin; //��������� ����� �����, ���� �����
//VAR BYTE _curpage0;
//VAR BYTE _curpage1;
//VAR BYTE _curpage2;
//VAR BYTE _curpage3;

VAR UINT _plabel_index; //����� findlabel �������� ��������� �� ������ ������ �����
VAR UINT _curplabel_index; //��������� ��������� _plabel_index ��� ����������� �����
VAR UINT _hash;
VAR UINT _curhash; //��������� ��������� ��� ����������� �����

VAR BYTE _isaddr; //����� "� ��������� ������������� �����" (���������� � ������ ������, ������� ��������� � ������� ����� ��� ����� ����� (label, =, dw, jp, call, ld))
//todo ���� ������ $-label!!! �� ����� 1*($-label)

////////����������, �� ����������� � post:

VAR PBYTE _labelN; //��������� �� ������� ������� �����
VAR BOOL _labelchanged; //���� "�������� �����" - ����� ��� ������ �� LABEL (�� �� �� REEQU)
VAR UINT _passindex; //����� �������
VAR BYTE _prefixedtoken; //�������������� ����� � ������ \n � �.�.

VAR BYTE _ninclfiles; //����� �������� ������

//////////////////////////////////////////////////////////////////////////////

VAR PBYTE _forg;
VAR PBYTE _fpost;
VAR PBYTE _fincb;
VAR PBYTE _fdecl;
VAR BOOL _asms;

#define _MAXVALS 0x10
VAR LONG _value[_MAXVALS];
#define _MAXINCLUDES 0x08
VAR PBYTE _inclfile[_MAXINCLUDES];

VAR UINT _nlabels;
VAR UINT _lenlabels;

CONST BYTE _LABELPAGEMASK = 0x00; //0x07
#ifdef TARGET_THUMB
#define _LABELPAGESIZE 0xfffffe
#else
#define _LABELPAGESIZE 0x6500 /**0x7300*/
#endif
#define _LABELPAGEMAXSHIFT (UINT)(_LABELPAGESIZE-_STRLEN-10)
//CONST UINT _LABELPAGEFREESTART = 0x0200;
VAR UINT _labelpagefreestart; //[1];
//CONST UINT _LABELPAGEDATASHIFT = 0x0202;
#ifdef TARGET_THUMB
CONST UINT _LABELPAGEEOF = 0xffffffff;
#else
CONST UINT _LABELPAGEEOF = 0xffff;
#endif

//todo ����������� ��������
VAR BYTE _labels0[_LABELPAGESIZE]; //0x4000
//VAR BYTE _labels1[0xc000]/**_LABELPAGESIZE*/;
//VAR BYTE _labels2[0xc000]/**_LABELPAGESIZE*/;
//VAR BYTE _labels3[0xc000]/**_LABELPAGESIZE*/;
//VAR BYTE _labels4[0xc000]/**_LABELPAGESIZE*/;
//VAR BYTE _labels5[0xc000]/**_LABELPAGESIZE*/;
//VAR BYTE _labels6[0xc000]/**_LABELPAGESIZE*/;
//VAR BYTE _labels7[0xc000]/**_LABELPAGESIZE*/;
VAR UINT _labelshift[0x400]; //todo 8 pages
VAR PBYTE _labelpage[1]; //8

VAR BYTE _m_curlabeltext[_STRLEN]; //��� �� fn?
VAR BYTE _m_evallabeltext[_STRLEN];

VAR PBYTE _curlabeltext;
VAR PBYTE _evallabeltext;
VAR UINT _labellen; //����� ����� ��� findlabel //������� 0

//0x0000..0x01ff - ������ ������ �������
//0x0200..0x0201 - ����� ������ ���������� ����� (freestart)
//������ ���� �������
//������ �������:
//2 ����� - ����� ��������� �����
//N ���� - ����� �����
//1 ���� - '\0'
//1 ���� - ����� ����� (defined, accessed, macro)
//4 ����� - �������� �����

CONST BYTE _ASMLABEL_DEFINED = 0x01;
CONST BYTE _ASMLABEL_ACCESSED= 0x02;
CONST BYTE _ASMLABEL_MACRO   = 0x04;
CONST BYTE _ASMLABEL_ISADDR  = 0x08;

PROC findlabel FORWARD(PBYTE labeltext);
FUNC LONG getlabel FORWARD();

//��� ��������� post labels?
//todo ����� ��������� �� ��������� ���������� � ������ �� �����, � ����� ������������� ��� ��, ��� ������� ���������������� �����
//� ���� ������ ��� ������� � ���������� ���������� �������

PROC decltoken(BYTE bb)
{
  writebyte(_fdecl,bb);
}

PROC asmpushvalue(LONG value)
{
/**  IF ((value & 0xffffL) != value) {
    errstr("val>65536"); enderr();
  };*/
  IF (_nvalues != _MAXVALS) {
    _value[_nvalues] = value;
    INC _nvalues;
  }ELSE {errstr("maxvals"); enderr(); };
}

PROC asmpushbool(BOOL b)
{
  //IF (b) {asmpushvalue(1L);
  //}ELSE asmpushvalue(0L);
  asmpushvalue((LONG)(BYTE)b);
}

FUNC LONG asmpopvalue()
{
//VAR LONG tempvalue;
//  IF (_nvalues!=0x00) {
    DEC _nvalues;
    //tempvalue = _value[_nvalues];
//  }ELSE {
    //tempvalue = 0xDEADBEEFL;
//    errstr("no data"); enderr(); //debug (can't be error after tokenizer)
//  };
  RETURN _value[_nvalues]; //tempvalue;
}
/**
PROC asmwritestate()
{
  writebyte(_fpost, +_DIR_READSTATE);

  writebyte(_fpost, _token);
  writebyte(_fpost, _curdir);

  writebyte(_fpost, _reg);
  writebyte(_fpost, _oldreg);
  writebyte(_fpost, _base);
  writebyte(_fpost, _base2);

  writebyte(_fpost, _nvalues);
  WHILE (_nvalues!=0x00) {
    writelong(_fpost, asmpopvalue());
  };

  writeuint(_fpost, _curaddr);
  writeuint(_fpost, _curshift);
  writeuint(_fpost, _curbegin);

  writebyte(_fpost, _curpage0); //������ ������
  writebyte(_fpost, _curpage1);
  writebyte(_fpost, _curpage2);
  writebyte(_fpost, _curpage3);

  //writepointer(_fpost, _labelN);
  //writeuint(_fpost, plabelqueuestart_index);
  writeuint(_fpost, _plabel_index);
  writeuint(_fpost, _curplabel_index);
  writeuint(_fpost, _hash);
  writeuint(_fpost, _curhash);

  writeuint(_fpost, _curlnbeg);

  writebyte(_fpost, _isaddr);
}
*/
/**
PROC asmreadstate()
{
VAR BYTE i;
  _token = readfin();
  _curdir = readfin();

  _reg = readfin();
  _oldreg = readfin();
  _base = readfin();
  _base2 = readfin();

  i = readfinbyte();
  WHILE (_nvalues!=i) {
    asmpushvalue(readfinlong());
  };

  _curaddr = readfinuint();
  _curshift = readfinuint();
  _curbegin = readfinuint();

  _curpage0 = readfin(); //������ ������
  _curpage1 = readfin();
  _curpage2 = readfin();
  _curpage3 = readfin();

  //_labelN = readfinpointer();
  //plabelqueuestart_index = readfinuint();
  _curplabel_index = readfinuint();
  _plabel_index = readfinuint();
  _hash = readfinuint();
  _curhash = readfinuint();

  _curlnbeg = readfinuint();

  _isaddr = readfinbyte();
}
*/

//0x0000..0x01ff - ������ ������ �������
//������ ���� �������
//������ �������:
//2 ����� - ����� ��������� �����
//N ���� - ����� �����
//1 ���� - '\0'
//1 ���� - ����� ����� (defined, accessed, macro)
//4 ����� - �������� �����

PROC clearlabels(UINT labelblock, PBYTE labelpointer)
{
  _labelpage[labelblock] = labelpointer;
  _hash = 0x0400;
  REPEAT {
    DEC _hash;
    _labelshift[_hash] = _LABELPAGEEOF;
  }UNTIL (_hash == 0);
  _labelpagefreestart = 0;
}

PROC readlabel()
{
VAR PBYTE pevalstr; //����� � ������ ������������� TOK_ENDTEXT
VAR BYTE cstr; //������ �� ������
VAR UINT labellen;
  _token = readfin(); //TOK_TEXT
  labellen = 0;
  REPEAT {
    cstr = readfin();
    IF (+(CHAR)cstr == '#') { //evallabelname=123:labelname#evallabelname ������ ������ labelname123
      //� ������� �������� �� �����!!!
      //��������� evallabel � _evallabeltext - TODO fgets
      pevalstr = (PBYTE)_evallabeltext;
      REPEAT {
        cstr = readfin();
        POKE *(PBYTE)(pevalstr) = cstr; //����� � ������ ������������� TOK_ENDTEXT
        INC pevalstr;
      }UNTIL (cstr == +_TOKENDTEXT);
      _labellen = +(UINT)(pevalstr - _evallabeltext); //������� 0
      //��������� evallabel
      //errstr("readevallabel "); errstr(_evallabeltext); enderr();
      findlabel(_evallabeltext);
      emitn(+(UINT)getlabel());
      labellen = strjoin((PCHAR)_curlabeltext, labellen, _nbuf); //����������
      //cstr == +_TOKENDTEXT
    };
    _curlabeltext[labellen] = cstr;
    INC labellen; //TODO ��������� ������������ �����
  }UNTIL (cstr == +_TOKENDTEXT);
  _labellen = labellen; //������� +_TOKENDTEXT
  //errstr("readlabel "); errstr(_curlabeltext); enderr();
}

PROC findlabel(PBYTE labeltext)
{
//VAR PBYTE _labelN; //��������� �� ������� ������� �����
VAR PBYTE plabel; //����� � ������� ������������� ����
//VAR PBYTE pstr; //����� � ������ ������������� TOK_ENDTEXT
//VAR BYTE cstr; //������ �� ������
//VAR BYTE clabel; //������ �� ������� �����
//VAR UINT pnext_index; //����� ��������� ����� � �������
//VAR UINT plabelqueuestart_index; //� (_labelN+plabelqueuestart_index) �������� ����� ������ ������� ��� �����
  _hash = hash(labeltext)&0x3ff;

  _labelN = _labels0; //_labelpage[+(UINT)(_hashhigh&_LABELPAGEMASK)]; //set page (todo ��� ����������? ��������� ������?)
  //plabelqueuestart_index = (+(UINT)(_hash))<<1; //todo ������ ����������� UINT
  _plabel_index = _labelshift[_hash];
  //todo ��� ������� ����� �������� � ������ ����������? ����� ������� � ����� ������� �����?
  //_labelflag = 0x00; //"label not found"
  WHILE (_plabel_index != _LABELPAGEEOF) { //���� ������� ����� �� �����������
    plabel = &_labelN[_plabel_index]; //(PBYTE)(+(POINTER)_labelN + +(POINTER)_plabel_index);
    _plabel_index = *(PUINT)(plabel);
    plabel = &plabel[+sizeof(UINT)]; //(PBYTE)(+(POINTER)plabel + +(POINTER)2);
    IF (strcp((PCHAR)labeltext, (PCHAR)plabel)) { //����� �������
      //plabel = (PBYTE)(+(POINTER)plabel + +(POINTER)_labellen); //������� 0
        //errstr("found label "); errstr(labeltext); enderr();
      _plabel_index = +(UINT)(plabel - _labelN) + _labellen; //������� 0 //��������� �� ������ ������ ����������� ����� (���� ����������� ���������!)
      //_labelflag = *(PBYTE)(plabel);
      //_labelvalue = *(PLONG)(+(POINTER)plabel + +(POINTER)1);
      BREAK; //������ _plabel_index ������ ������ ��������� �����
    };
    //_plabel_index = pnext_index;
  }; //���� �� �������, �� _plabel_index==_LABELPAGEEOF
}

PROC addlabel(LONG labelvalue) //�������� ��������������� ����� findlabel!!!
{
//VAR PBYTE _labelN; //��������� �� ������� ������� �����
VAR PBYTE plabel;
//VAR PBYTE pstr; //����� � ������ ������������� TOK_ENDTEXT
//VAR BYTE cstr;
//VAR UINT plabelqueuestart_index; //� (_labelN+plabelqueuestart_index) �������� ����� ������ ������� ��� �����
//VAR UINT oldqueuestart_index; //������ ��������� �� ������ �������
VAR UINT freestart_index;
VAR BYTE labelflag;
//VAR LONG oldlabelvalue;
  _labelchanged = +FALSE;
  _labelN = _labels0; //_labelpage[+(UINT)(_hashhigh&_LABELPAGEMASK)]; //set page (todo ��� ����������? ��������� ������?)
  //IF (_labelflag!=0x00) { //����� ����
  IF (_plabel_index!=_LABELPAGEEOF) { //����� ����
    plabel = &_labelN[_plabel_index]; //(PBYTE)(+(POINTER)_labelN + +(POINTER)_plabel_index); //��������� �� ������ ������ �����
    labelflag = *(PBYTE)(plabel); //findlabel();
    IF ((labelflag&_ASMLABEL_MACRO)!=0x00) {
      errstr("redefmacro"); /**errstr((PCHAR)_curlabeltext);*/ enderr();
    }ELSE IF ((labelflag&_ASMLABEL_DEFINED)!=0x00) {
      IF (*(PLONG)&plabel[1] != labelvalue) { //���� ����� ���������� � �������� �� �������������
        _labelchanged = +TRUE;
      };
    }ELSE { //����� (�� ����������) ����������
      POKE *(PBYTE)(plabel) = labelflag|_isaddr|_ASMLABEL_DEFINED;
      POKE *(PLONG)(&plabel[1]) = labelvalue;
    };
  }ELSE {//����� ���: ����� � ������ ������� ����� ����� �������� � ������ ����� ��� �� ������� �� ������ ������ �������
    //plabelqueuestart_index = +(UINT)_hash + +(UINT)_hash; //todo ������ ����������� UINT
    //oldqueuestart_index = *(PUINT)(+(POINTER)_labelN + +(POINTER)plabelqueuestart_index); //������ ��������� �� ������ �������
    freestart_index = _labelpagefreestart; //[0] //������ ���������� �����
    _plabel_index = freestart_index; //��������� �� ������ ������ ����������� ����� (���� ����������� ���������!)
    IF (freestart_index < _LABELPAGEMAXSHIFT) { //���� ����� ��� �����
      plabel = &_labelN[_plabel_index]; //��������� �� ������ ����������� �����
      //����� �����
      POKE *(PUINT)(plabel) = _labelshift[_hash]; //oldqueuestart_index; //������ ��������� �� ������ �������
      plabel = &plabel[+sizeof(UINT)];
      strcopy((PCHAR)_curlabeltext, _labellen /**-1*/, (PCHAR)plabel); //_labellen ������� 0
      plabel = &plabel[_labellen]; //������� 0
      POKE *(PBYTE)(plabel) = _ASMLABEL_DEFINED;
      _plabel_index = +(UINT)(plabel - _labelN); //��������� �� ������ ������ ����������� �����
      INC plabel;
      POKE *(PLONG)(plabel) = labelvalue;
      plabel = &plabel[+sizeof(LONG)];
      _labelshift[_hash] = freestart_index; //����� ��������� �� ������ �������
      _labelpagefreestart = +(UINT)(plabel - _labelN); //��������� �� ����� ����������� �����
      _lenlabels = _lenlabels + _labelpagefreestart - freestart_index;
      INC _nlabels;
      //_plabel_index ������ ��������� �� ������ ������ �����!
    }ELSE {errstr("mem"); enderr();
    };
  };
}

PROC changelabel(LONG labelvalue) //���������� �� ������ ������� findlabel (������ � reequ - TODO ��������)
{ //_plabel_index ��������� �� ������ ������ �����
//VAR PBYTE _labelN; //��������� �� ������� ������� �����
VAR PBYTE plabel;
//VAR BYTE labelflag;
  _labelN = _labels0; //_labelpage[+(UINT)(_hashhigh&_LABELPAGEMASK)]; //set page (todo ��� ����������? ��������� ������?)
  plabel = &_labelN[_plabel_index];
  //labelflag = *(PBYTE)(plabel);
  POKE *(PBYTE)(plabel) = (*(PBYTE)(plabel)&~_ASMLABEL_ISADDR)|_isaddr|_ASMLABEL_DEFINED;
  POKE *(PLONG)(&plabel[1]) = labelvalue;
}

FUNC LONG getlabel() //�������� ��������������� ����� findlabel!!!
{ //_plabel_index ��������� �� ������ ������ �����
//VAR PBYTE _labelN; //��������� �� ������� ������� �����
VAR PBYTE plabel;
VAR BYTE labelflag;
VAR LONG labelvalue; //=0L; //= 0xDEADBEEFL;
  _labelN = _labels0; //_labelpage[+(UINT)(_hashhigh&_LABELPAGEMASK)]; //set page (todo ��� ����������? ��������� ������?)
  IF (_plabel_index!=_LABELPAGEEOF) { //����� ����
    plabel = &_labelN[_plabel_index];
    labelflag = *(PBYTE)(plabel);
//    IF ((labelflag&_ASMLABEL_MACRO)!=0x00) {
//      errstr("label=macro"); enderr();
//    }ELSE {
      POKE *(PBYTE)(plabel) = labelflag|_ASMLABEL_ACCESSED;
      labelvalue = *(PLONG)(&plabel[1]);
      _isaddr = labelflag&_ASMLABEL_ISADDR;
//    };
  }ELSE {//����� ���: ������ //todo post
    //asmwritestate();
    //todo �������� � ���� ���� ����� �� ����� �������

    //todo �� ������� ������� ����������, ������� ���� ����������

    errstr("nolbl "); errstr((PCHAR)_curlabeltext); enderr();
  };
  RETURN labelvalue;
}

PROC asmdir_label() //����������, ������ ����� ��� reequ
{
  readlabel();
  findlabel(_curlabeltext);
  //������ ������ ��������������! ����� ������ label=label+1
  addlabel(+(LONG)(_curaddr+_curshift)); //��������� _labelchanged, ���� ��������
  _curplabel_index = _plabel_index; //��������� ��������� ������ ����� ��� REEQU
  _curhash = _hash;
}

PROC asmfmt_reequ()
{
  _plabel_index = _curplabel_index; //���������� ����� ����������� �� ������ ����� � ���������
  _hash = _curhash;
  IF (_plabel_index!=_LABELPAGEEOF) { //����� ����
    changelabel(asmpopvalue()); /**��������� ��������� ����� � ���������/������������� �� value*/
  };
}
/**
PROC asmfmt_dir_label()
{
  //_plabel_index=_curplabel_index; //���������� ����� ����������� �� ������ ����� � ���������
  //_hash=_curhash;
  //addlabel((_curaddr+curshift)); //������ ��� findlabel ��������������� ��!!!

  IF (_labelchanged) {
    errstr("redef "); errstr(+(PCHAR)_curlabeltext); enderr();
  };
}
*/
//////////////

PROC errwrongreg()
{
  errstr("badreg"); enderr();
}

PROC errwrongpar()
{
  errstr("badpar"); enderr();
}

PROC asmerrtext()
{
  loop:
  {
    _token = readfin();
    IF ((_token == +_TOKENDERR)||_waseof) goto end; //BREAK;
    //IF (_waseof) goto end; //BREAK; //�� ������ ������
    IF (_token == +_TOKTEXT) {
      txtloop:
      {
        _token = readfin();
        IF ((_token == +_TOKENDTEXT)||_waseof) goto loop; //BREAK;
        //IF (_waseof) goto loop; //BREAK; //�� ������ ������
        err(+(CHAR)_token);
        goto txtloop;
      };
    }ELSE err(' ');
    goto loop;
  };
  end:
  err('\"'); enderr();
}

PROC asmbyte(BYTE token)
{
  IF (_asms) writefout(token);
  INC _curaddr;
}

PROC asmemitblock() //�������� ������ ����� org
{
  IF (_curaddr != _curbegin) {
    asmorgword(+(LONG)_curbegin);
    asmorgword(+(LONG)(_curaddr-_curbegin));
  };
}

PROC asmreadprefixed()
{
  _token=readfin();
  IF (+(CHAR)_token=='\\') { //todo �������� TOK_TEXT...TOK_ENDTEXT
    _token=readfin();
    IF       (+(CHAR)_token=='n') {_prefixedtoken=0x0a;
    }ELSE IF (+(CHAR)_token=='r') {_prefixedtoken=0x0d;
    }ELSE IF (+(CHAR)_token=='t') {_prefixedtoken=0x09;
    }ELSE IF (+(CHAR)_token=='0') {_prefixedtoken=0x00;
    }ELSE _prefixedtoken=_token; //��������� ���������� ������� ��������� ��� ����
  }ELSE _prefixedtoken=_token;
}

PROC asmpass(PCHAR fn)
{
  initmemmodel();
  _curbegin = _curaddr;
  _curshift = 0;

  _nvalues = 0x00;
  _ninclfiles = 0x00;

  //setfin("tok.f");
  _fin = nfopen(fn, "rb");
  IF (_fin != (PBYTE)0) {
    _waseof = +FALSE;
    _curlnbeg = 1; //todo ��� include � ����� ���������������

    asmloop();

    fclose(_fin); //closefin();
  }ELSE {
    errstr("no "); errstr(fn); enderr();
  };
  asmemitblock(); //�������� ������ ����� org
  INC _passindex;
}

FUNC UINT findlast(PCHAR s, CHAR eol)
{
VAR UINT len;
VAR UINT last;
VAR CHAR c;
  len = 0;
  last = 0; //����� ���������� �����������
  loop: //REPEAT {
    c = *(PCHAR)s;
    IF (c == '\0') goto endloop; //BREAK; //���������� �� ����������
    IF (c == eol) last = len;
    INC s;
    INC len;
  goto loop; //}UNTIL ( (c==eol) || (len>=_STRMAX) ); //���������� �� ����������
  endloop:
RETURN last; //����� ���������� �����������
}

FUNC PBYTE openext(PCHAR ext)
{
  _lenfn = findlast(_fn, '.');
  _lenfn = strjoin(_fn, _lenfn, ext);
  _fn[_lenfn] = '\0'; //strclose
RETURN openwrite(_fn);
}

PROC asmcompile(PCHAR fn)
{
  _curlabeltext = (PBYTE)_m_curlabeltext;
  _evallabeltext = (PBYTE)_m_evallabeltext;

  _fn = _m_fn;
  _lenfn = 0;

  _nlabels = 0;
  _lenlabels = 0;
  clearlabels(0, (PBYTE)_labels0);
  //clearlabels(1, (PBYTE)_labels1);
  //clearlabels(2, (PBYTE)_labels2);
  //clearlabels(3, (PBYTE)_labels3);
  //clearlabels(4, (PBYTE)_labels4);
  //clearlabels(5, (PBYTE)_labels5);
  //clearlabels(6, (PBYTE)_labels6);
  //clearlabels(7, (PBYTE)_labels7);

  _passindex = 1;

  _errs = +FALSE;
  _asms = +FALSE;
  asmpass(fn);

  _lenfn = strjoineol(_fn, 0, fn, '\0'); //����� ��� �����������!
  _fn[_lenfn] = '\0'; //strclose
  _fout = openext(".bin");
  _forg = openext(".org");
  _fpost = openext(".pst");
  _fdecl = openext(".D_");
  //_fout = openwrite( "bin.f" );
  //_forg = openwrite( "org.f" );
  //_fpost = openwrite( "post.f" );
  //_fdecl = openwrite( "decl.f" );
  //setferr("asmerr.f");
  _errs = +TRUE;
  _asms = +TRUE;
  asmpass(fn);
  errstr("lbls "); erruint(_nlabels); errstr(" buf "); erruint(_lenlabels); enderr();
  //closeferr();
  decltoken(+_TOKEOF);
  fclose(_fdecl);
  fclose(_fpost);
  fclose(_forg);
  fclose(_fout);
}

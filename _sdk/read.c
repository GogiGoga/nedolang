//// imported
#include "../_sdk/str.h"

VAR PCHAR _tword; //������� �����
VAR UINT _lentword;
VAR CHAR _s0[_STRLEN]; //������� �����
//VAR PCHAR _tstr; //������� ������
//VAR UINT _lentstr;
//VAR CHAR _ss[_STRLEN]; //������� ������

EXTERN PBYTE _fin;
EXTERN BOOL _waseof;

EXTERN UINT  _curlnbeg; //����� ������ �� ������ ������ ������

FUNC BYTE readfin FORWARD();
//FUNC UINT readfinstr FORWARD(PBYTE pstr);

////

VAR UINT  _curline; //������� ����� ������
VAR CHAR _cnext; //��������� ������
VAR CHAR _c; //������� ������ (����������, ����� �� ��������� � ��������)
VAR UINT _spcsize; //����� �������� ����� ����������� �������
VAR UINT _waseols; //������� ���� EOL � �������� ����

VAR BOOL _doskip; //���������� ������, ����� ������������ � #

PROC rdword FORWARD();
PROC rdaddword FORWARD(); //join next word

CONST BOOL _isalphanum[256]={
  +FALSE,+FALSE,+FALSE,+FALSE,+FALSE,+FALSE,+FALSE,+FALSE, +FALSE,+FALSE,+FALSE,+FALSE,+FALSE,+FALSE,+FALSE,+FALSE, //0X
  +FALSE,+FALSE,+FALSE,+FALSE,+FALSE,+FALSE,+FALSE,+FALSE, +FALSE,+FALSE,+FALSE,+FALSE,+FALSE,+FALSE,+FALSE,+FALSE, //1X
  +FALSE,+FALSE,+FALSE,+FALSE,+FALSE,+FALSE,+FALSE,+FALSE, +FALSE,+FALSE,+FALSE,+FALSE,+FALSE,+FALSE,+FALSE,+FALSE, //2X
  +TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE , +TRUE ,+TRUE ,+FALSE,+FALSE,+FALSE,+FALSE,+FALSE,+FALSE, //3X
  +FALSE,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE , +TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE , //4X
  +TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE , +TRUE ,+TRUE ,+TRUE ,+FALSE,+FALSE,+FALSE,+FALSE,+TRUE , //5X
  +FALSE,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE , +TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE , //6X
  +TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE , +TRUE ,+TRUE ,+TRUE ,+FALSE,+FALSE,+FALSE,+FALSE,+FALSE, //7X

  +TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE , +TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE , //8X..FX
  +TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE , +TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE , //8X..FX
  +TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE , +TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE , //8X..FX
  +TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE , +TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE , //8X..FX
  +TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE , +TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE , //8X..FX
  +TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE , +TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE , //8X..FX
  +TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE , +TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE , //8X..FX
  +TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE , +TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE ,+TRUE   //8X..FX
};

//���� ������ ������� ����� � ��� ������, ����� _cnext = ������ ������ ������, � _waseols!=0
//�� ������ �� �������� ���� �� �� ���������: _cnext = ������ ������ ������, � _waseols!=0

PROC skiplines() //���������� ������ �� # (�� ������ _cnext == '#')
{
 //_curline = _curline; //������ _cnext = ������ ������ ������, � _waseols!=0
  skipline:
 //���������� ������, ���� ��� �� ���������� � # (����� �� ������ _cnext == '#')
 //������ _cnext == ������ ������ ������
 //���� �� #, �� ���������� ������
  WHILE ((_cnext != '#')&&(!_waseof)) {
    INC _curline;
    INC _waseols;
    loop:
    _cnext = +(CHAR)readfin();
    IF ((+(BYTE)_cnext != 0x0a)&&(!_waseof)) {
      goto loop;
    };
    _cnext = +(CHAR)readfin();
    //������ _cnext == ������ ������ ������
  };
 //_curline = _curline; //������ _cnext == '#' ��� EOF
}

//���������� ������� � ������
PROC rdch()
//������ ��� ������ ���������� + ������ ��� ���� ������
//��������� cnext � tword
//� ������ ��� ������ ���������� + ������ ��� ���� ������ cnext
{
//  _lentword = stradd(_tword, _lentword, _cnext);
  IF (_lentword < _STRMAX) {
    _tword[_lentword] = _cnext;
    INC _lentword;
  };
  loop:
    _cnext = +(CHAR)readfin();
    IF (+(BYTE)_cnext < +(BYTE)'!') { //��������� ������
      INC _spcsize; //spaces after tword
      IF (+(BYTE)_cnext == 0x0a) {
        INC _curline;
        _spcsize = 0;
        INC _waseols;
      }ELSE IF (_cnext == '\t') {
        _spcsize = _spcsize + 7; //TODO ������� � X
      };
      //_wasdieresis = +TRUE;
      IF (!_waseof) goto loop; //todo � ����� 0a
    };
  IF (_doskip)
    IF (_waseols!=0) {
      skiplines();
      //������ _cnext == '#'
    };
}

//���������� ������
PROC rdchcmt()
//��������� cnext � tword
//� ������ ��� ������ ���������� + ������ ��� ���� ������ cnext
{
//  _lentword = stradd(_tword, _lentword, _cnext);
  IF (_lentword < _STRMAX) {
    _tword[_lentword] = _cnext;
    INC _lentword;
  };
  loop:
    _cnext = +(CHAR)readfin();
    IF (+(BYTE)_cnext == 0x0a) {
      INC _curline;
      _spcsize = 0;
      INC _waseols;
      IF (!_waseof) goto loop;
    }ELSE IF (+(BYTE)_cnext == 0x0d) {
      IF (!_waseof) goto loop;
    };
  IF (_doskip) //todo ���� �� ���?
    IF (_waseols!=0) {
      skiplines();
      //������ _cnext == '#'
    };
}

PROC rdaddword() //��������� ��������� ������� � �������
{
    IF (_doskip)
      IF (_waseols!=0) {
        skiplines();
        //������ _cnext == '#'
      };
  beg:
  _spcsize = 0; //����� �������� ����� ����������� �������
  _waseols = 0;
  _curlnbeg = _curline;
  IF (_isalphanum[+(BYTE)_cnext] ) {
    loop1: //REPEAT { //��� ������������ (EOF �� ����������)
      IF (_lentword < _STRMAX) {
        _tword[_lentword] = _cnext;
        INC _lentword;
      };
      _cnext = +(CHAR)readfin();
    IF (_isalphanum[+(BYTE)_cnext]) goto loop1; //}UNTIL (!_isalphanum[+(BYTE)_cnext]/** || _waseof*/ );
    goto loopgo;
    loop2: //REPEAT { //��� ���������� ��� EOF
      _cnext = +(CHAR)readfin();
    loopgo:
      IF (+(BYTE)_cnext < +(BYTE)'!') { //��������� ������
        INC _spcsize; //spaces after tword
        IF (+(BYTE)_cnext == 0x0a) {
          INC _curline;
          _spcsize = 0;
          INC _waseols;
        };
        IF (!_waseof) goto loop2;
      };
    //}UNTIL (_wasdieresis || _waseof );
  }ELSE { //����� �� ����������������� �������
    rdch(); //������ ��� ������ ���������� + ������ ��� ���� ������
  }; //������ ��������� ��� ������� � �����, �.�. ��� ��� ������������ cnext � ���� ���

  _tword[_lentword] = '\0'; //strclose(_tword, _lentword); //todo �������� �������� clear..close

  IF (_lentword==1) {
    _c = *(PCHAR)_tword;
    IF (+(BYTE)_c < +(BYTE)'<') { //���������
      IF (
          (
           ((_c=='/')&&(_cnext=='*'))
         ||((_c=='*')&&(_cnext=='/'))
          )
         ) { //���������� /* � */
        rdch(); //rdaddword(); //���������� ������ ������ ����� �����������, ������ ��������� ������ ����� ��������
        IF ( _cnext=='*' ) { ///** */ ���� ��� �����������
          rdchcmt(); //rdaddword(); //���������� ������ ������ ����� �����������
          WHILE ( !( ( (_cnext=='/')&&(_c=='*') ) || _waseof ) ) {
            _c = _cnext;
            rdchcmt(); //���������� ��� ������
          };
          rdch(); //���������� ��������� ������ �����������, ������ ��������� ������ ����� ��������
          _tword[_lentword] = '\0'; //strclose(_tword, _lentword); //todo �������� �������� clear..close
  //#ifdef USE_COMMENTS
  ;;        //cmt(';'); cmtstr(_tword); endcmt();
  ;;        IF (_cmts) {writebyte(_fout, +(BYTE)';'); fputs(_tword, _fout); writebyte(_fout, +(BYTE)'\n'); };
  //#endif
        };
        //������ ����� ����� ����������� ��� ��������������� /* ��� */
        _lentword = 0/**strclear(_tword)*/; //todo �������� �������� clear..close
        goto beg; //rdaddword();
      }ELSE IF ( //(_lentword==1)&&
                (
                 (_c=='/')&&(_cnext=='/')
//               ||(_c=='#')
               ||(_c==';')&&(_cnext==';')
                )
               ) { //
        _waseols = 0; //����� ;; �������� ����� ������, ��������������� �� ; (TODO �� ��������)
        rdchcmt(); //rdaddword(); //���������� ������ ������ ����� �����������, ������ ��������� ������
        WHILE (_waseols==0/** && !_waseof*/ ) {
          rdchcmt(); //���������� ��� ������
        };
        _tword[_lentword] = '\0'; //strclose(_tword, _lentword); //todo �������� �������� clear..close
  //#ifdef USE_COMMENTS
  ;;        //cmt(';'); cmtstr(_tword); endcmt();
  ;;        IF (_cmts) {writebyte(_fout, +(BYTE)';'); fputs(_tword, _fout); writebyte(_fout, +(BYTE)'\n'); };
  //#endif
        IF (+(BYTE)_cnext < +(BYTE)'!') {
          rdch(); //���������� ��������� ������ �����������, ������ ��������� ������ (TODO ������������� ��� ����)
        };
        //������ ����� ����� �����������
        _lentword = 0/**strclear(_tword)*/; //todo �������� �������� clear..close
        goto beg; //rdaddword();
      };
    }; //���������
  }; //check cmt
}

PROC rdword()
//������ ����� _tword (�� ������ � ����������� ������ ��������, �������, ���������)
//����� �������������, ����� �������� ��� ���������������� ������ (�� ������ � cnext, � ������ � ����� ����� ����)
{
  _lentword = 0/**strclear(_tword)*/; //todo �������� �������� clear..close
  rdaddword();
}

PROC rdquotes(CHAR eol) //��������� �� ������� ��������������
{
  WHILE (_spcsize != 0) { //�������� ��������� �������
    _lentword = stradd(_tword, _lentword, ' ');
    DEC _spcsize;
  };
  WHILE ( (_cnext!=eol) && !_waseof) {
    IF ( _cnext=='\\' ) {
      _lentword = stradd(_tword, _lentword, _cnext);
      _cnext = +(CHAR)readfin();
    };
    _lentword = stradd(_tword, _lentword, _cnext);
    _cnext = +(CHAR)readfin();
  };
  _tword[_lentword] = '\0'; //strclose(_tword, _lentword);
}

PROC initrd()
{
  _doskip = +FALSE;
  _tword = (PCHAR)_s0;
//  _tstr = (PCHAR)_ss;
  _waseols = 0;
  _spcsize = 0;
  _lentword = 0/**strclear(_tword)*/; //todo �������� �������� clear..close
  rdch(); //������ ��� ������ ���������� + ������ ��� ���� ������
}


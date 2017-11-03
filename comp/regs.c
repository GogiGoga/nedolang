//// imported
#ifdef TARGET_THUMB
#include "sizesarm.h"
#else
#include "sizesz80.h"
#endif

//#ifdef DOUBLES
//CONST BYTE _NRGS; //����� ���������

  //���������� ���������:
CONST BYTE _RMAIN; //������� ���������� � 1-�� ��������� ����������� �������
CONST BYTE _RMAIN2; //������� ������� ����� ���������� � 2-�� ��������� ����������� �������
CONST BYTE _RMAIN3;
CONST BYTE _RMAIN4;
//#endif

PROC emitpoprg FORWARD(BYTE rnew);
PROC emitpushrg FORWARD(BYTE rnew);
PROC emitmovrg FORWARD(BYTE rsrc, BYTE rdest); //�� ���������� � �� ����������� (��. moverg)

//#ifdef USE_COMMENTS
//;;PROC cmt FORWARD(CHAR c);
//;;PROC cmtstr FORWARD(PCHAR s);
//;;PROC cmtuint FORWARD(UINT i);
//;;PROC endcmt FORWARD();
//#endif
#ifdef USE_HINTS
;;PROC hint FORWARD(CHAR c);
;;PROC hintstr FORWARD(PCHAR s);
;;PROC hintuint FORWARD(UINT i);
;;PROC endhint FORWARD();
#endif
PROC err FORWARD(CHAR c);
PROC errstr FORWARD(PCHAR s);
PROC erruint FORWARD(UINT i);
PROC enderr FORWARD();

////
VAR BYTE _usedrg[16]/**_NRGS*/;
VAR BYTE _usedrgs;
VAR BOOL _usedr1; //todo ������
VAR BOOL _usedr2;
VAR BOOL _usedr3;
VAR BOOL _usedr4;

VAR BYTE _rnew;
VAR BYTE _rold;
VAR BYTE _rold2;
VAR BYTE _rold3;

#ifdef USE_HINTS
;;PROC hintrg(PCHAR text, BYTE data) {
;;  hintstr("//"); hintstr(text); hintuint(+(UINT)data);
;;}

;;PROC hintrgstate() {
;;  hintrg("_fastrg4used=", +(BYTE)_usedr4); //bool!
;;  hintrg("_fastrg3used=", +(BYTE)_usedr3); //bool!
;;  hintrg("_fastrg2used=", +(BYTE)_usedr2); //bool!
;;  hintrg("_fastrgused=" , +(BYTE)_usedr1); //bool!
;;  endhint();
;;  hintrg("[0]=", _usedrg[0]); //#!
;;  hintrg("[1]=", _usedrg[1]); //#!
;;  hintrg("[2]=", _usedrg[2]); //#!
;;  hintrg("[3]=", _usedrg[3]); //#!
;;  endhint();
;;  hintrg("_usedrgs=", _usedrgs); //byte!
;;  endhint();
;;}
#endif

//////////////////////////////////////////////////////////////////////////////////////////
//rg pool
//(� ��������� ����� ���� �����, �� � �����)
//���������, ����������� �� commands

PROC nouserg FORWARD(BYTE rnew);  //������ ���������� ���� ������������� �������� � ������� �������!
FUNC BYTE userg FORWARD(BYTE rnew);  //������ ������������� ���� ������������� �������� � ������� �������!
PROC popgetrg FORWARD(BYTE rnew);  //userg + ���������� � ������� _usedrg ����� + pop

//////////////////////////////////////////
//��� ��������� ����

//����������� ������ � �������� ������� rdest
PROC moverg(BYTE rsrcindex, BYTE rdest) //������ ���������� � ����������� ��������
{
VAR BYTE rsrc;
  rsrc = _usedrg[rsrcindex];
  emitmovrg(rsrc,rdest); //todo ��� ���������� rsrc!=rdest ���?
  _usedrg[rsrcindex] = rdest; //��� ���� rsrc
  nouserg( rsrc ); //������ ���������� ���� ������������� �������� � ������� �������!
  userg( rdest ); //������ ������������� ���� ������������� �������� � ������� �������!
}

PROC pushtailrg()
{
VAR BYTE rnew;
VAR BYTE i;
#ifdef USE_HINTS
;;  hintstr( "//pusholdestrg" ); endhint();
#endif
;;  IF (_usedrgs<0x01) { errstr( "pusholdestrg _usedrgs=" ); erruint(+(UINT)_usedrgs); enderr();
;;  }ELSE
  {
    rnew = _usedrg[ 0 ];
    emitpushrg( rnew );
    i = 0x01;
    WHILE (i < _usedrgs/**_NRGS*/) { //������� ��� �� �������
      _usedrg[i-0x01] = _usedrg[i];
      INC i;
    };
    nouserg( rnew ); //������ ���������� ���� ������������� �������� � ������� �������!
  };
#ifdef USE_HINTS
;;  hintrgstate();
#endif
}

FUNC BYTE findrfree() //����� ��������� (��� ����� ����� �������) ������� � ������� ��� ���
{
VAR BYTE result;
  IF (_usedrgs==_NRGS) pushtailrg();
  IF       (!_usedr1) {result = 0x01;
  }ELSE IF (!_usedr2) {result = 0x02;
  }ELSE IF (!_usedr3) {result = 0x03;
  }ELSE result = 0x04;
  RETURN result;
}

PROC nouserg(BYTE rnew) //������ ���������� ���� ������������� �������� � ������� �������!
{
;;  IF (_usedrgs==0x00) { errstr( "nouserg _usedrgs=" ); erruint(+(UINT)_usedrgs); enderr();
;;  }ELSE
  {
    IF       (rnew==0x01) {_usedr1 = +FALSE;
    }ELSE IF (rnew==0x02) {_usedr2 = +FALSE;
    }ELSE IF (rnew==0x03) {_usedr3 = +FALSE;
    }ELSE _usedr4 = +FALSE;
    DEC _usedrgs;
  };
}

FUNC BYTE userg(BYTE rnew) //������ ������������� ���� ������������� �������� � ������� �������!
{
;;  IF (_usedrgs>=_NRGS) { errstr( "userg _usedrgs=" ); erruint(+(UINT)_usedrgs); enderr();
;;  }ELSE
  {
    IF       (rnew==0x01) {_usedr1 = +TRUE;
    }ELSE IF (rnew==0x02) {_usedr2 = +TRUE;
    }ELSE IF (rnew==0x03) {_usedr3 = +TRUE;
    }ELSE _usedr4 = +FALSE;
    INC _usedrgs;
  };
  RETURN rnew;
}

FUNC BYTE getrg(BYTE rnew) //userg + ���������� � ������� _usedrg
{
#ifdef USE_HINTS
;;  hintrg("getrg ",rnew); endhint();
;;  hintrgstate();
#endif
  _usedrg[_usedrgs] = rnew;
  userg( rnew ); //������ ������������� ���� ������������� �������� � ������� �������!
#ifdef USE_HINTS
;;  hintrgstate();
#endif
  RETURN rnew;
}

PROC popgetrg(BYTE rnew) //userg + ���������� � ������� _usedrg ����� + pop
{
VAR BYTE i;
#ifdef USE_HINTS
;;  hintstr( "//getoldestrg" ); endhint();
#endif
  i = _usedrgs; //������� ��� �� ������� //_NRGS-0x01;
  WHILE (i>0x00) {
    _usedrg[i] =_usedrg[i-0x01];
    DEC i;
  };
  _usedrg[ 0 ] =rnew;
  userg( rnew ); //������ ������������� ���� ������������� �������� � ������� �������!
#ifdef USE_HINTS
;;  hintrgstate();
#endif
  emitpoprg(rnew);
}

//�������� ������� ��� �������� �� ������� "�����" (����� ���������)
PROC swaptop()
{
VAR BYTE rtmp;
  IF ( _usedrgs==0x00 ) popgetrg(0x02);
  IF ( _usedrgs==0x01 ) popgetrg( findrfree() );
  rtmp = _usedrg[_usedrgs-0x02]; //old
  _usedrg[_usedrgs-0x02] = _usedrg[_usedrgs-0x01]; //new => old
  _usedrg[_usedrgs-0x01] = rtmp; //=> new
}

////////////////////////////////////////////////////////////////////////////////
//���������� �� �����������, �������� �� commands
//��� ��������� ����
//�� ����� � ...used (����� initrgs)

//rg pool

PROC rgs_initrgs()
{
  _usedr1 = +FALSE;
  _usedr2 = +FALSE;
  _usedr3 = +FALSE;
  _usedr4 = +FALSE;
  _usedrgs = 0x00;
}

PROC getrnew() //��������� ����� new=FASTRG
//����� ������ ������ getnothing (��-�� bytetoint, inttobyte)
{
#ifdef USE_HINTS
;;  hintstr( "//getrnew" ); endhint();
#endif
  IF ( _usedrgs==0x00 ) popgetrg(0x01);
  _rnew = _usedrg[_usedrgs-0x01];
}

PROC getrold() //��������� ����� old=FASTRG (������� �������� �� getrnew ��� +,-)
{
#ifdef USE_HINTS
;;  hintstr( "//getrold" ); endhint();
#endif
  IF ( _usedrgs==0x00 ) popgetrg(0x02);
  IF ( _usedrgs==0x01 ) popgetrg( findrfree() );
  _rold = _usedrg[_usedrgs-0x02];
}

PROC getrold2() //��������� ����� old2=FASTRG (������� �������� �� getrnew ��� +,-)
{
#ifdef USE_HINTS
;;  hintstr( "//getrold2" ); endhint();
#endif
  IF ( _usedrgs==0x00 ) popgetrg(0x03);
  IF ( _usedrgs==0x01 ) { //�������� ����� old=_RFAST2
    IF ( _usedrg[0]!=0x02 ) {popgetrg(0x02);
    }ELSE popgetrg( findrfree() );
  };
  IF ( _usedrgs==0x02 ) popgetrg( findrfree() );
  _rold = _usedrg[_usedrgs-0x03];
}

PROC getrold3()
{
#ifdef USE_HINTS
;;  hintstr( "//getrold3" ); endhint();
#endif
  IF ( _usedrgs==0x00 ) popgetrg(0x03);
  IF ( _usedrgs==0x01 ) popgetrg( findrfree() );
  IF ( _usedrgs==0x02 ) popgetrg( findrfree() );
  IF ( _usedrgs==0x03 ) popgetrg( findrfree() );
  _rold = _usedrg[_usedrgs-0x04];
}

PROC freernew() //���������� ������� � ������� �����
{
#ifdef USE_HINTS
;;  hintstr( "//freernew" ); endhint();
;;  hintrgstate();
#endif
  getrnew(); //� �� ����� ��� � ���������
  //������� usedrgs ����� �� �������, ������ �������� ������ ������� (�������)
  nouserg( _usedrg[_usedrgs-0x01] ); //������ ���������� ���� ������������� �������� � ������� �������!
#ifdef USE_HINTS
;;  hintrgstate();
;;  hintstr( "//end freernew" ); endhint();
#endif
}

//
//��������� ��������� �� ���������� ���� � ...used
//
PROC getnothing() //��������� �������� � ����� � ����� � ����������
{
  WHILE (_usedrgs>0x00) pushtailrg();
}

PROC getrfree() //������ ��������� ������� � ������� ��� ���
//����� ������ ������ getnothing (��-�� bytetoint, inttobyte)
{
#ifdef USE_HINTS
;;  hintstr( "//getrfree" ); endhint();
#endif
  _rnew = getrg(findrfree());
}

PROC getmainrg() //����� RMAIN=new � ���������� ��������� �������� (todo ���� �� � ���������, �� �������� RMAIN � oldestrg)
{
#ifdef USE_HINTS
;;  hintstr( "//getmainrg" ); endhint();
#endif
  IF ( _usedrgs==0x00 ) popgetrg( _RMAIN ); //emitpopoldestrg(RMAIN);
  WHILE ( _usedrgs>0x01 ) pushtailrg();
  //������ ����� ����� 1 �������
  moverg( (0x00), _RMAIN ); //todo �������������� ����� ex de,hl
#ifdef USE_HINTS
;;  hintstr( "//end getmainrg" ); endhint();
#endif
}

PROC getmain2rgs() //����� RMAIN=old, RMAIN2=new � ���������� ��������� �������� (todo ���� �� � ���������, �� �������� RMAIN, RMAIN2 � oldestrg)
{
#ifdef USE_HINTS
;;  hintstr( "//getmain2rgs" ); endhint();
#endif
  IF ( _usedrgs==0x00 ) popgetrg( _RMAIN2 );
  IF ( _usedrgs==0x01 ) {
    moverg( (0x00), _RMAIN2 ); //todo �������������� ����� ex de,hl
    popgetrg( _RMAIN );
  };
  WHILE ( _usedrgs>0x02 ) pushtailrg();
  //������ ������ ����� 2 ��������
  //RMAIN, XXX: moverg((1),RMAIN2);
  IF (_usedrg[0]==_RMAIN) {moverg( (0x01), _RMAIN2 );
  }ELSE IF (_usedrg[1]==_RMAIN ) {
    //RMAIN2, RMAIN: pusholdestrg;moverg((0),RMAIN2);popoldestrg(RMAIN);
    IF (_usedrg[0]==_RMAIN2) {
      pushtailrg();
      moverg( (0x00), _RMAIN2 ); //todo �������������� ����� ex de,hl
      popgetrg( _RMAIN );
    }ELSE { //XXX, RMAIN: moverg((1),RMAIN2);moverg((0),RMAIN);
     moverg( (0x01), _RMAIN2 ); //todo �������������� ����� ex de,hl //moverg((0),RMAIN) ��. ����
    };
  };
  //XXX, YYY: moverg((0),RMAIN);moverg((1),RMAIN2);
  moverg( (0x00), _RMAIN ); //todo �������������� ����� ex de,hl
  moverg( (0x01), _RMAIN2 ); //todo �������������� ����� ex de,hl
#ifdef USE_HINTS
;;  hintstr( "//end getmain2rgs" ); endhint();
#endif
}
/**
PROC getmain3rgs()
{
#ifdef USE_HINTS
;;  hintstr( "//getmain3rgs" ); endhint();
#endif
  getmain2rgs();
  getrold2();
#ifdef USE_HINTS
;;  hintstr( "//end getmain3rgs" ); endhint();
#endif
}
*/
PROC getmain4rgs()
{
#ifdef USE_HINTS
;;  hintstr( "//getmain4rgs" ); endhint();
#endif
  IF ( (_usedrgs!=0x04)
    || (_usedrg[0]!=_RMAIN)
    || (_usedrg[1]!=_RMAIN2)
    || (_usedrg[2]!=_RMAIN3)
    || (_usedrg[3]!=_RMAIN4)
     ) {
    getnothing(); //todo �� ������, � ��������� �� ������������
    popgetrg(_RMAIN4);
    popgetrg(_RMAIN3);
    popgetrg(_RMAIN2);
    popgetrg(_RMAIN);
  };
#ifdef USE_HINTS
;;  hintstr( "//end getmain4rgs" ); endhint();
#endif
}

PROC setmainrg() //���������� ��������, ��� ����� �� �������� ��������� � ��������
{ //������ ��� �������� ��������� ����������
  getrg( _RMAIN );
}

PROC setmain2rgs() //���������� ��������, ��� ����� �� �������� ��������� � ���������
{ //������ ��� �������� ��������� ����������
  getrg( _RMAIN );
  getrg( _RMAIN2 );
}


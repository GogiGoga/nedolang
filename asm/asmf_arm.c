//// ���������������� ��������� � ����������

BYTE _veryoldreg;
BYTE _baseN;
UINT _reglist;

CONST UINT _BIAS = 0;

PROC asmbytepopvalue()
{
  DEC _nvalues;
  IF (_asms) {
    ;;writefout((BYTE)_value[_nvalues]/**+(BYTE)asmpopvalue()*/); //compatible version
    /*writefout(*(PBYTE)&_value[_nvalues]);*/ //fast version (little endian)
  };
  INC _curaddr;
}

PROC asmwordpopvalue() //������ ����������� UINT/DW
{
VAR UINT uinttempvalue;
  DEC _nvalues;
  IF (_asms) {
    //����� ������ ��������� ��� ��������� _isaddr!
    IF (_isaddr!=0x00) {
      uinttempvalue = _curaddr+_curshift-_curbegin; //_curaddr+_curshift-_BASEADDR;
      fwrite((PBYTE)&uinttempvalue, 4, 1, _forg);
    }; //
    ;;uinttempvalue = +(UINT)_value[_nvalues]; //+(UINT)asmpopvalue()
    ;;writefout(+(BYTE)uinttempvalue); //compatible version (�� _SIZEOF_UINT, �.�. ��� ���� ������ ��� �������!)
    ;;writefout(+(BYTE)(uinttempvalue>>8)); //compatible version (�� _SIZEOF_UINT, �.�. ��� ���� ������ ��� �������!)
    ;;writefout(+(BYTE)(uinttempvalue>>16)); //compatible version (�� _SIZEOF_UINT, �.�. ��� ���� ������ ��� �������!)
    ;;writefout(+(BYTE)(uinttempvalue>>24)); //compatible version (�� _SIZEOF_UINT, �.�. ��� ���� ������ ��� �������!)
    /*fwrite((PBYTE)&_value[_nvalues], 4, 1, _fout);*/ //fast version (little endian) (�� _SIZEOF_UINT, �.�. ��� ���� ������ ��� �������!)
  };
  _curaddr = _curaddr + 4; //�� _SIZEOF_UINT, �.�. ��� ���� ������ ��� �������!
}

PROC asmlong(LONG value) //������ ����������� LONG/DL
{
  asmbyte(+(BYTE)value); //compatible version
  asmbyte(+(BYTE)(+(UINT)value>>8)); //compatible version
  asmbyte(+(BYTE)(value>>16L)); //compatible version
  asmbyte(+(BYTE)(value>>24L)); //compatible version
  asmbyte(+(BYTE)(value>>32L)); //compatible version
  asmbyte(+(BYTE)(value>>40L)); //compatible version
  asmbyte(+(BYTE)(value>>48L)); //compatible version
  asmbyte(+(BYTE)(value>>56L)); //compatible version
}

PROC asmorgword(LONG addr) //������ ����������� POINTER
{
//todo ORG
}

PROC asmdispbshort(INT ivalue) //��� �������� ��������� (-100..+100?) //ivalue = �������� ������
//Bcc addr
//%1101 cccc LLLL LLLL
//L ����������� �� 2 � ������������ � PC+4!!! (��� ���������� �� 4)
{
  IF (+(UINT)(ivalue + +(INT)0x0100) >= 0x0200) { //todo test
    errstr("far bcc +d"); enderr();
  };
  asmbyte((BYTE)(ivalue>>1)); //����������� ��������, ����� ������ ������!
  asmbyte(_base); //����������� ��������, ����� ������ ������!
}

PROC asmdispb(INT ivalue) //��� �������� ��������� (-800..+800) //ivalue = �������� ������
//B addr
//%1110 0sLL LLLL LLLL (S � ����������� �� 2)
//L ����������� �� 2 � ������������ � PC+4!!! (��� ���������� �� 4)
{
VAR UINT uvalue;
  IF (+(UINT)(ivalue + +(INT)0x0800) >= 0x1000) {
    errstr("far b +d"); enderr();
  };
  uvalue = (UINT)ivalue>>1;
  asmbyte((BYTE)uvalue); //����������� ��������, ����� ������ ������!
  asmbyte(((BYTE)((UINT)uvalue>>8)&0x07) + _base); //����������� ��������, ����� ������ ������!
}

PROC asmdispbl(LONG ivalue) //��� ������� ��������� (-400000..+400000) //ivalue = �������� ������
//%1111 0sHH HHHH HHHH
//%1111 1LLL LLLL LLLL
//HL ����������� �� 2 � ������������ � PC ����� ����� �������, �.�. PC+4 (��� ���������� �� 4)
{
  IF (+(UINT)(ivalue + +(INT)0x400000) >= 0x800000) {
    errstr("far bl +d"); enderr();
  };
  asmbyte((BYTE)(ivalue>>12L));
  asmbyte(((BYTE)(ivalue>>20L)&0x07) + 0xf0);
  asmbyte((BYTE)(ivalue>>1L));
  asmbyte(((BYTE)(ivalue>>9L)&0x07) + 0xf8);
}

PROC asmdispconst(UINT uvalue) //��� ������ �� ��������� (0..+400) //uvalue = �������� ������
{
  IF (uvalue >= 0x0400) {
    errstr("far const +d"); enderr();
  };
  asmbyte((BYTE)(uvalue>>2)); //����������� ��������, ����� ������ ������!
}

PROC asmdispconstpopvalue()
{
  asmdispconst(+(INT)asmpopvalue());
}

PROC err_onlylowregs()
{
  IF ((_reg>=0x08) || (_oldreg>=0x08)) errwrongreg();
}

PROC err_onlylowregs_forbidden()
{
  IF ((_reg<0x08) && (_oldreg<0x08)) errwrongreg();
}

PROC initmemmodel()
{
  //_curaddr = _BASEADDR;
}


// ����� ������� ������ ������� �� UDP

//---------------------------------------------------------------------------
#include <vcl.h>
#include <stdio.h>
#include <NMUDP.hpp>

//---------------------------------------------------------------------------
extern unsigned char BufEth[2000];
extern int kolEth;
int proshUDP;

//---------------------------------------------------------------------------
class TMyUDP:public TObject
{
__published:
        TNMUDP *NMUDP1;
        TNMUDP *NMUDP2;
        void __fastcall NMUDP1DataSend(TObject *Sender);
        void __fastcall NMUDP2DataReceived(TComponent *Sender,
          int NumberBytes, AnsiString FromIP, int Port);
public:		// User declarations
    __fastcall TMyUDP();
};

//---------------------------------------------------------------------------
//�����������
__fastcall TMyUDP::TMyUDP()
{
    NMUDP1=new TNMUDP(NULL);
    NMUDP2=new TNMUDP(NULL);
    NMUDP1->OnDataSend=NMUDP1DataSend;
    NMUDP2->OnDataReceived=NMUDP2DataReceived;
}

//---------------------------------------------------------------------------
TMyUDP *MyUDP;
//---------------------------------------------------------------------------
//������� ������
void ObUDP()
{
  proshUDP=1;
  MyUDP=new TMyUDP();     // ������� ������
}

//---------------------------------------------------------------------------
//�������� ������ (UDP �����)
void PeredUDP(AnsiString adr,int nport)
//   adr   - ����� ���� Ethernet ��� ��
//   nport - ����� �����
{
  MyUDP->NMUDP1->RemoteHost = adr;
  MyUDP->NMUDP1->ReportLevel = Status_Basic;
  MyUDP->NMUDP1->RemotePort = nport;
  MyUDP->NMUDP1->SendBuffer(BufEth,kolEth,kolEth);   // �������� �����
}

//---------------------------------------------------------------------------
// ������ �������� (UDP �����)
void __fastcall TMyUDP::NMUDP2DataReceived(TComponent *Sender,
      int NumberBytes, AnsiString FromIP, int Port)
{
  if (NumberBytes<2000)
    {
    int j;
    NMUDP2->ReadBuffer(BufEth, kolEth, j); //��������� �����
    proshUDP=0;
    }
  else
    proshUDP=1;
}

//---------------------------------------------------------------------------
// ������ �������� (UDP �����)
void __fastcall TMyUDP::NMUDP1DataSend(TObject *Sender)
{
    proshUDP=0;
}

//---------------------------------------------------------------------------
//������� ������
void DelUDP()
{
   delete MyUDP;
}
//---------------------------------------------------------------------------


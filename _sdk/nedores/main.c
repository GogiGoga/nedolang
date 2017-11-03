#include <stdio.h>
#include <stdlib.h>
#include <mem.h>
#include <string.h>

#define _STRMAX 79

#define BYTE unsigned char
#define MAXDEFB 8

BYTE filebuf[65536];
char labelbuf[_STRMAX+1];
char formatlabelbuf[_STRMAX+1];
BYTE sizeword[4];
BYTE pic[1024][1024];
BYTE pixrow[1024/8][1024+1024];
#define PIXROWSHIFT 1024
BYTE maskrow[1024/8][1024];
//BYTE pixrowshift[1024/8][1024]; //>>4
BYTE attrrow[1024/8];

BYTE ink;
BYTE paper;
BYTE curink;
BYTE curpaper;
BYTE defaultcolor;

char sprformat;

int hgt;
int wid;
int bpp;

int readnum(FILE * fin)
{
char c;
int num;
  num = 0;
  do{
    if (!fread(&c,1,1,fin)) break;
    if ((c<'0')||(c>'9')) break; //� ��� ����� 0x0a
    num = num*10 + (int)(c-'0');
  }while(1);
return num;
}

unsigned int readlabel(FILE * fin, char * s)
{
char c;
unsigned int i;
int iscomment;
  do{
    i = 0;
    iscomment = 0;
    do{
      if (!fread(&c,1,1,fin)) break;
      if (c == ';') iscomment = -1;
      if ((c=='=')||(c==',')) {
        if (iscomment) continue; //� ��������� ����� =
        break;
      };
      if (c == 0x0d) continue;
      if (c == 0x0a) {
        if (iscomment) break; //�������� ��������� �� ����� ������
        continue;
      };
      if (i==_STRMAX) break;
      s[i] = c;
      i++;
    }while(1);
    s[i] = '\0';
  }while(iscomment);
return i;
}

int read4b(FILE * fin)
{
  fread(sizeword, 4, 1, fin);
return sizeword[0] + (sizeword[1]<<8) + (sizeword[2]<<16) + (sizeword[3]<<24);
}

BYTE colstat[256];

void findinkpaper(int x, int y) //����� 0x00 �� ��������� ������
{
//BYTE b;
int i;
int j;
BYTE col1;
BYTE col1stat;
BYTE col2;
BYTE col2stat;
BYTE col3;
BYTE col3stat;
  i = 0; do {colstat[i] = 0x00; i++;}while (i!=256);

  //col1 = pic[x][y];
  j = y;
  while (j < (y+8)) {
//    b = 0x00;
    i = x;
    while (i < (x+8)) {
      colstat[pic[i][j]]++;
      //b = pic[i][j];
      //if (b!=col1) col2 = b;
      i++;
    };
    j++;
  };

//����� 0x00 �� ��������� ������, ������� � 1
  col1 = 0x00; col1stat = 0;
  col2 = 0x00; col2stat = 0;
  col3 = 0x00; col3stat = 0;
  i = 1; do {
    if (colstat[i] > col1stat) {
      col3 = col2;
      col3stat = col2stat;
      col2 = col1;
      col2stat = col1stat;
      col1 = (BYTE)i;
      col1stat = colstat[i];
    }else if (colstat[i] > col2stat) {
      col3 = col2;
      col3stat = col2stat;
      col2 = (BYTE)i;
      col2stat = colstat[i];
    }else if (colstat[i] > col3stat) {
      col3 = (BYTE)i;
      col3stat = colstat[i];
    };
    i++;
  }while (i!=256);

//����� 0x00 �� ��������� ������, ������� 0x08 ����������� � 0x00 (����� �� ������ �� bright)
//  if ((col1&0x07)==0x00) col1 = 0x00;
//  if ((col2&0x07)==0x00) col2 = 0x00;
  if (((col2&0x07)==(col1&0x07)) /**&& (col2!=0x00)*/) col2 = col3; //same colour with another bright
  ink = col1;
  paper = col2; //����
}

void setcurinkpaper(BYTE* pcurink, BYTE* pcurpaper) //paper ���� � ����� ���� 0x00 (��� ������� �����, ���� ����� ���������)
{
BYTE t;
  if (sprformat == 's') {
    paper = 0x08;
    ink = 0x0f;
  }; //��� �������� ��� ������ (� ����� 0x00)
  if (((ink&0x07)==(*pcurink&0x07))||((paper&0x07)==(*pcurpaper&0x07))) { //ink ��� paper ������������� �����������
    *pcurink = ink;
    *pcurpaper = paper;
  }else if (((paper&0x07)==(*pcurink&0x07))||((ink&0x07)==(*pcurpaper&0x07))) { //ink ��� paper ������������� ����������� � ���������
    *pcurink = paper;
    *pcurpaper = ink;
  }else { //��� ����� �� ������������� ����������� ����������
    if (ink > paper) {
      *pcurink = ink;
      *pcurpaper = paper;
    }else {
      *pcurink = paper;
      *pcurpaper = ink;
    };
  };
  //������ ������� ����������
  if (*pcurink == 0x00) *pcurink = defaultcolor;
  if (*pcurpaper == 0x00) *pcurpaper = defaultcolor;
  //�� ������/����� ���� ink ������ ���� ���� paper'� (��������� ������, �.�. ���������, ���� ������, ���������� ����� ����������� � ��������� ������������� ����� � ������������ ��������� ink-paper)
  //�� ���� ������ ink=0, � 1 ������ � ������ paper=0
  if (((*pcurink&0x07)<=0x01) && ((*pcurpaper&0x07)>(*pcurink&0x07))) { //������/����� ������ ���������� (��������� ������, �.�. ��������� ������ ����� ����������� � ��������� ������������� ����� � ������������ ��������� ink-paper)
    t = *pcurpaper;
    *pcurpaper = *pcurink;
    *pcurink = t;
  };
//    paper = 0x08;
//    ink = 0x0f;
}

void emitdb(BYTE b, FILE * fout)
{
  fputs("\tdb ", fout);
  fprintf(fout, "0x%x%x", b>>4, b&0x0f);
  fputs("\n", fout);
}

void emitnops(BYTE count, FILE * fout)
{
  fputs("\tds ", fout);
  fprintf(fout, "0x%x%x", count>>4, count&0x0f);
  fputs("\n", fout);
}

void emitspr(int xchr, int y, int sprwid8, int sprhgt, FILE * fout)
{
BYTE b;
int i;
int j;
  j = y;
  while (1) {
    fputs("\tdb ", fout);
    i = xchr;
    while (1) {
      b = maskrow[i][j];
      fprintf(fout, "0x%x%x", b>>4, b&0x0f);
      fputs(",", fout);
      b = b^pixrow[i][j];
      fprintf(fout, "0x%x%x", b>>4, b&0x0f);
      i++;
      if (i >= (xchr+sprwid8)) break;
      fputs(",", fout);
    };
    fputs("\n", fout);
    j++;
    if (j >= (y+sprhgt)) break;
  };
}

void emitchr_(int xchr, int y, FILE * fout)
{
BYTE b;
int j;
  fputs("\tdb ", fout);
  j = y;
  while (1) {
    b = pixrow[xchr][j];
    fprintf(fout, "0x%x%x", b>>4, b&0x0f);
    j++;
    if (j >= (y+8)) break;
    fputs(",", fout);
  };
  if (sprformat < 'a') { //capital letter format => attr used
    b = attrrow[xchr]; //0x07;
    fputs(",", fout);
    fprintf(fout, "0x%x%x", b>>4, b&0x0f);
  };
  fputs("\n", fout);
}

void emitchr(int xchr, int y, FILE * fout)
{
  emitchr_(xchr, y, fout);
}

void emitchrshift(int xchr, int y, FILE * fout)
{
  emitchr_(xchr, y+PIXROWSHIFT, fout);
}

//�������� ��� ��������� >>shiftbits, ��������� � pixrow[sprx][y+PIXROWSHIFT]
void shiftrow(int sprx, int y, int sprwid, int rowhgt, int pixrowshift, BYTE shiftbits)
{
int j;
int x;
BYTE b;
BYTE b0;
BYTE shiftmask;
  shiftmask = (BYTE)(0xff>>(0x08-shiftbits)); //���� shiftbits==0x01, �� shiftmask==0x01
  j = y;
  while (j < (y+rowhgt)) {
    b = 0x00;
    x = sprx;
    while (x < (sprx+sprwid+8)) {
      b0 = pixrow[x/8][j];
      pixrow[x/8][j+pixrowshift] = (BYTE)((b<<(0x08-shiftbits)) + (b0>>shiftbits));
      b = (BYTE)(b0&shiftmask/**0x0f*/); //���� shiftbits==0x01, �� shiftmask==0x01
      x = x+8;
    };
    j++;
  };
}

//data: wid8, hgt8, chrgfx, chrgfx...
void resfile(char * finname, char * fintxtname, char * foutname)
{
FILE* fin;
FILE* fintxt;
FILE* fout;
int i;
int j;
int size;
int y;
int x;

BYTE b;
BYTE bmask;
BYTE b0;

int sprx;
int spry;
int sprwid;
int sprhgt;
int rowhgt; //8 for tiles, sprhgt for sprites

  fin = fopen(finname, "rb");
  if (fin) {
    fread(filebuf, 10, 1, fin); //skip to 10 (header size)
    size = read4b(fin); //10 (header size)
    fread(filebuf, 4, 1, fin); //skip to 18
    wid = read4b(fin); //18
    hgt = read4b(fin); //22
    fread(filebuf, 2, 1, fin); //skip to 28
    fread(&bpp, 1, 1, fin); //28
    fread(filebuf, 1, size-29, fin); //skip to pic
    if ((wid>0)&&(wid<=1024)&&(hgt>0)&&(hgt<=1024)&&((wid&7)==0)&&((hgt&7)==0)) {
      y = hgt;
      while (y>0) {
        y--;
        x = 0;
        while (x<wid) {
          fread(&b, 1, 1, fin);
          if (bpp == 8) {
            pic[x][y] = b;
            x++;
          }else {
            pic[x][y] = (BYTE)((b&0xf0)>>4);
            x++;
            pic[x][y] = (BYTE)(b&0x0f);
            x++;
          };
        };
      };

      fintxt = fopen(fintxtname, "rb");
      if (fintxt) {
        fout = fopen(foutname, "wb");
        if (fout) {
/**          if (labelname[0]!='\0') {
            fputs(labelname, fout);
            fputs("\n", fout);
          };*/
          while (1) {
            size = readlabel(fintxt, labelbuf); //fread(filebuf, 1, MAXDEFB, fin);
            if (size == 0) break;
            readlabel(fintxt, formatlabelbuf); //format
            sprformat = *formatlabelbuf;
            sprx = readnum(fintxt);
            spry = readnum(fintxt);
            sprwid = readnum(fintxt);
            sprhgt = readnum(fintxt);
            defaultcolor = (BYTE)readnum(fintxt);

            if (sprformat == 'B') {
              fputs(labelbuf, fout);
              fputs("\n", fout);
              emitdb((BYTE)(sprwid>>3), fout);
              emitdb((BYTE)(sprhgt>>3), fout);
              rowhgt = 8;
            }else if (sprformat == 'T') {
              fputs("\tds (-$)&0xff\n", fout);
              fputs(labelbuf, fout);
              fputs("\n", fout);
              rowhgt = 8;
            }else { //'s'
              fputs(labelbuf, fout);
              fputs("\n", fout);
              emitdb((BYTE)(sprwid>>3), fout);
              emitdb((BYTE)(sprhgt), fout);
              rowhgt = sprhgt;
            };

            y = spry;
            while (y < (spry+sprhgt)) {
              //������������ ��� ��������� ������� rowhgt
              curink = 0x0f;
              curpaper = 0x08;
              x = sprx;
              while (x < (sprx+sprwid)) {
                findinkpaper(x, y);
                setcurinkpaper(&curink, &curpaper);
              //curink = 0x0f;
              //curpaper = 0x08;
                j = y;
                while (j < (y+rowhgt)) {
                  b = 0x00;
                  bmask = 0x00;
                  i = x;
                  while (i < (x+8)) {
                    b = (BYTE)(b<<1);
                    bmask = (BYTE)(bmask<<1);
                    //if (sprformat != 'B') {fprintf(fout, "0x%x%x\n", (pic[i][j])>>4, (pic[i][j])&0x0f);};
                    if (pic[i][j]==curink) b++;
                    if (pic[i][j]!=0x00) bmask++;
                    i++;
                  };
                  pixrow[x/8][j] = b;
                  maskrow[x/8][j] = bmask;
                  pixrow[(x/8)+1][j] = 0x00; //����� ��������
                  j++;
                };
//����� 0x00 �� ��������� ������, ������� 0x08 ����������� � 0x00 (����� �� ������ �� bright)
                b = 0x00; if (curink!=0x08) b = curink;
                b0 = 0x00; if (curpaper!=0x08) b0 = curpaper;
                attrrow[x/8    ] = (BYTE)( (((b|b0)&0x08)<<3)+((curpaper&0x07)<<3)+(curink&0x07) );
                attrrow[(x/8)+1] = (BYTE)( (((b|b0)&0x08)<<3)+((curpaper&0x07)<<3)+(curink&0x07) ); //����� ��������
                x = x+8;
              };
  //            shiftrow(sprx, y, sprwid, rowhgt, PIXROWSHIFT, 0x04); //�������� ��� ��������� >>4, ��������� � pixrow[sprx][y+PIXROWSHIFT]

              //������� � ���
              if (sprformat == 'B') { //tiles
                x = sprx;
                while (x < (sprx+sprwid)) {
  //                emitchrshift(x/8,y,fout);
                  emitchr(x/8,y,fout);
                  x = x+8;
                };
  //              emitchrshift(x/8,y,fout);
              }else if (sprformat == 'T') {
                x = sprx;
                while (x < (sprx+sprwid)) {
                  emitchr(x/8,y,fout);
                  x = x+8;
                };
                emitnops((BYTE)(0x100-((BYTE)(sprwid>>3)*0x09)),fout);
              }else { //sprite
                emitspr(sprx/8,y,sprwid/8,sprhgt,fout);
              };
              y = y+rowhgt;
            };

          };
          fclose(fout);
        }else {printf("can't open %s",foutname);};
        fclose(fintxt);
      }else {printf("can't open %s",fintxtname);};
    };
    fclose(fin);
  }else {printf("can't open %s",finname);};
}

int main(int argc,char* argv[])
{
//  int i;
  char *finname;
  char *fintxtname;
  char *foutname;
  finname = "testpic.bmp";
  fintxtname = "testpic.txt";
  foutname = "testpic.asm";

  if (argc<4) {
    printf(
      "NedoRes\n"
      "\tnedores.exe file.bmp file.txt file.asm\n"
    );
  }else {
    finname = argv[1];
    fintxtname = argv[2];
    foutname = argv[3];
  };

  resfile(finname, fintxtname, foutname);

  return 0;
}
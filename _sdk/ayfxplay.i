;-Minimal ayFX player2 v0.16 20.06.06--------------------------;
;                                                              ;
; ���⥩訩 ����� ��䥪⮢. �ந��뢠�� ��䥪�� �� ����� AY,  ;
; ��� ��모 �� 䮭�. �஬����� �ந��뢠��� ��䥪⮢ �������� ;
; �� �� �맮��. �ਮ��� �롮� �������: �᫨ ���� ᢮����� ;
; ������, �롨ࠥ��� ���� �� ���. �᫨ ᢮������ ������� ���,  ;
; �롨ࠥ��� �������� ����� ����騩. ��楤�� �ந��뢠���   ;
; �ᯮ���� ॣ����� AF,BC,DE,HL,IX.                          ;
;P.S. �� Alone Coder'�: ������⢮��� IY � �� 2 ����, �⮡�
;�� ����� ��䥪�, ����� �� �� ��모 (�࠭� ��� TFM ��모)
;                                                              ;
; ���樠������:                                               ;
;   ld hl, ���� ����� ��䥪⮢                                ;
;   call AFXINIT                                               ;
;                                                              ;
; ����� ��䥪�:                                              ;
;   ld a, ����� ��䥪� (0..255)                               ;
;   ld c, �⭮�⥫쭠� �஬����� (-15..15)                    ;
;   call AFXPLAY                                               ;
;                                                              ;
; � ��ࠡ��稪� ���뢠���:                                    ;
;   call AFXFRAME                                              ;
;                                                              ;
;--------------------------------------------------------------;

; ����⥫� �������, �� 5+2 ���� �� �����:
; +0 (2) ⥪�騩 ���� (����� ᢮�����, �᫨ ���訩 ���� =0x00)
; +2 (2) �६� ���砭�� ��䥪�
; +4 (1) �஬����� ��䥪�
; +6 (2) ������ ����������
; ...

afxChDesc       DS 3*(5+2)




;--------------------------------------------------------------;
; ���樠������ ����� ��䥪⮢.                               ;
; �몫�砥� �� ������, ��⠭�������� ��६����.              ;
; �室: HL = ���� ����� � ��䥪⠬�                           ;
;--------------------------------------------------------------;

afxINIT
	ld a,(hl)
	ld (afxEffectsAll),a
        INC HL
        LD (afxBnkAdr+1),HL    ;��࠭塞 ���� ⠡���� ᬥ饭��

        LD HL,afxChDesc         ;����砥� �� ������ ��� �����
        LD DE,0x00FF
        LD B,3
afxInit0
        LD (HL),D
        INC HL
        LD (HL),D
        INC HL
        LD (HL),E
        INC HL
        LD (HL),E
        INC HL
        LD (HL),D
        INC HL
       LD (HL),D
       INC HL
       LD (HL),D
       INC HL
        DJNZ afxInit0
;	ifdef TFM
;	ld a,(TURBOFMON)
;	or a
;	ret nz ;TFM present
;	ld a,0xc9 ;ret
;	ld (waitstatus),a
;	endif
        RET ;TFM absent

;--------------------------------------------------------------;
; �ந��뢠��� ⥪�饣� ����.                                 ;
; ��ࠬ��஢ �� �����.                                         ;
;--------------------------------------------------------------;

afxFRAME
        LD BC,0x03FD
        LD IX,afxChDesc
afxFrame0
        PUSH BC
        LD H,(IX+1)                    ;�ࠢ����� ���襣� ����
                                                 ;���� �� <11
        ld a,h
	or a
        jp z,afxFrame7         ;����� �� ��ࠥ�, �ய�᪠��
        LD L,(IX+0)

        LD E,(HL)          ;���� ���祭�� ���ଠ樮����� ����
        INC HL

        LD A,11
        SUB B                       ;�롨ࠥ� ॣ���� �஬����:
        LD D,B                        ;(11-3=8, 11-2=9, 11-1=10)
        LD B,0xFF                         ;��砫� �뢮�� ���祭��
;	ifndef TFM	;��� TurboSound!!! �몫�祭� ��� TFM
       OUT (C),B ;AY 0x1(0)      ;�.�.����� ���㡨���� �� HY=0xFF
       LD HY,B                                   ;�몫 �஬����
;	endif
                                ;
;	ifdef TFM
;	call waitstatus
;	endif
        OUT (C),A ;��ࠫ� ॣ���� �஬����
;	ifdef TFM
;	call waitstatus
;	endif
;	ifndef TFM	;��� TurboSound!!! �몫�祭� ��� TFM
       IN A,(C)

       CP 16
       JR NC,$+4
       SUB 3;4
       JR NC,$+3
       XOR A
       LD (afxcurvolCHN),A
;	endif
        LD A,E
        AND 0x0F
        ADD A,(IX+4)         ;�ਬ��塞 ⥪���� �஬����� ������
        CP 15
        JP M,$+5
        LD A,15
        OR A
        JP P,$+4
        XOR A
;	ifndef TFM	;��� TurboSound!!! �몫�祭� ��� TFM
      CP 13 ;�஬��� ���� ��䥪� �� ���訬
      JR NC,afxcurvON
afxcurvolCHN=$+1
       CP 0
       JR C,afxcurvOFF
afxcurvON
       LD HY,0xBF ;���
afxcurvOFF
        LD B,HY
;	else
;        LD B,0xBF
;	endif
        OUT (C),A                    ;ᮡ�⢥��� �뢮� �஬����

        BIT 5,E                         ;�㤥� ��������� ⮭�?
        JR Z,afxFrame1          ;⮭ �� ���������
        LD A,(HL)
        LD (IX+5),A
        INC HL
        LD A,(HL)
        LD (IX+6),A
        INC HL
afxFrame1
        LD A,3                          ;�롨ࠥ� ॣ����� ⮭�:
        SUB D                           ;3-3=0, 3-2=1, 3-1=2
        ADD A,A                         ;0*2=0, 1*2=2, 2*2=4

        LD B,0xFF                        ;�뢮��� ���祭�� ⮭�
;	ifdef TFM
;	call waitstatus
;	endif
        OUT (C),A
;	ifdef TFM
;	call waitstatus
;	endif
;	ifndef TFM	;��� TurboSound!!! �몫�祭� ��� TFM
        LD B,HY
;	else
;        LD B,0xBF
;	endif
       LD D,(IX+5)
        OUT (C),D
        LD B,0xFF
        INC A
;	ifdef TFM
;	call waitstatus
;	endif
        OUT (C),A
;	ifdef TFM
;	call waitstatus
;	endif
;	ifndef TFM	;��� TurboSound!!! �몫�祭� ��� TFM
        LD B,HY
;	else
;        LD B,0xBF
;	endif
       LD D,(IX+6)
        OUT (C),D
;afxFrame1
        BIT 6,E                         ;�㤥� ��������� �㬠?
        JR Z,afxFrame3          ;�� �� ���������

        LD A,(HL)                       ;�⠥� ���祭�� �㬠
        SUB 0x20
        JR C,afxFrame2          ;����� 0x20, ��ࠥ� �����
        LD H,A                          ;���� ����� ��䥪�
        LD (IX+2),0xFE           ;������ �६� �������訬-1
        LD (IX+3),0xFF
        LD E,0x90                ;�몫�砥� ⮭ � �� � �����
        JR afxFrame3

afxFrame2
        INC HL
        LD B,0xFF                        ;�뢮� ���祭�� �㬠
        LD D,6
;	ifdef TFM
;	call waitstatus
;	endif
        OUT (C),D
;	ifdef TFM
;	call waitstatus
;	endif
;	ifndef TFM	;��� TurboSound!!! �몫�祭� ��� TFM
        LD B,HY
;	else
;        LD B,0xBF
;	endif
        OUT (C),A

afxFrame3
        POP BC               ;����⠭�������� ���祭�� 横�� � B
        PUSH BC
        INC B ;4..2            ;������⢮ ᤢ���� ��� 䫠��� TN

        LD A,0b01101111          ;��᪠ ��� 䫠��� TN
afxFrame4
        RRC E                           ;ᤢ����� 䫠�� � ����
        RRCA 
        DJNZ afxFrame4
        LD D,A
       LD BC,0xFFFD                     ;�⠥� ॣ���� �����
        LD A,7
;	ifdef TFM
;	call waitstatus
;	endif
        OUT (C),A
;	ifdef TFM
;	call waitstatus
;	endif
        IN A,(C)
        XOR E                          ;������뢠�� 䫠�� ������
        AND D
        XOR E
;	ifndef TFM	;��� TurboSound!!! �몫�祭� ��� TFM
        LD B,HY
;	else
;        LD B,0xBF
;	endif
        OUT (C),A                      ;�뢮��� ���⭮ � �����
afxFrame5
        LD C,(IX+2)                 ;㢥��稢��� ����稪 �६���
        LD B,(IX+3)
        INC BC

afxFrame6
        LD (IX+2),C
        LD (IX+3),B

        LD (IX+0),L                ;��࠭塞 �������訩�� ����
        LD (IX+1),H

afxFrame7
        LD BC,5+2                 ;���室�� � ᫥���饬� ������
        ADD IX,BC
        POP BC
        DEC B
        JP NZ,afxFrame0
        RET 

;--------------------------------------------------------------;
; ����� ��䥪� �� ᢮������ ������. �� ������⢨�           ;
; ᢮������ ������� �롨ࠥ��� �������� ����� ����騩.        ;
; �室: A = ����� ��䥪� 0..255                               ;
;       � = �⭮�⥫쭠� �஬����� -15..15                    ;
;--------------------------------------------------------------;

afxPLAY
afxPLAY.A.=$+1
        ld a,0
afxPLAY.B.=$+1
        ld c,0
afxEffectsAll=$+1
	cp 0
	ret nc
	push ix
        PUSH BC                         ;��࠭塞 �஬�����
        LD DE,0                ;� DE �������襥 �६� �� ���᪥
        LD H,E
        LD L,A
        ADD HL,HL
afxBnkAdr
        LD BC,0                 ;���� ⠡���� ᬥ饭�� ��䥪⮢
        ADD HL,BC
        LD C,(HL)
        INC HL
        LD B,(HL)
        ADD HL,BC                    ;���� ��䥪� ����祭 � hl
        PUSH HL                ;��࠭塞 ���� ��䥪� �� �⥪�

        LD HL,afxChDesc         ;���� ���⮣� ������
        LD B,3
afxPlay0
        INC HL
        INC HL
        LD A,(HL)          ;�ࠢ������ �६� ������ � �������訬
        INC HL
        INC HL
        CP E
        JR C,afxPlay1
        LD C,A
        LD A,(HL)
        CP D
        JR C,afxPlay1
        LD E,C                      ;���������� �������襥 �६�
        LD D,A
        PUSH HL                 ;���������� ���� ������ +3 � IX
        POP IX
afxPlay1
        INC HL
       INC HL
       INC HL
        DJNZ afxPlay0

        POP DE                  ;����ࠥ� ���� ��䥪� �� �⥪�
        LD (IX-4),E                  ;����ᨬ � ����⥫� ������
        LD (IX-3),D
        LD (IX-2),B                     ;����塞 �६� ���砭��
        LD (IX-1),B
        POP BC                          ;����ࠥ� �஬�����
        LD (IX-0),C             ;� ����ᨬ �� � ����⥫� ������
	pop ix
	ret
 	
;	ifdef TFM
;waitstatus
;;bc=0xfffd
;        IN F,(C)
;        JP M,$-2
;	ret
;	endif 

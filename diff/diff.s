;startup
FCB1=0x4800 ;aligned ;len=0x0200*files
DOSBUF=0xff00 ;aligned
	org 0x6000
	pop hl
	push hl
	ld a,h
	cp 0x3d
	jr nz,start.
        ld hl,start.
        ld (23834),hl ;����� �������
        ld hl,467 ;����� (����� ������� � ������ ����� RUN "..." CODE ����������)
        push hl
        jp 0x3d2f ;call in DOS
start.
	ld hl,(23645) ;CH_ADD
	ld b,1;bc,256
	ld a,234 ;REM
	;cpir
	db 0xed,0xb1
	ld de,fn1.
	ld (diff.fn1),de
	call copyfn.
	ld de,fn2.
	ld (diff.fn2),de
	call copyfn.

        call diff
        ld hl,10072
        exx
        ret

copyfn.
l0.
	ld a,(hl)
	inc hl
	ld (de),a
	inc de
	cp ' '+1
	jr nc,l0. ;read until space or CR
	xor a
	dec de
	ld (de),a ;'\0'
	ret

	include "diff.asm"
	include "../_sdk/lib.i"
	include "../_sdk/print.i"
	include "../_sdk/str.i" ;for io
	include "../_sdk/io.i"
	include "../_sdk/io.asm"

fn1.
	db "nedoasm"
	db 0
        ds 50 ;for long filenames
fn2.
	db "bin.f"
	db 0
        ds 50 ;for long filenames

	include "../_sdk/io.var"
	include "diff.var"

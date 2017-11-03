ENUM {
_TOKENDTEXT, //(������ ���� ������, ����� �������� � ������������ ���������)
//#define TOK_ENDTEXT  (TOKBASE+0x0a) /*�������� ������ ������, �� ����� ���� ����� �����, �.�. ����������� � �����*/

_TOKOPENSQ,
//#define TOK_OPENSQ '['
_TOKCLOSESQ,
//#define TOK_CLOSESQ ']'

_OPWRVAL,
//#define TOK_WRITEVALUE 0x41 /*����� ���������*/
_TOKNUM,
//#define TOK_NUM      (TOKBASE+0x00) /*������� �����, ����� ���� <text>digits<endtext>*/
_ERR,
//#define TOK_ERR   (TOKBASE+0x02)
_TOKENDERR,
//#define TOK_ENDERR   (TOKBASE+0x09)
_TOKLABEL,
//#define TOK_LABEL    (TOKBASE+0x01) /*������� �����, ����� ���� <text>label.sublabel<endtext>*/

_TOKEOF,
//#define TOK_EOF 0x00 /*����� �����*/

//����������� ���������:
  //op (��������� value � ���������� 1..2 �������):
//#define ASMOPPUSHSKIP1   (ASMOPBASE+0x00)
//#define ASMOPPUSHSKIP2   (ASMOPBASE+0x01)
_OPPUSH0,
//#define ASMOPPUSH0SKIP1  (ASMOPBASE+0x02)

//�� ������� ���������� (���� ����� ����� � ���������):
//#define TOK_TAB 0x09 /*����� �����������*/
_TOKEOL,
//#define TOK_EOL 0x0a /*����� ������*/
_TOKPRIME,
//#define TOK_DBLQUOTE '\'' /*������� � �������� ���������� �������*/
_OPWRSTR,
//#define TOK_WRITESTRING 0x0c /*����� �������*/
_TOKCR, //NU

_TOKENDCOMMENT,
//#define TOK_ENDCOMMENT (TOKBASE+0x0b) /*�����, ������� �� ����������� � �������, ����� �� ������ �� text-endtext*/

  //opfmt:
_OPADD,
//#define OP_ADD     (OPBASE+0x00)
_OPSUB,
//#define OP_SUB     (OPBASE+0x01)
_OPMUL,
//#define OP_MUL     (OPBASE+0x02)
_OPDIV,
//#define OP_DIV     (OPBASE+0x03)
_OPAND,
//#define OP_AND     (OPBASE+0x04)
_OPOR,
//#define OP_OR      (OPBASE+0x05)
_OPXOR,
//#define OP_XOR     (OPBASE+0x06)
_OPSHL,
//#define OP_SHL     (OPBASE+0x07)
_OPSHR,
//#define OP_SHR     (OPBASE+0x08)
_OPEQ,
//#define OP_EQ      (OPBASE+0x09)
_OPNOTEQ,
//#define OP_NOTEQ   (OPBASE+0x0a)
_OPLESS,
//#define OP_LESS    (OPBASE+0x0b)
_OPLESSEQ,
//#define OP_LESSEQ  (OPBASE+0x0c)
_OPMORE,
//#define OP_MORE    (OPBASE+0x0d)
_OPMOREEQ,
//#define OP_MOREEQ  (OPBASE+0x0e)
_OPINV,
//#define OP_INV     (OPBASE+0x0f)
_OPPEEK,
//#define OP_PEEK    (OPBASE+0x10)

//�� ������� ���������� (���� ����� ����� � ���������):
_TOKSPC, //NU
//#define TOK_SPC ' ' /*����� �����������, ���� ����� �������� ������ ������*/
_TOKEXCL, //used as text
_TOKDBLQUOTESYM,
//#define TOK_TEXT     '\"' /*������� ������ ������*/
_TOKDIRECT, //NU?
//#define TOK_DIRECT '#' /*����� ���������������� ���������*/
_TOKDOLLAR,
//#define TOK_DOLLAR '$'
_TOKPERCENT, //NU
_TOKAND, //used as text
_TOKPRIMESYM,
//#define TOK_DBLQUOTE '\'' /*������� � �������� ���������� �������*/
_TOKOPEN,
//#define TOK_OPEN '('
_TOKCLOSE,
//#define TOK_CLOSE ')'
_TOKSTAR, //used as text
_TOKPLUS, //used as text
_TOKCOMMA,
//#define TOK_COMMA ',' /*����� �����������*/
_TOKMINUS, //used as text
_TOKDOT, //NU
_TOKSLASH, //used as text
_TOKSPC0,
_TOKSPC1,
_TOKSPC2,
_TOKSPC3,
_TOKSPC4,
_TOKSPC5,
_TOKSPC6,
_TOKSPC7,
_TOKSPC8,
//#define TOK_NOTHING '0' /*�������������� �����*/
//#define TOK_SPCBASE '0' /*����� �����������, ���� ����� �������� ������ ������*/
_TOKSPC9, //NU
_TOKCOLON,
//#define TOK_COLON ':' /*:*/ /*������ �� ������ �� ����� �����, �� ����� ���������*/
_TOKCOMMENT,
//#define TOK_COMMENT  0x0b /*(������������ � ��� ������������� �������) ����������� ��� ������, ����� ���� ����� <text>text<endtext><space><text>text<endtext>... <endcoment>*/
//#define TOK_COMMENTSYM  ';' /*����������� ��� ������, ����� ���� <comment> � ����� <text>text<endtext><space><text>text<endtext>... <endcoment>*/
_TOKLESS, //used as text
_TOKEQUAL, //used as text
//#define TOK_REEQU '='
_TOKMORE, //used as text
_TOKTEXT,
//#define TOK_TEXT     '\"' /*������� ������ ������*/

//������� ����������:
//��� ��� 12 ���������� ��������
_CMDREADSTATE,
//#define _CMDREADSTATE  (DIRBASE+0x00) /*��� post labels*/
//#define DIR_MAIN      (DIRBASE+0x00) /*main "filename" - ��������� �������?*/
_CMDLABEL,
//#define _CMDLABEL      (DIRBASE+0x01) /*����������� ����� ����� $+curdisp*/
//#define DIR_REEQU     (DIRBASE+0x02) /*����������� ����� ����� = (equ �� �����?) - ���� ������ writereequ*/
_CMDORG,
//#define _CMDORG        (DIRBASE+0x03) /*org nn - ���� ������ writeorg, ��������� ������� �� ��������� ��-�� ���������*/
_CMDALIGN,
//#define _CMDALIGN      (DIRBASE+0x04) /*align nn - ���� ������ writealign, ��������� ������� �� ��������� ��-�� ���������*/
_CMDPAGE,
//#define _CMDPAGE       (DIRBASE+0x05) /*page n - ���� ������ writepage, ��������� ������� �� ��������� ��-�� ���������*/
_CMDIF,
//#define _CMDIF         (DIRBASE+0x06) /*if nn - ���� ������ writeif, ��������� ������� �� ��������� ��-�� ���������*/
_CMDELSE,
//#define _CMDELSE       (DIRBASE+0x07) /*else*/
_CMDENDIF,
//#define _CMDENDIF      (DIRBASE+0x08) /*endif*/
_CMDDUP,
//#define _CMDDUP        (DIRBASE+0x09) /*dup nn - ���� ������ writedup, ��������� ������� �� ��������� ��-�� ���������*/
_CMDEDUP,
//#define _CMDEDUP       (DIRBASE+0x0a) /*edup*/
_CMDMACRO,
//#define _CMDMACRO      (DIRBASE+0x0b) /*macro name - ��������� �������?*/
_CMDENDM,
//#define _CMDENDM       (DIRBASE+0x0c) /*endm*/
_CMDEXPORT,//_CMDUSEMACRO,
//#define _CMDUSEMACRO   (DIRBASE+0x0d) /*usemacro name ... - ��������� �������?*/
_CMDLOCAL,
//#define _CMDLOCAL      (DIRBASE+0x0e) /*local name - ��������� �������?*/
_CMDENDL,
//#define _CMDENDL       (DIRBASE+0x0f) /*endl*/
_CMDDISP,
//#define _CMDDISP       (DIRBASE+0x10) /*disp nn - ���� ������ writedisp*/
_CMDENT,
//#define _CMDENT        (DIRBASE+0x11) /*ent*/
_CMDINCLUDE,
//#define _CMDINCLUDE    (DIRBASE+0x12) /*include "filename" - ��������� �������?*/
_CMDINCBIN,
//#define _CMDINCBIN     (DIRBASE+0x13) /*incbin "filename" - ��������� �������?*/
_CMDDB,
//#define _CMDDB         (DIRBASE+0x14) /*db ..., ������ defb - ���� ����� ������� ��������� ������ writeN, ��������� ������� �� ��������� ��-�� ���������*/
_CMDDW,
//#define _CMDDW         (DIRBASE+0x15) /*dw ..., ������ defw - ���� ����� ������� ��������� ������ writeNN*/
_CMDDL,
//#define _CMDDL         (DIRBASE+0x16) /*dl ..., ������ defl - ���� ����� ������� ��������� ������ writeNNNN*/
_CMDDS,
//#define _CMDDS         (DIRBASE+0x17) /*ds ..., ������ defs - ���� ������ writeds*/
_CMDDISPLAY,
//#define _CMDDISPLAY    (DIRBASE+0x18) /*display nn - ������� displaynum, displaystring - ��������� ������� �� ��������� ��-�� ���������*/
_CMDREPEAT,
//#define _CMDREPEAT     (DIRBASE+0x19) /*repeat*/
_CMDUNTIL,
//#define _CMDUNTIL      (DIRBASE+0x1a) /*until nn - ���� ������ writeuntil*/
_CMDSTRUCT,
//#define _CMDSTRUCT     (DIRBASE+0x1b) /*struct name - ��������� �������?*/
_CMDENDSTRUCT,
//#define _CMDENDSTRUCT  (DIRBASE+0x1c) /*endstruct*/
//max 0x1c

_TOKPIPE, //����� �� ��������!
_TOKCARON, //����� �� ��������!
_TOKTILDE, //����� �� ��������!

//// ������� ������ ������� �� �������

//��� ��������
_RG_R0,
_RG_R1,
_RG_R2,
_RG_R3,
_RG_R4,
_RG_R5,
_RG_R6,
_RG_R7,
_RG_R8,
_RG_R9,
_RG_R10,
_RG_R11,
_RG_R12,
_RG_SP,
_RG_LR,
_RG_PC,
_RG_RPBYNAME,

_ASMNOP,

_ASMADR, //???

_ASMADCS,
_ASMADDS,
_ASMSBCS,
_ASMSUBS,
_ASMRSBS,
_ASMCMN,
_ASMCMP,
_ASMTST,
_ASMNEG, //???
_ASMMULS,

_ASMANDS,
_ASMORRS,
_ASMEORS,

_ASMASRS,
_ASMLSLS,
_ASMLSRS,
_ASMRORS,

_ASMB,
_ASMBEQ,
_ASMBNE,
_ASMBCS, //HS
_ASMBCC, //LO
_ASMBMI,
_ASMBPL,
_ASMBVS,
_ASMBVC,
_ASMBHI,
_ASMBLS,
_ASMBGE,
_ASMBLT,
_ASMBGT,
_ASMBLE,
_ASMBAL, //???
_ASMBL,
_ASMBLX,
_ASMBX,

_ASMBICS,

_ASMBKPT,

_ASMDMB,
_ASMDSB,
_ASMISB,
_ASMCPSID,
_ASMCPSIE,
_ASMSEV,
_ASMSVC,
_ASMYIELD,

_ASMLDR,
_ASMLDRB,
_ASMLDRH,
_ASMLDRSB,
_ASMLDRSH,
_ASMLDM, //???
_ASMLDMFD, //???
_ASMLDMIA, //???
_ASMSTR,
_ASMSTRB,
_ASMSTRH,
_ASMSTM, //???
_ASMSTMEA, //???
_ASMSTMIA, //???

_ASMMOV,
_ASMMOVS,
_ASMMVNS,
_ASMMRS,
_ASMMSR,
_ASMCPY, //???

_ASMPOP,
_ASMPUSH,

_ASMREV,
_ASMREV16,
_ASMREVSH,
_ASMSXTB,
_ASMSXTH,
_ASMUXTB,
_ASMUXTH,

_TOKOPENBRACE,
_TOKCLOSEBRACE,

   //�������:
//comma==keepreg: �������� ����� ������ ���������/rp � ������� (����� ����������, ���� reg �������� � ����� �����, � rp � ������, �� ��� add rp,rp ���� ��� rp)

_FMTXX,
_FMTR,
_FMTR0N,
_FMTR0R0,
_FMTR0R0SAME,
_FMTR8R8,
_FMTR0R0NX4,
_FMTR0R0R0,
_FMTR0R0N31,
_FMTR0R0N7,
_FMTR0R0ZERO,
_FMTRADDR,
_FMTBSHORTADDR,
_FMTBADDR,
_FMTBLONGADDR,
_FMTPUSHPOP,

_FMTCMD,
_FMTREEQU /**������ � FMT_DIR, ������ ��� ��������������� ����� ���� � ��������� DIR_LABEL*/

};

typedef BYTE TYPE;

#define _T_BYTE  (TYPE)(0x00)
#define _T_UINT  (TYPE)(0x01)
#define _T_INT   (TYPE)(0x02)
#define _T_BOOL  (TYPE)(0x03)
#define _T_LONG  (TYPE)(0x04)
#define _T_CHAR  (TYPE)(0x05) /**������ �� ����������� ����� �����*/
//#define _T_FLOAT (TYPE)(0x06)

#define _T_POI   (TYPE)(0x10) /**������������� �� ��� �� OR (��� ����������)*/
#define _T_ARRAY (TYPE)(0x20) /**������������� �� ��� �� OR (��� ����������/��������)*/
//#define _T_FARPOI (TYPE)(0x30) /**������������� �� ��� �� OR (��� ����������)*/

#define _T_RECURSIVE (TYPE)(0x40) /**������������� �� ��� �� OR (��� �������)*/
#define _T_CONST (TYPE)(0x40) /**������������� �� ��� �� OR (��� ����������/��������)*/

#define _T_STRUCT (TYPE)(0x0f) /**������������� �� _T_POI, �� ����� _T_UNKNOWN*/
#define _T_STRUCTWORD (TYPE)(0x0e) /**������������� �� _T_POI, �� ����� _T_STRUCT - ������ ��� �������� ����� STRUCT*/
#define _T_UNKNOWN (TYPE)(0x2f) /**�� �������������*/
#define _T_PROC (TYPE)(0x3f) /**�� �������������, �� ����� _T_UNKNOWN. ��� ����������� ��� PPROC?*/

#define _T_TYPE (TYPE)(0x80) /**������������� �� ��� �� OR (��� typedef, ��� ����� ���� ��� ������, ����� _T_PROC, _T_RECURSIVE)*/
//todo �������� ����� ������ � _istype (�������� ��� ������ � ����� ���������)
//����� ����� 2 ��������� ���� (0x08, 0x80)

#define _TYPEMASK (TYPE)(0x1f) /**�������� ������� ��� � ������� POI*/


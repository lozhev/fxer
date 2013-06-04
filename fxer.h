
#include <d3dx10.h>

#ifndef FXER_STATIC
#ifdef FXER_EXPORTS
#define FXER_API __declspec(dllexport)
#else
#define FXER_API __declspec(dllimport)
#endif
#else
#define FXER_API
#endif

// ������, ������� � �������������
// ���������� ������������� ����, ����� ���� ����
// ��� ����� ��� �������� temp � ������� �����
FXER_API int fnfxdll_edit(ID3D10Effect* effect, HWND parent);

// ������, ������� � �������������
// ��� �����
// ���������� ������������� ����
FXER_API int fnfxdll_edit_withname(ID3D10Effect* effect,const TCHAR* file_name, HWND parent=0);

// ������, ������� � �������������
// ��� �����
// ����� ����������
FXER_API int fnfxdll_setvars(ID3D10Effect* effect,const TCHAR* file_name);

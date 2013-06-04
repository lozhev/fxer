
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

// эффект, готовый к использований
// дескриптор родительского окна, может быть ноль
// имя файла при сохрание temp в текушей папке
FXER_API int fnfxdll_edit(ID3D10Effect* effect, HWND parent);

// эффект, готовый к использований
// имя файла
// дескриптор родительского окна
FXER_API int fnfxdll_edit_withname(ID3D10Effect* effect,const TCHAR* file_name, HWND parent=0);

// эффект, готовый к использований
// имя файла
// метод сохранения
FXER_API int fnfxdll_setvars(ID3D10Effect* effect,const TCHAR* file_name);

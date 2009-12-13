// ���� ifdef ������ DLL���� ���������ϴ� �۾��� ���� �� �ִ� ��ũ�θ� ����� 
// ǥ�� ����Դϴ�. �� DLL�� ��� �ִ� ������ ��� �����ٿ� ���ǵ� _EXPORTS ��ȣ��
// �����ϵǸ�, ������ DLL�� ����ϴ� �ٸ� ������Ʈ������ �� ��ȣ�� ������ �� �����ϴ�.
// �̷��� �ϸ� �ҽ� ���Ͽ� �� ������ ��� �ִ� �ٸ� ��� ������Ʈ������ 
// AUTOFINDER_API �Լ��� DLL���� �������� ������ ����, �� DLL��
// �� DLL�� �ش� ��ũ�η� ���ǵ� ��ȣ�� ���������� ������ ���ϴ�.
#include "resource.h"		// main symbols
#include "../../Common/DefStruct.h"


// Export Functions
/*
extern "C" __declspec(dllexport) BOOL __stdcall GetPluginInfo(PLUGIN_INFO* pPluginInfo);
extern "C" __declspec(dllexport) BOOL __stdcall OnPluginInit(HWND hAralWnd, LPWSTR wszPluginOption);
extern "C" __declspec(dllexport) BOOL __stdcall OnPluginOption();
extern "C" __declspec(dllexport) BOOL __stdcall OnPluginClose();
*/

int MyWideCharToMultiByte(
						  UINT CodePage, 
						  DWORD dwFlags, 
						  LPCWSTR lpWideCharStr, 
						  int cchWideChar, 
						  LPSTR lpMultiByteStr, 
						  int cbMultiByte, 
						  LPCSTR lpDefaultChar, 
						  LPBOOL lpUsedDefaultChar 
);

int MyMultiByteToWideChar(
						  UINT CodePage, 
						  DWORD dwFlags, 
						  LPCSTR lpMultiByteStr, 
						  int cbMultiByte, 
						  LPWSTR lpWideCharStr, 
						  int cchWideChar 
);

// CATCodeApp
// See ATCode.cpp for the implementation of this class
//
class ITextManager;

class CAutoFinderApp : public CWinApp
{
private:
	HMODULE				m_hContainer;
	HWND				m_hContainerWnd;
	LPWSTR				m_wszOptionString;	
	ITextManager*		m_pTextMgr;

	//////////////////////////////////////////////////////////////////////////
	// GetGlyphOutlineA ��ü �Լ�
	//////////////////////////////////////////////////////////////////////////
	static DWORD __stdcall NewGetGlyphOutlineA(
		HDC hdc,             // handle to device context
		UINT uChar,          // character to query
		UINT uFormat,        // format of data to return
		LPGLYPHMETRICS lpgm, // pointer to structure for metrics
		DWORD cbBuffer,      // size of buffer for data
		LPVOID lpvBuffer,    // pointer to buffer for data
		CONST MAT2 *lpmat2   // pointer to transformation matrix structure
		);

	//////////////////////////////////////////////////////////////////////////
	// GetGlyphOutlineW ��ü �Լ�
	//////////////////////////////////////////////////////////////////////////
	static DWORD __stdcall NewGetGlyphOutlineW(
		HDC hdc,             // handle to device context
		UINT uChar,          // character to query
		UINT uFormat,        // format of data to return
		LPGLYPHMETRICS lpgm, // pointer to structure for metrics
		DWORD cbBuffer,      // size of buffer for data
		LPVOID lpvBuffer,    // pointer to buffer for data
		CONST MAT2 *lpmat2   // pointer to transformation matrix structure
		);

	//////////////////////////////////////////////////////////////////////////
	// NewTextOutA ��ü �Լ�
	//////////////////////////////////////////////////////////////////////////
	static BOOL __stdcall NewTextOutA(
		HDC hdc,           // handle to DC
		int nXStart,       // x-coordinate of starting position
		int nYStart,       // y-coordinate of starting position
		LPCSTR lpString,   // character string
		int cbString       // number of characters
		);

	//////////////////////////////////////////////////////////////////////////
	// NewTextOutW ��ü �Լ�
	//////////////////////////////////////////////////////////////////////////
	static BOOL __stdcall NewTextOutW(
		HDC hdc,           // handle to DC
		int nXStart,       // x-coordinate of starting position
		int nYStart,       // y-coordinate of starting position
		LPCWSTR lpString,   // character string
		int cbString       // number of characters
		);

	//////////////////////////////////////////////////////////////////////////
	// ExtTextOutA ��ü �Լ�
	//////////////////////////////////////////////////////////////////////////
	static BOOL __stdcall NewExtTextOutA(
		HDC hdc,			// handle to DC
		int nXStart,		// x-coordinate of reference point
		int nYStart,		// y-coordinate of reference point
		UINT fuOptions,		// text-output options
		CONST RECT* lprc,	// optional dimensions
		LPCSTR lpString,	// string
		UINT cbCount,		// number of characters in string
		CONST INT* lpDx		// array of spacing values
		);

	//////////////////////////////////////////////////////////////////////////
	// ExtTextOutW ��ü �Լ�
	//////////////////////////////////////////////////////////////////////////
	static BOOL __stdcall NewExtTextOutW(
		HDC hdc,          // handle to DC
		int nXStart,            // x-coordinate of reference point
		int nYStart,            // y-coordinate of reference point
		UINT fuOptions,   // text-output options
		CONST RECT* lprc, // optional dimensions
		LPCWSTR lpString, // string
		UINT cbCount,     // number of characters in string
		CONST INT* lpDx   // array of spacing values
		);

	//////////////////////////////////////////////////////////////////////////
	// DrawTextA ��ü �Լ�
	//////////////////////////////////////////////////////////////////////////
	static int __stdcall NewDrawTextA(
		HDC hDC,          // handle to DC
		LPCSTR lpString,  // text to draw
		int nCount,       // text length
		LPRECT lpRect,    // formatting dimensions
		UINT uFormat      // text-drawing options
		);

	//////////////////////////////////////////////////////////////////////////
	// DrawTextW ��ü �Լ�
	//////////////////////////////////////////////////////////////////////////
	static int __stdcall NewDrawTextW(
		HDC hDC,          // handle to DC
		LPCWSTR lpString, // text to draw
		int nCount,       // text length
		LPRECT lpRect,    // formatting dimensions
		UINT uFormat      // text-drawing options
		);

	//////////////////////////////////////////////////////////////////////////
	// DrawTextExA ��ü �Լ�
	//////////////////////////////////////////////////////////////////////////
	static int __stdcall NewDrawTextExA(
		HDC hDC,                     // handle to DC
		LPSTR lpString,              // text to draw
		int nCount,                 // length of text to draw
		LPRECT lpRect,                 // rectangle coordinates
		UINT uFormat,             // formatting options
		LPDRAWTEXTPARAMS lpDTParams  // more formatting options
		);

	//////////////////////////////////////////////////////////////////////////
	// DrawTextExW ��ü �Լ�
	//////////////////////////////////////////////////////////////////////////
	static int __stdcall NewDrawTextExW(
		HDC hdc,                     // handle to DC
		LPWSTR lpchText,             // text to draw
		int cchText,                 // length of text to draw
		LPRECT lprc,                 // rectangle coordinates
		UINT dwDTFormat,             // formatting options
		LPDRAWTEXTPARAMS lpDTParams  // more formatting options
		);

public:
	static CAutoFinderApp*	g_Inst;
	CONTAINER_PROC_ENTRY	m_sContainerFunc;
	CAutoFinderApp();

	// Overrides
	virtual BOOL InitInstance();
	virtual BOOL ExitInstance();

public:
	BOOL Init(HWND hAralWnd, LPWSTR wszPluginOption);
	BOOL Option();
	BOOL Close();

	BOOL ApplyOption( LPCTSTR strCachingType );
	void ResetOption();
};

#define MAINAPP (CAutoFinderApp::g_Inst)
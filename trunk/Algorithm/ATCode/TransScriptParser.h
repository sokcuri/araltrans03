
//====================================================================
// CTransScriptParser : 번역 스크립트 처리 모듈
//====================================================================

#if !defined(AFX_SCRIPT_H__B660F339_1BCC_4680_ADCA_A55BE12755D9__INCLUDED_)
#define AFX_SCRIPT_H__B660F339_1BCC_4680_ADCA_A55BE12755D9__INCLUDED_

#include <vector>
#include "../../DefStruct.h"

using namespace std;

class CVariable;
//class CTransScriptParserCallback;

// 내부 변수 클래스
class CVariable
{
public:
	CString m_strVarName;		// 변수명
	int m_nVarType;				// 변수형 (0:void 1:int 2:string)
	void* m_pValue;				// 실제 값이 있는 포인터
	int m_nBlockLevel;			// 변수가 선언된 블록 레벨

public:

	BOOL m_bIsArray;

	int GetType();
	void SetType(int nVarType);
	virtual void DeleteInstance();
	int GetBlockLevel();
	void SetBlockLevel(int level);
	CString GetName();
	void SetName(CString strName);
	virtual void* GetValue(vector<CString*>* pParams=NULL);
	virtual void SetValue(void* pValue);
	CVariable* GetVariableElemant(int idx);

	CVariable()
	{
		m_strVarName = L"";
		m_nVarType = 0;
		m_pValue = NULL;
		m_nBlockLevel = 0;		
		m_bIsArray = FALSE;
	};

	virtual ~CVariable(){
		DeleteInstance();
	};

};

class CTransScriptParser;

class CFunction : public CVariable
{
public:
	void* RemakeValue(void *pSrc, int before, int after);
	vector<CVariable*> m_aParams;
	CTransScriptParser* m_pScriptContainer;

	virtual void* GetValue(vector<CString*>* pParams=NULL);
	virtual void SetValue(void* pCodeTokens);
	CFunction();
	virtual ~CFunction();

protected:
	vector<CString*>* m_pCodeTokens;
};



class CTransScriptParser  
{
	// 내부 전처리식별자 클래스
	class CReplaceIdentifier
	{
	public:
		CString m_strAfter;
		CString m_strBefore;
		CReplaceIdentifier()
		{
			m_strAfter = L"";
			m_strBefore = L"";
		};

		virtual ~CReplaceIdentifier(){};
	};

	
public:
	vector<CVariable*> m_aVariableTable;		// 사용자가 정의한 변수 테이블
	vector<CFunction*> m_aFunctionTable;		// 사용자가 정의한 함수 테이블
	LPCTSTR m_pSourcePtr;
	vector<CReplaceIdentifier*> m_aReplaceArray;
	CString m_strLastError;
	int nBlockLevel;				// 블록 차수
	int nQuitLevel;					// 블록탈출 차수
	CString stackLoopBlock;			// 루프문장 스택
	bool m_bContinuousMode;						// 연속모드 플래그
	//CTransScriptParserCallback* m_pCallbackClass;

	CFunction* GetFunction(CString strFuncName);
	void DeleteAllFunctions(vector<CFunction*> *pArray);
	void IncludeScript(wchar_t *buffer, vector<CString*>* paTokens);
	void IncludeScriptFile(CString filename, vector<CString*>* paTokens);
	void DeleteAllIdentifiers(vector<CReplaceIdentifier*>* pArray);
	void DeleteAllVariables(vector<CVariable*>* pArray);
	void DeleteAllTokens(vector<CString*>* pArray);
	int atoi2(CString str);
	void DeleteInstance(void* ptr, int type);

	int SkipStatement(vector<CString*>* paTokens, int tindex);
	void* RemakeValue(void* pSrc, int before, int after);
	BOOL GetSubPtrArray(vector<CString*> *paTar, vector<CString*> *paSrc, int beginindex, int endindex);
	void* GetValue(vector<CString*>* paTokens, int* pnRetType);
	void* GetValue(CString strSentence, int* pnRetType);	// 직접 호출하여 사용할 수 있다 (수식같은거)
	void* InterpretBlock(vector<CString*>* paTokens, int* pRetType=NULL);
	void ShowTokens(vector<CString*>* paTokens);
	BOOL RegisterFunction(int nReturnType, CString strFuncName,  int* naParamTypeList);
	BOOL Tokenization(vector<CString*>* paTokens);
	BOOL DeleteElementsAll(vector<void*>* pArray);
	int SkipBlock(vector<CString*>* paTokens, int tindex);
	int GetDataType(CString strType);
	CString* AddToken(vector<CString*>* pTokenList, CString* pString);
	int GetReplaceIndex(CString before);
	BOOL Tokenize(CString str, vector<CString*>* tokens);
	CString GetLine();
	CVariable* GetVariable(CString strVarName);

	wchar_t* AllocateBufferFromScriptFile(CString filename);
	void ExecuteScript(LPCTSTR buffer);
	void ExecuteScriptFile(LPCTSTR filename);

	void SetRegisterValues(REGISTER_ENTRY* pRegs);

	CTransScriptParser();
	virtual ~CTransScriptParser();
};

#endif // !defined(AFX_SCRIPT_H__B660F339_1BCC_4680_ADCA_A55BE12755D9__INCLUDED_)

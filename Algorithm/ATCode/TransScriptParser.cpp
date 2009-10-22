// EffectScript.cpp: implementation of the CScript class.
//
//////////////////////////////////////////////////////////////////////

#pragma warning(disable:4996)
#pragma warning(disable:4267)

#include "stdafx.h"
#include "TransScriptParser.h"
//#include "EffectScriptCallback.h"
//#include "io.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTransScriptParser::CTransScriptParser()
{
	//m_pCallbackClass = pCallbackObj;		// 스크립트 콜백 컨테이너
	//m_pCallbackClass->m_pScriptContainer = this;
	
	m_bContinuousMode = true;

	m_pSourcePtr = NULL;					// 스크립트 파싱 포인터
	m_strLastError = L"";					// 마지막 에러 설명
	nBlockLevel = 0;						// 블록 차수
	nQuitLevel = 0;							// 블록탈출 차수
	stackLoopBlock = L"";					// 루프문장 스택

	ExecuteScript(_T("int EAX;int EBX;int ECX;int EDX;int ESI;int EDI;int EBP;int ESP;"));
	//// (0:void, 1:int, 2:string)
}


CTransScriptParser::~CTransScriptParser()
{
	DeleteAllVariables(&m_aVariableTable);		// 변수테이블을 메모리에서 해제
	DeleteAllFunctions(&m_aFunctionTable);		// 함수테이블을 메모리에서 해제
	DeleteAllIdentifiers(&m_aReplaceArray);		// #define 설정들 삭제
}

// 스크립트 데이터를 지정하여 수행한다.
void CTransScriptParser::ExecuteScript(LPCTSTR buffer)
{
	m_pSourcePtr = (wchar_t*)buffer;
	vector<CString*> m_aTokens;

	/////////////////// 토큰화 ///////////////////
	if(Tokenization(&m_aTokens)==FALSE){
		//AfxMessageBox(m_strLastError, MB_OK);
	}
	int a = (int)m_aTokens.size();
	//ShowTokens(&m_aTokens);

	/////////////////// 처리 /////////////////////
	InterpretBlock(&m_aTokens);


	/////////////// 다쓴 데이터 삭제 //////////////
	DeleteAllTokens(&m_aTokens);				// 토큰 스트링들을 메모리에서 해제

}

// 현재 소스포인터로부터 한라인을 읽어 반환
// 소스포인터를 다음 라인으로 자동 증가시킴
CString CTransScriptParser::GetLine()
{
	CString returnLine = _T("");
	while(*m_pSourcePtr){
		if(*m_pSourcePtr==0x0A){		// 리턴이 검출되었다면
			m_pSourcePtr++;
			break;							// 루프문 빠져나옴
		}
		else if(*m_pSourcePtr==0x0D){
			m_pSourcePtr++;
		}
		else{							// 리턴이 검출 안됐으면
			returnLine += *m_pSourcePtr;
			m_pSourcePtr++;
		}
	}
	return returnLine;
}

// 특정 스트링을 받아서 토큰으로 나누어준다
// tokens라는 포인터 배열에 CString들의 주소를 담는다
// 성공할 경우 TRUE반환
BOOL CTransScriptParser::Tokenize(CString str, vector<CString*> *tokens)
{
	int length = str.GetLength();
	int strindex = 0;
	CString* identifier = new CString;
	*identifier = _T("");
	while(strindex<length){
		wchar_t ch = str[strindex];

		// 스페이스, 탭, 엔터가 나왔을 경우
		if(ch==_T(' ') || ch==_T('\t') || ch==0x0A || ch==0x0D){
			if(!identifier->IsEmpty()){
				identifier = AddToken(tokens, identifier);
			}
			strindex++;
		}

		// 쌍따옴표가 나왔을때
		else if(ch==0x22){
			if(!identifier->IsEmpty()){
				identifier = AddToken(tokens, identifier);
			}
			*identifier += ch;
			strindex++;
			do{
				if(strindex>=length){
					m_strLastError = _T("문자열 상수 선언이 잘못되었습니다.");
					return FALSE;
				}
				ch = str[strindex];
				if(ch==_T('\\')){				// 특수문자라면
					strindex++;
					wchar_t tempch = str[strindex];
					switch(tempch){
						case 'n': tempch = '\n'; break;
						case 'r': tempch = '\r'; break;
						case 't': tempch = '\t'; break;
						case '\"': tempch = '\"'; break;
						case 'b': tempch = '\b'; break;
						case '0': tempch = '\0'; break;
					}
					*identifier += tempch;
				}
				else *identifier += ch;		// 일반문자라면
				strindex++;
			}while(ch!=0x22);
			identifier = AddToken(tokens, identifier);			
		}

		// 제어 특수문자가 나왔을때
		else if( ch==';' || ch=='(' || ch==')' || ch=='{' || ch=='}' || ch==',' || ch=='[' || ch==']' ){
			if(!identifier->IsEmpty()){
				identifier = AddToken(tokens, identifier);
			}
			*identifier = ch;
			identifier = AddToken(tokens, identifier);
			strindex++;
		}

		// 논리 연산자가 나왔을때
		else if(ch=='&' || ch=='|'){
			if(!identifier->IsEmpty()){
				identifier = AddToken(tokens, identifier);	
			}

			if(str[strindex+1]==ch){
				*identifier = ch;
				*identifier += ch;
				identifier = AddToken(tokens, identifier);
				strindex += 2;
			}
			else{
				m_strLastError = L"잘못된 논리연산자 입니다. &&(o) &(x)";
				return FALSE;
			}
		}

		//  단항 연산자가 나왔을때
		else if((ch=='+' || ch=='-') && ( str.GetLength()>strindex+1 && str[strindex+1]==ch )){
			if(!identifier->IsEmpty()){
				identifier = AddToken(tokens, identifier);	
			}
			*identifier = ch;
			*identifier += ch;
			identifier = AddToken(tokens, identifier);
			strindex += 2;
		}

		// 주석 문자가 나왔을때
		else if(ch=='/' && str[strindex+1]=='/'){
			if(!identifier->IsEmpty()){
				identifier = AddToken(tokens, identifier);
			}
			while(strindex<str.GetLength()){
				if(str[strindex]==0x0A){strindex++; break;}
				strindex++;
			}
		}

		// 대입기호가 나왔을때
		else if(ch=='=' || (ch=='<' || ch=='>' || ch=='!' || ch=='+' || ch=='-' || ch=='*' || ch=='/' || ch=='%')&&(str.GetLength()>strindex+1)){
			if(!identifier->IsEmpty()){
				identifier = AddToken(tokens, identifier);
			}

			*identifier = ch;
			strindex++;
			if(str[strindex]=='='){
				*identifier += '=';
				strindex ++;
			}
			identifier = AddToken(tokens, identifier);
		}

		// 숫자가 나왔을때
		else if('0'<= ch && ch<='9'){
			if(identifier->IsEmpty()){
				while(('0'<= ch && ch<='9') || ('A'<= ch && ch<='Z') || ('a'<= ch && ch<='z')){
					*identifier += ch;
					strindex++;
					if(strindex>=str.GetLength()) break;
					ch = str[strindex];					
				}
				identifier = AddToken(tokens, identifier);
			}
			else{
				*identifier += ch;
				strindex++;
			}
		}

		// 알파벳 또는 한글 문자일때
		else if(('A'<=ch && ch<='Z') || ('a'<=ch && ch<='z') || (0xAC00<=ch && ch<=0xD743) || ch=='_'){
			*identifier += ch;
			strindex++;
		}

		// 그외
		else{
			m_strLastError = ch + _T("는 알수없는 문자입니다.");
			return FALSE;
		}
	}


	if(identifier->IsEmpty()){
		delete identifier;
	}
	else{
		tokens->push_back(identifier);
	}
	return TRUE;
}

// #define문으로 정의된 식별자를 알아낸다
// 해당문자열에 대해 변환이 필요하면 m_aReplaceArray의 인덱스를 리턴
// 변환할 필요 없으면 -1을 리턴
int CTransScriptParser::GetReplaceIndex(CString before)
{
	CString temp;
	for(size_t i=0; i<m_aReplaceArray.size(); i++){
		temp = ((CReplaceIdentifier*)m_aReplaceArray[i])->m_strBefore;
		if(temp.Compare(before)==0) return i;
	}
	return -1;
}

// 포인터배열에 해당 스트링의 주소를 넣는다
// 넣고나면 새로운 CString객체를 생성하여 그 주소를 리턴한다
CString* CTransScriptParser::AddToken(vector<CString*> *pTokenList, CString *pString)
{
	int index = GetReplaceIndex(*pString);
	if(index>=0) *pString = ((CReplaceIdentifier*)m_aReplaceArray[index])->m_strAfter;
	pTokenList->push_back(pString);
	pString = new CString;
	*pString = _T("");
	return pString;
}

int CTransScriptParser::GetDataType(CString strType)
{
	if(strType.Compare(_T("void"))==0) return 0;
	else if(strType.Compare(_T("int"))==0) return 1;
	else if(strType.Compare(_T("string"))==0) return 2;
	// 만일 세가지형중 모두 일치하지 않는다면 -1 리턴
	else return -1;

}

// 괄호 {,(로 시작하는 부분부터 대응되는 },)까지를 스킵하는
// 토큰 인덱스를 가르쳐준다
// 괄호 },)로 시작하면 반대로 검색한다

int CTransScriptParser::SkipBlock(vector<CString*>* paTokens, int tindex)
{
	CString tk, opentrace, closetrace;
	opentrace = *((CString*)(*paTokens)[tindex]);
	if(opentrace.Compare(L"{")==0) closetrace = L"}";
	else if(opentrace.Compare(L"}")==0) closetrace = L"{";
	else if(opentrace.Compare(L"(")==0) closetrace = L")";
	else if(opentrace.Compare(L")")==0) closetrace = L"(";
	else if(opentrace.Compare(L"[")==0) closetrace = L"]";
	else if(opentrace.Compare(L"]")==0) closetrace = L"[";
	else return -1;

	int level = 1;
	if(opentrace.Compare(L"{")==0 || opentrace.Compare(L"(")==0 || opentrace.Compare(L"[")==0){
		int maxindex = paTokens->size();
		tindex++;
		while(tindex<maxindex){
			tk = *((CString*)(*paTokens)[tindex]);
			if(tk.Compare(opentrace)==0) level++;
			else if(tk.Compare(closetrace)==0){
				level--;
				if(level==0) return tindex;
			}
			tindex++;
		}
	}
	else{
		tindex--;
		while(tindex>=0){
			tk = *((CString*)(*paTokens)[tindex]);
			if(tk.Compare(opentrace)==0) level++;
			else if(tk.Compare(closetrace)==0){
				level--;
				if(level==0) return tindex;
			}
			tindex--;
		}
	}

	return -1;
}

// 포인터배열의 모든 원소들의 delete를 수행해주는 함수
BOOL CTransScriptParser::DeleteElementsAll(vector<void*> *pArray)
{
	int i, count;
	count = (int)pArray->size();
	for(i=0; i<count; i++){
		delete (*pArray)[i];				// 모든 원소들을 메모리에서 해제
	}
	pArray->clear();
	return TRUE;
}

// 토큰화 과정을 수행하는 함수 성공시 TRUE반환
BOOL CTransScriptParser::Tokenization(vector<CString*>* paTokens)
{
	//m_pSourcePtr = m_pSourceBuffer;
	while(*m_pSourcePtr){		// NULL문자를 만날때까지 루프

		CString line = GetLine();		// 한줄을 읽어온다
		line = line.Trim();
		if(line.IsEmpty()) continue;

		if(line[0]=='#'){			// 전처리문이라면
			//int cntToken;
			CString command = _tcstok((LPTSTR)(LPCTSTR)line, _T(" \t"));

			if(command.Compare(_T("#define"))==0){	// 그중 define명령이면
				CReplaceIdentifier* pRI = new CReplaceIdentifier;
				CString before = _tcstok(NULL, _T(" \t"));
				CString after = _tcstok(NULL, _T(" \t"));
				if(before.IsEmpty() || after.IsEmpty()){
					m_strLastError = L"#define 문법이 잘못되었습니다.";
					//MessageBox(NULL, m_strLastError, L"스크립트오류", MB_OK);
					return FALSE;
				}
				pRI->m_strBefore = before;
				pRI->m_strAfter = after;

				m_aReplaceArray.push_back(pRI);
			}
			else if(command.Compare(_T("#include"))==0){	// 그중 include명령이면
				CString filename = _tcstok(NULL, _T(" \t"));
				
				filename.Replace(_T("\""), _T(""));
				filename.Replace(_T("<"), _T(""));
				filename.Replace(_T(">"), _T(""));

				if(filename.IsEmpty()){
					m_strLastError = L"#include 문법이 잘못되었습니다.";
					//MessageBox(NULL, m_strLastError, L"스크립트오류", MB_OK);
					return FALSE;
				}
				// 인클루드 스크립트 실행
				IncludeScriptFile(filename, paTokens);
			}
			else{								// 존재하지 않는 전처리 명령일때
				m_strLastError = L"존재하지 않는 전처리 명령어입니다.";
				//MessageBox(NULL, m_strLastError, L"스크립트오류", MB_OK);
				return FALSE;
			}
		}
		else{							// 일반 문장이면
			if(!Tokenize(line, paTokens)) return FALSE;
		}
	}

	return TRUE;	
}


void CTransScriptParser::ShowTokens(vector<CString*> *paTokens)
{
	CString temp = L"";
	for(size_t j=0; j<paTokens->size(); j++){
		temp += (wchar_t)'(';
		temp += *((CString*)(*paTokens)[j]);
		temp += (wchar_t)')';
	}
	MessageBox(NULL, temp, L"해석된 토큰들", MB_OK);
}

// 주어진 토큰들을 수행
void* CTransScriptParser::InterpretBlock(vector<CString*> *paTokens, int* pRetType)
{	
	nBlockLevel++;

	int tindex = 0;
	int maxindex = paTokens->size();
	int startSP = m_aVariableTable.size();
	int startFP = m_aFunctionTable.size();
	CString token;
	void* pRetPtr = NULL;

	while(tindex<maxindex){
		token = *((CString*)(*paTokens)[tindex]);

		///////////////////////// {블럭}이 나타났을때 /////////////////////////////
		if(token.Compare(L"{")==0){
			int beginindex = tindex+1;
			tindex = SkipBlock(paTokens, tindex);
			int endindex = tindex - 1;
			vector<CString*> temptokens;
			if(GetSubPtrArray(&temptokens, paTokens, beginindex, endindex)){
				pRetPtr = InterpretBlock(&temptokens, pRetType);
			}
		}


		///////////////////////// 변수, 함수 선언문일때 /////////////////////////////
		else if(token.Compare(L"int")==0 || token.Compare(L"string")==0 || token.Compare(L"void")==0){
			// 함수 정의부라면
			if(*((*paTokens)[tindex+2]) == L"(")
			{
				CFunction* newfunc = new CFunction();
				newfunc->SetBlockLevel(nBlockLevel);
				newfunc->SetType(GetDataType(token));
				newfunc->SetName(*((*paTokens)[++tindex]));

				// 매개변수 정의
				tindex += 2;
				while(*((*paTokens)[tindex]) != L")")
				{
					CVariable* newparam = new CVariable();
					newparam->SetType(GetDataType(*(*paTokens)[tindex]));
					newparam->SetName(*((*paTokens)[++tindex]));
					newfunc->m_aParams.push_back(newparam);
					if(*((*paTokens)[++tindex]) != L",") break;
					else tindex++;
				}

				// 함수몸체 정의
				if(*((*paTokens)[++tindex]) != L"{")
				{
					//MessageBox(NULL, L"함수 몸체가 없습니다.", L"스크립트오류", MB_OK);
				}

				int beginindex = tindex+1;
				tindex = SkipBlock(paTokens, tindex);
				int endindex = tindex - 1;
				vector<CString*>* pTemptokens = new vector<CString*>();
				if(GetSubPtrArray(pTemptokens, paTokens, beginindex, endindex))
					newfunc->SetValue(pTemptokens);

				newfunc->m_pScriptContainer = this;
				m_aFunctionTable.push_back(newfunc);

			}

			// 변수선언이라면
			else
			{
				CVariable* newvar = new CVariable();
				int vartype = GetDataType(token);
				newvar->SetBlockLevel(nBlockLevel);
				newvar->SetType(vartype);
				newvar->SetName(*((*paTokens)[++tindex]));

				// 배열이라면
				if(*((*paTokens)[tindex+1]) == L"[")
				{
					// 사이즈 얻어오기
					int nArraySize = 0;
					int endindex = SkipBlock(paTokens, tindex+1);
					if(endindex<0)
					{
						//MessageBox(NULL, L"배열선언에서 닫는 괄호가 없습니다 (']')", L"스크립트오류", MB_OK);
					}

					vector<CString*> temptks;
					if(!GetSubPtrArray(&temptks, paTokens, tindex+2, endindex-1))
					{
						//MessageBox(NULL, L"배열선언의 사이즈 지정부분이 잘못되었습니다", L"스크립트오류", MB_OK);
					}

					int retType = 0;
					nArraySize = *((int*)GetValue(&temptks, &retType));

					// 배열 백터 생성
					vector<CVariable*>* pVector = new vector<CVariable*>();
					
					// 초기화
					for(int i=0; i<nArraySize; i++)
					{
						CVariable* pElement = new CVariable();
						pElement->SetType(vartype);
						pElement->SetBlockLevel(nBlockLevel);

						if(vartype==1){
							pElement->SetValue((void*)new int);
							*((int*)pElement->GetValue()) = 0;		// 0으로 초기화 
						}
						else if(vartype==2){
							pElement->SetValue((void*)new CString);
							*((CString*)pElement->GetValue()) = L"";	// ""으로 초기화
						}

						pVector->push_back(pElement);

					}

					// 배열 백터를 값으로 삽입
					newvar->SetValue((void*)pVector);
					newvar->m_bIsArray = TRUE;
				}
				// 배열이 아닌경우
				else
				{
					if(vartype==1){
						newvar->SetValue((void*)new int);
						*((int*)newvar->GetValue()) = 0;		// 0으로 초기화 
					}
					else if(vartype==2){
						newvar->SetValue((void*)new CString);
						*((CString*)newvar->GetValue()) = L"";	// ""으로 초기화
					}
					else
					{
						//MessageBoxA(NULL, "알 수 없는 변수타입", "스크립트오류", MB_OK);
					}
				}

				m_aVariableTable.push_back(newvar);

				token = *((*paTokens)[++tindex]);
				while(token.Compare(L";")){
					if(token.Compare(L"=")==0){
						vector<CString*> temptokens;
						CString nexttoken;
						while(1){
							temptokens.push_back((*paTokens)[++tindex]);
							nexttoken = *((*paTokens)[tindex+1]);
							if(nexttoken.Compare(L"(")==0){
								int endtrace = SkipBlock(paTokens, tindex+1);
								for(tindex++; tindex < endtrace; tindex++){
									temptokens.push_back((*paTokens)[tindex]);
								}
								tindex--;
							}
							else if(nexttoken.Compare(L";")==0 || nexttoken.Compare(L",")==0) break;
						}
						int rettype;
						void* retptr = GetValue(&temptokens, &rettype);
						if(retptr){
							newvar->SetValue(RemakeValue(retptr, rettype, newvar->GetType()));
							DeleteInstance(retptr, rettype);
						}
					}
					else if(token.Compare(L",")==0){
						newvar = new CVariable();
						newvar->SetName(*((*paTokens)[++tindex]));
						newvar->SetType(vartype);
						if(vartype==1){
							newvar->SetValue((void*)new int);
							*((int*)newvar->GetValue()) = 0;		// 0으로 초기화 
						}
						else{
							newvar->SetValue((void*)new CString);
							*((CString*)newvar->GetValue()) = L"";	// ""으로 초기화
						}					
						m_aVariableTable.push_back(newvar);
					}
					token = *((*paTokens)[++tindex]);
				}
			}
		}

		//////////////////////// 제어문일때 ////////////////////////////////
		else if(token.Compare(L"break")==0){
			int nWantLevel = 1;		// 나가고싶은 반복문 차수
			token = *((*paTokens)[++tindex]);
			if(token.Compare(L";")!=0){
				nWantLevel = _wtoi(token);
				tindex++;
			}
			if(nWantLevel > stackLoopBlock.GetLength())
			{
				//MessageBoxA(NULL, "break 차수 지정이 잘못되었습니다", "스크립트오류", MB_OK);
			}
			else{
				nQuitLevel = nBlockLevel - stackLoopBlock[nWantLevel-1];
			}
		}
		else if(token.Compare(L"return")==0){
			vector<CString*> temptokens;
			int rettype;
			int beginindex, endindex;

			tindex++;
			beginindex = tindex;
			tindex = SkipStatement(paTokens, tindex);
			endindex = tindex - 1;
			if(beginindex<=endindex && GetSubPtrArray(&temptokens, paTokens, beginindex, endindex)){	// 수행블럭 추출에 성공했다면
				pRetPtr = GetValue(&temptokens, &rettype);
				if(pRetType) *pRetType = rettype;
			}

			nQuitLevel = nBlockLevel;
		}
		else if(token.Compare(L"if")==0){
			int beginindex = tindex+2;
			tindex = SkipBlock(paTokens, tindex+1);
			int endindex = tindex - 1;
			vector<CString*> temptokens;
			if(GetSubPtrArray(&temptokens, paTokens, beginindex, endindex)){
				int rettype;
				int* retptr = (int*)GetValue(&temptokens, &rettype);
				if(*retptr){		// 조건이 참이라면					
					tindex++;
					beginindex = tindex;
					tindex = SkipStatement(paTokens, tindex);
					endindex = tindex;
					if(GetSubPtrArray(&temptokens, paTokens, beginindex, endindex)){	// 수행블럭 추출에 성공했다면
						pRetPtr = InterpretBlock(&temptokens, pRetType);
					}
					if(tindex+1 < (int)paTokens->size()){
						token = *((*paTokens)[tindex+1]);
						if(token.Compare(L"else")==0){		// 뒤에 else문이 존재하면
							tindex = SkipStatement(paTokens, tindex+2);		// 스킵
						}
					}
				}
				else{					// 조건이 거짓이면
					tindex = SkipStatement(paTokens, tindex + 1);		// 스킵
					if(tindex+1 < (int)paTokens->size()){
						token = *((*paTokens)[tindex+1]);
						if(token.Compare(L"else")==0){		// 뒤에 else문이 존재하면
							tindex+=2;
							beginindex = tindex;
							tindex = SkipStatement(paTokens, tindex);
							endindex = tindex;
							if(GetSubPtrArray(&temptokens, paTokens, beginindex, endindex)){	// 수행블럭 추출에 성공했다면
								pRetPtr = InterpretBlock(&temptokens, pRetType);
							}
						}
					}
				}
				delete retptr;
			}
		}
		else if(token.Compare(L"for")==0){			// for문이라면
			int beginindex;
			int endindex;
			vector<CString*> initstate;
			vector<CString*> conditionstate;
			vector<CString*> updatestate;
			vector<CString*> block;
			int rettype;
			void* retptr;
			int condition;


			beginindex = tindex+2;
			endindex = SkipStatement(paTokens, beginindex) - 1;
			GetSubPtrArray(&initstate, paTokens, beginindex, endindex);

			beginindex = endindex+2;
			endindex = SkipStatement(paTokens, beginindex) - 1;
			GetSubPtrArray(&conditionstate, paTokens, beginindex, endindex);

			beginindex = endindex+2;
			tindex = SkipBlock(paTokens, tindex + 1);
			endindex = tindex - 1;
			GetSubPtrArray(&updatestate, paTokens, beginindex, endindex);

			tindex++;
			beginindex = tindex;
			tindex = SkipStatement(paTokens, tindex);
			endindex = tindex;
			GetSubPtrArray(&block, paTokens, beginindex, endindex);			

			// 초기화
			retptr = GetValue(&initstate, &rettype);
			if(retptr) DeleteInstance(retptr, rettype); 
			stackLoopBlock.Insert(0, (TCHAR)nBlockLevel);
			nBlockLevel++;

			// 초기 조건검사
			retptr = GetValue(&conditionstate, &rettype);
			if(retptr){
				condition = *(int*)retptr;
				DeleteInstance(retptr, rettype);
			}
			else condition = 0;

			while(condition){
				// 블럭수행
				pRetPtr = InterpretBlock(&block, pRetType);
				if(nQuitLevel > 0){
					nQuitLevel--;
					break;
				}

				// 업데이트
				retptr = GetValue(&updatestate, &rettype);
				if(retptr) DeleteInstance(retptr, rettype); ;

				// 조건검사
				retptr = GetValue(&conditionstate, &rettype);
				if(retptr){
					condition = *(int*)retptr;
					DeleteInstance(retptr, rettype);
				}
				else condition = 0;
			}
			nBlockLevel--;
			stackLoopBlock.Delete(0, 1);
		}
		else if(token.Compare(L"while")==0){			// while문이라면
			int beginindex;
			int endindex;
			vector<CString*> conditionstate;
			vector<CString*> block;
			int rettype;
			void* retptr;
			int condition;


			beginindex = tindex+2;
			tindex = SkipBlock(paTokens, tindex + 1);
			endindex = tindex - 1;
			GetSubPtrArray(&conditionstate, paTokens, beginindex, endindex);

			tindex++;
			beginindex = tindex;
			tindex = SkipStatement(paTokens, tindex);
			endindex = tindex;
			GetSubPtrArray(&block, paTokens, beginindex, endindex);			

			stackLoopBlock.Insert(0, (TCHAR)nBlockLevel);
			nBlockLevel++;

			// 초기 조건검사
			retptr = GetValue(&conditionstate, &rettype);
			if(retptr){
				condition = *(int*)retptr;
				DeleteInstance(retptr, rettype);
			}
			else condition = 0;

			while(condition){
				// 블럭수행
				pRetPtr = InterpretBlock(&block, pRetType);
				if(nQuitLevel > 0){
					nQuitLevel--;
					break;
				}

				// 조건검사
				retptr = GetValue(&conditionstate, &rettype);
				if(retptr){
					condition = *(int*)retptr;
					DeleteInstance(retptr, rettype);
				}
				else condition = 0;
			}
			nBlockLevel--;
			stackLoopBlock.Delete(0, 1);
		}
		else if(token.Compare(L"do")==0){			// do-while문이라면
			int beginindex;
			int endindex;
			vector<CString*> conditionstate;
			vector<CString*> block;
			int rettype;
			void* retptr;
			int condition;

			tindex++;
			beginindex = tindex;
			tindex = SkipStatement(paTokens, tindex);
			endindex = tindex;
			GetSubPtrArray(&block, paTokens, beginindex, endindex);

			token = *((*paTokens)[++tindex]);
			if(token.Compare(L"while")) continue;
			tindex++;
			beginindex = tindex + 1;
			tindex = SkipBlock(paTokens, tindex);
			endindex = tindex - 1;
			GetSubPtrArray(&conditionstate, paTokens, beginindex, endindex);
			tindex++;	// 세미콜론 뛰어넘기

			stackLoopBlock.Insert(0, (TCHAR)nBlockLevel);
			nBlockLevel++;

			do{
				// 블럭수행
				pRetPtr = InterpretBlock(&block, pRetType);
				if(nQuitLevel > 0){
					nQuitLevel--;
					break;
				}

				// 조건검사
				retptr = GetValue(&conditionstate, &rettype);
				if(retptr){
					condition = *(int*)retptr;
					DeleteInstance(retptr, rettype);
				}
				else condition = 0;
			}while(condition);
			nBlockLevel--;
			stackLoopBlock.Delete(0, 1);
		}

		/////////////////////// 일반 문장 (대입, 수식, 함수호출 등..) //////
		else{
			int beginindex = tindex;
			token=*((*paTokens)[tindex]);
			while(token.Compare(L";")){
				tindex++;
				token = *((*paTokens)[tindex]);
				if(token.Compare(L"(")==0 || token.Compare(L"{")==0 || token.Compare(L"[")==0)
				{
					tindex = SkipBlock(paTokens, tindex);
					if(tindex == -1) break;
				}
			}

			vector<CString*> temptks;
			if(GetSubPtrArray(&temptks, paTokens, beginindex, tindex-1)){
				int rettype;
				void* retptr = GetValue(&temptks, &rettype);
				if(retptr) DeleteInstance(retptr, rettype);
			}
		}

		if(nQuitLevel>0){
			nQuitLevel--;
			break;
		}
		tindex++;
	}

	if(!(m_bContinuousMode==true && nBlockLevel==1))
	{
		// 블럭이 끝났으므로 이 블럭에서 선언했었던 지역변수들 삭제	
		CVariable* ptr;
		int a_index;
		for(a_index = (int)m_aVariableTable.size()-1; a_index>=startSP; a_index--){
			ptr = (CVariable*)m_aVariableTable[a_index];
			if(ptr) delete ptr;
			m_aVariableTable.pop_back();
		}

		// 블럭이 끝났으므로 이 블럭에서 선언했었던 펑션들 삭제	
		CFunction* fptr;
		for(a_index = (int)m_aFunctionTable.size()-1; a_index>=startFP; a_index--){
			fptr = (CFunction*)m_aFunctionTable[a_index];
			if(fptr) delete fptr;
			m_aFunctionTable.pop_back();
		}
	}

	nBlockLevel--;

	return pRetPtr;
}


void* CTransScriptParser::GetValue(CString strSentence, int* pnRetType)
{
	void* pRetVal = NULL;
	vector<CString*> vTokens;

	/////////////////// 토큰화 ///////////////////
	if(Tokenize(strSentence, &vTokens)==FALSE){
		*pnRetType = 0;
		return NULL;
		//AfxMessageBox(m_strLastError, MB_OK);
	}
	//int a = (int)vTokens.size();
	//ShowTokens(&m_aTokens);

	/////////////////// 처리 /////////////////////
	pRetVal = GetValue(&vTokens, pnRetType);

	/////////////// 다쓴 데이터 삭제 //////////////
	DeleteAllTokens(&vTokens);				// 토큰 스트링들을 메모리에서 해제

	return pRetVal;
}


void* CTransScriptParser::GetValue(vector<CString*> *paTokens, int* pnRetType)
{
	void* pRetPtr = NULL;
	int tindex;
	CString token;
	int count = paTokens->size();
	*pnRetType = 0;	


	if(count==0){
		*pnRetType = 0;
		return pRetPtr;
	}
	else if(count==1){
		token = *((*paTokens)[0]);

		if('0' <= token[0] && token[0] <= '9'){	// 정수상수라면
			pRetPtr = new int;
			*((int*)pRetPtr) = atoi2(token);
			*pnRetType = 1;			
			return pRetPtr;
		}
		else if(token[0]==0x22){						// 문자열 상수라면
			pRetPtr = new CString();
			token = token.Mid(1, token.GetLength()-2);
			*(CString*)pRetPtr = token;
			*pnRetType = 2;
			return pRetPtr;
		}
		else{
			CVariable* var = GetVariable(token);
			if(var==NULL){
				m_strLastError = L"선언되지 않은 알 수 없는 식별자입니다 ";
				m_strLastError += token;
				//MessageBox(NULL, m_strLastError, L"스트립트 오류", MB_OK);
				return NULL;
			}
			if(var->GetType()==1){
				pRetPtr = new int;
				*(int*)pRetPtr = *(int*)(var->GetValue());
			}
			else{
				pRetPtr = new CString();
				*(CString*)pRetPtr = *(CString*)(var->GetValue());
			}
			*pnRetType = var->GetType();
			return pRetPtr;
		}
	}
	else{
		// 대입기호 검색 루프
		for(tindex=0; tindex<count; tindex++){
			token = *((*paTokens)[tindex]);
			if(token.Compare(L"(")==0 || token.Compare(L"[")==0){
				tindex = SkipBlock(paTokens, tindex);
				if(tindex<0) return NULL;
			}
			else if(token.Compare(L"=")==0 || token.Compare(L"+=")==0 || token.Compare(L"-=")==0 || token.Compare(L"*=")==0 || token.Compare(L"/=")==0 || token.Compare(L"%=")==0){
				CVariable* var = NULL;
				CString lValurStart = *((*paTokens)[tindex-1]);
				if( lValurStart == L"]" )
				{
					int startindex = SkipBlock(paTokens, tindex-1);
					if(startindex<0) return pRetPtr;
					vector<CString*> temptks;
					if(!GetSubPtrArray(&temptks, paTokens, startindex+1, tindex-2)) return pRetPtr;
					
					int tmpRetType = 0;
					int nArrayIndex = *((int*)GetValue(&temptks, &tmpRetType));

					CVariable* pArray = GetVariable(*((*paTokens)[startindex-1]));
					if(!pArray) return pRetPtr;
					var = pArray->GetVariableElemant(nArrayIndex);
				}
				else
				{
					var = GetVariable(lValurStart);
				}

				if(var==NULL){
					m_strLastError = L"잘못된 변수명입니다 ";
					m_strLastError += *((*paTokens)[tindex-1]);
					//MessageBox(NULL, m_strLastError, L"스크립트오류", MB_OK);
					return NULL;
				}

				int r_type;
				vector<CString*> temptks;
				void* r_valuePtr;
				if(token.Compare(L"=")==0){		// 단순대입
					GetSubPtrArray(&temptks, paTokens, tindex+1, paTokens->size()-1);
					r_valuePtr = GetValue(&temptks, &r_type);
				}
				else{							// 연산후 대입
					CString varname = var->GetName();
					CString opentrace = L"(";
					CString closetrace = L")";
					CString op = token.Mid(0, 1);

					temptks.push_back(&varname);			// 변수명
					temptks.push_back(&op);					// 연산자
					temptks.push_back(&opentrace);			// '('
					for(size_t i=tindex+1; i < paTokens->size(); i++){
						temptks.push_back((*paTokens)[i]);
					}
					temptks.push_back(&closetrace);			// ')'
					r_valuePtr = GetValue(&temptks, &r_type);
				}



				// 형변환 후 새로 생성된 인스턴스 대입
				var->SetValue(RemakeValue(r_valuePtr, r_type, var->GetType()));
				// 리턴값 생성
				pRetPtr = RemakeValue(r_valuePtr, r_type, var->GetType());
				*pnRetType = var->GetType();
				// 방금전 받은 리턴값 해제
				DeleteInstance(r_valuePtr, r_type);
				// 리턴
				return pRetPtr;
			}
		}

		// &&, || 논리연산자 검색 루프
		for((int)tindex=paTokens->size()-1; tindex>=0; tindex--){
			token = *((*paTokens)[tindex]);
			if(token.Compare(L")")==0 || token.Compare(L"]")==0){
				tindex = SkipBlock(paTokens, tindex);
				if(tindex<0) return NULL;
			}
			else if(token.Compare(L"&&")==0 || token.Compare(L"||")==0){
				int l_type, r_type;
				void *l_valuePtr, *r_valuePtr;
				vector<CString*> temptks;

				GetSubPtrArray(&temptks, paTokens, 0, tindex-1);
				l_valuePtr = GetValue(&temptks, &l_type);

				GetSubPtrArray(&temptks, paTokens, tindex+1, paTokens->size()-1);
				r_valuePtr = GetValue(&temptks, &r_type);

				// 정수형으로 캐스트
				int* tempptr;

				tempptr = (int*)RemakeValue(l_valuePtr, l_type, 1);
				DeleteInstance(l_valuePtr, l_type);
				l_valuePtr = tempptr; l_type = 1;

				tempptr = (int*)RemakeValue(r_valuePtr, r_type, 1);
				DeleteInstance(r_valuePtr, r_type);
				r_valuePtr = tempptr; r_type = 1;

				*pnRetType = 1;
				pRetPtr = new int;
				*(int*)pRetPtr = 0;					
				if(token.Compare(L"&&")==0){
					if(*(int*)l_valuePtr && *(int*)r_valuePtr) *(int*)pRetPtr = 1;
				}
				else if(token.Compare(L"||")==0){
					if(*(int*)l_valuePtr || *(int*)r_valuePtr) *(int*)pRetPtr = 1;
				}

				// 방금전 받은 리턴값 해제
				DeleteInstance(r_valuePtr, r_type);
				DeleteInstance(l_valuePtr, l_type);
				// 리턴
				return pRetPtr;
			}		
		}

		// 비교연산자 검색 루프
		for(tindex=paTokens->size()-1; tindex>=0; tindex--){
			token = *((*paTokens)[tindex]);
			if(token.Compare(L")")==0 || token.Compare(L"]")==0){
				tindex = SkipBlock(paTokens, tindex);
				if(tindex<0) return NULL;
			}
			else if(token.Compare(L"==")==0 || token.Compare(L"!=")==0 || token.Compare(L"<")==0 || token.Compare(L">")==0 || token.Compare(L"<=")==0 || token.Compare(L">=")==0){
				int l_type, r_type;
				void *l_valuePtr, *r_valuePtr;
				vector<CString*> temptks;

				GetSubPtrArray(&temptks, paTokens, 0, tindex-1);
				l_valuePtr = GetValue(&temptks, &l_type);

				GetSubPtrArray(&temptks, paTokens, tindex+1, paTokens->size()-1);
				r_valuePtr = GetValue(&temptks, &r_type);

				if(l_type != r_type){		// 비교할 두 데이터형이 다르다면 정수형으로 캐스트
					void* tempptr;

					tempptr = RemakeValue(l_valuePtr, l_type, 1);
					DeleteInstance(l_valuePtr, l_type);
					l_valuePtr = tempptr; l_type = 1;

					tempptr = RemakeValue(r_valuePtr, r_type, 1);
					DeleteInstance(r_valuePtr, r_type);
					r_valuePtr = tempptr; r_type = 1;
				}

				*pnRetType = 1;					// 참거짓을 리턴하므로 정수형
				pRetPtr = new int;				// 리턴될 비교결과(참 1, 거짓 0)
				*(int*)pRetPtr = 0;
				if(l_type==1){				// 정수비교
					if(token.Compare(L"==")==0){
						if(*(int*)l_valuePtr == *(int*)r_valuePtr) *(int*)pRetPtr = 1;
						else *(int*)pRetPtr = 0;
					}
					else if(token.Compare(L"!=")==0){
						if(*(int*)l_valuePtr != *(int*)r_valuePtr) *(int*)pRetPtr = 1;
						else *(int*)pRetPtr = 0;
					}
					else if(token.Compare(L"<")==0){
						if(*(int*)l_valuePtr < *(int*)r_valuePtr) *(int*)pRetPtr = 1;
						else *(int*)pRetPtr = 0;
					}
					else if(token.Compare(L">")==0){
						if(*(int*)l_valuePtr > *(int*)r_valuePtr) *(int*)pRetPtr = 1;
						else *(int*)pRetPtr = 0;
					}
					else if(token.Compare(L"<=")==0){
						if(*(int*)l_valuePtr <= *(int*)r_valuePtr) *(int*)pRetPtr = 1;
						else *(int*)pRetPtr = 0;
					}
					else if(token.Compare(L">=")==0){
						if(*(int*)l_valuePtr >= *(int*)r_valuePtr) *(int*)pRetPtr = 1;
						else *(int*)pRetPtr = 0;
					}
				}
				else if(l_type==2 && r_type==2){			// 문자열비교
					CString strltemp = *(CString*)l_valuePtr;
					CString strrtemp = *(CString*)r_valuePtr;
					if(token.Compare(L"==")==0){
						if(strltemp.Compare(strrtemp)==0) *(int*)pRetPtr = 1;
						else *(int*)pRetPtr = 0;
					}
					else if(token.Compare(L"!=")==0){
						if(strltemp.Compare(strrtemp)) *(int*)pRetPtr = 1;
						else *(int*)pRetPtr = 0;
					}
				}


				// 방금전 받은 리턴값 해제
				DeleteInstance(r_valuePtr, r_type);
				DeleteInstance(l_valuePtr, l_type);
				// 리턴
				return pRetPtr;
			}		
		}

		// +,- 산술연산자 검색 루프
		for(tindex=paTokens->size()-1; tindex>=0; tindex--){
			token = *((*paTokens)[tindex]);
			if(token.Compare(L")")==0 || token.Compare(L"]")==0){
				tindex = SkipBlock(paTokens, tindex);
				if(tindex<0) return NULL;
			}
			else if(token.Compare(L"+")==0 || token.Compare(L"-")==0){
				int l_type, r_type;
				void *l_valuePtr, *r_valuePtr;
				vector<CString*> temptks;

				GetSubPtrArray(&temptks, paTokens, 0, tindex-1);
				l_valuePtr = GetValue(&temptks, &l_type);

				GetSubPtrArray(&temptks, paTokens, tindex+1, paTokens->size()-1);
				r_valuePtr = GetValue(&temptks, &r_type);

				if(l_type != r_type){		// 연산할 두 데이터형이 다르다면
					if(l_type==0 && r_type==1){		// 부호를 표현하고자 앞이 void형이라..
						l_valuePtr = new int;
						*(int*)l_valuePtr = 0;
						l_type = 1;
					}
					else{							// 그 외의 경우 스트링으로 캐스트
						void* tempptr;

						tempptr = RemakeValue(l_valuePtr, l_type, 2);
						DeleteInstance(l_valuePtr, l_type);
						l_valuePtr = tempptr; l_type = 2;

						tempptr = RemakeValue(r_valuePtr, r_type, 2);
						DeleteInstance(r_valuePtr, r_type);
						r_valuePtr = tempptr; r_type = 2;
					}
				}

				if(l_type==1){							// 정수연산
					*pnRetType = 1;
					pRetPtr = new int;
					*(int*)pRetPtr = 0;	
					int l_val = *(int*)l_valuePtr;
					int r_val = *(int*)r_valuePtr;
					if(token.Compare(L"+")==0)
						*(int*)pRetPtr = l_val + r_val;						
					else if(token.Compare(L"-")==0)
						*(int*)pRetPtr = l_val - r_val;						
				}
				else if(l_type==2 && r_type==2){		// 문자열연산
					*pnRetType = 2;
					pRetPtr = new CString;
					*(CString*)pRetPtr = L"";					
					CString strltemp = *(CString*)l_valuePtr;
					CString strrtemp = *(CString*)r_valuePtr;
					if(token.Compare(L"+")==0){
						*(CString*)pRetPtr = strltemp;
						*(CString*)pRetPtr += strrtemp;
					}
					else if(token.Compare(L"-")==0){
						//strltemp.Replace(strrtemp, "");
						*(CString*)pRetPtr = strltemp;
					}
				}


				// 방금전 받은 리턴값 해제
				DeleteInstance(r_valuePtr, r_type);
				DeleteInstance(l_valuePtr, l_type);
				// 리턴
				return pRetPtr;
			}		
		}

		// *, /, % 산술연산자 검색 루프
		for(tindex=paTokens->size()-1; tindex>=0; tindex--){
			token = *((*paTokens)[tindex]);
			if(token.Compare(L")")==0 || token.Compare(L"]")==0){
				tindex = SkipBlock(paTokens, tindex);
				if(tindex<0) return NULL;
			}
			else if(token.Compare(L"*")==0 || token.Compare(L"/")==0 || token.Compare(L"%")==0){
				int l_type, r_type;
				void *l_valuePtr, *r_valuePtr;
				vector<CString*> temptks;

				GetSubPtrArray(&temptks, paTokens, 0, tindex-1);
				l_valuePtr = GetValue(&temptks, &l_type);

				GetSubPtrArray(&temptks, paTokens, tindex+1, paTokens->size()-1);
				r_valuePtr = GetValue(&temptks, &r_type);

				// 정수형으로 캐스트
				void* tempptr;

				tempptr = RemakeValue(l_valuePtr, l_type, 1);
				DeleteInstance(l_valuePtr, l_type);
				l_valuePtr = tempptr; l_type = 1;

				tempptr = RemakeValue(r_valuePtr, r_type, 1);
				DeleteInstance(r_valuePtr, r_type);
				r_valuePtr = tempptr; r_type = 1;

				*pnRetType = 1;
				pRetPtr = new int;
				*(int*)pRetPtr = 0;					
				if(token.Compare(L"*")==0)
					*(int*)pRetPtr = *(int*)l_valuePtr * *(int*)r_valuePtr;
				else if(token.Compare(L"/")==0)
					*(int*)pRetPtr = *(int*)l_valuePtr / *(int*)r_valuePtr;
				else if(token.Compare(L"%")==0)
					*(int*)pRetPtr = *(int*)l_valuePtr % *(int*)r_valuePtr;


				// 방금전 받은 리턴값 해제
				DeleteInstance(r_valuePtr, r_type);
				DeleteInstance(l_valuePtr, l_type);
				// 리턴
				return pRetPtr;
			}		
		}

		// ++, -- 단항연산자 검색 루프
		for(tindex=0; tindex<count; tindex++){
			token = *((*paTokens)[tindex]);
			if(token.Compare(L"(")==0 || token.Compare(L"[")==0){
				tindex = SkipBlock(paTokens, tindex);
				if(tindex<0) return NULL;
			}
			else if(token.Compare(L"++")==0 || token.Compare(L"--")==0){
				*pnRetType = 1;
				pRetPtr = new int;
				*(int*)pRetPtr = 0;					
				CVariable* var;
				int addval = 0;
				if(token.Compare(L"++")==0) addval = 1;
				else if(token.Compare(L"--")==0) addval = -1;

				if(tindex>0){
					var = GetVariable(*((*paTokens)[tindex-1]));
					if(var==NULL) return pRetPtr;
					*(int*)pRetPtr = *(int*)var->GetValue();
					*(int*)var->GetValue() += addval;
				}
				else{
					var = GetVariable(*((*paTokens)[tindex+1]));
					if(var==NULL) return pRetPtr;
					*(int*)var->GetValue() += addval;						
					*(int*)pRetPtr = *(int*)var->GetValue();
				}

				return pRetPtr;
			}		
		}

		// '!'기호 검사
		token = *((*paTokens)[0]);
		if(token.Compare(L"!")==0){
			vector<CString*> temptks;
			if(!GetSubPtrArray(&temptks, paTokens, 1, paTokens->size()-1)) return pRetPtr;
			int rettype;
			void* retptr = GetValue(&temptks, &rettype);
			pRetPtr = RemakeValue(retptr, rettype, 1);
			DeleteInstance(retptr, rettype);
			rettype = *(int*)pRetPtr;
			*(int*)pRetPtr = !rettype;
			*pnRetType = 1;
			return pRetPtr;			
		}

		// '(' 검사
		token = *((*paTokens)[0]);
		if(token.Compare(L"(")==0){
			int endindex = SkipBlock(paTokens, 0);
			if(endindex<0) return pRetPtr;
			vector<CString*> temptks;
			if(!GetSubPtrArray(&temptks, paTokens, 1, endindex-1)) return pRetPtr;
			pRetPtr = GetValue(&temptks, pnRetType);
			return pRetPtr;
		}

		// '[' 검사
		token = *((*paTokens)[0]);
		if(token.Compare(L"[")==0){
			int endindex = SkipBlock(paTokens, 0);
			if(endindex<0) return pRetPtr;
			vector<CString*> temptks;
			if(!GetSubPtrArray(&temptks, paTokens, 1, endindex-1)) return pRetPtr;

			int tmpRetType = 0;
			void* pRetValue = GetValue(&temptks, &tmpRetType);
			
			// 메모리 주소를 제대로 얻어왔다면
			if(NULL != pRetValue && tmpRetType == 1)
			{
				*pnRetType = 1;
				pRetPtr = new int;
				memcpy(pRetPtr, *(void**)pRetValue, 4);
			}

			DeleteInstance(pRetValue, tmpRetType);

			return pRetPtr;
		}

		// 배열값 또는 함수 호출 검사
		//token = *(CString*)paTokens->ElementAt(0);
		//AfxMessageBox("앗 함수호출검사");
		CString funcname = *((*paTokens)[0]);
		token = *((*paTokens)[1]);
		if(token.Compare(L"[")==0){			// 배열값이라면
			int endindex = SkipBlock(paTokens, 1);
			if(endindex<0) return pRetPtr;
			vector<CString*> temptks;
			if(!GetSubPtrArray(&temptks, paTokens, 2, endindex-1)) return pRetPtr;
			
			int tmpRetType;
			void* pRetValue = GetValue(&temptks, &tmpRetType);
			int nArrayIndex = *((int*)pRetValue);
			DeleteInstance(pRetValue, tmpRetType);
			
			CVariable* pArray = GetVariable(funcname);
			if(!pArray) return pRetPtr;
			pRetValue = pArray->GetValue((vector<CString*>*)(UINT_PTR)nArrayIndex);
			if(pArray->GetType()==1)
			{
				pRetPtr = new int;
				*(int*)pRetPtr = *(int*)pRetValue;
			}
			else if(pArray->GetType()==2)
			{
				pRetPtr = new CString;
				*(CString*)pRetPtr = *(CString*)pRetValue;
			}

			*pnRetType = pArray->GetType();
		}		
		else if(token.Compare(L"(")==0){	// 함수호출이라면
			vector<CString*> aVarList;
			tindex = 2;
			token = *((*paTokens)[2]);
			int beginindex = 2;
			int tsize = paTokens->size();
			while(tindex<tsize){
				if(token.Compare(L"(")==0 || token.Compare(L"[")==0){
					tindex = SkipBlock(paTokens, tindex);
				}
				else if(token.Compare(L",")==0 || token.Compare(L")")==0){
					vector<CString*> temptokens;
					if(GetSubPtrArray(&temptokens, paTokens, beginindex, tindex-1)){
						int rettype;
						void* retptr = GetValue(&temptokens, &rettype);
						aVarList.push_back((CString*)RemakeValue(retptr, rettype, 2));
						if(retptr) DeleteInstance(retptr, rettype);
					}
					if(token.Compare(L")")==0) break;
					beginindex = tindex + 1;
				}
				tindex++;
				token = *((*paTokens)[tindex]);
			}

			CFunction* func = GetFunction(funcname);
			if(func){	// 사용자 정의함수 호출시
				int tmpBlockLevel = nBlockLevel;
				int tmpQuitLevel = nQuitLevel;
				CString tmpLoopBlock = stackLoopBlock;				
				vector<CVariable*> tmpVariableTable;					// 변수 대피
				tmpVariableTable = m_aVariableTable;
				// 전역변수빼고 다 삭제
				vector<CVariable*>::iterator iter = m_aVariableTable.begin();
				while(iter != m_aVariableTable.end())
				{
					if((*iter)->GetBlockLevel() == 1)
					{
						iter++;
					}
					else
					{
						iter = m_aVariableTable.erase(iter);
					}

				}

				pRetPtr = func->GetValue(&aVarList);
				*pnRetType = func->GetType();

				m_aVariableTable.clear();				// 변수 복구
				m_aVariableTable = tmpVariableTable;
				nBlockLevel = tmpBlockLevel;
				nQuitLevel = tmpQuitLevel;
				stackLoopBlock = tmpLoopBlock;
			}
			else
			{		// 기본 함수 호출시
				//pRetPtr = m_pCallbackClass->FunctionCall(funcname, &aVarList, pnRetType);		// 함수 호출
			}

			DeleteAllTokens(&aVarList);							// 매개변수들 메모리에서 해제
			return pRetPtr;
		}
	}

	return pRetPtr;
}

BOOL CTransScriptParser::GetSubPtrArray(vector<CString*> *paTar, vector<CString*> *paSrc, int beginindex, int endindex)
{
	if((paSrc==NULL) || (paTar==NULL) || (endindex >= (int)paSrc->size()) || (endindex < beginindex)) return FALSE;

	paTar->clear();
	for(int i=beginindex; i<=endindex; i++){
		paTar->push_back((*paSrc)[i]);
	}

	return TRUE;
}

void* CTransScriptParser::RemakeValue(void *pSrc, int before, int after)
{
	void* retptr = NULL;

	if(after==0){			// void 형으로 반환해야 할때

	}
	else if(after==1){		// 정수형으로 반환해야할때
		retptr = new int;
		if(before==0){
			*(int*)retptr = 0;
		}
		else if(before==1){
			*(int*)retptr = *(int*)pSrc;
		}
		else if(before==2){
			*(int*)retptr = _wtoi( *(CString*)pSrc );
		}
	}
	else if(after==2){		// 스트링형으로 반환해야할때
		retptr = new CString;
		if(before==0){
			*(CString*)retptr = L"";
		}
		else if(before==1){
			CString temp;
			wchar_t temp_buf[512];
			swprintf(temp_buf, L"%d", *(int*)pSrc);
			temp = temp_buf;
			*(CString*)retptr = temp;
		}
		else if(before==2){
			*(CString*)retptr = *(CString*)pSrc;		
		}
	}

	return retptr;
}

CVariable* CTransScriptParser::GetVariable(CString strVarName)
{
	int i, count;
	count = m_aVariableTable.size();
	CVariable* temp;
	for(i=0; i<count; i++){
		temp = (CVariable*)m_aVariableTable[i];
		if(strVarName.CompareNoCase(temp->GetName())==0) return temp;
	}
	return NULL;
}

int CTransScriptParser::SkipStatement(vector<CString*> *paTokens, int tindex)
{
	int maxindex = paTokens->size();
	CString token = *((*paTokens)[tindex]);
	if(token.Compare(L"{") == 0){			// 블럭문장이라면
		tindex = SkipBlock(paTokens, tindex);
	}
	else if(token.Compare(L"if") == 0){		// if문이라면
		tindex = SkipBlock(paTokens, tindex+1) + 1;
		tindex = SkipStatement(paTokens, tindex);
		if(tindex+1 < (int)paTokens->size()){
			token = *((*paTokens)[tindex+1]);
			if(token.Compare(L"else") == 0){		// 뒤에 else가 따라오면
				tindex = SkipStatement(paTokens, tindex+2);
			}
		}
	}
	else{									// 위 조건에 모두 맞지 않을 경우
		while(tindex<maxindex){
			token = *((*paTokens)[tindex]);
			if(token.Compare(L"(")==0) tindex = SkipBlock(paTokens, tindex);
			else if(token.Compare(L";")==0) break;
			tindex++;
		}
	}

	return tindex;
}


wchar_t* CTransScriptParser::AllocateBufferFromScriptFile(CString filename)
{
	// 스크립트 파일 열기 (파일 -> 버퍼)
	FILE* fp = _wfopen(filename, L"rb");

	if(!fp)
	{
		MessageBoxA(NULL, "지정한 스크립트 파일을 찾을 수 없습니다.", "스크립트오류", MB_OK);
		return NULL;
	}
	fseek(fp,0l,SEEK_END);

	int sourceSize = ftell(fp);

	char* pTempBuffer = new char[sourceSize+1];
	memset(pTempBuffer, 0, sourceSize+1);


	fseek(fp, 0l, SEEK_SET);
	fread(pTempBuffer, sizeof(char), sourceSize, fp);
	fclose(fp);

	wchar_t* pScriptBuffer = new wchar_t[sourceSize+1];
	memset(pScriptBuffer, 0, (sourceSize+1)*2);
	MultiByteToWideChar(CP_ACP, 0, pTempBuffer, sourceSize, pScriptBuffer, sourceSize );	
	delete [] pTempBuffer;

	return pScriptBuffer;
}

void CTransScriptParser::ExecuteScriptFile(LPCTSTR filename)
{
	// 스크립트 버퍼 생성
	wchar_t* pScriptBuffer = AllocateBufferFromScriptFile(filename);
	if(pScriptBuffer==NULL) return;

	// 스크립트 실행
	ExecuteScript(pScriptBuffer);

	// 스크립트 버퍼 삭제
	delete [] pScriptBuffer;
}

void CTransScriptParser::DeleteInstance(void *ptr, int type)
{
	if(type==1){
		int* ptr_int = (int*)ptr;
		if(ptr_int) {
			delete ptr_int;
			ptr_int = 0;
		}
	}
	else if(type==2){
		CString* ptr_str = (CString*)ptr;
		if(ptr_str) {
			delete ptr_str;
			ptr_str = 0;
		}
	}
	else{
		if(ptr) {
			delete ptr;
			ptr = 0;
		}
	}
}

// 앞에 "0x"가 붙은 16진수 문자열도 처리 가능하게
int CTransScriptParser::atoi2(CString str)
{
	int nRetValue = 0;

	str = str.MakeUpper();

	if(str.GetLength()>2 && str[1]=='X'){
		wchar_t tmp;
		for(int strIdx=2; strIdx<str.GetLength(); strIdx++){
			nRetValue <<= 4;
			tmp = str[strIdx];
			if('0' <= tmp && tmp <= '9') nRetValue += (tmp-'0');
			else if('A' <= tmp && tmp <= 'F') nRetValue += (10 + tmp-'A');
		}
	}
	else{
		nRetValue = _wtoi(str);
	}

	return nRetValue;
}

void CTransScriptParser::DeleteAllTokens(vector<CString*> *pArray)
{
	int i, count;
	CString* ptrString;
	count = pArray->size();
	for(i=0; i<count; i++){
		ptrString = (*pArray)[i];
		delete ptrString;				// 모든 원소들을 메모리에서 해제
	}
	pArray->clear();

}

void CTransScriptParser::DeleteAllIdentifiers(vector<CReplaceIdentifier*> *pArray)
{
	int i, count;
	CReplaceIdentifier* ptrString;
	count = pArray->size();
	for(i=0; i<count; i++){
		ptrString = (*pArray)[i];
		delete ptrString;				// 모든 원소들을 메모리에서 해제
	}
	pArray->clear();

}

void CTransScriptParser::DeleteAllVariables(vector<CVariable*> *pArray)
{
	int i, count;
	CVariable* ptrString;
	count = pArray->size();
	for(i=0; i<count; i++){
		ptrString = (CVariable*)(*pArray)[i];
		delete ptrString;				// 모든 원소들을 메모리에서 해제
	}
	pArray->clear();
}

void CTransScriptParser::IncludeScriptFile(CString filename, vector<CString*>* paTokens)
{
	// 스크립트 파일 열기 (파일 -> 버퍼)
	FILE* fp = _wfopen(filename, L"rb");

	if(!fp)
	{
		MessageBoxA(NULL, "지정한 인클루드 파일을 찾을 수 없습니다. : ", "스크립트오류", MB_OK);
		return;
	}
	fseek(fp,0l,SEEK_END);

	int sourceSize = ftell(fp);

	char* pTempBuffer = new char[sourceSize+1];
	memset(pTempBuffer, 0, sourceSize+1);


	fseek(fp, 0l, SEEK_SET);
	fread(pTempBuffer, sizeof(char), sourceSize, fp);
	fclose(fp);

	wchar_t* pScriptBuffer = new wchar_t[sourceSize+1];
	memset(pScriptBuffer, 0, (sourceSize+1)*2);
	MultiByteToWideChar(CP_ACP, 0, pTempBuffer, sourceSize, pScriptBuffer, sourceSize );	
	delete [] pTempBuffer;

	// 스크립트 실행
	IncludeScript(pScriptBuffer, paTokens);

	// 스크립트 버퍼 삭제
	delete [] pScriptBuffer;

}

void CTransScriptParser::IncludeScript(wchar_t *buffer, vector<CString*>* paTokens)
{
	LPCTSTR backupPtr = m_pSourcePtr;
	m_pSourcePtr = buffer;

	/////////////////// 토큰화 ///////////////////
	if(Tokenization(paTokens)==FALSE){
		//AfxMessageBox(m_strLastError, MB_OK);
	}
	//ShowTokens(&m_aTokens);

	m_pSourcePtr = backupPtr;

}



/* ========================= CVariable ====================================== */

void CVariable::SetBlockLevel(int level)
{
	m_nBlockLevel = level;
}

int CVariable::GetBlockLevel()
{
	return m_nBlockLevel;
}

void CVariable::SetValue(void *pValue)
{
	if(m_pValue) DeleteInstance();
	m_pValue = pValue;
}

void* CVariable::GetValue(vector<CString*>* pParams)
{
	if(m_bIsArray)	// 배열이면
	{
		int nArrayIndex = (int)(UINT_PTR)pParams;
		vector<CVariable*>* pVector = (vector<CVariable*>*)(UINT_PTR)m_pValue;
		if((int)pVector->size() <= nArrayIndex)
		{
			//MessageBoxA(NULL, "배열 첨자의 범위를 넘었습니다", "스크립트오류", MB_OK);
		}
		CVariable* pVar = (*pVector)[nArrayIndex];
		return pVar->GetValue();
	}

	return m_pValue;
}

CVariable* CVariable::GetVariableElemant(int idx)
{
	if(m_bIsArray)	// 배열이면
	{
		CVariable* pVar = (*(vector<CVariable*>*)m_pValue)[idx];
		return pVar;
	}

	return NULL;
}

void CVariable::SetName(CString strName)
{
	m_strVarName = strName;
}

CString CVariable::GetName()
{
	return m_strVarName;
}

void CVariable::DeleteInstance()
{
	if(m_pValue){

		if(m_bIsArray){	// 배열일 경우
			vector<CVariable*>* pVector = (vector<CVariable*>*)m_pValue;

			vector<CVariable*>::iterator iter = pVector->begin();
			for(; iter != pVector->end(); iter++)
			{
				CVariable* pVar = (*iter);
				delete pVar;
			}

			delete pVector;
		}		

		else
		{
			if(m_nVarType==1){
				int* ptr_int = (int*)m_pValue;
				delete ptr_int;
			}
			else if(m_nVarType==2){
				CString* ptr_str = (CString*)m_pValue;
				delete ptr_str;
			}
			else{
				delete m_pValue;
			}
		}
	}
	m_pValue = NULL;
}

void CVariable::SetType(int nVarType)
{
	m_nVarType = nVarType;
}

int CVariable::GetType()
{
	return m_nVarType;
}


/* ========================= CFunction ====================================== */
CFunction::CFunction()
{
	CVariable::CVariable();
	m_pCodeTokens = NULL;
	m_pScriptContainer = NULL;
}

CFunction::~CFunction()
{

	// 코드토큰 삭제 (각 스트링들의 delete는 CScript소멸자가 해주므로 여기선 안해도 됨)
	if(m_pCodeTokens)
	{
		vector<CString*>::iterator iter = m_pCodeTokens->begin();
		for(; iter!=m_pCodeTokens->end(); iter++)
		{
			delete (*iter);
		}
		delete m_pCodeTokens;
	}

	// 매개변수 삭제
	int i, count;
	CVariable* ptrString;
	count = m_aParams.size();

	for(i=0; i<count; i++){
		ptrString = (CVariable*)m_aParams[i];
		delete ptrString;				// 모든 원소들을 메모리에서 해제
	}
	m_aParams.clear();

}

void CFunction::SetValue(void *pCodeTokens)
{
	m_pCodeTokens = (vector<CString*>*)pCodeTokens;
	vector<CString*>::iterator iter = m_pCodeTokens->begin();
	for(; iter!=m_pCodeTokens->end(); iter++)
	{
		CString* pTmpToken = new CString;
		*pTmpToken = *(*iter);
		(*iter) = pTmpToken;
	}
}

void* CFunction::GetValue(vector<CString*> *pParams)
{
	void* pRetPtr = NULL;

	if(m_pScriptContainer && m_pCodeTokens)
	{
		int startSP = m_pScriptContainer->m_aVariableTable.size();

		// 매개변수 생성
		int i, count;
		CVariable* pVar;
		count = m_aParams.size();
		if(count != pParams->size()) {
			//MessageBoxA(NULL, "사용자정의함수 에러 : 매개변수 개수가 일치하지 않습니다.", "스크립트오류", MB_OK);
			return NULL;
		}

		for(i=0; i<count; i++){
			pVar = m_aParams[i];
			pVar->SetValue(RemakeValue((*pParams)[i], 2, pVar->GetType()));
			m_pScriptContainer->m_aVariableTable.push_back(pVar);
		}

		// 처리
		int   tempType;
		void* tempPtr = m_pScriptContainer->InterpretBlock(m_pCodeTokens, &tempType);
		pRetPtr = RemakeValue(tempPtr, tempType, m_nVarType);
		m_pScriptContainer->DeleteInstance(tempPtr, tempType);

		// 매개변수 삭제
		for(int a_index = m_pScriptContainer->m_aVariableTable.size()-1; a_index>=startSP; a_index--){
			m_pScriptContainer->m_aVariableTable.pop_back();
		}

	}

	return pRetPtr;
}

void CTransScriptParser::DeleteAllFunctions(vector<CFunction*> *pArray)
{
	int i, count;
	CFunction* ptrString;
	count = pArray->size();
	for(i=0; i<count; i++){
		ptrString = (CFunction*)(*pArray)[i];
		delete ptrString;				// 모든 원소들을 메모리에서 해제
	}
	pArray->clear();
}

CFunction* CTransScriptParser::GetFunction(CString strFuncName)
{
	int i, count;
	count = m_aFunctionTable.size();
	CFunction* temp;
	for(i=0; i<count; i++){
		temp = (CFunction*)m_aFunctionTable[i];
		if(strFuncName.Compare(temp->GetName())==0) return temp;
	}
	return NULL;
}

void CTransScriptParser::SetRegisterValues( REGISTER_ENTRY* pRegs )
{
	*(DWORD*)(GetVariable(_T("EAX"))->m_pValue) = pRegs->_EAX;
	*(DWORD*)(GetVariable(_T("EBX"))->m_pValue) = pRegs->_EBX;
	*(DWORD*)(GetVariable(_T("ECX"))->m_pValue) = pRegs->_ECX;
	*(DWORD*)(GetVariable(_T("EDX"))->m_pValue) = pRegs->_EDX;
	*(DWORD*)(GetVariable(_T("ESI"))->m_pValue) = pRegs->_ESI;
	*(DWORD*)(GetVariable(_T("EDI"))->m_pValue) = pRegs->_EDI;
	*(DWORD*)(GetVariable(_T("EBP"))->m_pValue) = pRegs->_EBP;
	*(DWORD*)(GetVariable(_T("ESP"))->m_pValue) = pRegs->_ESP;
}

void* CFunction::RemakeValue(void *pSrc, int before, int after)
{
	void* retptr = NULL;

	if(after==0){			// void 형으로 반환해야 할때

	}
	else if(after==1){		// 정수형으로 반환해야할때
		retptr = new int;
		if(before==0){
			*(int*)retptr = 0;
		}
		else if(before==1){
			*(int*)retptr = *(int*)pSrc;
		}
		else if(before==2){
			*(int*)retptr = _wtoi( *(CString*)pSrc );
		}
	}
	else if(after==2){		// 스트링형으로 반환해야할때
		retptr = new CString;
		if(before==0){
			*(CString*)retptr = L"";
		}
		else if(before==1){
			CString temp;
			wchar_t temp_buf[512];
			swprintf(temp_buf, L"%d", *(int*)pSrc);
			temp = temp_buf;
			*(CString*)retptr = temp;
		}
		else if(before==2){
			*(CString*)retptr = *(CString*)pSrc;		
		}
	}

	return retptr;
}

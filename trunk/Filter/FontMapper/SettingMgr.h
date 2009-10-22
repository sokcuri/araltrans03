#pragma once

class CSettingMgr
{
	static BOOL GetElemText(LPCWSTR cwszXml, LPCWSTR cwszElemName, LPWSTR wszBuf, size_t cch);
public:
	static BOOL XmlToOption(LPCWSTR cwszXml, FONT_MAPPER_OPTION* pOption);
	static BOOL	OptionToXml(FONT_MAPPER_OPTION* pOption, LPWSTR wszBuf, size_t cch);
};

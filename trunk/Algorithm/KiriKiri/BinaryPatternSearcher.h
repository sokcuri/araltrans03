#pragma once

class CBinaryPatternSearcher
{
private:
	CBinaryPatternSearcher(void);
	~CBinaryPatternSearcher(void);

public:

	static int Search( LPBYTE pBinData, UINT nBinSize, LPCTSTR cszBinPattern, int nAfter=0 );
};

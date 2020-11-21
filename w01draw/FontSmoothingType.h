#ifndef __FONTSMOOTHINGTYPE_H__
#define __FONTSMOOTHINGTYPE_H__
#pragma once

class CFontSmoothingType
{
public:
	CFontSmoothingType(void);
	~CFontSmoothingType(void);

	void SetClearType();
	void SetNoSmoothing();
private:
	BOOL bSmoothingEnabled;
	UINT nTypeOfFontSmoothing;

	void GetFontSmoothing();
	void ResetFontSmoothing();
};

#endif
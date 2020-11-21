#include "stdafx.h"
#include "FontSmoothingType.h"


CFontSmoothingType::CFontSmoothingType(void)
{
	bSmoothingEnabled = FALSE;

	GetFontSmoothing();
}


CFontSmoothingType::~CFontSmoothingType(void)
{
	ResetFontSmoothing();
}


void CFontSmoothingType::SetClearType()
{
	::SystemParametersInfo(SPI_GETFONTSMOOTHING, TRUE, 0, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
	::SystemParametersInfo(SPI_SETFONTSMOOTHINGTYPE, 0, (PVOID)FE_FONTSMOOTHINGCLEARTYPE, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);

}

void CFontSmoothingType::SetNoSmoothing()
{
	::SystemParametersInfo(SPI_GETFONTSMOOTHING, FALSE, 0, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
}

void CFontSmoothingType::GetFontSmoothing()
{
	::SystemParametersInfo(SPI_GETFONTSMOOTHING, 0, &bSmoothingEnabled, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
	if(bSmoothingEnabled)
	{
		::SystemParametersInfo(SPI_GETFONTSMOOTHINGTYPE, 0, &nTypeOfFontSmoothing, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
	}
}

void CFontSmoothingType::ResetFontSmoothing()
{
	::SystemParametersInfo(SPI_SETFONTSMOOTHING, bSmoothingEnabled, 0, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
	if(bSmoothingEnabled)
	{
		::SystemParametersInfo(SPI_SETFONTSMOOTHINGTYPE, 0, (PVOID)nTypeOfFontSmoothing, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
	}
}




#ifndef BROWSER_HTML_IMAGE_H
#define BROWSER_HTML_IMAGE_H

class	NoddyBrowser;
class	NoddyBrowserSection;

#include "HTMLElement.h"

enum
{
	BROWSER_IMG_STATE_NONE = 0,
	BROWSER_IMG_STATE_AWAITING_DOWNLOAD,
	BROWSER_IMG_STATE_DOWNLOADING,
	BROWSER_IMG_STATE_LOADED,
	BROWSER_IMG_STATE_NOT_AVAILABLE,
};

class	HTMLImageTarget
{
friend class HTMLImageFactory;
friend class HTMLImage;
public:
	HTMLImageTarget();

	static	void		FreeAllImages( void );
	char*	GetName() { return( m_szImageName ); }
	void	Load( char* );
protected:
	void	GetImageTexture( void );

	int		m_hTexture;

	char	m_szImageName[256];	
	char	m_szLocalFileName[256];	
	int		m_LoadState;

	int		m_ImageWidth;
	int		m_ImageHeight;
	float	m_fVMod;
	float	m_fUMod;

};

class	HTMLImage : public HTMLElement
{
public:
	HTMLImage();

	virtual	char*		ParseTag( char* );
	void		Display( void );
	virtual void	CalcSize( void );

protected:
	HTMLImageTarget*	m_pImageTarget;
	int		m_DrawWidth;
	int		m_DrawHeight;
};




#endif
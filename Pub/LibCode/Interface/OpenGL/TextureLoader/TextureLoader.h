#ifndef _TEXTURE_LOADER_H_
#define _TEXTURE_LOADER_H_

#include <windows.h>	
#include <glew.h>
// Header File For Windows
#include <gl\gl.h>											// Header File For The OpenGL32 Library
#include <gl\glu.h>											// Header File For The GLu32 Library
#include <olectl.h>											// Header File For The OLE Controls Library
#include <math.h>											// Header File For The Math Library
#include <STDIO.H>											// Header File For I/O Library

// Define Interface Data Types / Structures
//
typedef enum {
	txUnknown	= 0,	// images
	txBmp		= 1,
	txJpg		= 2,
	txPng		= 3,
	txTga		= 4,
	txGif		= 5,
	txIco		= 6,
	txEmf		= 7,
	txWmf		= 8,
	// add new ones at the end
} eglTexType;

typedef enum {
	txNoFilter	= 0,
	txBilinear	= 1,
	txTrilinear	= 2,
	// add new ones at the end
} eglTexFilterType;

typedef	struct
{
	GLuint		TextureID;									// Texture ID Used To Select A Texture
	eglTexType	TexType;									// Texture Format
	GLuint		Width;										// Image Width
	GLuint		Height;										// Image Height
	GLuint		Type;										// Image Type (GL_RGB, GL_RGBA)
	GLuint		Bpp;										// Image Color Depth In Bits Per Pixel
} glTexture;

typedef struct {
	float	s;
	float	t;
} _glTexturCord;


typedef struct {
	_glTexturCord TopRight;
	_glTexturCord TopLeft;
	_glTexturCord BottomLeft;
	_glTexturCord BottomRight;
} glTexturCordTable;

// Define Private Structurs
//

// Header
typedef struct {
	unsigned char ImgIdent;
	unsigned char ignored[ 1 ];
	unsigned char ImgType;
	unsigned char ignored2[ 9 ];
	unsigned char WidthLo;
	unsigned char WidthHi;
	unsigned char HeightLo;
	unsigned char HeightHi;
	unsigned char Bpp;
	unsigned char ignored3[ 1 ];
} _TGAHeader;

// define TextureLoader Class
//
class TextureLoader
{
public:
	// methods
						TextureLoader();
	virtual				~TextureLoader();
	void				SetAlphaMatch(GLboolean fEnabled, GLubyte RedAlphaMatch, GLubyte GreenAlphaMatch, GLubyte BlueAlphaMatch);
	void				SetHighQualityTextures(GLboolean fEnabled);
	void				SetMipMapping(GLboolean fEnabled);
	void				SetTextureFilter(eglTexFilterType type);

	int					LoadTextureFromDisk(const char *szFileName, glTexture *pglTexture);
	int					LoadTextureFromRam(unsigned char *pData, int Length, glTexture *pglTexture, eglTexType TexType);
	int					LoadTextureFromResource(unsigned int ResourceName, char *pResourceType, glTexture *pglTexture, eglTexType TexType);
	void				FreeTexture(glTexture *pglTexture);
	// variables

private:
	// methods
	int					BuildTexture(const char *szPathName, glTexture *pglTexture);
	int					LoadTGAFromDisk(char *pszFileName, glTexture *pglTexture);
	int					LoadPNG( const char *szPathName, glTexture *pglTexture);

	int					LoadJPG_GIFResource(char *pResourceName, char *pResourceType, glTexture *pglTexture);
	int					LoadTGAResource(char *pResourceName, char *pResourceType, glTexture *pglTexture);

	int					GenerateTexture(glTexture *pglTexture, GLubyte *pImgData);

	void				ExtensionFromFilename(const char *szFileName, char *szExtension);

	// variables
	GLboolean			m_fAlphaConversion;
	GLboolean			m_fHighQualityTextures;
	GLboolean			m_fMipMapping;
	eglTexFilterType	m_TextureFilterType;

	GLubyte				m_RedAlphaMatch;
	GLubyte				m_GreenAlphaMatch;
	GLubyte 			m_BlueAlphaMatch;
};

#endif // _TEXTURE_LOADER_H_


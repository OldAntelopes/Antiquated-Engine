/****************************************************************************
*                                                                           *
* Texture Loader                                                            *
*                                                                           *
* Originally Based on Jeff Molofee's IPicture Basecode                      *
* Extensions By Chris Leathley (http://members.iinet.net.au/~cleathley/)    *
*                                                                           *
*****************************************************************************
*                                                                           *
*  Loads  : BMP, EMF, GIF, ICO, JPG, WMF and TGA                            *
*  Source : Reads From Disk, Ram, Project Resource or the Internet          *
*  Extras : Images Can Be Any Width Or Height                               *
*           Low Quality Textures can be created                             *
*           Different Filter Level Support (None, Bilinear and Trilinear    *
*           Mipmapping Support                                              *
*                                                                           *
*****************************************************************************
*                                                                           *
*  Free To Use In Projects Of Your Own.  All I Ask For Is A Simple Greet    *
*  Or Mention of my site in your readme or the project itself :)            *
*                                                                           *
*****************************************************************************
*                                                                           *
* Revision History                                                          *
*                                                                           *
* Version 1.0 Released                                                      *
* Version 1.1 Added FreeTexture and LoadTextureFromResource                 *
*			  Added TexType to the glTexture Struction                      *
*             Optimisations to the Alpha Conversion Loop			        *
* Version 1.2 Added Support PSP8 TGA files...                               *
*             Single TGA file loader                                        *
* Version 1.3 Added Support for low quality textures..                      *
*             Added ScaleTGA function                                       *
* Version 1.4 Added Support for gluScaleImage                               *
*             Removed ScaleTGA (replacew with gluScaleImage)                *
*             Added TextureFilter and MipMapping Support                    *
*                                                                           *
****************************************************************************/

#include "TextureLoader.h"											// Our Header

#include <StandardDef.h>
#include <Interface.h>
#include <png.h>

// Constructor
//
TextureLoader::TextureLoader()
{
	// default to alpha matching BLACK
	SetAlphaMatch(TRUE, 0, 0, 255);										// Set the Alpha Matching State

	// default to full sized textures
	SetHighQualityTextures(TRUE);

	// no mipmap textures
	SetMipMapping(FALSE);

	// no texture filtering
//	SetTextureFilter(txBilinear);
	SetTextureFilter(txTrilinear);

	
}

// Destructor
//
TextureLoader::~TextureLoader()
{
}


// Set Alpha Matching State and Match Colour
//
void TextureLoader::SetAlphaMatch(GLboolean fEnabled, GLubyte RedAlphaMatch, GLubyte GreenAlphaMatch, GLubyte BlueAlphaMatch)
{
	m_fAlphaConversion	= fEnabled;
	// only set the colour match if the conversion is enabled
	if (fEnabled == TRUE)
	{
		m_RedAlphaMatch 	= RedAlphaMatch;
		m_GreenAlphaMatch 	= GreenAlphaMatch;
		m_BlueAlphaMatch 	= BlueAlphaMatch;
	}
}


// set the high quality texture flag
//
void TextureLoader::SetHighQualityTextures(GLboolean fEnabled)
{
	m_fHighQualityTextures = fEnabled;
}


// set the mipmapping flag
//
void TextureLoader::SetMipMapping(GLboolean fEnabled)
{
	m_fMipMapping = fEnabled;
}


// set the texture filtering flag
//
void TextureLoader::SetTextureFilter(eglTexFilterType type)
{
	m_TextureFilterType = type;
}


// Load A Texture from Disk (based on the current location of the executable)
//
int	TextureLoader::LoadTextureFromDisk(const char *szFileName, glTexture *pglTexture)
{
	char szFullPath[MAX_PATH+1];										// Full Path To Picture
	char szExtension[16];												// Extenstion of Picture

	if (strstr(szFileName, "http://"))									// If PathName Contains http:// Then...
	{
		strcpy(szFullPath, szFileName);									// Append The PathName To FullPath
	}
	else																// Otherwise... We Are Loading From A File
	{
		GetCurrentDirectory(MAX_PATH, szFullPath);						// Get Our Working Directory
		strcat(szFullPath, "\\");										// Append "\" After The Working Directory
		strcat(szFullPath, szFileName);									// Append The PathName
	}

	ExtensionFromFilename(szFileName, szExtension);

	// if the file is a TGA then use the TGA file loader
	if (lstrcmpi(szExtension, "tga") == 0)
	{
		return(LoadTGAFromDisk(szFullPath, pglTexture));				// Load TGA (Compressed/Uncompressed)
	}
	else if (lstrcmpi(szExtension, "png") == 0)
	{
		return(LoadPNG(szFullPath, pglTexture));				// Load TGA (Compressed/Uncompressed)
	}
	else
	{
		// else load BMP, JPG, GIF

		pglTexture->TexType = txUnknown;
		if (lstrcmpi(szExtension, "bmp") == 0)
		{
			pglTexture->TexType = txBmp;
		}
		else if ((lstrcmpi(szExtension, "jpg") == 0) ||
				 (lstrcmpi(szExtension, "jpeg") == 0) )
		{
			pglTexture->TexType = txJpg;
		}
		else if (lstrcmpi(szExtension, "gif") == 0)
		{
			pglTexture->TexType = txGif;
		}
		else if (lstrcmpi(szExtension, "ico") == 0)
		{
			pglTexture->TexType = txIco;
		}
		else if (lstrcmpi(szExtension, "emf") == 0)
		{
			pglTexture->TexType = txEmf;
		}
		else if (lstrcmpi(szExtension, "wmf") == 0)
		{
			pglTexture->TexType = txWmf;
		}
		else if (lstrcmpi(szExtension, "png") == 0)
		{
			pglTexture->TexType = txPng;
		}

		return(BuildTexture(szFullPath, pglTexture));					// Load BMP, JPG, GIF etc..
	}
}


// Load A Texture from RAM
//
int	TextureLoader::LoadTextureFromRam(unsigned char *pData, int Length, glTexture *pglTexture, eglTexType TexType)
{
	// it would be much easier to allocate some global memory and make a stream out of it but that
	// would require changing the TGA loading code to use RAM and not Disk access commands

	char szTempFileName[MAX_PATH+1];

	GetCurrentDirectory(MAX_PATH, szTempFileName);						// Get Our Working Directory
	lstrcat(szTempFileName, "glTEMP$$.tmp");

	HANDLE hFile = CreateFile(szTempFileName,
							  GENERIC_WRITE,
							  0,
							  NULL,
							  CREATE_ALWAYS,
							  FILE_ATTRIBUTE_TEMPORARY,
							  NULL);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		int	  rc;
		DWORD BytesWritten;

		WriteFile(hFile, pData, Length, &BytesWritten, NULL);			// Write The Data To disk (To A Temp File)

		CloseHandle(hFile);												// Close The File

		// set the texture type
		pglTexture->TexType = TexType;

		// if the file is a TGA then use the TGA file loader
		if (TexType == txTga)
		{
			rc = LoadTGAFromDisk(szTempFileName, pglTexture);
		}
		else
		{
			// else load BMP, JPG, GIF
			rc = BuildTexture(szTempFileName, pglTexture);
		}

		DeleteFile(szTempFileName);										// Delete The Temp File

		return (rc);													// Teturn State Of Texture Load
	}
	else
	{
		return FALSE;													// Return Failed
	}
}


// Load A Texture from the project Resource
//
int	TextureLoader::LoadTextureFromResource(unsigned int ResourceName, char *pResourceType, glTexture *pglTexture, eglTexType TexType)
{
	int	rc = FALSE;

	if ((TexType == txJpg) ||
		(TexType == txGif) )
	{
		// set the texture type
		pglTexture->TexType = TexType;

		rc = LoadJPG_GIFResource(MAKEINTRESOURCE(ResourceName), pResourceType, pglTexture);
	}

	if (TexType == txBmp)
	{
		// set the texture type
		pglTexture->TexType = TexType;

		// some bug with the bitmap resource loader..
		//rc = LoadJPG_GIFResource(MAKEINTRESOURCE(ResourceName), pResourceType, pglTexture);
		//rc = LoadBMPResource(pResourceName, pResourceType, pglTexture);
	}

	if (TexType == txTga)
	{
		// set the texture type
		pglTexture->TexType = TexType;

		rc = LoadTGAResource(MAKEINTRESOURCE(ResourceName), pResourceType, pglTexture);
	}

	return (rc);
}


// Free a Texture from openGL
//
void TextureLoader::FreeTexture(glTexture *pglTexture)
{
	glDeleteTextures(1, &pglTexture->TextureID);
}


int TextureLoader::LoadPNG( const char *szPathName, glTexture *pglTexture)
{
FILE* fp = ::fopen(szPathName, "rb");

        png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        png_infop info_ptr  = png_create_info_struct(png_ptr);

        if (::setjmp(png_jmpbuf(png_ptr)))
		{
		char	acErrorMsg[512];
			sprintf( acErrorMsg, "Error reading PNG: %s", szPathName );
			PANIC_IF(TRUE, acErrorMsg );
			return(-1);//Exit("liPNG had some error.");
        }
        
        png_init_io(png_ptr, fp);
        
        unsigned int sig_read = 0;
        png_set_sig_bytes(png_ptr, sig_read);
        png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND, NULL);
        
        
        png_uint_32 width, height; int bit_depth, color_type, interlace_type;
        png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, NULL, NULL);
      
        unsigned long row_bytes = png_get_rowbytes(png_ptr, info_ptr);
        unsigned char* data = (unsigned char*) ::malloc(row_bytes * height);

        png_bytepp row_pointers = png_get_rows(png_ptr, info_ptr);
    
        // Align bytes as OpenGL expects them.
        for (unsigned int i = 0; i < height; i++) {
            memcpy(data+(row_bytes * i), row_pointers[i], row_bytes);
        }
        
        // Clean up after the read, and free any memory allocated
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL /* png_infopp_NULL */);

        // Close the file
        ::fclose(fp);
    
        
        GLint internalFormat = 0;
        
        // Available PNG formats:
        switch (color_type) {
            case PNG_COLOR_TYPE_RGBA:
                internalFormat = GL_RGBA;
                break;
                
            case PNG_COLOR_TYPE_RGB:
                internalFormat = GL_RGB;
                break;
            case PNG_COLOR_TYPE_GRAY:
                internalFormat = GL_LUMINANCE;
                break;
                
            case PNG_COLOR_TYPE_GRAY_ALPHA:
                internalFormat = GL_LUMINANCE_ALPHA;
                break;
                
            default:
                // This probably means the switch case needs an addition.
                return(-1);//Exit("unsupported PNG color type: [%d].", color_type);
        }
        
        // Find a free texture index or "name".
        GLuint texture;
        glGenTextures(1, &texture);
//        GLError();

        // Find the index to global state
        glBindTexture(GL_TEXTURE_2D, texture);
//        GLError();

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        // uv are not in [0,1] for some models, they assume uvs are repeated.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        
        
        printf("Uploading %lu bytes to the GPU...", row_bytes * height);
        
        glTexImage2D(GL_TEXTURE_2D,                 // What (target)
                 0,                                 // Mip-map level
                 internalFormat,                    // Internal format
                 width,                             // Width
                 height,                            // Height
                 0,                                 // Border
                 internalFormat,                    // Format (how to use)
                 GL_UNSIGNED_BYTE,                  // Type   (how to intepret)
                 data);                             // Data
 //       GLError();
        
        printf(" done.\n");
      
        GLenum error = glGetError();
    
        if (error != GL_NO_ERROR) {
           return(-1);// Exit("Error uploading PNG texture %s to GPU. glError: 0x%04X\n", filename.c_str(), error);
        }
        
           
        // Main reason why we use PNG for this demo. Random models from the
        // internet come with random formats, and generating mipmaps may be
        // non-trivial. This will always work :)
        glGenerateMipmap(GL_TEXTURE_2D);
        
        
        // Data is on the GPU's RAM, release it from the CPU's RAM.
        ::free(data);
    
        return texture;
}

// Load BMP, GIF and JPG and Convert To A Texture
//
int TextureLoader::BuildTexture(const char *szPathName, glTexture *pglTexture)
{
	HDC			hdcTemp;												// The DC To Hold Our Bitmap
	HBITMAP		hbmpTemp;												// Holds The Bitmap Temporarily
	IPicture	*pPicture;												// IPicture Interface
	OLECHAR		wszPath[MAX_PATH+1];									// Full Path To Picture (WCHAR)
	long		lWidth;													// Width In Logical Units
	long		lHeight;												// Height In Logical Units
	long		lWidthPixels;											// Width In Pixels
	long		lHeightPixels;											// Height In Pixels
	GLint		glMaxTexDim;											// Holds Maximum Texture Size

	MultiByteToWideChar(CP_ACP, 0, szPathName, -1, wszPath, MAX_PATH);	// Convert From ASCII To Unicode

	VARIANT	xVariant;

	xVariant.vt = VT_LPSTR;
	xVariant.pcVal = (CHAR*)wszPath;
	HRESULT hr = OleLoadPicturePath(wszPath,							// Path
									NULL,								// punkCaller
									0,									// Reserved
									0,									// Reserved Transparent Colour
									IID_IPicture,						// riid of Inertface to return
									(void**)&pPicture);					// pointer to returned interface

	if(FAILED(hr))														// If Loading Failed
	{
		return FALSE;													// Return False
	}

	hdcTemp = CreateCompatibleDC(GetDC(0));								// Create The Windows Compatible Device Context
	if(!hdcTemp)														// Did Creation Fail?
	{
		pPicture->Release();											// Decrements IPicture Reference Count
		return FALSE;													// Return False (Failure)
	}

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &glMaxTexDim);					// Get Maximum Texture Size Supported

	pPicture->get_Width(&lWidth);										// Get IPicture Width (Convert To Pixels)
	lWidthPixels	= MulDiv(lWidth, GetDeviceCaps(hdcTemp, LOGPIXELSX), 2540);
	pPicture->get_Height(&lHeight);										// Get IPicture Height (Convert To Pixels)
	lHeightPixels	= MulDiv(lHeight, GetDeviceCaps(hdcTemp, LOGPIXELSY), 2540);

	// Resize Image To Closest Power Of Two
	if (lWidthPixels <= glMaxTexDim) // Is Image Width Less Than Or Equal To Cards Limit
	{
		lWidthPixels = 1 << (int)floor((log((double)lWidthPixels)/log(2.0f)) + 0.5f);
	}
	else  // Otherwise  Set Width To "Max Power Of Two" That The Card Can Handle
	{
		lWidthPixels = glMaxTexDim;
	}

	if (lHeightPixels <= glMaxTexDim) // Is Image Height Greater Than Cards Limit
	{
		lHeightPixels = 1 << (int)floor((log((double)lHeightPixels)/log(2.0f)) + 0.5f);
	}
	else  // Otherwise  Set Height To "Max Power Of Two" That The Card Can Handle
	{
		lHeightPixels = glMaxTexDim;
	}

	// if low quality textures then make them halve the size which saved 4 times the texture space
	if ((m_fHighQualityTextures == FALSE) && (lWidthPixels > 64))
	{
		lWidthPixels /= 2;
		lHeightPixels /= 2;
	}

	//	Create A Temporary Bitmap
	BITMAPINFO	bi = {0};												// The Type Of Bitmap We Request
	DWORD		*pBits = 0;												// Pointer To The Bitmap Bits

	bi.bmiHeader.biSize			= sizeof(BITMAPINFOHEADER);				// Set Structure Size
	bi.bmiHeader.biBitCount		= 32;									// 32 Bit
	bi.bmiHeader.biWidth		= lWidthPixels;							// Power Of Two Width
	bi.bmiHeader.biHeight		= lHeightPixels;						// Make Image Top Up (Positive Y-Axis)
	bi.bmiHeader.biCompression	= BI_RGB;								// RGB Encoding
	bi.bmiHeader.biPlanes		= 1;									// 1 Bitplane

	//	Creating A Bitmap This Way Allows Us To Specify Color Depth And Gives Us Imediate Access To The Bits
	hbmpTemp = CreateDIBSection(hdcTemp,
								&bi,
								DIB_RGB_COLORS,
								(void**)&pBits,
								0,
								0);

	if(!hbmpTemp)														// Did Creation Fail?
	{
		DeleteDC(hdcTemp);												// Delete The Device Context
		pPicture->Release();											// Decrements IPicture Reference Count
		return FALSE;													// Return False (Failure)
	}

	SelectObject(hdcTemp, hbmpTemp);									// Select Handle To Our Temp DC And Our Temp Bitmap Object

	// Render The IPicture On To The Bitmap
	pPicture->Render(hdcTemp,
					 0,
					 0,
					 lWidthPixels,
					 lHeightPixels,
					 0,
					 lHeight,
					 lWidth,
					 -lHeight,
					 0);

	// Convert From BGR To RGB Format And Add An Alpha Value Of 255
	if (m_fAlphaConversion == TRUE)
	{
		// it is important to remember that the colour order is different as we are reading in a DWORD
		// so the intel processor will read it in as little endian so the order is reversed (ABGR)
		DWORD AlphaMatch = (((m_RedAlphaMatch) | (m_GreenAlphaMatch << 8)) | (m_BlueAlphaMatch << 16));

		DWORD *pRGBA = (DWORD*)pBits;

		long NumPixels = lWidthPixels * lHeightPixels;
		while(NumPixels--)												// Loop Through All Of The Pixels
		{
			DWORD	tempRGB;

			tempRGB = *pRGBA;
			// swap red and blue over
			tempRGB = ((tempRGB & 0x00FF0000) >> 16) | (tempRGB & 0x0000FF00) | ((tempRGB & 0x000000FF) << 16);

			if (AlphaMatch != tempRGB)
			{
				tempRGB |= 0xFF000000;
			}
			else	// this is the pure blue alpha channel
			{
				// Make alpha black
				tempRGB = 0;
			}

			*pRGBA++ = tempRGB;
		}
	}
	else
	{
		BYTE *pPixel = (BYTE*)pBits;									// Grab The Current Pixel

		long NumPixels = lWidthPixels * lHeightPixels;
		while(NumPixels--)
		{

			BYTE  temp	= pPixel[0];									// Store 1st Color In Temp Variable (Blue)
			pPixel[0]	= pPixel[2];									// Move Red Value To Correct Position (1st)
			pPixel[2]	= temp;											// Move Temp Value To Correct Blue Position (3rd)
			pPixel[3]	= 255;											// Set The Alpha Value To 255
			pPixel		+= 4;
		}
	}

	pglTexture->Width = bi.bmiHeader.biWidth;
	pglTexture->Height = bi.bmiHeader.biHeight;
	pglTexture->Bpp = bi.bmiHeader.biBitCount;							// Image Color Depth In Bits Per Pixel
	pglTexture->Type = GL_RGBA;

	glGenTextures(1, &pglTexture->TextureID);							// Create The Texture

	// generate the texture using the filtering model selected
	(void)GenerateTexture(pglTexture, (BYTE *)pBits);

	DeleteObject(hbmpTemp);												// Delete The Object
	DeleteDC(hdcTemp);													// Delete The Device Context

	pPicture->Release();												// Decrements IPicture Reference Count

	return TRUE;														// Return True (All Good)
}


// Load a TGA file
//
int TextureLoader::LoadTGAFromDisk(char *pszFileName, glTexture *pglTexture)
{
	FILE		*fTGA;													// File pointer to texture file
	_TGAHeader	header;
	GLubyte		*pImgData;
	GLint		glMaxTexDim;											// Holds Maximum Texture Size

	fTGA = fopen(pszFileName, "rb");									// Open file for reading

	if(fTGA == NULL)													// If it didn't open....
	{
		return FALSE;													// Exit function
	}

	if(fread(&header, sizeof(_TGAHeader), 1, fTGA) == 0)				// Attempt to read 12 byte header from file
	{
		if(fTGA != NULL)												// Check to seeiffile is still open
		{
			fclose(fTGA);												// If it is, close it
		}
		return FALSE;													// Exit function
	}

	// Precalc some values from the header
	const unsigned int imageType		= header.ImgType;
	const unsigned int imageWidth		= header.WidthLo  + header.WidthHi  * 256;
	const unsigned int imageHeight		= header.HeightLo + header.HeightHi * 256;
	const unsigned int imageBytesPerPel	= header.Bpp / 8;
	const unsigned int imageSize		= imageWidth * imageHeight * imageBytesPerPel;

	// load up our texture information
	pglTexture->Width  = imageWidth;
	pglTexture->Height = imageHeight;
	pglTexture->Bpp	   = header.Bpp;

	if(pglTexture->Bpp == 24)											// If the BPP of the image is 24...
	{
		pglTexture->Type = GL_RGB;										// Set Image type to GL_RGB
	}
	else																// Else if its 32 BPP
	{
		pglTexture->Type = GL_RGBA;										// Set image type to GL_RGBA
	}

	// Validate header info
	if( ( imageType != 2 && imageType != 10 ) ||
	    ( imageWidth == 0 ) || ( imageHeight == 0 ) ||
		( imageBytesPerPel != 3 && imageBytesPerPel != 4 ) )
	{
		// invalid header, bomb out
		fclose( fTGA );
		return (FALSE);
	}

	// Allocate the memory for the image size
	pImgData = (GLubyte *)malloc(imageSize);

	if(pImgData == NULL)												// If no space was allocated
	{
		fclose(fTGA);													// Close the file
		return FALSE;													// Return failed
	}

	// Skip image ident field
	if( header.ImgIdent > 0 )
	{
		fseek(fTGA, header.ImgIdent, SEEK_CUR);
	}

	// un-compresses image ?
	if (imageType == 2)
	{
		if(fread(pImgData, 1, imageSize, fTGA) != imageSize)			// Attempt to read image data
		{
			if(pImgData != NULL)										// If imagedata has data in it
			{
				free(pImgData);											// Delete data from memory
			}

			fclose(fTGA);												// Close file
			return FALSE;												// Return failed
		}

		// Byte Swapping Optimized By Steve Thomas
		for(GLuint cswap = 0; cswap < imageSize; cswap += imageBytesPerPel)
		{
			pImgData[cswap] ^= pImgData[cswap+2] ^=
			pImgData[cswap] ^= pImgData[cswap+2];
		}
	}
	else
	{
		// compressed image
		GLuint pixelcount	= imageHeight * imageWidth;					// Nuber of pixels in the image
		GLuint currentpixel	= 0;										// Current pixel being read
		GLuint currentbyte	= 0;										// Current byte
		GLubyte * colorbuffer = (GLubyte *)malloc(imageBytesPerPel);	// Storage for 1 pixel

		do
		{
			GLubyte chunkheader = 0;										// Storage for "chunk" header

			if(fread(&chunkheader, sizeof(GLubyte), 1, fTGA) == 0)			// Read in the 1 byte header
			{
				if(fTGA != NULL)											// If file is open
				{
					fclose(fTGA);											// Close file
				}
				if(pImgData != NULL)										// If there is stored image data
				{
					free(pImgData);											// Delete image data
				}
				return FALSE;												// Return failed
			}

			if(chunkheader < 128)											// If the ehader is < 128, it means the that is the number of RAW color packets minus 1
			{																// that follow the header
				chunkheader++;												// add 1 to get number of following color values
				for(short counter = 0; counter < chunkheader; counter++)	// Read RAW color values
				{
					if(fread(colorbuffer, 1, imageBytesPerPel, fTGA) != imageBytesPerPel) // Try to read 1 pixel
					{
						if(fTGA != NULL)									// See if file is open
						{
							fclose(fTGA);									// If so, close file
						}

						if(colorbuffer != NULL)							// See if colorbuffer has data in it
						{
							free(colorbuffer);							// If so, delete it
						}

						if(pImgData != NULL)							// See if there is stored Image data
						{
							free(pImgData);								// If so, delete it too
						}

						return FALSE;									// Return failed
					}
																		// write to memory
					pImgData[currentbyte		] = colorbuffer[2];		// Flip R and B vcolor values around in the process
					pImgData[currentbyte + 1	] = colorbuffer[1];
					pImgData[currentbyte + 2	] = colorbuffer[0];

					if (imageBytesPerPel == 4)							// if its a 32 bpp image
					{
						pImgData[currentbyte + 3] = colorbuffer[3];		// copy the 4th byte
					}

					currentbyte += imageBytesPerPel;					// Increase thecurrent byte by the number of bytes per pixel
					currentpixel++;										// Increase current pixel by 1

					if(currentpixel > pixelcount)						// Make sure we havent read too many pixels
					{
						if(fTGA != NULL)								// If there is a file open
						{
							fclose(fTGA);								// Close file
						}

						if(colorbuffer != NULL)							// If there is data in colorbuffer
						{
							free(colorbuffer);							// Delete it
						}

						if(pImgData != NULL)							// If there is Image data
						{
							free(pImgData);								// delete it
						}

						return FALSE;									// Return failed
					}
				}
			}
			else														// chunkheader > 128 RLE data, next color reapeated chunkheader - 127 times
			{
				chunkheader -= 127;										// Subtract 127 to get rid of the ID bit
				if(fread(colorbuffer, 1, imageBytesPerPel, fTGA) != imageBytesPerPel)	// Attempt to read following color values
				{
					if(fTGA != NULL)									// If thereis a file open
					{
						fclose(fTGA);									// Close it
					}

					if(colorbuffer != NULL)								// If there is data in the colorbuffer
					{
						free(colorbuffer);								// delete it
					}

					if(pImgData != NULL)								// If there is image data
					{
						free(pImgData);									// delete it
					}

					return FALSE;										// return failed
				}

				for(short counter = 0; counter < chunkheader; counter++)// copy the color into the image data as many times as dictated
				{														// by the header
					pImgData[currentbyte		] = colorbuffer[2];		// switch R and B bytes areound while copying
					pImgData[currentbyte + 1	] = colorbuffer[1];
					pImgData[currentbyte   + 2	] = colorbuffer[0];

					if(imageBytesPerPel == 4)							// If TGA images is 32 bpp
					{
						pImgData[currentbyte + 3] = colorbuffer[3];		// Copy 4th byte
					}

					currentbyte += imageBytesPerPel;					// Increase current byte by the number of bytes per pixel
					currentpixel++;										// Increase pixel count by 1

					if(currentpixel > pixelcount)						// Make sure we havent written too many pixels
					{
						if(fTGA != NULL)								// If there is a file open
						{
							fclose(fTGA);								// Close file
						}

						if(colorbuffer != NULL)							// If there is data in colorbuffer
						{
							free(colorbuffer);							// Delete it
						}

						if(pImgData != NULL)							// If there is Image data
						{
							free(pImgData);								// delete it
						}

						return FALSE;									// Return failed
					}
				} // for(counter)
			} // if(chunkheader)
		}
		while(currentpixel < pixelcount);								// Loop while there are still pixels left
	} // if (imageType == 2)

	fclose (fTGA);														// Close the TGA file

	/*
	** Scale Image to be a power of 2
	*/

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &glMaxTexDim);					// Get Maximum Texture Size Supported

	int lWidthPixels  = imageWidth;
	int lHeightPixels = imageHeight;

	// Resize Image To Closest Power Of Two
	if (lWidthPixels <= glMaxTexDim) // Is Image Width Less Than Or Equal To Cards Limit
		lWidthPixels = 1 << (int)floor((log((double)lWidthPixels)/log(2.0f)) + 0.5f);
	else  // Otherwise  Set Width To "Max Power Of Two" That The Card Can Handle
		lWidthPixels = glMaxTexDim;

	if (lHeightPixels <= glMaxTexDim) // Is Image Height Greater Than Cards Limit
		lHeightPixels = 1 << (int)floor((log((double)lHeightPixels)/log(2.0f)) + 0.5f);
	else  // Otherwise  Set Height To "Max Power Of Two" That The Card Can Handle
		lHeightPixels = glMaxTexDim;

	// if low quality textures then make them halve the size which saved 4 times the texture space
	if ((m_fHighQualityTextures == FALSE) && (lWidthPixels > 64))
	{
		lWidthPixels /= 2;
		lHeightPixels /= 2;
	}

	// if the size needs to change, the rescale the raw image data
	if ( (lWidthPixels  != (int)imageWidth)	&&
		 (lHeightPixels != (int)imageHeight) )
	{
		// allocated the some memory for the new texture
		GLubyte	*pNewImgData = (GLubyte *)malloc(lWidthPixels * lHeightPixels * imageBytesPerPel);

		GLenum format;
		if (imageBytesPerPel == 4)
		{
			format = GL_RGBA;
		}
		else
		{
			format = GL_RGB;
		}

		gluScaleImage(format, imageWidth, imageHeight, GL_UNSIGNED_BYTE, pImgData,
							  lWidthPixels, lHeightPixels, GL_UNSIGNED_BYTE, pNewImgData);

		// free the original image data
		free(pImgData);

		// old becomes new..
		pImgData = pNewImgData;

		// update our texture structure
		pglTexture->Width  = lWidthPixels;
		pglTexture->Height = lHeightPixels;
	}

	// Typical Texture Generation Using Data From The TGA loader
	glGenTextures(1, &pglTexture->TextureID);						// Create The Texture

	// generate the texture using the filtering model selected
	(void)GenerateTexture(pglTexture, (BYTE *)pImgData);

	// free the memory allocated
	free(pImgData);

	return TRUE;														// All went well, continue on
}


// extract the extension from the specified filename
//
int TextureLoader::LoadJPG_GIFResource(char *pResourceName, char *pResourceType, glTexture *pglTexture)
{
	HDC			hdcTemp;												// The DC To Hold Our Bitmap
	HBITMAP		hbmpTemp;												// Holds The Bitmap Temporarily
	IPicture	*pPicture;												// IPicture Interface
	long		lWidth;													// Width In Logical Units
	long		lHeight;												// Height In Logical Units
	long		lWidthPixels;											// Width In Pixels
	long		lHeightPixels;											// Height In Pixels
	GLint		glMaxTexDim ;											// Holds Maximum Texture Size

	// from resources
	HRSRC	hRes;
	hRes = FindResource(NULL, pResourceName, pResourceType);

	DWORD	dwDataSize	= SizeofResource(NULL,hRes);
	HGLOBAL	hGlob		= LoadResource(NULL,hRes);
	LPVOID	pData		= LockResource(hGlob);

	HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, dwDataSize);
	void* pData2 = GlobalLock(hGlobal);
	memcpy(pData2, pData, dwDataSize);
	GlobalUnlock(hGlobal);
	UnlockResource(hGlob);
	FreeResource(hGlob);

	IStream* pStream = NULL;
	pPicture = NULL;
	HRESULT hr;

	if (CreateStreamOnHGlobal(hGlobal, TRUE, &pStream) == S_OK)
	{
		hr = OleLoadPicture(pStream, 0, 0, IID_IPicture, (void**)&pPicture);
		pStream->Release();
	}

	if(FAILED(hr))														// If Loading Failed
		return FALSE;													// Return False

	hdcTemp = CreateCompatibleDC(GetDC(0));								// Create The Windows Compatible Device Context
	if(!hdcTemp)														// Did Creation Fail?
	{
		pPicture->Release();											// Decrements IPicture Reference Count
		return FALSE;													// Return False (Failure)
	}

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &glMaxTexDim);					// Get Maximum Texture Size Supported

	pPicture->get_Width(&lWidth);										// Get IPicture Width (Convert To Pixels)
	lWidthPixels	= MulDiv(lWidth, GetDeviceCaps(hdcTemp, LOGPIXELSX), 2540);
	pPicture->get_Height(&lHeight);										// Get IPicture Height (Convert To Pixels)
	lHeightPixels	= MulDiv(lHeight, GetDeviceCaps(hdcTemp, LOGPIXELSY), 2540);

	// Resize Image To Closest Power Of Two
	if (lWidthPixels <= glMaxTexDim) // Is Image Width Less Than Or Equal To Cards Limit
		lWidthPixels = 1 << (int)floor((log((double)lWidthPixels)/log(2.0f)) + 0.5f);
	else  // Otherwise  Set Width To "Max Power Of Two" That The Card Can Handle
		lWidthPixels = glMaxTexDim;

	if (lHeightPixels <= glMaxTexDim) // Is Image Height Greater Than Cards Limit
		lHeightPixels = 1 << (int)floor((log((double)lHeightPixels)/log(2.0f)) + 0.5f);
	else  // Otherwise  Set Height To "Max Power Of Two" That The Card Can Handle
		lHeightPixels = glMaxTexDim;

	// if low quality textures then make them halve the size which saved 4 times the texture space
	if ((m_fHighQualityTextures == FALSE) && (lWidthPixels > 64))
	{
		lWidthPixels /= 2;
		lHeightPixels /= 2;
	}

	//	Create A Temporary Bitmap
	BITMAPINFO	bi = {0};												// The Type Of Bitmap We Request
	DWORD		*pBits = 0;												// Pointer To The Bitmap Bits

	bi.bmiHeader.biSize			= sizeof(BITMAPINFOHEADER);				// Set Structure Size
	bi.bmiHeader.biBitCount		= 32;									// 32 Bit
	bi.bmiHeader.biWidth		= lWidthPixels;							// Power Of Two Width
	bi.bmiHeader.biHeight		= lHeightPixels;						// Make Image Top Up (Positive Y-Axis)
	bi.bmiHeader.biCompression	= BI_RGB;								// RGB Encoding
	bi.bmiHeader.biPlanes		= 1;									// 1 Bitplane

	//	Creating A Bitmap This Way Allows Us To Specify Color Depth And Gives Us Imediate Access To The Bits
	hbmpTemp = CreateDIBSection(hdcTemp, &bi, DIB_RGB_COLORS, (void**)&pBits, 0, 0);

	if(!hbmpTemp)														// Did Creation Fail?
	{
		DeleteDC(hdcTemp);												// Delete The Device Context
		pPicture->Release();											// Decrements IPicture Reference Count
		return FALSE;													// Return False (Failure)
	}

	SelectObject(hdcTemp, hbmpTemp);									// Select Handle To Our Temp DC And Our Temp Bitmap Object

	// Render The IPicture On To The Bitmap
	pPicture->Render(hdcTemp, 0, 0, lWidthPixels, lHeightPixels, 0, lHeight, lWidth, -lHeight, 0);

	// Convert From BGR To RGB Format And Add An Alpha Value Of 255
	if ((m_fAlphaConversion == TRUE) && (pglTexture->TexType != txJpg))
	{
		// it is important to remember that the colour order is different as we are reading in a DWORD
		// so the intel processor will read it in as little endian so the order is reversed (ABGR)
		DWORD AlphaMatch = (((m_RedAlphaMatch) | (m_GreenAlphaMatch << 8)) | (m_BlueAlphaMatch << 16));

		DWORD *pRGBA = (DWORD*)pBits;

		long NumPixels = lWidthPixels * lHeightPixels;
		while(NumPixels--)												// Loop Through All Of The Pixels
		{
			DWORD	tempRGB;

			tempRGB = *pRGBA;
			// swap red and blue over
			tempRGB = ((tempRGB & 0x00FF0000) >> 16) | (tempRGB & 0x0000FF00) | ((tempRGB & 0x000000FF) << 16);

			if (AlphaMatch != tempRGB)
			{
				tempRGB |= 0xFF000000;
			}

			*pRGBA++ = tempRGB;
		}
	}
	else
	{
		BYTE *pPixel = (BYTE*)pBits;									// Grab The Current Pixel

		long NumPixels = lWidthPixels * lHeightPixels;
		while(NumPixels--)
		{

			BYTE  temp	= pPixel[0];									// Store 1st Color In Temp Variable (Blue)
			pPixel[0]	= pPixel[2];									// Move Red Value To Correct Position (1st)
			pPixel[2]	= temp;											// Move Temp Value To Correct Blue Position (3rd)
			pPixel[3]	= 255;											// Set The Alpha Value To 255
			pPixel		+= 4;
		}
	}

	pglTexture->Width = bi.bmiHeader.biWidth;
	pglTexture->Height = bi.bmiHeader.biHeight;
	pglTexture->Bpp = bi.bmiHeader.biBitCount;							// Image Color Depth In Bits Per Pixel
	pglTexture->Type = GL_RGBA;

	glGenTextures(1, &pglTexture->TextureID);							// Create The Texture

	// generate the texture using the filtering model selected
	(void)GenerateTexture(pglTexture, (BYTE *)pBits);

	DeleteObject(hbmpTemp);												// Delete The Object
	DeleteDC(hdcTemp);													// Delete The Device Context

	pPicture->Release();												// Decrements IPicture Reference Count

	return TRUE;														// Return True (All Good)
}


int	TextureLoader::LoadTGAResource(char *pResourceName, char *pResourceType, glTexture *pglTexture)
{
	// from resources
	HRSRC	hRes;
	hRes = FindResource(NULL, pResourceName, pResourceType);

	DWORD	dwDataSize	= SizeofResource(NULL,hRes);
	HGLOBAL	hGlob		= LoadResource(NULL,hRes);
	LPVOID	pData		= LockResource(hGlob);

	// allocate some global memory and copy the resource data into it..
	HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, dwDataSize);
	void* pData2 = GlobalLock(hGlobal);
	memcpy(pData2, pData, dwDataSize);

	// unlock and free the resource
	GlobalUnlock(hGlobal);
	UnlockResource(hGlob);
	FreeResource(hGlob);

	// load it
	int rc = LoadTextureFromRam((unsigned char *)pData2, dwDataSize, pglTexture, txTga);

	// free our global memory (16 bit os's only)
	GlobalFree(hGlobal);

	return rc;
}


// Set the Texture parameters to match the type of filtering we want.
//
int TextureLoader::GenerateTexture(glTexture *pglTexture, GLubyte *pImgData)
{
	int result = 0;

	int	components;
	// set the bytes per pixel
	if (pglTexture->Type == GL_RGBA)
	{
		components = 4;
	}
	else
	{
		components = 3;
	}

	// Typical Texture Generation Using Data From The Bitmap
	glBindTexture(GL_TEXTURE_2D, pglTexture->TextureID);				// Bind To The Texture ID

	// the texture wraps over at the edges (repeat)
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

	BOOL	Mipping;

	switch(m_TextureFilterType)
	{
		default:
		case txNoFilter:	glPixelStorei(GL_UNPACK_ALIGNMENT, 1 );
							glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
							glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
							Mipping = FALSE;
							break;

		case txBilinear:	if (m_fMipMapping == FALSE)
							{
								glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
								glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
								// set the build type flag
								Mipping = FALSE;
							}
							else
							{
								glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
								glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
								Mipping = TRUE;
							}
							break;

		case txTrilinear:	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
							glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
							// always mip mapping for trilinear
							Mipping = TRUE;
							break;
	}


	// crank out the texture
	if (Mipping == FALSE)
	{
		glTexImage2D(GL_TEXTURE_2D,
					 0,
					 components,
					 pglTexture->Width,
					 pglTexture->Height,
					 0,
					 pglTexture->Type,
					 GL_UNSIGNED_BYTE,
					 pImgData);
	}
	else
	{
		// Build Mipmaps (builds different versions of the picture for distances - looks better)
		result = gluBuild2DMipmaps(GL_TEXTURE_2D,
								   components,
								   pglTexture->Width,
								   pglTexture->Height,
								   pglTexture->Type,
								   GL_UNSIGNED_BYTE,
								   pImgData);
	}

	return 0;
}


// extract the extension from the specified filename
//
void TextureLoader::ExtensionFromFilename(const char *szFileName, char *szExtension)
{
	int len = lstrlen(szFileName);
	int begin;

	for ( begin=len;begin>=0;begin--)
	{
		if (szFileName[begin] == '.')
		{
			begin++;
			break;
		}
	}

	if (begin <= 0)
	{
		szExtension[0] = '\0';
	}
	else
	{
		lstrcpy(szExtension, &szFileName[begin]);
	}
}


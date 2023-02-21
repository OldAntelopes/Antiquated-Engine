#ifndef TRACTOR_STD_DEF_H
#define TRACTOR_STD_DEF_H
#ifdef __cplusplus
extern "C"
{
#endif

#ifdef MEMLEAK_DEBUG
#define	_CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include "crtdbg.h"
#else
#include <stdlib.h>
#endif
#include <string.h>
#include <stdio.h>

#ifdef I3D_PLATFORM_S3E
#define MARMALADE
#endif

		//------------Start of windows file includes
// All the stuff here is stuff from windows.h and other windows header files that we need to compile
// Its likely to be bad practice to copy these all, but it saves including all those windows header
// files globally and helps link times enormously.
#ifndef __WINE_WINDEF_H
#ifndef WINVER
#define WINVER 0x0400
#ifndef NO_STRICT
#ifndef STRICT
#define STRICT 1
#endif
#endif /* NO_STRICT */
// Win32 defines _WIN32 automatically,
// but Macintosh doesn't, so if we are using
// Win32 Functions, we must do it here
#ifdef _MAC
#ifndef _WIN32
#define _WIN32
#endif
#endif //_MAC
#ifdef __cplusplus
extern "C" {
#endif
#ifndef WINVER
#define WINVER 0x0500
#endif /* WINVER */
/*
 * BASETYPES is defined in ntdef.h if these types are already defined
 */
#ifndef BASETYPES
#define BASETYPES
typedef unsigned long ULONG;
typedef ULONG *PULONG;
typedef unsigned long long u64;
typedef unsigned short USHORT;
typedef USHORT *PUSHORT;
typedef unsigned char UCHAR;
typedef UCHAR *PUCHAR;
typedef char *PSZ;
#endif  /* !BASETYPES */
#define MAX_PATH          260
#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif
#ifndef FALSE
#define FALSE               0
#endif
#ifndef TRUE
#define TRUE                1
#endif
#ifndef IN
#define IN
#endif
#ifndef OUT
#define OUT
#endif
#ifndef OPTIONAL
#define OPTIONAL
#endif
#undef far
#undef near
#undef pascal
#define far
#define near
#if (!defined(_MAC)) && ((_MSC_VER >= 800) || defined(_STDCALL_SUPPORTED))
#define pascal __stdcall
#else
#define pascal
#endif
#if defined(DOSWIN32) || defined(_MAC)
#define cdecl _cdecl
#ifndef CDECL
#define CDECL _cdecl
#endif
#else 
#define cdecl
#ifndef CDECL
#define CDECL
#endif
#endif
#ifdef _MAC
#define CALLBACK    PASCAL
#define WINAPI      CDECL
#define WINAPIV     CDECL
#define APIENTRY    WINAPI
#define APIPRIVATE  CDECL
#ifdef _68K_
#define PASCAL      __pascal
#else
#define PASCAL
#endif
#elif (_MSC_VER >= 800) || defined(_STDCALL_SUPPORTED)
#define CALLBACK    __stdcall
#define WINAPI      __stdcall
#define WINAPIV     __cdecl
#define APIENTRY    WINAPI
#define APIPRIVATE  __stdcall
#define PASCAL      __stdcall
#else
#define CALLBACK
#define WINAPI
#define WINAPIV
#define APIENTRY    WINAPI
#define APIPRIVATE
#define PASCAL      pascal
#endif
#undef FAR
#undef NEAR
#define FAR                 far
#define NEAR                near
#ifndef CONST
#define CONST               const
#endif
typedef unsigned long       DWORD;
typedef int                 BOOL;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef float               FLOAT;
typedef FLOAT               *PFLOAT;
typedef BOOL near           *PBOOL;
typedef BOOL far            *LPBOOL;
typedef BYTE near           *PBYTE;
typedef BYTE far            *LPBYTE;
typedef int near            *PINT;
typedef int far             *LPINT;
typedef WORD near           *PWORD;
typedef WORD far            *LPWORD;
typedef long far            *LPLONG;
typedef DWORD near          *PDWORD;
typedef DWORD far           *LPDWORD;
typedef void far            *LPVOID;
typedef CONST void far      *LPCVOID;
typedef int                 INT;
typedef unsigned int        UINT;
typedef unsigned int        *PUINT;
typedef long                 LONG;
/* Types use for passing & returning polymorphic values */
typedef UINT WPARAM;
typedef LONG LPARAM;
typedef LONG LRESULT;
#ifndef NOMINMAX
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif
#endif  /* NOMINMAX */
#define MAKEWORD(a, b)      ((WORD)(((BYTE)(a)) | ((WORD)((BYTE)(b))) << 8))
#define MAKELONG(a, b)      ((LONG)(((WORD)(a)) | ((DWORD)((WORD)(b))) << 16))
#define LOWORD(l)           ((WORD)(l))
#define HIWORD(l)           ((WORD)(((DWORD)(l) >> 16) & 0xFFFF))
#define LOBYTE(w)           ((BYTE)(w))
#define HIBYTE(w)           ((BYTE)(((WORD)(w) >> 8) & 0xFF))
#ifdef STRICT
typedef void *HANDLE;
#define DECLARE_HANDLE(name) struct name##__ { int unused; }; typedef struct name##__ *name
#else
typedef PVOID HANDLE;
#define DECLARE_HANDLE(name) typedef HANDLE name
#endif
typedef HANDLE *PHANDLE;
#ifndef WIN_INTERNAL
DECLARE_HANDLE            (HWND);
DECLARE_HANDLE            (HHOOK);
#ifdef WINABLE
DECLARE_HANDLE            (HEVENT);
#endif
#endif


#define ZeroMemory(pb,cb)           memset((pb),0,(cb))
#define FillMemory(pb,cb,b)         memset((pb),(b),(cb))
#define CopyMemory(pbDst,pbSrc,cb)  do                              \
                                    {                               \
                                        size_t _cb = (size_t)(cb);  \
                                        if (_cb)                    \
                                            memcpy(pbDst,pbSrc,_cb);\
                                    } while (FALSE)
#define MoveMemory(pbDst,pbSrc,cb)  memmove((pbDst),(pbSrc),(cb))
#ifndef HRESULT
typedef LONG HRESULT;
#endif
#define UNALIGNED
#ifndef tagRECT
typedef struct tagRECT
{
    LONG    left;
    LONG    top;
    LONG    right;
    LONG    bottom;
} RECT, *PRECT, NEAR *NPRECT, FAR *LPRECT;
#endif
#ifdef __cplusplus
}
#endif
#endif //ifndef WINVER

#endif //ifndef __WINE_WINDEF_H
	//------------End of windows file includes
#ifndef _WINSOCKAPI_
#ifdef WIN32
struct in_addr {
        union {
                struct { BYTE s_b1,s_b2,s_b3,s_b4; } S_un_b;
                struct { WORD s_w1,s_w2; } S_un_w;
                DWORD S_addr;
        } S_un;
#define s_addr  S_un.S_addr
                                /* can be used for most tcp & ip code */
#define s_host  S_un.S_un_b.s_b2
                                /* host on imp */
#define s_net   S_un.S_un_b.s_b1
                                /* network */
#define s_imp   S_un.S_un_w.s_w2
                                /* imp */
#define s_impno S_un.S_un_b.s_b4
                                /* imp # */
#define s_lh    S_un.S_un_b.s_b3
                                /* logical host */
};
#define s_addr  S_un.S_addr
struct sockaddr_in {
        short   sin_family;
        WORD sin_port;
        struct  in_addr sin_addr;
        char    sin_zero[8];
};
#endif	// def WIN32
#endif	// ndef _WINSOCKAPI_
/**********************************************
 *********			Defines			***********
 **********************************************/
#define		MAX_PLAYERS_ON_CLIENT	128	// If this needs go above 255, bPlayerID in messaging will need expanding
// Our custom FVF, which describes our custom vertex structure
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE|D3DFVF_TEX2|D3DFVF_TEXCOORDSIZE2(0)|D3DFVF_TEXCOORDSIZE3(1))
#define D3DFVF_CUSTOMVERTEXUV2 (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE|D3DFVF_TEX1|D3DFVF_TEX2)
#define D3DFVF_CUSTOMVERTEXUV3 (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE|D3DFVF_TEX1|D3DFVF_TEX2|D3DFVF_TEX3)
#define D3DFVF_CUSTOMVERTEXUV4 (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE|D3DFVF_TEX1|D3DFVF_TEX2|D3DFVF_TEX3|D3DFVF_TEX4)

#define		NOTFOUND			-1
#ifndef PI
#define		PI					3.1415926535f
#define		Pi					3.1415926535f
#endif
#define		HALFPI				(PI*0.5f)
#define		PIANDHALF			(PI*1.5f)
#define		TwoPi				(Pi*2.0f)
#define		TWOPI				(Pi*2.0f)
#define		INCHES_PER_METER	39.3700787	//gotta verify that one fer me, please. -- Guy
#define		METERS_PER_INCH		1/INCHES_PER_METER
#define		SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#define		SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }
#define		SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#define		SAFE_FREE(p)	{ if(p) { free(p);  (p)=NULL; } }

//#define LARGE_MAP_SIZE_TEST

#ifdef _MSC_VER
#define UNI_INLINE	_inline
#else
#define UNI_INLINE	inline
#endif

extern int		gnHeightmapWidth;
extern int		gnHeightmapShift;
extern int		gnHeightmapMask;

UNI_INLINE int		INDEX( int x, int y ) { return( ((y&gnHeightmapMask)<< gnHeightmapShift)+(x&gnHeightmapMask) ); }
UNI_INLINE int		MAPINDEX( int x, int y ) { return( ((y&gnHeightmapMask)<< gnHeightmapShift)+(x&gnHeightmapMask) ); }
UNI_INLINE int		XFROMINDEX( int index ) { return( index & gnHeightmapMask ); }
UNI_INLINE int		YFROMINDEX( int index ) { return( index >> gnHeightmapShift ); }

extern int		gnLandscapeWorldHeightToHeightmapScale;


/**********************************************
 *********			Structures		***********
 **********************************************/
#ifndef MIT_TYPES
#define MIT_TYPES
#ifndef _SYS_TYPES_H_
typedef unsigned short			ushort;
typedef unsigned int			uint;
#endif //not _SYS_TYPES_H_
typedef unsigned long long		u64;
typedef unsigned long			ulong;
typedef unsigned char			uchar;
#ifndef __WINE_RPCNDR_H
typedef unsigned char			byte;
#endif //not __WINE_RPCNDR_H
#endif	//MIT_TYPES

typedef unsigned long long		u64;

#ifndef TRACTOR_ENGINE_H
typedef struct
{
   float	x;
   float	y;
   float	z;
} VECT;

typedef struct
{
	float	x;
	float	y;
	float	z;
	float	w;
} ENGINEQUATERNION;

#ifndef D3DMATRIX_DEFINED
typedef struct _D3DMATRIX {
    union {
        struct {
            float        _11, _12, _13, _14;
            float        _21, _22, _23, _24;
            float        _31, _32, _33, _34;
            float        _41, _42, _43, _44;

        };
        float m[4][4];
    };
} D3DMATRIX;
#define D3DMATRIX_DEFINED
#endif

#define	ENGINEMATRIX	D3DMATRIX

#endif

typedef VECT		MVECT;

#ifndef FLTPI
#define	FLTPI					3.1415926535f

#define	A10		(FLTPI/18.0f)
#define	A30		(FLTPI*0.166666667f)
#define	A45		(FLTPI*0.25f)
#define	A60		(FLTPI*0.333333333f)
#define	A90		(FLTPI*0.5f)
#define	A180	(FLTPI)
#define	A270	(FLTPI*1.5f)
#define	A360	(FLTPI*2.0f)
#endif

typedef struct
{
	ushort	uwItemNumber;
	ushort	uwQuantity;
	ushort	b4PurchasedFlag:4,
			b4Quality:4,
			b4Lives:4,
			b4Param4:4;
	ushort	uwProductDamageField;
	int		nSellPrice;
	int		nBuyPrice;
	ulong	ulLastProduced;
} AN_ITEM;

extern	int		MyRand( void );

#include "CodeUtil.h"
#include "System.h"	// This file (should) include all the 'main' system specific functions

extern void	DebugPrint( int, const char*, ... );


#ifdef __cplusplus
}
#endif

 //We'd need to replace those structs instead, but for now...
 #ifndef WIN32
 typedef struct {
     FLOAT x;
     FLOAT y;
     FLOAT z;
 } D3DXVECTOR3;
 
 typedef DWORD D3DCOLOR;
 
 //Structures for the heightmap
 typedef struct { 
   WORD    bfType; 
   DWORD   bfSize; 
   WORD    bfReserved1; 
   WORD    bfReserved2; 
   DWORD   bfOffBits; 
 } BITMAPFILEHEADER;
 
 typedef struct {
   DWORD  biSize; 
   LONG   biWidth; 
   LONG   biHeight; 
   WORD   biPlanes; 
   WORD   biBitCount; 
   DWORD  biCompression; 
   DWORD  biSizeImage; 
   LONG   biXPelsPerMeter; 
   LONG   biYPelsPerMeter; 
   DWORD  biClrUsed; 
   DWORD  biClrImportant; 
 } BITMAPINFOHEADER;
 
 typedef struct {
   BYTE    rgbBlue; 
   BYTE    rgbGreen; 
   BYTE    rgbRed; 
   BYTE    rgbReserved; 
 } RGBQUAD;
 #endif        // ndef WIN32

#endif

#ifndef INTERFACE_TEXTURED_POLY_LISTS_H
#define INTERFACE_TEXTURED_POLY_LISTS_H


#ifdef __cplusplus
extern "C"				// All interfaces use a C-linkage
{
#endif

#define		MAX_DIFFERENT_TEXTURED_OVERLAYS		64
#define		MAX_VERTS_IN_SINGLE_OVERLAY			600

enum
{
	TEX_OVLY_NONE = 0,
	TEX_OVLY_RECT,
	TEX_OVLY_TRI,
	TEX_OVLY_SPRITE,
};

struct TEXTURED_RECT_DEF
{
	BYTE	nType;
	BYTE	bPad;
	short	 nX;
	short	 nY;
	short	nWidth;
	short	nHeight;
	short	nX2;
	short	nY2;

	uint32	ulCol;
	float	fU1;
	float	fU2;
	union {	float	fU3;	float fRot; };

	float	fV1;
	float	fV2;
	float	fV3;

	void*	pNext;

} ;



extern void InterfaceInitTexturedPolyLists( void );
extern void InterfaceTexturedPolyListsDraw( int nLayer );
extern void	InterfaceTexturedPolyListsReset( void );
	
extern void InterfaceFreeTexturedPolyLists( void );

extern void* InterfaceTexturedOverlaysGetPlatformMaterial( int nOverlayNum );



#ifdef __cplusplus
}
#endif



#endif

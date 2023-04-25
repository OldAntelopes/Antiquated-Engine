#ifndef MODEL_CONVERTER_H
#define MODEL_CONVERTER_H

#ifdef __cplusplus
extern "C"
{
#endif


enum	eSUBMODEL_LIST
{
	kMAIN = -1,
	kWHEEL1 = 0,
	kWHEEL2,
	kHORIZONTAL_TURRET,
	kVERTICAL_TURRET,
	kMEDLOD,
	kLOWLOD,
	kCOLLISION,
};

extern const char*	ModelConvGetCurrentRenderTexture( void );

extern void		ModelConvSetCurrentModel( int nHandle );
extern int		ModelConvGetCurrentModel( void );
extern int		ModelConvGetOverrideTexture( void );


extern void		ModelConvSetTemporaryDisplayModel( int nHandle );
extern int		ModelConvGetTemporaryDisplayModel( void );

extern void		ModelConvSaveAsDialog( int nModelHandle, BOOL bStoreSavedFilenameAsCurrent );
extern void		ModelConvOpenDialog( eSUBMODEL_LIST nSubModelNum );
extern void		ModelConvSelectByMaterial( int attribID );
extern void		ModelConvSetPickerMode( int mode );
extern void		ModelConvSetLastUsedLoadPath( const char* szFile );

extern void		ModelConvTextAdd( const char* szString, ... );
extern void		ModelConvTextBoxSet( const char* szString, ... );

extern void		ModelConverterDisplayFrame( BOOL bModelHasChanged );

extern float	ModelConvGetScaleUnit( void );

extern VECT		ModelConvGetScreenMoveVector( float fScreenXDelta, float fScreenYDelta, float fScale );

extern void		ModelConvGetLastLoadFolder( char* szLoadFolder );

extern int DevLog( int eLogType, const char *format, ... );

#ifdef __cplusplus
}
#endif


#endif

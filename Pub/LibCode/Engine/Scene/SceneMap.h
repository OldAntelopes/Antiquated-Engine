

#ifndef ISLAND_Scene_MAP_H
#define ISLAND_Scene_MAP_H

#include "SceneMapElements.h"

typedef	void(*fnAddItemCallback)( SceneMapElement* );

enum eREGION_CALLBACK_TYPE
{
	kEnter_Region,
	kLeave_Region,
};

typedef	void(*fnRegionCallback)( eREGION_CALLBACK_TYPE, SceneRegionElement* );
typedef	void(*fnLoadCompleteCallback)( int );
typedef	void(*fnSceneExitCallback)( int );


class SceneMap
{
public:
	SceneMap() { mboIsDownloading = false; 
					mpcScriptMem = NULL; 
					m_szHTTPRoot[0] = 0; 
					m_szCacheSubfolder[0] = 0;
					mpCurrentParseElement = NULL; 
					mpCurrentParseRoom = NULL;
					mboIsWaitingForElementDownload = false;
					mnNumberOfRooms = 0;
					mnNumberOfRoomsParsed = 0;
					mboIsHTTPDownload = false;
					mpfnRegionCallback = NULL;
					mpfnLoadCompleteCallback = NULL;
				}
	~SceneMap();
	
	void		SetCacheSubFolder( const char* szFolderName );

	BOOL		Load( const char* szFilename );
	BOOL		LoadFromWebsite( const char* szFilename );
	BOOL		LoadFromServer( const char* szFilename );

#ifdef TOOL
	BOOL		Save( char* szFilename );
	BOOL		SaveArchive( char* szFilename );
#endif
	void		Display( const VECT* pxPos, const VECT* pxRot, int nFlags );
	void		Update( const VECT* pxPlayerPos );
	BOOL		Raycast( VECT* pxPos1, VECT* pxPos2, VECT* pxHit, VECT* pxNormal );

	BOOL		HasFullyLoaded( void );

	void		RegisterRegionCallback( fnRegionCallback pfnCallback ) { mpfnRegionCallback = pfnCallback; }
	void		RegisterLoadCompleteCallback( fnLoadCompleteCallback pfnCallback ) { mpfnLoadCompleteCallback = pfnCallback; }
	void		RegisterSceneExitCallback( fnSceneExitCallback pfnCallback ) { mpfnSceneExitCallback = pfnCallback; }
	void		TriggerRegionEvent( eREGION_CALLBACK_TYPE, SceneRegionElement* );

	SceneRegionElement*		FindRegion( const char* szRegionType, const char* szRegionParam );

	SceneMapElement*		GetFirstElement() { return mFirstElement.mpChildElement; }
	SceneMapElement*		GetRoot() { return &mFirstElement; }
	SceneMapElement* 	AddNewElement( SceneMapElement* pParent, SceneMapElement::ELEMENT_TYPES );

	SceneMapElement* 	FindParentOf( SceneMapElement* pChild );
private:
	enum eELEMENT_DOWNLOAD_TYPE
	{
		ELEMENT_DOWNLOAD_ROOM_MODEL,
		ELEMENT_DOWNLOAD_ROOM_TEXTURE,
	};

	static void		ScriptHTTPDownloadCallback( FETCHFILE_RETURN_VAL nRet, long lParam, const char* szName );
	static void		ElementHTTPDownloadCallback( FETCHFILE_RETURN_VAL nRet, long lParam, const char* szName );

	static void		ScriptServerDownloadCallback( FETCHFILE_RETURN_VAL nRet, long lParam, const char* szName );
	static void		ElementServerDownloadCallback( FETCHFILE_RETURN_VAL nRet, long lParam, const char* szName );

	VECT		ParseGetVector3( char* pcParam );
	VECT		ParseDegreesAngleVector3( char* pcParam );

	BOOL		LoaderParseNextLine( char* pcMem );	
	BOOL		ParseLoadedScript( void );
	void		ParseDownloadedElement( const char* szName );

	void		LoaderPreParseNextLine( char* pcMem );	
	BOOL		PreParseLoadedScript( void );

	BOOL		FetchElement( char* szName, eELEMENT_DOWNLOAD_TYPE, bool bUsingHTTP );

	SceneMapElement*		mpCurrentParseElement;
	SceneMapElement*		mpCurrentParseRoom;
	SceneMapElement		mFirstElement;

	eELEMENT_DOWNLOAD_TYPE		m_DownloadType;

	bool		mboIsDownloading;
	bool		mboIsHTTPDownload;
	bool		mboIsWaitingForElementDownload;

	char		m_szHTTPRoot[256];
	char		m_szCacheSubfolder[256];

	char*		mpcScriptMem;
	char*		mpcScriptPos;
	char*		mpcScriptEnd;

	int			mnNumberOfRooms;
	int			mnNumberOfRoomsParsed;

	fnRegionCallback			mpfnRegionCallback;
	fnLoadCompleteCallback		mpfnLoadCompleteCallback;
	fnSceneExitCallback			mpfnSceneExitCallback;
};


extern SceneMap*			SceneMapGet();


extern void		SceneMapReset(void);
extern BOOL		SceneMapLoad( char* szFilename, fnAddItemCallback pAddCallback, const char* szCacheSubFolder );

#ifdef TOOL
extern BOOL		SceneMapSave( char* szFilename );
extern BOOL		SceneMapSaveArchive( char* szFilename );
extern const char*		SceneMapEditorGetExeDirectory( void );

#endif

extern SceneMapElement*		SceneMapAddElement( SceneMapElement::ELEMENT_TYPES, char* szModelName, char* szTextureName, SceneMapElement* pParent );
extern void		SceneMapUpdate( const VECT* pxPos );
extern void		SceneMapDisplay( const VECT* pxPos, const VECT* pxRot, int nFlags );
extern void		SceneMapInitNew( void );
extern void		SceneMapRegisterRegionCallback( fnRegionCallback );
extern void		SceneMapRegisterLoadCompleteCallback( fnLoadCompleteCallback );
extern void		SceneMapRegisterSceneExitCallback( fnSceneExitCallback );
extern BOOL		SceneMapHasFullyLoaded( void );

extern SceneMapElement*		SceneMapGetBaseElement( void );

extern BOOL		SceneMapRaycast( VECT* pxPos1, VECT* pxPos2, VECT* pxHit, VECT* pxNormal );

#endif 
#ifndef RENDERUTIL_RENDER_OBJECT_H
#define RENDERUTIL_RENDER_OBJECT_H

#include <vector>
#include <string>
#include <map>
#include <stdarg.h>

#include "StandardDef.h"

// Whenever we're rendering stuff (particles, lines, whatever), process will be to 
// create a RenderObject for each thing we want to render, set the appropriate render flags on it, and add it to the current RenderObjectList. When the RenderObjectList is flushed, it will loop through all the RenderObjects and call their Render function which will apply the appropriate render states and then call the OnRender function which will actually do the rendering of the object (using whatever render util it needs to). The RenderObjectList will group RenderObjects
// by texture handle so that objects using the same texture can be rendered together to reduce texture switches.

// RenderObject base class sets texture and blend flags, 
// the RenderObject class implementation will be responsible for DrawPrimitive

enum 
{
	kRenderFlag_Default = 0,
	kRenderFlag_Additive = 0x1,
	kRenderFlag_Orientation_Flat = 0x2,
	kRenderFlag_Rotated = 0x4,
	kRenderFlag_Orientation_XAxis = 0x8,
	kRenderFlag_ColourBlend = 0x10,
	kRenderFlag_Orientation_YAxis = 0x20,
	kRenderFlag_SoftEdges = 0x40,
	kRenderFlag_Subtractive = 0x80,
	kRenderFlag_SingleColTexAlpha = 0x100,
	kRenderFlag_IncAlpha = 0x200,
	kRenderFlag_CustomAspect = 0x400,
	kRenderFlag_DestInv = 0x800,
	kRenderFlag_DestAdd = 0x1000,
};

#define eRenderFlags	uint32

enum
{
	kRenderType_Sprite3d = 0,
	kRenderType_Line,
	kRenderType_RibbonTrail,	

	kNumRenderTypes,		// EOL
};

class RenderObject
{
friend class RenderObjectGroup;
public:		
	virtual ~RenderObject() {}

	virtual int		OnPreRender() { return 0; };
	virtual int		OnRender() = 0;
	virtual const char* GetName() const { return "Unnamed RenderObject"; }
	virtual void    GetDesc(char* szBufferOut) const { sprintf(szBufferOut, "%s ((%d) Tex: %d RFlags: %08x)", GetName(), GetRenderType(), GetTextureHandle(), GetRenderFlags() ); }

	// TODO - Minor optimisation, we can probably remove PreRender now - it was only used for particle system to chuck out sprite3d (which were then rendered at the render stage)
	//   but now its all done within ParticleLayer.. so probably no need for PreRender at all now.  Remove once we're content that the new way covers all cases
	int		PreRender();
	int		Render();

	void	SetRenderFlags(uint32 ulBlendFlags) { mRenderFlags = ulBlendFlags; }	
	uint32	GetRenderFlags() const { return mRenderFlags; }	

	int		GetTextureHandle() const { return(mTextureHandle); }
	int		GetRenderType() const { return(mRenderType); }
protected:
	void	SetTextureHandle(int hTex);
	void	SetRenderType(int nType) { mRenderType = nType; }

private:
	void	ApplyRenderFlags( uint32 renderFlags );

	uint32	mRenderFlags = 0;
	int		mTextureHandle = NOTFOUND;
	int		mRenderType = kRenderType_Sprite3d;
};

class RenderObjectGroup
{
public:
	RenderObjectGroup( int hTex ) { mTextureHandle = hTex; }

	int		Render();
	int		PreRender();
	int		GetTextureHandle() { return(mTextureHandle); }

	void	AddRenderObjectIfNotPresent(RenderObject* pObject) 
	{
		int		nType = pObject->GetRenderType();	

		// TODO - Find if the object is already in the mRenderObjectsByType list and drop out if so
		auto it = mRenderObjectsByType.find(nType);
		if ( it != mRenderObjectsByType.end() )
		{
			std::vector<RenderObject*>& typeList = it->second;
			for ( RenderObject* pExisting : typeList )
			{
				if ( pExisting == pObject )
					return;
			}
		}

		// Otherwise.. add to the list
		mRenderObjectsByType[nType].push_back(pObject);
	}

	
	void	AddRenderObject(RenderObject* pObject) 
	{
		int		nType = pObject->GetRenderType();	
		mRenderObjectsByType[nType].push_back(pObject);
	}

	std::map<int, std::vector<RenderObject*>>	mRenderObjectsByType;
	int		mTextureHandle = NOTFOUND;
	static int		msLastSetTexture;

};

enum eRenderObjectLogStats
{
	kActiveRenderObjects,
	kRenderObjectInstances,
	kRibbonPolys,

	kNumRenderObjectLogStats,		// EOL
};

struct RenderObjectLogStats
{
public:
	int		maStats[kNumRenderObjectLogStats];
};

class RenderObjectLog
{
public:
	void		AddLog(const char* szMsg);

	void		OnNewFrame( BOOL bDumpPreviousFrameDebugLog = FALSE );

	static int		GetStat(eRenderObjectLogStats stat ) { return( msLastFrameStats.maStats[stat] ); }
	static void		AddStatCount( eRenderObjectLogStats stat, int count ) { msCurrentFrameStats.maStats[stat] += count; }

private:
	std::vector<std::string>	mLogEntries;

	static RenderObjectLogStats		msCurrentFrameStats;
	static RenderObjectLogStats		msLastFrameStats;

};

//-------------------------------------------------------------------
// Use  
//		RenderObjectList::GetCurrent().AddRenderObject( pMyRenderObject ) 
// 
// to add a RenderObject to the current list, then call 
// 
//		RenderObjectList::GetCurrent().Flush() 
// 
// to render them all out (and clear the list for the next layer)
//------------------------
class RenderObjectList
{
public:
	static RenderObjectList& GetCurrent() { static RenderObjectList instance; return instance; }

	void	Flush( int channelLayer );

	void		Log(const char* szTitle, ... ) 
	{
	char		acString[1024];
	va_list		marker;
	uint32*		pArgs;

		pArgs = (uint32*)( &szTitle ) + 1;

	    va_start( marker, szTitle );     
		vsprintf( acString, szTitle, marker );

		mRenderLog.AddLog(acString); 
	}

	void		FlushLog(const char* szTitle, ... ) 
	{
		if ( mbIsFlushActive )
		{
		char		acString[1024];
		va_list		marker;
		uint32*		pArgs;

			pArgs = (uint32*)( &szTitle ) + 1;

			va_start( marker, szTitle );     
			vsprintf( acString, szTitle, marker );

			mRenderLog.AddLog(acString); 
		}
	}

	void		NewFrame( BOOL bDumpPreviousFrameDebugLog = FALSE )
	{
		mRenderLog.OnNewFrame(bDumpPreviousFrameDebugLog);
	}

	void	AddRenderObjectIfNotPresent(RenderObject* pObject) 
	{
		int		hTex = pObject->GetTextureHandle();

		if ( mRenderObjectGroupsByTextureHandle.find(hTex) == mRenderObjectGroupsByTextureHandle.end() )
		{
			// TODO - Object pooling here rather than deleting and re-creating groups each frame
			RenderObjectGroup* pNewGroup = new RenderObjectGroup( hTex );
			mRenderObjectGroupsByTextureHandle[hTex] = pNewGroup;
		}

		mRenderObjectGroupsByTextureHandle[hTex]->AddRenderObjectIfNotPresent(pObject);
	}
	

	void	AddRenderObject(RenderObject* pObject ) 
	{
		int		hTex = pObject->GetTextureHandle();

		if ( mRenderObjectGroupsByTextureHandle.find(hTex) == mRenderObjectGroupsByTextureHandle.end() )
		{
			// TODO - Object pooling here rather than deleting and re-creating groups each frame
			RenderObjectGroup* pNewGroup = new RenderObjectGroup( hTex );
			mRenderObjectGroupsByTextureHandle[hTex] = pNewGroup;
		}

		mRenderObjectGroupsByTextureHandle[hTex]->AddRenderObject(pObject);
	}
private:
	BOOL		mbIsFlushActive = FALSE;
	// 
	RenderObjectLog		mRenderLog;
	std::map<int, RenderObjectGroup*>	mRenderObjectGroupsByTextureHandle;	
};

#endif
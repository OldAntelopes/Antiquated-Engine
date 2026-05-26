
#include "RenderProfiler.h"
#include "RenderMarkers.h"

#include "StandardDef.h"
#include "Engine.h"

#include "RenderObject.h"


RenderObjectLogStats			RenderObjectLog::msLastFrameStats = { 0 };
RenderObjectLogStats			RenderObjectLog::msCurrentFrameStats = { 0 };


uint32		mulLastAppliedRenderFlags = 0;

void	RenderObject::ApplyRenderFlags( uint32 renderFlags  )
{
	if (renderFlags & kRenderFlag_DestAdd )
	{
		EngineSetBlendMode( BLEND_MODE_DESTADD );
	}
	else if (renderFlags & kRenderFlag_DestInv)
	{
		EngineSetBlendMode( BLEND_MODE_DESTINV);
	}
	else if ( renderFlags & kRenderFlag_Subtractive )
	{
		if ( renderFlags & kRenderFlag_ColourBlend )
		{
			if ( renderFlags & kRenderFlag_IncAlpha)
			{
				EngineSetBlendMode( BLEND_MODE_COLOUR_SUBTRACTIVE_ALPHA );				
			}
			else
			{
				EngineSetBlendMode( BLEND_MODE_COLOUR_SUBTRACTIVE );		
			}
		}
		else
		{
			EngineSetBlendMode( BLEND_MODE_ALPHA_SUBTRACTIVE );
		}
	}
	else if ( renderFlags & kRenderFlag_Additive )
	{
		if ( renderFlags & kRenderFlag_ColourBlend )
		{
			EngineSetBlendMode( BLEND_MODE_COLOUR_ADDITIVE );		
		}
		else
		{
			EngineSetBlendMode( BLEND_MODE_SRCALPHA_ADDITIVE );
		}
	}
	else if ( renderFlags & kRenderFlag_ColourBlend )
	{
		if ( renderFlags & kRenderFlag_IncAlpha)
		{
			EngineSetBlendMode( BLEND_MODE_COLOUR_BOTHALPHA );		
		}
		else
		{
			EngineSetBlendMode( BLEND_MODE_COLOUR_BLEND );
		}
	}
	else if ( renderFlags & kRenderFlag_SingleColTexAlpha )
	{
		EngineSetBlendMode( BLEND_MODE_COLOUR_INVALPHA );
	}
	else
	{
		EngineSetBlendMode( BLEND_MODE_ALPHABLEND );
	}
}


void	RenderObject::SetTextureHandle(int hTex) 
{
	mTextureHandle = hTex; 
}

int		RenderObject::Render()
{
	if ( mBlendFlags != mulLastAppliedRenderFlags )
	{
		ApplyRenderFlags( mBlendFlags );
		mulLastAppliedRenderFlags = mBlendFlags;
	}
	return ( OnRender() );
}

int		RenderObject::PreRender()
{

	return ( OnPreRender() );
}


int		RenderObjectGroup::Render()
{
int	count = 0;
int		instanceCount = 0;
char	acLogBuff[512];
BOOL	bShouldLogRO = TRUE;

	// Apply texture for the group
	EngineSetTexture( 0, mTextureHandle );
	mulLastAppliedRenderFlags = 0;

	for ( int nType = 0; nType < kNumRenderTypes; nType++ )
	{
		for ( RenderObject* pObject : mRenderObjectsByType[nType] )
		{
			instanceCount = pObject->Render();
			if ( instanceCount > 0 )
			{
				count += instanceCount;
				if ( bShouldLogRO )
				{
					pObject->GetDesc( acLogBuff );
					RenderObjectList::GetCurrent().FlushLog( acLogBuff );
					RenderObjectLog::AddStatCount(kActiveRenderObjects, 1);
					RenderObjectLog::AddStatCount(kRenderObjectInstances, instanceCount );
				}
			}
		}
	}
	return count;
}

int		RenderObjectGroup::PreRender()
{
int	count = 0;
int		instanceCount = 0;
char	acLogBuff[512];
BOOL	bShouldLogRO = TRUE;

	for ( int nType = 0; nType < kNumRenderTypes; nType++ )
	{
		for ( RenderObject* pObject : mRenderObjectsByType[nType] )
		{
			instanceCount = pObject->PreRender();
			if ( instanceCount > 0 )
			{
				count += instanceCount;
				if ( bShouldLogRO )
				{
					pObject->GetDesc( acLogBuff );
					RenderObjectList::GetCurrent().FlushLog( acLogBuff );
					RenderObjectLog::AddStatCount(kActiveRenderObjects, 1);
					RenderObjectLog::AddStatCount(kRenderObjectInstances, instanceCount );
				}
			}
		}
	}

	return count;
}

//--------------------------------------------
// RenderObjectLog
void	RenderObjectLog::AddLog(const char* szMsg) 
{
	std::string		string = szMsg;
	mLogEntries.push_back(string); 
}

void		RenderObjectLog::OnNewFrame( BOOL bDumpPreviousFrameDebugLog ) 
{
	if ( bDumpPreviousFrameDebugLog )
	{
		SysDebugPrint("---**************************************************************---");
		RenderProfiler::Get().DumpFrame();
		SysDebugPrint("---**************************************************************---");

		SysDebugPrint("--- RenderObjectList Debug Log ---");
		for ( const std::string& logEntry : mLogEntries )
		{
			SysDebugPrint( logEntry.c_str() );
		}
		SysDebugPrint("--- RenderObjectList Complete ---");
	}
	mLogEntries.clear();
	msLastFrameStats = msCurrentFrameStats;
			
	memset( &msCurrentFrameStats, 0, sizeof(msCurrentFrameStats));
}

//-----------------------------------------------------

void	RenderObjectList::Flush( int channelLayer ) 
{
	if ( mRenderObjectGroupsByTextureHandle.size() > 0 )
	{
		mbIsFlushActive = TRUE;
		int		initialSize = (int)mRenderObjectGroupsByTextureHandle.size();

		wchar_t wLabel[64];
		swprintf_s(wLabel, L"RenderObjectList::Flush Ch.%d", channelLayer + 1);
		RENDER_GPU_SCOPE(wLabel);
		RENDER_PROFILE_SCOPE("RenderObjectList::Flush");

		Log(">>Ch. %d: Flushing RenderObjectList with %d groups (pre-render)", channelLayer + 1, initialSize);
		int		numParticles = 0;

		{
			RENDER_GPU_SCOPE(L"PreRender");
			RENDER_PROFILE_SCOPE("Flush/PreRender");
			for (auto& texGroup : mRenderObjectGroupsByTextureHandle )
			{
				RenderObjectGroup* pGroup = texGroup.second;
				numParticles += pGroup->PreRender();
			}
		}

		if ( (int)mRenderObjectGroupsByTextureHandle.size() != initialSize )
		{
			Log(">>Ch. %d: Render phase %d groups", channelLayer + 1, mRenderObjectGroupsByTextureHandle.size());
		}

		{
			RENDER_GPU_SCOPE(L"Render");
			RENDER_PROFILE_SCOPE("Flush/Render");
			for (auto& texGroup : mRenderObjectGroupsByTextureHandle )
			{
				RenderObjectGroup* pGroup = texGroup.second;
				pGroup->Render();
				delete pGroup;
			}	
		}

		mRenderObjectGroupsByTextureHandle.clear();
		mbIsFlushActive = FALSE;
	}
}

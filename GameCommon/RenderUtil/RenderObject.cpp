
#include "StandardDef.h"
#include "Engine.h"

#include "RenderObject.h"

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

void	RenderObject::Render()
{
	// TODO - Apply texture

	ApplyRenderFlags(mBlendFlags);

	OnRender();
}

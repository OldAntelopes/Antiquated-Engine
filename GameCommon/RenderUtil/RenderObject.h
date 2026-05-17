#ifndef RENDERUTIL_RENDER_OBJECT_H
#define RENDERUTIL_RENDER_OBJECT_H

#include <vector>
#include <map>

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

class RenderObject
{
public:
	virtual void		OnRender() = 0;

	void	Render();

	void	SetBlendFlags(uint32 ulBlendFlags) { mBlendFlags = ulBlendFlags; }	

	int		GetTextureHandle() { return(mTextureHandle); }
private:
	void	ApplyRenderFlags( uint32 renderFlags );

	uint32	mBlendFlags = 0;
	int		mTextureHandle = NOTFOUND;

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

	void	AddRenderObject(RenderObject* pObject) 
	{
		int		hTex = pObject->GetTextureHandle();

		mRenderObjectsByTextureHandle[hTex].push_back(pObject); 
	}

	void	Flush() 
	{
		// Render objects are grouped by texture handle to reduce texture switches. We loop through each group and render them together.
		for (auto& texGroup : mRenderObjectsByTextureHandle)
		{
			for (RenderObject* pObject : texGroup.second)
			{
				pObject->Render();
			}
		}	
		mRenderObjectsByTextureHandle.clear();
	}

private:

	// 

	std::map<int, std::vector<RenderObject*>>	mRenderObjectsByTextureHandle;	
};

#endif
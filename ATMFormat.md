
ATM Binary Format
------------

The ATM format is a binary data format for model and related data designed for use in The Game. 

Pros:  
* Very compact (was built for when we had 56k modems n stuff so needed small data size)
* Expandable (chunk-based structure so easy to add/maintain ; atms created years ago still load)
* Exposes some specific engine behaviours in the data format (e.g. you can have a single model file with working wheels, propellers etc)

Cons:  
* Custom format no-one understands.
* No tools except the model converter deal with it
* Doesn't support anything more modern than the things that existed at the end of the last century
* Its daft

All ATM files start with a standard header (ATM_FILE_HEADER) followed by a variable number of 'chunks'.
Each chunk begins with a chunk header (ATM_CHUNK_HEADER) followed by binary data relevant to that chunk.

ATM files can contain other ATMs inside themselves. Subsequent ATMs are separated by a chunk with a 0xFF chunk code.
(e.g. A vehicle ATM may have an ATM for its chassis, followed an ATM for a wheel. The chassis ATM can reference the wheel atm and the renderer will know to attach the wheel at a certain point).

ATM_FILE_HEADER
--------------
    0x0 	[2 bytes] =	File Header   		(Value=0x923b		// Defines this as an ATM file)
    0x2   	[2 bytes] = File Version Num  	(Current version is 2)
    0x4 	[1 byte] = Size of this header	(Version 2 atm = 12, but could be larger if a later version)
    0x5 	[1 byte] = Size of the Chunk Header 	(Version 2 atm = 12, but could be larger if a later version)
    0x6		[1 byte] = Compression flag		(1 indicates the rest of the file is zlib compressed)
    0x7		[1 byte] = Pad. Ignored.
    0x8		[4 bytes] = Uncompressed size. (Size of the compressed data after it is uncompressed)

If the ATM is compressed, all data after the ATM_FILE_HEADER will be zlib compressed and will need uncompressing before
being parsed. Once done, immediately following the ATM_FILE_HEADER will be the first chunk header..

ATM_CHUNK_HEADER
---------------
    0x0		[1 byte] = Chunk header marker. Always 0x55 (ATM_CHUNK_HEADER_CODE) - the loader stops reading chunks if it isn't
    0x1		[1 byte] = Chunk Data Num.  	
			Some chunks can appear multiple times within the ATM, this indicates which chunk number this is.
			(For the 0xFF separator chunk this says what the following sub-model is - 1 horiz turret, 2 vert turret,
			 3 front wheel, 4 rear wheel, 5 medium LOD, 6 low LOD, 7 collision model. The loader attaches a collision
			 model to the level it follows, and only one on the base model is used, so it goes before 5 and 6)
    0x2 	[2 byte] = ChunkID. Defines the type of this chunk - the 'Chunk Codes' listed below
    0x4		[4 bytes] = Num Elements in Chunk
			Some chunks contain many elements (e.g. a vertices chunk may contain 1000s of vertices.)
    0x8		[4 bytes] = Size of each element

The chunk data (Num Elements x Size of each element bytes) follows the header. In version 2+ files the data is padded with
zeros to a 4-byte boundary. Loaders skip chunk IDs they don't recognise, which is how new chunk types stay backwards compatible.
Chunks are processed in three passes (basic contents first, then geometry and most other data, then colours and skin data),
so chunk order within a model doesn't matter.

Chunk Codes
------------

	0x1		Basic Animation Description
	0x2		Vertices
	0x3		Normals
	0x4		Colours
	0x5		UVs
	0x6		Face Indices
	0x7 	(Not currently used)
	0x8		(Not currently used)
	0x9 	Vertex Keyframe Data
	0xA		Normal Keyframe Data
	0xB 	Low-resolution UVs
	0xC		Low-resolution vertices
	0xD		(Not currently used)
	0xE		Low-resolution vertex keyframes
	0xF		Low-resolution normal keyframes
	0x10	Animated UVs
	0x11	Shared low-resolution normals
	0x12	Keyframe Data
	0x13	Turret Data
	0x14	Sub-model Attach Data
	0x15	Global properties
	0x16	Creator Info
	0x17	LOD data
	0x18	Effect Data
	0x19	Texture Material
	0x1A	Attribute Data
	0x1B	Basic Contents
	0x1C	Skeleton				(Experimental - see Pub\skeletalanimation.md)
	0x1D	Skin Weights			(Experimental)
	0x1E	Skeletal Animation Links	(Experimental)
	0x1F	Material Relief
	0x20	Material Channels
	
	0xFF	Chunk Separator

**Chunk ID register.** This file is the single register of ATM chunk IDs for every reader and writer of the format:
Antiquated's Engine and ModelConverter, and MitGames' Shared\MitAtm (agreed with the developer, 31 Oct 2026).
- Add a new chunk to the list above, with its layout below, *before* any code writes it. **0x21 is the next free ID.**
- An ID means the same layout everywhere. Any reader may support a subset of the chunks and skip the rest, but no
  one reuses an ID or gives it a different layout.
- The same goes for reserved and unused fields (such as the flags fields, the skeleton's bind rotation and the
  animation links' speed). Their meaning is defined here before anyone starts using them.
- Change a layout only by appending fields to the end of an element. Loaders step by the stored element size,
  so older readers keep working.

** Definition of data in each chunk by Chunk Codes **
----------------------------

0x1		Basic Animation Description
------------------------------------

TBD


0x2		Vertices
------------------------------------
A stream of 3x float vertex positions  

	0x0		[4 bytes]	position X
	0x4		[4 bytes]	position Y
	0x8		[4 bytes]	position Z

 x number of vertices in the model

0x3		Normals
------------------------------------
A stream of 3x float normals  

	0x0		[4 bytes]	normal X
	0x4		[4 bytes]	normal Y
	0x8		[4 bytes]	normal Z

 x number of normals in the model. (Usually, but not always, equal to the number of vertices)
 
0x4		Colours
------------------------------------
A stream of 4 byte colour values in the format 0xAARRGGBB	(e.g. 0x00FF0000 is red, no alpha)  

	0x0		[4 bytes]	colour

x number of vertices in the model. 
 

0x5		UVs
------------------------------------
A stream of 2x float UV coordinates  

	0x0		[4 bytes]	U position
	0x4		[4 bytes]	V position

x number of vertices in the model. 

 
0x6		Face Indices
------------------------------------
A stream of either 2 byte (if number of vertices < 65536) or 4 byte face indices (if number of vertices >= 65536)  

	0x0		[2 or 4 bytes]		index number

	x number of indices in the model
	
Generally the ATM will only contain triangles, so a set of 3 indices will usually define a triangle in the mesh


0x8		Shared Normals
------------------------------------
TBD

0x9 	Vertex Keyframe Data
------------------------------------
ChunkHeader -> NumElements = Number of keyframes in the chunk  
ChunkHeader -> SizeOfElement = Number of vertices in each keyframe.  (Will usually = number of vertices in model)  

	[ 
		[
			A stream of 3x float vertex positions
			0x0		[4 bytes]	position X
			0x4		[4 bytes]	position Y
			0x8		[4 bytes]	position Z

			x number of vertices in the model
		]
	
		x number of keyframes in the chunk
	]


0xA		Normal Keyframe Data
------------------------------------
ChunkHeader -> NumElements = Number of keyframes in the chunk  
ChunkHeader -> SizeOfElement = Number of vertices in each keyframe.  (Will usually = number of vertices in model)  

	[ 
		[
			A stream of 3x float vertex positions
			0x0		[4 bytes]	position X
			0x4		[4 bytes]	position Y
			0x8		[4 bytes]	position Z

			x number of vertices in the model
		]
	
		x number of keyframes in the chunk
	]

Mostly this is not used and in-game normals are recalculated at load time from face & vertex positions.



0xB 	Low-resolution UVs
------------------------------------
A stream of 2x byte UV coordinates  

	0x0		[1 bytes]	U position ( 0->255 = 0.0f -> 1.0f )
	0x1		[1 bytes]	V position ( 0->255 = 0.0f -> 1.0f )

x number of vertices in the model. 


0xC		Low-resolution vertices
------------------------------------
Low resolution vertices are stored as 2-byte short values, where the value is a fractional element of the full range of the model,
as defined by its min and max positions. i.e. If the model world positions range from 10.0f to 200.0f, the low resolution value of 0 = 10.0f and 65535 = 200.0f
i.e. The larger the model is, the less accurate the low resolution vertices are.  

Chunk starts with a custom header :  

	0x0	[2 bytes]		Size of this header
	0x2	[2 bytes]		Unused
	0x4	[4 bytes]		Number of vertices
	0x8	[4 bytes]		Min Position X
	0xC	[4 bytes]		Min Position Y
	0x10	[4 bytes]		Min Position Z
	0x14	[4 bytes]		Max Position X
	0x18	[4 bytes]		Max Position Y
	0x1C	[4 bytes]		Max Position Z

This header is followed by 3x short vertex positions  

	0x0	[2 bytes]		Low-res position X
	0x2	[2 bytes]		Low-res position Y
	0x4	[2 bytes]		Low-res position Z

x number of vertices in the model

0xE		Low-resolution vertex keyframes
------------------------------------
Keyframe stream of low resolution vertices. Packing works in same way as described in 0xC Low-resolution vertices

Chunk starts with a custom header :  

	0x0		[2 bytes]		Size of this header
	0x2		[2 bytes]		Number of keyframes
	0x4		[4 bytes]		Number of vertices
	0x8		[4 bytes]		Min Position X
	0xC		[4 bytes]		Min Position Y
	0x10	[4 bytes]		Min Position Z
	0x14	[4 bytes]		Max Position X
	0x18	[4 bytes]		Max Position Y
	0x1C	[4 bytes]		Max Position Z

This header is followed by 3x short vertex positions  

	[
		[
			0x0		[2 bytes]		Low-res position X
			0x2		[2 bytes]		Low-res position Y
			0x4		[2 bytes]		Low-res position Z

			x number of vertices in the model
		]
		x number of keyframes in chunk
	]	

0xF		Low-resolution normal keyframes
------------------------------------

TBD

0x10	Animated UVs
------------------------------------

TBD

0x11	Shared low-resolution normals
------------------------------------

TBD

0x12	Keyframe Data
------------------------------------

TBD

0x13	Turret Data
------------------------------------

TBD

0x14	Sub-model Attach Data
------------------------------------

TBD

0x15	Global properties
------------------------------------

TBD

0x16	Creator Info
------------------------------------

TBD

0x17	LOD data
------------------------------------

TBD

0x18	Effect Data
------------------------------------

TBD

0x19	Texture Material
------------------------------------
ChunkHeader -> NumElements = 1  
ChunkHeader -> SizeOfElement = The size of the whole chunk  

Chunk starts with a custom header :  

	0x0		[1 byte]		Number of materials (must equal the count in Basic Contents, or the loader panics)
	0x1		[1 byte]		Size of this header (4)
	0x2		[2 bytes]		Unused

This header is followed, for each material, by :  

	0x0		[16 bytes]		Diffuse colour R,G,B,A (floats)
	0x10	[16 bytes]		Specular colour R,G,B,A (floats)
	0x20	[16 bytes]		Emissive colour R,G,B,A (floats)
	0x30	[16 bytes]		Ambient colour R,G,B,A (floats)
	0x40	[1 byte]		Material number
	0x41	[1 byte]		Number of textures
	0x42	[1 byte]		Blend type (ModelMaterialData::eBLEND_TYPES)
	0x43	[1 byte]		Flags : 0x1 = use this material's colours and blend type (otherwise they're ignored), 0x2 = double sided,
							0x4 = only use them at night (EngineGameInterface::IsDaytime), for lit windows and lamps. By day the faces draw
							as if 0x1 weren't set. A reader that doesn't know this flag uses them all the time
	0x44	[4 bytes]		Size of the texture filename
	0x48	[4 bytes]		Size of the embedded texture data
	0x4C	[4 bytes]		Specular power (float)
	0x50	[4 bytes]		Size of the embedded normal map data
	0x54	[1 byte]		Share the diffuse texture of another material : its number + 1 (0 = none)
	0x55	[1 byte]		The same for the second texture
	0x56	[1 byte]		Size of the normal map filename
	0x57	[1 byte]		Size of the specular map filename
	0x58	[4 bytes]		Size of the embedded specular map data

	followed by the filename and then the data of the diffuse, normal and specular textures, for each that has both

A material with no texture of its own leaves the texture the caller set, so a game can supply it per use. A surface colour is
texture x lighting + texture x emissive, so a material with diffuse and ambient 0 and emissive 1 draws as its plain texture colour.

0x1A	Attribute Data
------------------------------------
One byte per face, in the order of the Face Indices chunk : the number of the material that face uses.


0x1B	Basic Contents
------------------------------------
Usually (but not necessarily) this will be the first chunk in the ATM file, and any ATM parser will probably parse this chunk first to get an understanding of the structure of the ATM.  

	0x0 	[1 byte]		Model Type
	0x1		[1 byte]		Number of materials
	0x2		[2 bytes]		Currently unused
	0x4		[4 bytes]		Number of vertices
	0x8		[4 bytes]		'Unique Model ID'
	0xC		[4 bytes]		Number of faces
	0x10 	[2 bytes]		Content flags
	0x12	[2 bytes]		Currently unused
	0x14	[4 bytes]		Number of normals
	0x18	[1 byte]		Wheel Display Mode
	0x19	[1 byte]		Currently unused
	0x20	[2 bytes]		Currently unused
	0x24	[4 bytes]		LockID
	0x28	[4 bytes]		Lock Code


0x1C	Skeleton  (Experimental)
------------------------------------
One element per bone (ChunkHeader -> SizeOfElement is currently 64; loaders step by the stored size so fields can be appended).
Bones are ordered so that a parent always comes before its children.

	0x0		[32 bytes]		Bone name (zero terminated). Animations are matched to bones by name - see Pub\skeletalanimation.md
	0x20	[2 bytes]		Parent bone index (-1 for the root)
	0x22	[2 bytes]		Flags (currently unused, 0)
	0x24	[12 bytes]		Bind pose joint position X,Y,Z (floats, model space)
	0x30	[16 bytes]		Bind pose joint orientation quaternion X,Y,Z,W (floats, model space). Currently always identity

x number of bones. The bind pose is the pose the mesh's vertices are stored in.

0x1D	Skin Weights  (Experimental)
------------------------------------
One element per vertex, in the same order as the vertex chunk (SizeOfElement = 8)

	0x0		[4 bytes]		Bone index for each of up to 4 influences
	0x4		[4 bytes]		Weight for each influence, 0-255 (the four weights sum to 255)

x number of vertices

0x1E	Skeletal Animation Links  (Experimental)
------------------------------------
One element per linked animation (SizeOfElement = 72). The animation files live in the same folder as the .atm and
are loaded when the model is loaded. Currently they are .bvh files.

	0x0		[64 bytes]		Animation filename, relative to the folder containing the .atm (zero terminated)
	0x40	[2 bytes]		Animation use (the ANIM_xxx enum in Rendering.h - e.g. 2 = ANIM_RUN)
	0x42	[2 bytes]		Flags (currently unused, 0)
	0x44	[4 bytes]		Playback speed multiplier (float, currently unused - 1.0)

x number of linked animations

A skinned model is normally a static mesh (model type 0) with these three chunks added - older loaders ignore the
chunks and show it in its bind pose.

0x1F	Material Relief
------------------------------------
A height map for a material, for parallax and for lighting that follows the surface. Needs a level of detail of its own
to apply to: each sub-model carries its own, usually only the nearest. A reader that doesn't know it draws the plain model.
ChunkHeader -> NumElements = 1  
ChunkHeader -> SizeOfElement = The size of the whole chunk  

Chunk starts with a custom header :  

	0x0		[1 byte]		Number of entries
	0x1		[1 byte]		Size of this header (4)
	0x2		[2 bytes]		Unused

This header is followed, for each entry, by :  

	0x0		[1 byte]		Material number
	0x1		[1 byte]		Flags (currently unused, 0)
	0x2		[2 bytes]		Unused
	0x4		[4 bytes]		Depth of the relief in world units (float), after any scale the model is drawn at
	0x8		[2 bytes]		Width of the height map in pixels
	0xA		[2 bytes]		Height of the height map in pixels
	0xC		[4 bytes]		Size of the image file data

	followed by the image file data. An image with an alpha channel, in the material's UV layout: alpha 255 is the surface,
	0 is the full depth below it. The colour channels are unused

Only drawn when the app turns it on (EngineSetModelRelief) and its pixel shader supports it.

0x20	Material Channels
------------------------------------
Materials that light up with something the model's owner is doing, such as a vehicle's brake lamps. Goes with the sub-model whose
materials it names. A reader that doesn't know it draws those materials with their ordinary colours, so give them the look of an
unlit lamp in the Texture Material chunk.
ChunkHeader -> NumElements = 1  
ChunkHeader -> SizeOfElement = The size of the whole chunk  

Chunk starts with a custom header :  

	0x0		[1 byte]		Number of entries
	0x1		[1 byte]		Size of this header (4)
	0x2		[1 byte]		Size of each entry (16). Loaders step by this, so fields can be appended
	0x3		[1 byte]		Unused

This header is followed, for each entry, by :  

	0x0		[1 byte]		Material number
	0x1		[1 byte]		Channel, 1 to 8 (below)
	0x2		[1 byte]		Flags (currently unused, 0)
	0x3		[1 byte]		Unused
	0x4		[12 bytes]		Emissive colour R,G,B while the channel is on (floats). Multiplied by the channel's strength and, in the
							shader, by the texture, so values over 1 are normal

While the app has the channel's strength above 0 (EngineSetModelMaterialChannel), the material uses its own colours whatever its flags,
with this as its emissive colour. Strengths apply to the draw, not the model, so clones can differ.

	Channel 1	Headlights
	Channel 2	Brake. The Universal also lights these in white for reversing, with a colour it sets for the draw (EngineSetModelMaterialChannelColour)
	Channel 3	Left indicator (reserved, nothing sets it yet)
	Channel 4	Right indicator (reserved, nothing sets it yet)

The numbers are the same for every model and app. Add new ones to this list.

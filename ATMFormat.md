
ATM Binary Format
-------------------

The ATM format is a binary data format for model and related data designed for use in The Game.
All ATM files start with a standard header (ATM_FILE_HEADER) followed by a variable number of 'chunks'
Each chunk beings with a chunk header (ATM_CHUNK_HEADER) followed by binary data relevant to that chunk.

ATM files can contain other ATMs inside themselves. Subsequent ATMs are separated by a chunk with a 0xFF chunk code.
(e.g. A vehicle ATM may have an ATM for its chassis, followed an ATM for a wheel. The chassis ATM can reference the wheel atm and the renderer will know to attach the wheel at a certain point).


ATM_FILE_HEADER
0x0 	[2 bytes] =	File Header   		(Value=0x923b		// Defines this as an ATM file)
0x2   	[2 bytes] = File Version Num  	(Current version is 2)
0x4 	[1 byte] = Size of this header	(Version 2 atm = 12, but could be larger if a later version)
0x5 	[1 byte] = Size of the Chunk Header 	(Version 2 atm = 12, but could be larger if a later version)
0x6	[1 byte] = Compression flag		(1 indicates the rest of the file is zlib compressed)
0x7	[1 byte] = Pad. Ignored.
0x8	[4 bytes] = Uncompressed size. (Size of the compressed data after it is uncompressed)

If the ATM is compressed, all data after the ATM_FILE_HEADER will be zlib compressed and will need uncompressing before
being parsed. Once done, immediately following the ATM_FILE_HEADER will be the first chunk header..


ATM_CHUNK_HEADER
0x0	[1 byte] = Chunk code.  Defines the type of this chunk. (See individual chunk definitions later)
0x1	[1 byte] = Chunk Data Num.  	
			Some chunks can appear multiple times within the ATM, this indicates which chunk number this is
0x2 	[2 byte] = ChunkID
0x4	[4 bytes] = Num Elements in Chunk
			Some chunks contain many elements (e.g. a vertices chunk may contain 1000s of vertices.)
0x8	[4 bytes] = Size of each element

-------------------------------
Chunk Codes

0x1	Basic Animation Description
0x2	Vertices
0x3	Normals
0x4	Colours
0x5	UVs
0x6	Face Indices
0x7 	(Not currently used)
0x8	(Not currently used)
0x9 	Vertex Keyframe Data
0xA	Normal Keyframe Data
0xB 	Low-resolution UVs
0xC	Low-resolution vertices
0xD	(Not currently used)
0xE	Low-resolution vertex keyframes
0xF	Low-resolution normal keyframes
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

0xFF	Chunk Separator
---------------------------

*******************************************************
*** Definition of data in each chunk by Chunk Codes ***
*******************************************************

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
as defined by its min and max positions.
i.e. If the model world positions range from 10.0f to 200.0f, the low resolution value of 0 = 10.0f and 65535 = 200.0f
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

0x0	[2 bytes]		Size of this header
0x2	[2 bytes]		Number of keyframes
0x4	[4 bytes]		Number of vertices
0x8	[4 bytes]		Min Position X
0xC	[4 bytes]		Min Position Y
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

TBD

0x1A	Attribute Data
------------------------------------

TBD


0x1B	Basic Contents
------------------------------------
Usually (but not necessarily) this will be the first chunk in the ATM file, and any ATM parser will probably
parse this chunk first to get an understanding of the structure of the ATM.

0x0 	[1 byte]		Model Type
0x1	[1 byte]		Number of materials
0x2	[2 bytes]		Currently unused
0x4	[4 bytes]		Number of vertices
0x8	[4 bytes]		'Unique Model ID'
0xC	[4 bytes]		Number of faces
0x10 	[2 bytes]		Content flags
0x12	[2 bytes]		Currently unused
0x14	[4 bytes]		Number of normals
0x18	[1 byte]		Wheel Display Mode
0x19	[1 byte]		Currently unused
0x20	[2 bytes]		Currently unused
0x24	[4 bytes]		LockID
0x28	[4 bytes]		Lock Code



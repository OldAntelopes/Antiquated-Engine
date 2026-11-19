
// Global variables
extern float4x4		gMatWorld;
extern float4x4		gMatView;
extern float4x4		gMatProj;
extern float4x4		gMatLightViewProj;

extern float3		gEyePosition;
extern float		gZBias;

// Wind sway, for trees only: The Universal's TreeRenderDraw sets gTreeWindAmount.x above 0 while it draws trees and
// back to 0 afterwards, so every other model is unaffected. gTreeWind: x = time (s), yz = the direction the wind blows
// towards (unit), the same wind as the grass (IslandGetWindState). gTreeWindWave: the two trains of gust waves, xy = phase,
// zw = wave number (rad/m), on axes that don't turn with the wind. gTreeWindAmount: x = how far the tree bends at 10 m
// above its base (grows with height squared), y = leaf flutter (m), z = branch sway (m, 10 m out from the trunk), set only for
// generated trees (Docs\ProceduralTreesSpec.md), 0 for every other model. w = how much a generated tree's leaves glow with the sun
// behind them (TreeRender.cpp, render.leafglow x the species' "leaves.translucency"; main view only, else 0), passed to the pixel
// shader as Position3D.w - 1 (every other model's w is 1).
extern float4		gTreeWind;
extern float4		gTreeWindWave;
extern float4		gTreeWindAmount;
// For a generated tree (gTreeWindAmount.z above 0): its bark's UV rectangle (u0, v0, u1, v1). What's inside is wood, kept rigid
extern float4		gTreeBarkRect;
// A generated tree's season (TreeRender.cpp, TreeRenderSetSlotSeason): x = its leaf cards whose own random is below this are
// gone (bare in winter), w = 2468 when on (0 for every other model). The pixel shader gets the rest
extern float4		gTreeSeason;


// A leaf card's own random number, 0-1: the lowest two bits of each of its vertex colour's channels (TreeGen.h,
// TreeGenCardRandomFromColour). Kept the same in TreeBake.vsh and ShadowMapPass1.vsh
float	TreeSeasonCardRandom( float3 vColour )
{
	float3	vBytes = floor( ( vColour * 255.0 ) + 0.5 );
	float3	vLow = vBytes - ( floor( vBytes * 0.25 ) * 4.0 );
	return( ( dot( vLow, float3( 16.0, 4.0, 1.0 ) ) + 0.5 ) / 64.0 );
}

// Vertex shader input structure
struct VS_INPUT
{
    float4 Position   : POSITION;
    float4 Color	  : COLOR0;
	float3 Normal	  : NORMAL0;
    float2 TexCoord   : TEXCOORD0;
    float3 Tangent    : TEXCOORD1;
};

// Vertex shader output structure
struct VS_OUTPUT
{
    float4 Position   : POSITION;
	float4 VertCol : COLOR0;
    float4 TexCoord : TEXCOORD0;
    float3 View : TEXCOORD1;
	float3 vNormal : TEXCOORD2;  
	float4 vPosLight : TEXCOORD3;
    float4 Position3D : TEXCOORD4;
    float3  TangentWorldSpace : TEXCOORD5;
    float3  BiTangentWorldSpace : TEXCOORD6;
	float2	TreeSeason : TEXCOORD7;			// A generated tree in season (gTreeSeason): x = the leaf card's random, y = 1 for a leaf; else 0

};


// Name: Simple Vertex Shader
// Type: Vertex shader
// Desc: Vertex transformation and texture coord pass-through
//
VS_OUTPUT vs_main( VS_INPUT input )
{
    VS_OUTPUT output;
 
    float4 worldPosition = mul(input.Position, gMatWorld);

	float	fLeafGlow = 0.0;
	if ( ( gTreeWindAmount.x > 0.0 ) || ( gTreeWindAmount.z > 0.0 ) || ( gTreeWindAmount.w > 0.0 ) )
	{
		float3	treeBase = float3( gMatWorld[3][0], gMatWorld[3][1], gMatWorld[3][2] );
		float	h = max( worldPosition.z - treeBase.z, 0.0 );
		// Gusts rolling across the land (as the grass), plus a slow sway of the whole tree
		float	gust = 0.5 + ( 0.25 * ( sin( ( dot( treeBase.xy, float2( 0.8660254, 0.5 ) ) * gTreeWindWave.z ) - gTreeWindWave.x ) + sin( ( dot( treeBase.xy, float2( -0.258819, 0.9659258 ) ) * gTreeWindWave.w ) - gTreeWindWave.y ) ) );
		float	sway = 0.35 + ( gust * 0.65 ) + ( 0.2 * sin( ( gTreeWind.x * 1.3 ) + ( treeBase.x * 0.31 ) + ( treeBase.y * 0.27 ) ) );
		float	bend = gTreeWindAmount.x * ( h * h * 0.01 ) * sway;
		// A generated tree's wood (its texture coordinates in the species' bark rectangle, gTreeBarkRect) stays rigid: no flutter,
		// and its sway the same phase all over, so a trunk or a thick branch doesn't ripple. (0 for every other model)
		float	isWood = 0.0;
		if ( ( gTreeWindAmount.z > 0.0 ) || ( gTreeWindAmount.w > 0.0 ) )
		{
			isWood = step( gTreeBarkRect.x, input.TexCoord.x ) * step( input.TexCoord.x, gTreeBarkRect.z ) * step( gTreeBarkRect.y, input.TexCoord.y ) * step( input.TexCoord.y, gTreeBarkRect.w );
		}
		fLeafGlow = gTreeWindAmount.w * ( 1.0 - isWood );
		// Leaves flutter, each part of the crown out of step with the next; not the foot of the trunk
		float	flutter = gTreeWindAmount.y * ( 1.0 - isWood ) * saturate( h * 0.3 ) * sin( ( gTreeWind.x * 4.3 ) + dot( worldPosition.xyz, float3( 1.7, 1.3, 2.1 ) ) );

		// Branches: the further out from the trunk, the more they sway on top of the trunk's bend, mostly with the wind and bobbing
		// up and down, the whole crown together; leaves also drift a little out of step across the crown (slowly, so a leaf card
		// isn't stretched). (0 for every model but a generated tree)
		float2	reachOut = worldPosition.xy - treeBase.xy;
		float	branchPhase = ( gTreeWind.x * 1.9 ) + ( treeBase.x * 0.13 ) + ( treeBase.y * 0.17 ) + ( ( 1.0 - isWood ) * dot( reachOut, float2( 0.07, 0.05 ) ) );
		float	branch = gTreeWindAmount.z * min( length( reachOut ) * 0.1, 2.0 ) * saturate( h * 0.15 ) * ( 0.4 + ( gust * 0.6 ) ) * sin( branchPhase );

		worldPosition.xy += ( gTreeWind.yz * ( bend + branch ) ) + ( float2( -gTreeWind.z, gTreeWind.y ) * flutter );
		worldPosition.z += ( flutter * 0.5 ) - ( abs( bend ) * bend * 0.05 ) + ( branch * 0.45 );
	}
    float4 viewPosition = mul(worldPosition, gMatView);

    output.Position = mul(viewPosition, gMatProj);
	output.Position.z -= gZBias;
    output.Position3D = float4( worldPosition.xyz, 1.0 + fLeafGlow );
    output.TexCoord = float4(input.TexCoord,input.Tangent.x,input.Tangent.y);

//	float4	worldNormal = mul( input.Normal, gMatWorld );
	float3 worldNormal = mul( input.Normal, (float3x3)gMatWorld );
	output.vNormal = normalize( worldNormal );

	output.vPosLight = mul(worldPosition, gMatLightViewProj);
	
	float3 BiTangent = cross(normalize(input.Tangent), normalize(input.Normal));

    output.TangentWorldSpace = mul(normalize(input.Tangent), gMatWorld);
    output.BiTangentWorldSpace = mul(normalize(BiTangent), gMatWorld);
     
    output.View = normalize(float4(gEyePosition,1.0) - worldPosition);
	output.VertCol = input.Color;

	// Seasons: a leaf card whose random is below the cut has fallen, so all its corners go behind the camera and it isn't drawn
	output.TreeSeason = float2( 0.0, 0.0 );
	if ( gTreeSeason.w == 2468.0 )
	{
		float	isBark = step( gTreeBarkRect.x, input.TexCoord.x ) * step( input.TexCoord.x, gTreeBarkRect.z ) * step( gTreeBarkRect.y, input.TexCoord.y ) * step( input.TexCoord.y, gTreeBarkRect.w );
		float	fCard = TreeSeasonCardRandom( input.Color.rgb );

		output.TreeSeason = float2( fCard, 1.0 - isBark );
		if ( ( isBark < 0.5 ) && ( fCard < gTreeSeason.x ) )
		{
			output.Position = float4( 0.0, 0.0, -2.0, 1.0 );
		}
	}

    return output;                     //return output vertex
}

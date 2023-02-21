
// Global variables
extern float4x4		gMatWorld;
extern float4x4		gMatView;
extern float4x4		gMatProj;
extern float4x4		gMatLightViewProj;

extern float3		gEyePosition;
extern float		gZBias;


// Vertex shader input structure
struct VS_INPUT
{
    float4 Position   : POSITION;
    float4 Color	  : COLOR0;
	float3 Normal	  : NORMAL0;
    float2 TexCoord    : TEXCOORD0;
    float3 Tangent    : TEXCOORD1;
};

// Vertex shader output structure
struct VS_OUTPUT
{
    float4 Position   : POSITION;
    float2 TexCoord : TEXCOORD0;
    float3 View : TEXCOORD1;
	float3 vNormal : TEXCOORD2;  
	float4 vPosLight : TEXCOORD3;
	float4	VertCol : TEXCOORD4;
    float4 Position3D : TEXCOORD5;
    float3  TangentWorldSpace : TEXCOORD6;
    float3  BiTangentWorldSpace : TEXCOORD7;

};


// Name: Simple Vertex Shader
// Type: Vertex shader
// Desc: Vertex transformation and texture coord pass-through
//
VS_OUTPUT vs_main( VS_INPUT input )
{
    VS_OUTPUT output;
 
    float4 worldPosition = mul(input.Position, gMatWorld);
    float4 viewPosition = mul(worldPosition, gMatView);

    output.Position = mul(viewPosition, gMatProj);
	output.Position.z -= gZBias;
    output.Position3D = worldPosition;
    output.TexCoord = input.TexCoord;

//	float4	worldNormal = mul( input.Normal, gMatWorld );
	float3 worldNormal = mul( input.Normal, (float3x3)gMatWorld );
	output.vNormal = normalize( worldNormal );

	output.vPosLight = mul(worldPosition, gMatLightViewProj);
	
	float3 BiTangent = cross(normalize(input.Tangent), normalize(input.Normal));

    output.TangentWorldSpace = mul(normalize(input.Tangent), gMatWorld);
    output.BiTangentWorldSpace = mul(normalize(BiTangent), gMatWorld);
     
    output.View = normalize(float4(gEyePosition,1.0) - worldPosition);
	output.VertCol = input.Color;

    return output;                      //return output vertex
}

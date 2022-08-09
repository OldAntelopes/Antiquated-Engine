
// Global variables
extern float4x4	matWorld;
extern float4x4	matView;
extern float4x4	matProj;
extern float3 EyePosition;
extern float4x4	matLightWorldViewProj;




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
    float4 Position : POSITION0;
    float2 TexCoord : TEXCOORD0;
    float3 View : TEXCOORD1;
	float4 ShadowMapSamplingPos : TEXCOORD2;  
	float4 RealDistance         : TEXCOORD3;
    float3x3 WorldToTangentSpace : TEXCOORD4;
};


// Name: Simple Vertex Shader
// Type: Vertex shader
// Desc: Vertex transformation and texture coord pass-through
//
VS_OUTPUT vs_main( VS_INPUT input )
{
    VS_OUTPUT output;
 
	float		xMaxDepth = 2.0f;

    float4 worldPosition = mul(input.Position, matWorld);
    float4 viewPosition = mul(worldPosition, matView);
    output.Position = mul(viewPosition, matProj);
    output.TexCoord = input.TexCoord;
	output.ShadowMapSamplingPos = mul(input.Position, matLightWorldViewProj);
	output.RealDistance = output.ShadowMapSamplingPos.z / 100.0f;
	
	float3 BiTangent = cross(normalize(input.Tangent), normalize(input.Normal));

    output.WorldToTangentSpace[0] = mul(normalize(input.Tangent), matWorld);
    output.WorldToTangentSpace[1] = mul(normalize(BiTangent), matWorld);
    output.WorldToTangentSpace[2] = mul(normalize(input.Normal), matWorld);
     
    output.View = normalize(float4(EyePosition,1.0) - worldPosition);
 
    return output;                      //return output vertex
}

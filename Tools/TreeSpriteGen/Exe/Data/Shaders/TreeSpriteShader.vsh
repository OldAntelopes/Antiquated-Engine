
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
};


// Name: Simple Vertex Shader
// Type: Vertex shader
// Desc: Vertex transformation and texture coord pass-through
//
VS_OUTPUT vs_main( VS_INPUT input )
{
    VS_OUTPUT output;
 
    float4 worldPosition = mul(input.Position, matWorld);
    float4 viewPosition = mul(worldPosition, matView);
    output.Position = mul(viewPosition, matProj);
    output.TexCoord = input.TexCoord;
 
    return output;                      //return output vertex
}

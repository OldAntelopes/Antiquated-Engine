
extern float4x4		gMatWorld;
extern float4x4		gMatView;
extern float4x4		gMatProj;

// Vertex shader input structure
struct VS_INPUT
{
    float4 Position   : POSITION;
    float2 TexCoord    : TEXCOORD0;
};

// Vertex shader output structure
struct VS_OUTPUT
{
    float4 Position   : POSITION;
    float4 Color	  : COLOR0;
    float2 TexCoord : TEXCOORD0;
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
	output.Color = float4(1.0f,1.0f,1.0f,1.0f);
    output.TexCoord = input.TexCoord;

    return output;                      //return output vertex
}

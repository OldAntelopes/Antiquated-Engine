
// Global variables
extern float4x4	gMatLightViewProjection;
extern float4x4	gMatWorld;


void vs_main(  float4 Position : POSITION,
			   float2 InTexCoord    : TEXCOORD0,
               out float4 vPosition : POSITION,
			   out float2 TexCoord : TEXCOORD0,
			   out float2 Depth : TEXCOORD1 )
{
	float4 worldPosition = mul( Position, gMatWorld );
	vPosition = mul( worldPosition, gMatLightViewProjection);
	TexCoord = InTexCoord;
    Depth.x = vPosition.z;
	Depth.y = vPosition.w;

}

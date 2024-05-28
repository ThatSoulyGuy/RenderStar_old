

//Texture2D diffuse : register(t0);
//SamplerState samplerState : register(s0);

struct PixelInputType
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float3 normal : NORMAL;
    float2 textureCoordinates : TEXCOORD0;
};

float4 Main(PixelInputType input) : SV_TARGET
{
    return input.color; //diffuse.Sample(samplerState, input.textureCoordinates) * input.color;
}
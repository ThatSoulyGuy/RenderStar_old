

struct VertexInputType
{
    float3 position : POSITION;
    float3 color : COLOR;
    float3 normal : NORMAL;
    float2 textureCoordinates : TEXCOORD0;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float3 normal : NORMAL;
    float2 textureCoordinates : TEXCOORD0;
};

PixelInputType Main(VertexInputType input)
{
    PixelInputType output;
    
    float4 worldPosition = float4(input.position, 1.0f);
    
    output.position = worldPosition;
    
    output.color = float4(input.color, 1.0f);
    output.normal = input.normal;
    
    output.textureCoordinates = input.textureCoordinates;

    return output;
}
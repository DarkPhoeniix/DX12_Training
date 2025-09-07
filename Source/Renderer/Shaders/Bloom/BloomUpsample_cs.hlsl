
#include "../UnifiedRootSignature.hlsli"
#include "../CommonResources.hlsli"

struct PassConstants
{
    uint InputTextureIndex;
    uint OutputTextureIndex;
    
    float FilterRadius;
};

ConstantBuffer<PassConstants> PassCB : register(b1);

[numthreads(16, 16, 1)]
[RootSignature(URootSignature)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    Texture2D InputTexture = ResourceDescriptorHeap[PassCB.InputTextureIndex];
    RWTexture2D<float4> OutputTexture = ResourceDescriptorHeap[PassCB.OutputTextureIndex];
    
    uint2 pixel = DTid.xy;
    
    uint width, height;
    OutputTexture.GetDimensions(width, height);
    
    if (pixel.x >= width || pixel.y >= height)
    {
        return;
    }
    
    uint srcWidth, srcHeight;
    InputTexture.GetDimensions(srcWidth, srcHeight);
    
    float2 uv = (float2(pixel) + 0.5f) / float2(width, height); // Current uv
    float2 filterSize = PassCB.FilterRadius / float2(srcWidth, srcHeight);
    
    // Take 9 samples around current texel:
    // a - b - c
    // d - e - f
    // g - h - i
    
    float4 a = InputTexture.SampleLevel(LinearClampSampler, uv, 0, int2(-1,  1));
    float4 b = InputTexture.SampleLevel(LinearClampSampler, uv, 0, int2( 0,  1));
    float4 c = InputTexture.SampleLevel(LinearClampSampler, uv, 0, int2( 1,  1));
    
    float4 d = InputTexture.SampleLevel(LinearClampSampler, uv, 0, int2(-1,  0));
    float4 e = InputTexture.SampleLevel(LinearClampSampler, uv, 0, int2( 0,  0));
    float4 f = InputTexture.SampleLevel(LinearClampSampler, uv, 0, int2( 1,  0));
    
    float4 g = InputTexture.SampleLevel(LinearClampSampler, uv, 0, int2(-1, -1));
    float4 h = InputTexture.SampleLevel(LinearClampSampler, uv, 0, int2( 0, -1));
    float4 i = InputTexture.SampleLevel(LinearClampSampler, uv, 0, int2( 1, -1));
    
    float4 lowColor = e * 4.0f +
                      (b + d + f + h) * 2.0f +
                      (a + c + g + i);
           lowColor *= 1.0f / 16.0f;
    float4 highColor = OutputTexture.Load(uint3(pixel, 0));
    
    OutputTexture[pixel] = highColor + lowColor * PassCB.FilterRadius;
}
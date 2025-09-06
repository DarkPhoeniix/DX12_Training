
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
    
    uint width, height;
    OutputTexture.GetDimensions(width, height);
    
    if (DTid.x >= width || DTid.y >= height)
    {
        return;
    }
    
    uint srcWidth, srcHeight;
    InputTexture.GetDimensions(srcWidth, srcHeight);
    
    float2 pixel = (float2(DTid.xy) + 0.5f) / float2(width, height); // Current pixel
    float2 filterSize = PassCB.FilterRadius / float2(srcWidth, srcHeight);
    
    // Take 9 samples around current texel:
    // a - b - c
    // d - e - f
    // g - h - i
    
    float4 a = InputTexture.SampleLevel(LinearClampSampler, float2(pixel.x - filterSize.x, pixel.y + filterSize.y), 0);
    float4 b = InputTexture.SampleLevel(LinearClampSampler, float2(pixel.x               , pixel.y + filterSize.y), 0);
    float4 c = InputTexture.SampleLevel(LinearClampSampler, float2(pixel.x + filterSize.x, pixel.y + filterSize.y), 0);
    
    float4 d = InputTexture.SampleLevel(LinearClampSampler, float2(pixel.x - filterSize.x, pixel.y               ), 0);
    float4 e = InputTexture.SampleLevel(LinearClampSampler, float2(pixel.x               , pixel.y               ), 0);
    float4 f = InputTexture.SampleLevel(LinearClampSampler, float2(pixel.x + filterSize.x, pixel.y               ), 0);
    
    float4 g = InputTexture.SampleLevel(LinearClampSampler, float2(pixel.x - filterSize.x, pixel.y - filterSize.y), 0);
    float4 h = InputTexture.SampleLevel(LinearClampSampler, float2(pixel.x               , pixel.y - filterSize.y), 0);
    float4 i = InputTexture.SampleLevel(LinearClampSampler, float2(pixel.x + filterSize.x, pixel.y - filterSize.y), 0);
    
    float4 ourColor = e * 4.0f +
                      (b + d + f + h) * 2.0f +
                      (a + c + g + i);
    OutputTexture[DTid.xy] = ourColor * (1.0f / 16.0f);
}
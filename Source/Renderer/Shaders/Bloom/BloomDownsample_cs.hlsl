
#include "../UnifiedRootSignature.hlsli"
#include "../CommonResources.hlsli"

struct PassConstants
{
    uint InputTextureIndex;
    uint OutputTextureIndex;
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
    
    float2 pixel = (float2(DTid.xy) + 0.5f) / float2(width, height); // Current pixel
    
    // Take 13 samples around current texel:
    // a - b - c
    // - j - k -
    // d - e - f
    // - l - m -
    // g - h - i
    
    float2 texelSize = 1.0f / float2(width, height);
    
    float3 a = InputTexture.SampleLevel(LinearClampSampler, float2(pixel.x - 2.0f * texelSize.x, pixel.y + 2.0f * texelSize.y), 0).rgb;
    float3 b = InputTexture.SampleLevel(LinearClampSampler, float2(pixel.x                     , pixel.y + 2.0f * texelSize.y), 0).rgb;
    float3 c = InputTexture.SampleLevel(LinearClampSampler, float2(pixel.x + 2.0f * texelSize.x, pixel.y + 2.0f * texelSize.y), 0).rgb;
    
    float3 d = InputTexture.SampleLevel(LinearClampSampler, float2(pixel.x - 2.0f * texelSize.x, pixel.y                     ), 0).rgb;
    float3 e = InputTexture.SampleLevel(LinearClampSampler, float2(pixel.x                     , pixel.y                     ), 0).rgb;
    float3 f = InputTexture.SampleLevel(LinearClampSampler, float2(pixel.x + 2.0f * texelSize.x, pixel.y                     ), 0).rgb;
    
    float3 g = InputTexture.SampleLevel(LinearClampSampler, float2(pixel.x - 2.0f * texelSize.x, pixel.y - 2.0f * texelSize.y), 0).rgb;
    float3 h = InputTexture.SampleLevel(LinearClampSampler, float2(pixel.x                     , pixel.y - 2.0f * texelSize.y), 0).rgb;
    float3 i = InputTexture.SampleLevel(LinearClampSampler, float2(pixel.x + 2.0f * texelSize.x, pixel.y - 2.0f * texelSize.y), 0).rgb;
    
    float3 j = InputTexture.SampleLevel(LinearClampSampler, float2(pixel.x - texelSize.x, pixel.y + texelSize.y), 0).rgb;
    float3 k = InputTexture.SampleLevel(LinearClampSampler, float2(pixel.x + texelSize.x, pixel.y + texelSize.y), 0).rgb;
    float3 l = InputTexture.SampleLevel(LinearClampSampler, float2(pixel.x - texelSize.x, pixel.y - texelSize.y), 0).rgb;
    float3 m = InputTexture.SampleLevel(LinearClampSampler, float2(pixel.x + texelSize.x, pixel.y - texelSize.y), 0).rgb;
    
    OutputTexture[DTid.xy] = float4(e * 0.125f +
                           (j + k + l + m) * 0.125f +
                           (b + d + f + h) * 0.0625f +
                           (a + c + g + i) * 0.03125f, 1.0f);
}
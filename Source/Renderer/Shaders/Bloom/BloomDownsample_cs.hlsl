
#include "../UnifiedRootSignature.hlsli"
#include "../CommonResources.hlsli"
#include "../ToneMapping.hlsli"

struct PassConstants
{
    uint InputTextureIndex;
    uint OutputTextureIndex;
    
    float Gamma;
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
    
    float2 uv = (float2(DTid.xy) + 0.5f) / float2(width, height); // Current uv
    
    // Take 13 samples around current texel:
    // a - b - c
    // - j - k -
    // d - e - f
    // - l - m -
    // g - h - i
    
    float3 a = InputTexture.SampleLevel(LinearClampSampler, uv, 0, int2(-2,  2)).rgb;
    float3 b = InputTexture.SampleLevel(LinearClampSampler, uv, 0, int2( 0,  2)).rgb;
    float3 c = InputTexture.SampleLevel(LinearClampSampler, uv, 0, int2( 2,  2)).rgb;
    
    float3 d = InputTexture.SampleLevel(LinearClampSampler, uv, 0, int2(-2,  0)).rgb;
    float3 e = InputTexture.SampleLevel(LinearClampSampler, uv, 0, int2( 0,  0)).rgb;
    float3 f = InputTexture.SampleLevel(LinearClampSampler, uv, 0, int2( 2,  0)).rgb;
    
    float3 g = InputTexture.SampleLevel(LinearClampSampler, uv, 0, int2(-2, -2)).rgb;
    float3 h = InputTexture.SampleLevel(LinearClampSampler, uv, 0, int2( 0, -2)).rgb;
    float3 i = InputTexture.SampleLevel(LinearClampSampler, uv, 0, int2( 2, -2)).rgb;
    
    float3 j = InputTexture.SampleLevel(LinearClampSampler, uv, 0, int2(-1,  1)).rgb;
    float3 k = InputTexture.SampleLevel(LinearClampSampler, uv, 0, int2( 1,  1)).rgb;
    float3 l = InputTexture.SampleLevel(LinearClampSampler, uv, 0, int2(-1, -1)).rgb;
    float3 m = InputTexture.SampleLevel(LinearClampSampler, uv, 0, int2( 1, -1)).rgb;
    
#ifdef FIRST_PASS
    float3 groups[5];
    
    groups[0] = (a + b + d + e) * 0.03125f;
    groups[1] = (b + c + e + f) * 0.03125f;
    groups[2] = (d + e + g + h) * 0.03125f;
    groups[3] = (e + f + h + i) * 0.03125f;
    groups[4] = (j + k + l + m) * 0.125f;
    
    groups[0] *= KarisAverage(groups[0], PassCB.Gamma);
    groups[1] *= KarisAverage(groups[1], PassCB.Gamma);
    groups[2] *= KarisAverage(groups[2], PassCB.Gamma);
    groups[3] *= KarisAverage(groups[3], PassCB.Gamma);
    groups[4] *= KarisAverage(groups[4], PassCB.Gamma);
    
    float3 outColor = groups[0] + groups[1] + groups[2] + groups[3] + groups[4];
#else
    float3 outColor = e * 0.125f +
                      (j + k + l + m) * 0.125f +
                      (b + d + f + h) * 0.0625f +
                      (a + c + g + i) * 0.03125f;
#endif
    outColor = max(0.00001f, outColor); // Avoid NaNs
    
    OutputTexture[DTid.xy] = float4(outColor, 1.0f);
}

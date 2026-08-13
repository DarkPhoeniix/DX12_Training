
#include "../UnifiedRootSignature.hlsli"
#include "../CommonResources.hlsli"

struct PassConstants
{
    uint BloomTextureIndex;
    uint HDRTextureIndex;
    float BloomStrength;
};

URootConstants(PassConstants, PassCB);

[numthreads(16, 16, 1)]
[RootSignature(URootSignature)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    Texture2D BloomTexture = ResourceDescriptorHeap[PassCB.BloomTextureIndex];
    RWTexture2D<float4> HDRTexture = ResourceDescriptorHeap[PassCB.HDRTextureIndex];
    
    uint width, height;
    HDRTexture.GetDimensions(width, height);
    
    if (DTid.x > width || DTid.y > height)
    {
        return;
    }
    
    float2 uv = DTid.xy / float2(width, height); // Current pixel
    
    float4 color = HDRTexture[DTid.xy];
    float4 bloom = BloomTexture.SampleLevel(LinearClampSampler, uv, 0);
    
    HDRTexture[DTid.xy] = float4(color.rgb + bloom.rgb * PassCB.BloomStrength, color.a);
}


#include "../UnifiedRootSignature.hlsli"
#include "../CommonResources.hlsli"
#include "../ToneMapping.hlsli"

struct PassConstants
{
    uint InputTextureIndex;
    uint OutputTextureIndex;
    float Threshold;
};

URootConstants(PassConstants, PassCB);

[numthreads(16, 16, 1)]
[RootSignature(URootSignature)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    Texture2D InputTexture = ResourceDescriptorHeap[PassCB.InputTextureIndex];
    RWTexture2D<float4> OutputTexture = ResourceDescriptorHeap[PassCB.OutputTextureIndex];
    
    uint width, height;
    InputTexture.GetDimensions(width, height);
    
    uint2 pixel = DTid.xy;
    
    if (pixel.x >= width || pixel.y >= height)
    {
        return;
    }
    
    if (Luminance(InputTexture[pixel].rgb)  >= PassCB.Threshold)
    {
        OutputTexture[pixel] = InputTexture[pixel];
    }
    else
    {
        OutputTexture[pixel] = float4(0.0f, 0.0f, 0.0f, 1.0f);
    }
}

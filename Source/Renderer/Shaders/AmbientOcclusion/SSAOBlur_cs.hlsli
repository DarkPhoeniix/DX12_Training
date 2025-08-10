
#include "../UnifiedRootSignature.hlsli"
#include "../CommonResources.hlsli"
#include "../DepthFuncs.hlsli"

#define NUM_THREADS 16

struct PassConstants
{
    int Radius;
    float DepthThreshold;
    float Sharpness;
    
    uint WeightsBufferIndex;
    uint DepthTextureIndex;
    uint InputTextureIndex;
    uint OutputTextureIndex;
};

ConstantBuffer<PassConstants> PassCB : register(b1);

inline float LinearDepth(in float zBufferSample, in float A, in float B)
{
    return A / (zBufferSample - B);
}

[numthreads(NUM_THREADS, NUM_THREADS, 1)]
[RootSignature(URootSignature)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    StructuredBuffer<float> Weights = ResourceDescriptorHeap[PassCB.WeightsBufferIndex];
    Texture2D<float> Depth          = ResourceDescriptorHeap[PassCB.DepthTextureIndex];
    Texture2D<float> Input          = ResourceDescriptorHeap[PassCB.InputTextureIndex];
    RWTexture2D<float> Output       = ResourceDescriptorHeap[PassCB.OutputTextureIndex];
    
    uint2 pixel = DTid.xy;
    
    uint targetWidth, targetHeight;
    Depth.GetDimensions(targetWidth, targetHeight);
    
    if (pixel.x >= targetWidth || pixel.y >= targetHeight)
    {
        return;
    }
    
    float centerZ = Depth.Load(int3(pixel, 0));
    centerZ = LinearDepth(centerZ, FrameCB.Projection[3][2], FrameCB.Projection[2][2]);
    float sum = 0;
    float weight = 0;
    
    for (int i = -PassCB.Radius; i <= PassCB.Radius; ++i)
    {
#ifdef BLUR_VERTICAL
        int2 s = int2(pixel.x, pixel.y + i);
#else
        int2 s = int2(pixel.x + i, pixel.y);
#endif
        if (s.x < 0 || s.x >= targetWidth || s.y < 0 || s.y >= targetHeight)
        {
            continue;
        }
        
        float z = Depth[s];
        z = LinearDepth(z, FrameCB.Projection[3][2], FrameCB.Projection[2][2]);
        float dz = abs(z - centerZ);
        
        float dWeight = (dz < PassCB.DepthThreshold) ? 1.0 : exp(-(dz - PassCB.DepthThreshold) * PassCB.Sharpness);
        
        float w = Weights[PassCB.Radius + i] * dWeight;
        sum += Input[s] * w;
        weight += w;
    }

    Output[pixel] = sum / weight;
}
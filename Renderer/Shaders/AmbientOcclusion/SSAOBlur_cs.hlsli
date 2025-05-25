
#define SSAOBlur_RootSig \
	"RootFlags(0), " \
    "CBV(b0, visibility = SHADER_VISIBILITY_ALL), " \
    "CBV(b1, visibility = SHADER_VISIBILITY_ALL), " \
    "SRV(t0, visibility = SHADER_VISIBILITY_ALL), " \
    "DescriptorTable(SRV(t1), visibility = SHADER_VISIBILITY_ALL), " \
    "DescriptorTable(SRV(t2), visibility = SHADER_VISIBILITY_ALL), " \
    "DescriptorTable(UAV(u0), visibility = SHADER_VISIBILITY_ALL), " \
    "StaticSampler(s0," \
        "addressU = TEXTURE_ADDRESS_CLAMP," \
        "addressV = TEXTURE_ADDRESS_CLAMP," \
        "addressW = TEXTURE_ADDRESS_CLAMP," \
        "filter = FILTER_MIN_MAG_MIP_POINT)"

#include "../CommonResources.hlsli"
#include "../DepthFuncs.hlsli"

#define NUM_THREADS 16

struct Constants
{
    int Radius;
    float DepthThreshold;
    float Sharpness;
};

ConstantBuffer<Constants> CB    : register(b1);
StructuredBuffer<float> Weights : register(t0);
Texture2D<float> Depth          : register(t1);
Texture2D<float> Input          : register(t2);
RWTexture2D<float> Output       : register(u0);

SamplerState PointerSampler     : register(s0);

inline float LinearDepth(in float zBufferSample, in float A, in float B)
{
    return A / (zBufferSample - B);
}

[numthreads(NUM_THREADS, NUM_THREADS, 1)]
[RootSignature(SSAOBlur_RootSig)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint2 pixel = DTid.xy;
    
    uint targetWidth, targetHeight;
    Depth.GetDimensions(targetWidth, targetHeight);
    
    if (pixel.x >= targetWidth || pixel.y >= targetHeight)
    {
        return;
    }
    
    float centerZ = Depth.Load(int3(pixel, 0));
    centerZ = LinearDepth(centerZ, Scene.Projection[3][2], Scene.Projection[2][2]);
    float sum = 0;
    float weight = 0;
    
    for (int i = -CB.Radius; i <= CB.Radius; ++i)
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
        z = LinearDepth(z, Scene.Projection[3][2], Scene.Projection[2][2]);
        float dz = abs(z - centerZ);
        
        float dWeight = (dz < CB.DepthThreshold) ? 1.0 : exp(-(dz - CB.DepthThreshold) * CB.Sharpness);
        
        float w = Weights[CB.Radius + i] * dWeight;
        sum += Input[s] * w;
        weight += w;
    }

    Output[pixel] = sum / weight;
}
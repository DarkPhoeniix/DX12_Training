
#define SSAOBlur_RootSig \
	"RootFlags(0), " \
    "CBV(b0, visibility = SHADER_VISIBILITY_ALL), " \
    "DescriptorTable(SRV(t0), visibility = SHADER_VISIBILITY_ALL), " \
    "DescriptorTable(SRV(t1), visibility = SHADER_VISIBILITY_ALL), " \
    "DescriptorTable(UAV(u0), visibility = SHADER_VISIBILITY_ALL), " \
    "StaticSampler(s0," \
        "addressU = TEXTURE_ADDRESS_CLAMP," \
        "addressV = TEXTURE_ADDRESS_CLAMP," \
        "addressW = TEXTURE_ADDRESS_CLAMP," \
        "filter = FILTER_MIN_MAG_MIP_POINT)"

#include "../CommonResources.hlsli"
#include "../DepthFuncs.hlsli"

#define NUM_THREADS 16

Texture2D<float> Depth      : register(t0);
Texture2D<float> Input      : register(t1);
RWTexture2D<float> Output   : register(u0);

SamplerState PointerSampler : register(s0);

static const int Radius = 4;
static const float DepthThreshold = 0.2f;
static const float Sharpness = 50.0f;
static const float Weights[5] =
{
    1.0000000f,
    0.8824969f,
    0.6065307f,
    0.3246525f, 
    0.1353353f
};

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
    
    [unroll]
    for (int i = -Radius; i <= Radius; ++i)
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
        
        float dWeight = (dz < DepthThreshold) ? 1.0 : exp(-(dz - DepthThreshold) * Sharpness);
        
        float w = Weights[abs(i)] * dWeight;
        sum += Input[s] * w;
        weight += w;
    }

    Output[pixel] = sum / weight;
}

#define SSAOCompute_RootSig \
	"RootFlags(0), " \
    "CBV(b0, visibility = SHADER_VISIBILITY_ALL), " \
    "SRV(t0, visibility = SHADER_VISIBILITY_ALL), " \
    "SRV(t1, visibility = SHADER_VISIBILITY_ALL), " \
    "DescriptorTable(SRV(t2), visibility = SHADER_VISIBILITY_ALL), " \
    "DescriptorTable(SRV(t3), visibility = SHADER_VISIBILITY_ALL), " \
    "DescriptorTable(UAV(u0), visibility = SHADER_VISIBILITY_ALL), " \
    "StaticSampler(s0," \
        "addressU = TEXTURE_ADDRESS_CLAMP," \
        "addressV = TEXTURE_ADDRESS_CLAMP," \
        "addressW = TEXTURE_ADDRESS_CLAMP," \
        "filter = FILTER_MIN_MAG_MIP_POINT)"

#include "../CommonResources.hlsli"
#include "../DepthFuncs.hlsli"

#define NUM_THREADS 16

StructuredBuffer<float4> Kernel : register(t0);
StructuredBuffer<float4> Noise  : register(t1);
Texture2D<float> Depth          : register(t2);
Texture2D<float4> NormalMap     : register(t3);
RWTexture2D<float> AOTexture    : register(u0);

SamplerState PointerSampler     : register(s0);

static const uint KernelSize = 16;
static const float Radius = 2.5f;
static const float Bias = 0.025f;

static float3 GetViewPosition(float2 texcoord, float depth)
{
    float4 clipSpaceLocation;
    clipSpaceLocation.xy = texcoord * 2.0f - 1.0f;
    clipSpaceLocation.y *= -1;
    clipSpaceLocation.z = depth;
    clipSpaceLocation.w = 1.0f;
    float4 homogenousLocation = mul(clipSpaceLocation, Scene.InvProjection);
    return homogenousLocation.xyz / homogenousLocation.w;
}

inline float LinearDepth(in float zBufferSample, in float A, in float B)
{
    return A / (zBufferSample - B);
}
[numthreads(NUM_THREADS, NUM_THREADS, 1)]
[RootSignature(SSAOCompute_RootSig)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint2 pixel = DTid.xy;
    
    uint ScreenWidth, ScreenHeight;
    Depth.GetDimensions(ScreenWidth, ScreenHeight);
    
    if (pixel.x >= ScreenWidth || pixel.y >= ScreenHeight)
    {
        return;
    }
    
    float2 uv = float2(
        (((float) pixel.x + 0.5f) / (float) ScreenWidth),
        (((float) pixel.y + 0.5f) / (float) ScreenHeight)
    );

    float depth = Depth.SampleLevel(PointerSampler, uv, 0.0f);
    float3 positionVS = GetViewPosition(uv, depth);
    
    float3 normalWS = normalize(NormalMap.Load(int3(pixel, 0)).xyz);
    float3 normalVS = normalize(mul(normalWS, (float3x3) Scene.View));
    
    uint noiseIndex = (pixel.y % 8) * 8 + (pixel.x % 8);
    float3 randomVector = normalize(float3(Noise[noiseIndex].xy, 0.0f));
    
    float3 tangentVS = normalize(randomVector - normalVS * dot(randomVector, normalVS));
    float3 bitangentVS = cross(normalVS, tangentVS);
    float3x3 TBN = float3x3(tangentVS, bitangentVS, normalVS);
    
    float occlusion = 0.0f;
    for (uint i = 0; i < KernelSize; ++i)
    {
        float3 sampleVS = positionVS + mul(Kernel[i].xyz, TBN) * Radius;
        
        float4 offset = float4(sampleVS, 1.0);
        offset = mul(offset, Scene.Projection);
        offset.xy = ((offset.xy / offset.w) * float2(1.0f, -1.0f)) * 0.5f + 0.5f;
        
        float sampleDepth = Depth.SampleLevel(PointerSampler, offset.xy, 0.0f);
        sampleDepth = LinearDepth(sampleDepth, Scene.Projection[3][2], Scene.Projection[2][2]);
        
        float rangeCheck = smoothstep(0.0f, 1.0f, Radius / abs(positionVS.z - sampleDepth));
        if (sampleDepth < sampleVS.z - Bias)
        {
            occlusion += rangeCheck;
        }
    }
    occlusion = 1.0f - (occlusion / KernelSize);
    
    AOTexture[pixel] = pow(occlusion, 1.5f);
}

#include "../UnifiedRootSignature.hlsli"
#include "../CommonResources.hlsli"
#include "../DepthFuncs.hlsli"

#define NUM_THREADS 16

struct PassConstants
{
    uint KernelSize;
    float Radius;
    float Bias;
    
    uint KernelBufferIndex;
    uint NoiseBufferIndex;
    uint DepthTextureIndex;
    uint NormalMapTextureIndex;
    uint AmbientOcclusionTextureIndex;
};

URootConstants(PassConstants, PassCB);

// TODO: remove GetViewPosition
static float3 GetViewPosition(float2 texcoord, float depth)
{
    float4 clipSpaceLocation;
    clipSpaceLocation.xy = texcoord * 2.0f - 1.0f;
    clipSpaceLocation.y *= -1;
    clipSpaceLocation.z = depth;
    clipSpaceLocation.w = 1.0f;
    float4 homogenousLocation = mul(clipSpaceLocation, FrameCB.InvProjection);
    return homogenousLocation.xyz / homogenousLocation.w;
}

// TODO: remove LinearDepth
inline float LinearDepth(in float zBufferSample, in float A, in float B)
{
    return A / (zBufferSample - B);
}

[numthreads(NUM_THREADS, NUM_THREADS, 1)]
[RootSignature(URootSignature)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    StructuredBuffer<float4> Kernel = ResourceDescriptorHeap[PassCB.KernelBufferIndex];
    StructuredBuffer<float4> Noise  = ResourceDescriptorHeap[PassCB.NoiseBufferIndex];
    Texture2D<float> Depth          = ResourceDescriptorHeap[PassCB.DepthTextureIndex];
    Texture2D<float4> NormalMap     = ResourceDescriptorHeap[PassCB.NormalMapTextureIndex];
    RWTexture2D<float> AOTexture    = ResourceDescriptorHeap[PassCB.AmbientOcclusionTextureIndex];

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

    float depth = Depth.SampleLevel(PointClampSampler, uv, 0.0f);
    float3 positionVS = GetViewPosition(uv, depth);
    
    float3 normalWS = normalize(NormalMap.Load(int3(pixel, 0)).xyz);
    float3 normalVS = normalize(mul(normalWS, (float3x3) FrameCB.View));
    
    uint noiseIndex = (pixel.y % 8) * 8 + (pixel.x % 8);
    float3 randomVector = normalize(float3(Noise[noiseIndex].xy, 0.0f));
    
    float3 tangentVS = normalize(randomVector - normalVS * dot(randomVector, normalVS));
    float3 bitangentVS = cross(normalVS, tangentVS);
    float3x3 TBN = float3x3(tangentVS, bitangentVS, normalVS);
    
    float occlusion = 0.0f;
    for (uint i = 0; i < PassCB.KernelSize; ++i)
    {
        float3 sampleVS = positionVS + mul(Kernel[i].xyz, TBN) * PassCB.Radius;
        
        float4 offset = float4(sampleVS, 1.0);
        offset = mul(offset, FrameCB.Projection);
        offset.xy = ((offset.xy / offset.w) * float2(1.0f, -1.0f)) * 0.5f + 0.5f;
        
        float sampleDepth = Depth.SampleLevel(PointClampSampler, offset.xy, 0.0f);
        sampleDepth = LinearDepth(sampleDepth, FrameCB.Projection[3][2], FrameCB.Projection[2][2]);
        
        float rangeCheck = smoothstep(0.0f, 1.0f, PassCB.Radius / abs(positionVS.z - sampleDepth));
        if (sampleDepth < sampleVS.z - PassCB.Bias)
        {
            occlusion += rangeCheck;
        }
    }
    occlusion = 1.0f - (occlusion / PassCB.KernelSize);
    
    AOTexture[pixel] = pow(occlusion, 1.5f);
}
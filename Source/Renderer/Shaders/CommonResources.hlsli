#pragma once

struct FrameConstants   // 400 bytes
{
    row_major matrix    View;
    row_major matrix    Projection;
    row_major matrix    ViewProjection;
    
    row_major matrix    InvView;
    row_major matrix    InvProjection;
    
    float4              EyePosition;
    float4              EyeDirection;
    
    uint2               WindowSize;
    float2              ReciprocalWindowSize;
    float2              NearFar;
    
    uint                InstancesBufferIndex;
    uint                LightsBufferIndex;
    uint                LightsNum;
    
    float               DeltaTime;
    uint                pad[2];
};

struct ModelDesc        // 96 bytes
{
    row_major matrix    Transform;
    
    uint                AlbedoTextureIndex;
    uint                NormalTextureIndex;
    uint                MetalnessTextureIndex;
    uint                RoughnessTextureIndex;
    
    uint                HasMesh;
    uint                BonesBufferIndex;
    uint                pad[2];
};

////////////////////////////////////////////////////////////////////////////////

ConstantBuffer<FrameConstants> FrameCB      : register(b0);

SamplerState PointClampSampler              : register(s0);
SamplerState PointWrapSampler               : register(s1);
SamplerState PointMirrorSampler             : register(s2);
SamplerState PointBorderSampler             : register(s3);

SamplerState LinearClampSampler             : register(s4);
SamplerState LinearWrapSampler              : register(s5);
SamplerState LinearMirrorSampler            : register(s6);
SamplerState LinearBorderSampler            : register(s7);

SamplerComparisonState ShadowClampSampler   : register(s8);

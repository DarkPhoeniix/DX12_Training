
struct SceneDesc
{
    row_major matrix ViewProjection;
    float4 EyePosition;
    float4 EyeDirection;
    
    uint LightsNum;
};

struct ModelDesc
{
    row_major matrix Transform;
    
    uint AlbedoTextureIndex;
    uint NormalTextureIndex;
    uint MetalnessTextureIndex;
};

////////////////////////////////////////////////////////////////////////////////

ConstantBuffer<SceneDesc> Scene : register(b0);
ConstantBuffer<ModelDesc> Model : register(b1);

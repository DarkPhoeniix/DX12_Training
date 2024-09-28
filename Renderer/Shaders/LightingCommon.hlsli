
struct Surface
{
    float4 Positon;
    
    float4 Albedo;
    float4 Normal;
    float4 Metalness;
    
    float4 FinalColor;
};

#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1

struct LightDesc
{
    float4 Position;
    float4 Direction;
    float4 Color;
    
    float Intesity;
    float Range;
    
    uint Type;
    
    uint pad;
};

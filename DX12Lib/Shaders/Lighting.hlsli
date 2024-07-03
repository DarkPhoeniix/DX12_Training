
struct DirectionalLightDesc
{
    float4 Direction;
    float4 Color;
};

struct PointLightDesc
{
    float4 Position;
    float4 Color;
    float3 Attenuation;
};

struct LightPropertiesDesc
{
    uint DirectionalLightsNum;
    uint PointLightsNum;
};

struct LightResultDesc
{
    float4 Diffuse;
    float4 Specular;
    float4 Ambient;
};




float3 CalculateAmbient(float4 albedo)
{
    return 0.1 * albedo;
}

float3 CalculateDiffuse(float3 surfaceNormal, float4 surfaceColor, float3 lightDirection, float4 lightColor)
{
    float diffuseFactor = max(dot(surfaceNormal, -lightDirection), 0.0f);
    
    return surfaceColor * lightColor * diffuseFactor;
}

float3 CalculateSpecular(float3 surfaceNormal, float4 surfaceColor, float3 lightDirection, float4 lightColor)
{
    float diffuseFactor = max(dot(surfaceNormal, -lightDirection), 0.0f);
    
    return surfaceColor * lightColor * diffuseFactor;
}

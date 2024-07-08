
float4 CalculateAmbient(float4 albedo)
{
    return 0.1f * albedo;
}

float4 CalculateDiffuse(float3 surfaceNormal, float4 surfaceColor, float3 lightDirection, float4 lightColor)
{
    float diffuseFactor = max(dot(surfaceNormal, -lightDirection), 0.0f);
    
    return surfaceColor * lightColor * diffuseFactor;
}

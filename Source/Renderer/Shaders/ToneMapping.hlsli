
static const float4 LUM_FACTOR = float4(0.2126729f, 0.7151522f, 0.0721750f, 0.0f);

static float Luminance(float3 color)
{
    return dot(color, LUM_FACTOR.rgb);
}

static float3 LinearToRGB(float3 color, float gamma)
{
    return pow(color, 1.0f / gamma);
}

float KarisAverage(float3 color, float gamma)
{
    float luma = Luminance(LinearToRGB(color, gamma)) * 0.25f;
    return 1.0f / (1.0f + luma);
}

float3 ReinhardToneMapping(float3 HDRColor, float gamma)
{
    // Find the luminance scale for the current pixel
    float LScale = Luminance(HDRColor);
    LScale = (LScale) / (1.0 + LScale);
    
    // Apply the luminance scale to the pixels color
    return LinearToRGB(HDRColor * LScale, gamma);
}

float3 ExtendedReinhardToneMapping(float3 HDRColor, float averageLum, float middleGrey, float lumWhiteSqr, float gamma)
{
    // Find the luminance scale for the current pixel
    float LScale = Luminance(HDRColor);
    LScale *= middleGrey / averageLum;
    LScale = (LScale + LScale * LScale / lumWhiteSqr) / (1.0 + LScale);
    
    // Apply the luminance scale to the pixels color
    return LinearToRGB(HDRColor * LScale, gamma);
}

float3 HableToneMapping(float3 HDRColor, float gamma)
{
    float A = 0.15f;
    float B = 0.50f;
    float C = 0.10f;
    float D = 0.20f;
    float E = 0.02f;
    float F = 0.30f;
    float exposure_bias = 2.0f;
    float3 white = float3(1.0f, 1.0f, 1.0f);
    
    float3 color = HDRColor * exposure_bias;
    color = ((color * (A * color + C * B) + D * E) / (color * (A * color + B) + D * F)) - E / F;
    white = ((white * (A * white + C * B) + D * E) / (white * (A * white + B) + D * F)) - E / F;
    color /= white;
    
    return LinearToRGB(color, gamma);
}

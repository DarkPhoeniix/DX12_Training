
// https://developer.download.nvidia.com/assets/gamedev/files/sdk/11/FXAA_WhitePaper.pdf
// https://blog.simonrodriguez.fr/articles/2016/07/implementing_fxaa.html
// https://gist.github.com/kosua20/0c506b81b3812ac900048059d2383126

#include "FXAA_rootsig.hlsli"

#include "Common.hlsli"
#include "FXAA_helpers.hlsli"

#define THREAD_NUM 8

#define EDGE_THRESHOLD_MIN 1.0f / 8.0f
#define EDGE_THRESHOLD_MAX 1.0f / 4.0f

#define FXAA_SUBPIX_TRIM 1.0f / 4.0f
#define FXAA_SUBPIX_TRIM_SCALE (1.0f / (1.0f - FXAA_SUBPIX_TRIM))
#define FXAA_SUBPIX_CAP 7.0f / 8.0f

#define FXAA_SEARCH_STEPS 1
#define FXAA_SEARCH_ACCELERATION 1
#define FXAA_SEARCH_THRESHOLD 1.0f / 4.0f

Texture2D TargetTexture         : register(t1);
RWTexture2D<float4> AATexture   : register(u0);

SamplerState SamplerAnisotropic : register(s0);

[RootSignature(FXAA_RootSig)]
[numthreads(THREAD_NUM, THREAD_NUM, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    if (DTid.x > Scene.WindowSize.x || DTid.y > Scene.WindowSize.y)
    {
        return;
    }
    
    int xDim, yDim, zDim;
    TargetTexture.GetDimensions(0, xDim, yDim, zDim);
    
    float2 uv = float2(
        DTid.x / (float)xDim, 
        DTid.y / (float)yDim
    );
    
    // Color at the four direct neighbours of the current pixel
    float3 rgbM = TextureOffset(SamplerAnisotropic, TargetTexture, uv).rgb;
    float3 rgbS = TextureOffset(SamplerAnisotropic, TargetTexture, uv, int2( 0, -1)).rgb;
    float3 rgbN = TextureOffset(SamplerAnisotropic, TargetTexture, uv, int2( 0,  1)).rgb;
    float3 rgbW = TextureOffset(SamplerAnisotropic, TargetTexture, uv, int2(-1,  0)).rgb;
    float3 rgbE = TextureOffset(SamplerAnisotropic, TargetTexture, uv, int2( 1,  0)).rgb;
    
    // Luma at the four direct neighbours of the current pixel
    float lumaM = rgbToLuma(rgbM);
    float lumaS = rgbToLuma(rgbS);
    float lumaN = rgbToLuma(rgbN);
    float lumaW = rgbToLuma(rgbW);
    float lumaE = rgbToLuma(rgbE);

    // Find the maximum and minimum luma around the current pixel
    float lumaMin = min(lumaM, min(min(lumaS, lumaN), min(lumaW, lumaE)));
    float lumaMax = max(lumaM, max(max(lumaS, lumaN), max(lumaW, lumaE)));

    // Compute the delta
    float lumaRange = lumaMax - lumaMin;
    
    // If the luma variation is lower that a threshold (or if we are in a really dark area), we are not on an edge, don't perform any AA.
    if (lumaRange < max(EDGE_THRESHOLD_MIN, lumaMax * EDGE_THRESHOLD_MAX))
    {
        AATexture[DTid.xy] = float4(rgbM, 1.0f);
        return;
    }
    
    float lumaL = (lumaN + lumaW + lumaE + lumaS) * 0.25f;
    float rangeL = abs(lumaL - lumaM);
    float blendL = max(0.0f, (rangeL / lumaRange) - FXAA_SUBPIX_TRIM) * FXAA_SUBPIX_TRIM_SCALE;
    blendL = min(FXAA_SUBPIX_CAP, blendL);
    
    float3 rgbL = rgbN + rgbW + rgbM + rgbE + rgbS;
    
    // Color at the four corners of the current pixel
    float3 rgbNW = TextureOffset(SamplerAnisotropic, TargetTexture, uv, int2(-1, -1)).rgb;
    float3 rgbNE = TextureOffset(SamplerAnisotropic, TargetTexture, uv, int2( 1, -1)).rgb;
    float3 rgbSW = TextureOffset(SamplerAnisotropic, TargetTexture, uv, int2(-1,  1)).rgb;
    float3 rgbSE = TextureOffset(SamplerAnisotropic, TargetTexture, uv, int2( 1,  1)).rgb;
    
    // Luma at the four corners of the current pixel
    float lumaNW = rgbToLuma(rgbNW);
    float lumaNE = rgbToLuma(rgbNE);
    float lumaSW = rgbToLuma(rgbSW);
    float lumaSE = rgbToLuma(rgbSE);

    rgbL += (rgbNW + rgbNE + rgbSW + rgbSE);
    rgbL *= float3(1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f);
    
    // Compute an estimation of the gradient along the horizontal and vertical axis.
    float edgeVertical =
               abs(lumaNW + (-2.0f * lumaN) + lumaNE) +
        2.0f * abs(lumaW  + (-2.0f * lumaM) + lumaE ) +
               abs(lumaSW + (-2.0f * lumaS) + lumaSE);
    float edgeHorizontal =
               abs(lumaNW + (-2.0f * lumaW) + lumaSW) +
        2.0f * abs(lumaN  + (-2.0f * lumaM) + lumaS ) +
               abs(lumaNE + (-2.0f * lumaE) + lumaSE);
    bool isHorizontal = edgeHorizontal >= edgeVertical;
    
    float lengthSign = isHorizontal ? -Scene.ReciprocalWindowSize.y : -Scene.ReciprocalWindowSize.x;
    if (!isHorizontal)
    {
        lumaN = lumaW;
        lumaS = lumaE;
    }
    float gradientN = abs(lumaN - lumaM);
    float gradientS = abs(lumaS - lumaM);
    lumaN = (lumaN + lumaM) * 0.5f;
    lumaS = (lumaS + lumaM) * 0.5f;
    
    //CHOOSE SIDE OF PIXEL WHERE GRADIENT IS HIGHEST
    bool pairN = gradientN >= gradientS;
    if (!pairN)
    {
        lumaN = lumaS;
        gradientN = gradientS;
        lengthSign *= -1.0f;
    }
    float2 posN;
    posN.x = uv.x + (isHorizontal ? 0.0f : lengthSign * 0.5f);
    posN.y = uv.y + (isHorizontal ? lengthSign * 0.5f : 0.0f);
    
    gradientN *= FXAA_SEARCH_THRESHOLD;
    
    float2 posP = posN;
    float2 offNP = isHorizontal ?
        float2(Scene.ReciprocalWindowSize.x, 0.0f) :
        float2(0.0f, Scene.ReciprocalWindowSize.y);
    float lumaEndN = lumaN;
    float lumaEndP = lumaN;
    bool doneN = false;
    bool doneP = false;
#if FXAA_SEARCH_ACCELERATION == 1
    posN += offNP * float2(-1.0f, -1.0f);
    posP += offNP * float2( 1.0f,  1.0f);
#endif
    for (int i = 0; i < FXAA_SEARCH_STEPS; i++)
    {
#if FXAA_SEARCH_ACCELERATION == 1
        if (!doneN)
        {
            lumaEndN = rgbToLuma(TextureOffset(SamplerAnisotropic, TargetTexture, posN.xy).rgb);
        }
        if (!doneP)
        {
            lumaEndN = rgbToLuma(TextureOffset(SamplerAnisotropic, TargetTexture, posP.xy).rgb);
        }
#endif
        doneN = doneN || (abs(lumaEndN - lumaN) >= gradientN);
        doneP = doneP || (abs(lumaEndP - lumaN) >= gradientN);
        if (doneN && doneP)
            break;
        if (!doneN)
            posN -= offNP;
        if (!doneP)
            posP += offNP;
    }
    
    float dstN = isHorizontal ? uv.x - posN.x : uv.y - posN.y;
    float dstP = isHorizontal ? posP.x - uv.x : posP.y - uv.y;
    
    bool directionN = dstN < dstP;
    lumaEndN = directionN ? lumaEndN : lumaEndP;
    
    if (((lumaM - lumaN) < 0.0f) == ((lumaEndN - lumaN) < 0.0f))
    {
        lengthSign = 0.0f;
    }
    
    float spanLength = (dstP + dstN);
    dstN = directionN ? dstN : dstP;
    float subPixelOffset = (0.5f + (dstN * (-1.0f / spanLength))) * lengthSign;
    float3 rgbF = TextureOffset(SamplerAnisotropic, TargetTexture, float2(
        uv.x + (isHorizontal ? 0.0 : subPixelOffset),
        uv.y + (isHorizontal ? subPixelOffset : 0.0f))).rgb;
    
    AATexture[DTid.xy] = float4(lerp(rgbL, rgbF, blendL), 1.0f);
}
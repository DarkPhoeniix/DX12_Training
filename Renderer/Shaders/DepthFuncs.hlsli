
// converting depth value into linear 
float LinearizeDepth(float depth, float2 NearFar)
{
    float zNear = NearFar.x; //	0.01f;
    float zFar = NearFar.y; //1000.0;
    return (2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear));
}

// converting depth value into linear 
float LinearizeDepthRev(float depth, float2 NearFar)
{
    float zNear = NearFar.x; //	0.01f;
    float zFar = NearFar.y; //1000.0;
    return (2.0 * zFar) / (zFar + zNear - depth * (zNear - zFar));
}
 
// Convert clip space coordinates to view space
float4 ClipToView(in float4 clip, in float4x4 ProjInv)
{
    // View space position.
    float4 view = mul(clip, ProjInv);
	
    // Perspective projection.
    view = view / view.w;
 
    return view;
}
 
// Convert screen space coordinates to view space.
float4 ScreenToView(in float4 screen, in float4x4 ProjInv, in float2 ScreenDimension)
{
    // Convert to normalized texture coordinates
    float2 texCoord = screen.xy / ScreenDimension;
 
    // Convert to clip space
    float4 clip = float4(float2(texCoord.x, 1.0f - texCoord.y) * 2.0f - 1.0f, screen.z, screen.w);
 
    return ClipToView(clip, ProjInv);
}

float4 ReconstructPosW(in float depth, in float2 posInScreen, in float2 ScreenDimension, in float4x4 ProjInv, in float4x4 ViewInv)
{
	// reconstruction position in View space
    float4 posV = ScreenToView(float4(posInScreen, depth, 1.0f), ProjInv, ScreenDimension);
	
	
	// posV.a = 1.0f;  // not sure about that instruction. .w of view space should be 1.0,need to check debugger
	// yes, posV.a is equal to 1.0, all fine
	
	// reconstruct position in World space
    float4 posW = mul(posV, ViewInv);
    return posW;
}
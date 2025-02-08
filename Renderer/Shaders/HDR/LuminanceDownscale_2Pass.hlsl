
#define LuminanceDownscale_2Pass_RootSig \
    "RootFlags " \
	"( " \
		"DENY_VERTEX_SHADER_ROOT_ACCESS | " \
		"DENY_HULL_SHADER_ROOT_ACCESS | " \
		"DENY_DOMAIN_SHADER_ROOT_ACCESS | " \
		"DENY_GEOMETRY_SHADER_ROOT_ACCESS | " \
		"DENY_PIXEL_SHADER_ROOT_ACCESS " \
	"), " \
    "RootConstants(num32BitConstants = 4, b0, visibility = SHADER_VISIBILITY_ALL), " \
    "DescriptorTable(SRV(t0), visibility = SHADER_VISIBILITY_ALL)," \
    "UAV(u0, visibility = SHADER_VISIBILITY_ALL), " \
    "StaticSampler(s0," \
        "addressU = TEXTURE_ADDRESS_WRAP," \
        "addressV = TEXTURE_ADDRESS_WRAP," \
        "addressW = TEXTURE_ADDRESS_WRAP," \
        "filter = FILTER_MIN_MAG_MIP_POINT)"

#define MAX_GROUPS 64

cbuffer DownscaleConstants : register(b0)
{
    // Resolution of the down scaled target: x - width, y - height
    uint2 Res : packoffset(c0);
    // Total pixel in the downscaled image
    uint Domain : packoffset(c0.z);
    // Number of groups dispached on the first pass
    uint GroupSize : packoffset(c0.w);
}
StructuredBuffer<float> AverageValues1D : register(t0);
RWStructuredBuffer<float> AverageLum : register(u0);

// Group shared memory to store the intermediate results
groupshared float SharedAvgFinal[MAX_GROUPS];

[RootSignature(LuminanceDownscale_2Pass_RootSig)]
[numthreads(MAX_GROUPS, 1, 1)]
void main(uint3 groupId : SV_GroupID, uint3 groupThreadId : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID)
{
    // Fill the shared memory with the 1D values
    float avgLum = 0.0;
    if (dispatchThreadId.x < GroupSize)
    {
        avgLum = AverageValues1D[dispatchThreadId.x];
    }
    SharedAvgFinal[dispatchThreadId.x] = avgLum;
    GroupMemoryBarrierWithGroupSync(); // Sync before next step
    
    // Downscale from 64 to 16
    if (dispatchThreadId.x % 4 == 0)
    {
    // Calculate the luminance sum for this step
        float stepAvgLum = avgLum;
        stepAvgLum += dispatchThreadId.x + 1 < GroupSize ? SharedAvgFinal[dispatchThreadId.x + 1] : avgLum;
        stepAvgLum += dispatchThreadId.x + 2 < GroupSize ? SharedAvgFinal[dispatchThreadId.x + 2] : avgLum;
        stepAvgLum += dispatchThreadId.x + 3 < GroupSize ? SharedAvgFinal[dispatchThreadId.x + 3] : avgLum;
 
        // Store the results
        avgLum = stepAvgLum;
        SharedAvgFinal[dispatchThreadId.x] = stepAvgLum;
    }
    GroupMemoryBarrierWithGroupSync(); // Sync before next step
    
    // Downscale from 16 to 4
    if (dispatchThreadId.x % 16 == 0)
    {
    // Calculate the luminance sum for this step
        float stepAvgLum = avgLum;
        stepAvgLum += dispatchThreadId.x + 4  < GroupSize ? SharedAvgFinal[dispatchThreadId.x +  4] : avgLum;
        stepAvgLum += dispatchThreadId.x + 8  < GroupSize ? SharedAvgFinal[dispatchThreadId.x +  8] : avgLum;
        stepAvgLum += dispatchThreadId.x + 12 < GroupSize ? SharedAvgFinal[dispatchThreadId.x + 12] : avgLum;
        // Store the results
        avgLum = stepAvgLum;
        SharedAvgFinal[dispatchThreadId.x] = stepAvgLum;
    }
    GroupMemoryBarrierWithGroupSync(); // Sync before next step
    
    // Downscale from 4 to 1
    if (dispatchThreadId.x == 0)
    {
         // Calculate the average luminace
        float fFinalLumValue = avgLum;
        fFinalLumValue += dispatchThreadId.x + 16 < GroupSize ? SharedAvgFinal[dispatchThreadId.x + 16] : avgLum;
        fFinalLumValue += dispatchThreadId.x + 32 < GroupSize ? SharedAvgFinal[dispatchThreadId.x + 32] : avgLum;
        fFinalLumValue += dispatchThreadId.x + 48 < GroupSize ? SharedAvgFinal[dispatchThreadId.x + 48] : avgLum;
        fFinalLumValue /= 64.0;
        
        AverageLum[0] = fFinalLumValue;
    }
}
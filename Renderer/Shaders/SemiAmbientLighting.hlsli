
struct SemiAmbientDesc
{
    float4 Up;
    float4 Down;
};

float4 CalculateSemiAmbient(float3 normal, SemiAmbientDesc desc)
{
    // Convert from [-1, 1] to [0, 1]
    float up = normal.y * 0.5f + 0.5f;
    // Calculate the ambient value
    float4 ambient = desc.Down + up * desc.Up;

    // Apply the ambient value to the color
    return ambient;
}

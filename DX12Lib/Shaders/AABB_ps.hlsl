
struct PixelInput
{
    float4 position : SV_POSITION;
};

float4 main(PixelInput input) : SV_Target
{
    return float4(0, 1, 0, 1);
}

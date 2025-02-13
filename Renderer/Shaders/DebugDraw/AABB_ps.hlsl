
struct Pixelinput
{
    float4 position : SV_POSITION;
};

float4 main(Pixelinput input) : SV_Target
{
    return float4(0.0f, 1.0f, 0.0f, 1.0f);
}

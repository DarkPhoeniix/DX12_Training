
struct Color
{
    float4 C;
};

ConstantBuffer<Color> Col : register(b2);

float4 main() : SV_TARGET
{
    return Col.C;
}
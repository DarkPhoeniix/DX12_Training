
struct GeometryInput
{
    uint primitive : INDEX;
};

struct PixelInput
{
    float4 posH : SV_POSITION;
};

struct ViewData
{
    row_major matrix ViewProj;
};
ConstantBuffer<ViewData> View : register(b0);

struct MeshData
{
    row_major matrix Bounds;
};

ConstantBuffer<MeshData> Instance : register(b1);

const static float3 _kBoxMin = float3(-1.0f, -1.0f, -1.0f);
const static float3 _kBoxMax = float3(+1.0f, +1.0f, +1.0f);
const static float4 _kBoxVerts[8] =
{
	// front rect
    float4(_kBoxMin, 1.0f),
	float4(_kBoxMin.x, _kBoxMax.y, _kBoxMin.z, 1.0f),
	float4(_kBoxMax.x, _kBoxMax.y, _kBoxMin.z, 1.0f),
	float4(_kBoxMax.x, _kBoxMin.y, _kBoxMin.z, 1.0f),

	// back rect
	float4(_kBoxMin.x, _kBoxMin.y, _kBoxMax.z, 1.0f),
	float4(_kBoxMin.x, _kBoxMax.y, _kBoxMax.z, 1.0f),
	float4(_kBoxMax, 1.0f),
	float4(_kBoxMax.x, _kBoxMin.y, _kBoxMax.z, 1.0f)
};

const static uint _kusBoxIndeces[24] =
{
    0, 1, 1, 2, 2, 3, 3, 0,
	4, 5, 5, 6, 6, 7, 7, 4,
	0, 4, 1, 5, 3, 7, 2, 6
};

[maxvertexcount(24)]
void main(point GeometryInput input[1], inout LineStream<PixelInput> lineStream)
{
	// temp box values 
    float4 _kBoxVertsW[8];
    
	// converting all 8 box vertexes to Homo coordinate
	[unroll]
    for (uint i = 0; i < 8; ++i)
    {
        _kBoxVertsW[i] = mul(_kBoxVerts[i], Instance.Bounds);
        _kBoxVertsW[i] = mul(float4(_kBoxVertsW[i]), View.ViewProj);
    }
	
	// for each pair of line, adding to stream
	[unroll]
    for (uint i = 0; i < 24; i += 2)
    {
        lineStream.Append((PixelInput) _kBoxVertsW[_kusBoxIndeces[i + 0]]);
        lineStream.Append((PixelInput) _kBoxVertsW[_kusBoxIndeces[i + 1]]);
		
        lineStream.RestartStrip();
    }
}
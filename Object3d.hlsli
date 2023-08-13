struct VertexShaderOutput
{
    float32_t4 position : SV_POSITION;
    float32_t2 texcoord : TEXCOORD0;
};
struct transformationMatrix
{
    float32_t4x4 WVP;
};
struct VertexShaderInput
{
    float32_t4 position : POSITION0;
    float32_t2 texcoord : TEXCOORD0;
};


struct Material
{
    float32_t4 color;
};
struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};
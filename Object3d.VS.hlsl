float4 main( float4 pos : POSITION ) : SV_POSITION
{
	return pos;
}

struct VertexShaderOutput {
	float32_t4 position : SV_POSITION;
};

struct VertexShaderInput {
	float32_t4 position : POSITION;
};

VertexShaderOutput main(VertexShaderInput input) {
	VertexShaderOutput output;
	output.position = input.position;
	return output;
}

struct PixelShaderOutput {
	float32_t4 color : SVTARGET0;
};

PixelShaderOutput main() {
	PixelShaderOutput output;
	output.color = float32_t4(1.0f, 1.0f, 1.0f, 1.0f);
	return output;
}

struct VertexShaderInput {
	float32_t4 position : POSITION0;
};
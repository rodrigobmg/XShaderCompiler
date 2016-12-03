
// Register Test 1
// 02/12/2016

cbuffer Matrices : register(b1)//register(vs, b0) : register(ps, b1)
{
	float4 diffuse;
};

//Buffer<float4> buffer0 : register(t0);

Texture2D tex0 : register(vs, t[1])
               : register(ps, t1[2]);

SamplerState smpl0 : register(ps_5_0, s0)
                   : register(z[12]); // this register is ignored

float4 PS(float2 texCoord : TEXCOORD) : SV_Target
{
	tex0 = tex0;
	//return tex0.Sample(smpl0, texCoord);
	return diffuse;
}


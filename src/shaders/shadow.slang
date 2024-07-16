#version 460

#include "shader-common.slang"

[[vk::binding(0)]]cbuffer ubo{ UBO_SHADOW ubo; }

struct VSOutput
{
    float4 pos : SV_POSITION;
    [[vk::location(0)]] int InstanceIndex : TEXCOORD0;
}

struct GSOutput
{
    float4 pos : SV_POSITION;
    int Layer : SV_RenderTargetArrayIndex;
}

[shader("vertex")]
VSOutput mainVS([[vk::location(0)]] float4 Pos : POSITION0, uint InstanceIndex : SV_InstanceID)
{
    VSOutput output;
    output.InstanceIndex = InstanceIndex;
    output.pos = Pos;
    return output;
}

[shader("geometry")]
[maxvertexcount(3)]
[instance(3)]
void main(triangle VSOutput input[3], uint InvocationID: SV_GSInstanceID, inout TriangleStream<GSOutput> outStream)
{
    float4 instancedPos = ubo.instancePos[input[0].InstanceIndex];
    for (int i = 0; i < 3; i++)
    {
        float4 tmpPos = input[i].pos + instancedPos;
        GSOutput output = (GSOutput)0;
        output.pos = mul(ubo.mvp[InvocationID], tmpPos);
        output.Layer = InvocationID;
        outStream.Append(output);
    }
    outStream.RestartStrip();
}

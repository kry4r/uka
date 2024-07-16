#version 460
#include "shader-common.slang"
struct VSInput
{
    [[vk::location(0)]] float4 Pos : POSITION0;
    [[vk::location(1)]] float2 UV : TEXCOORD0;
    [[vk::location(2)]] float3 Color : COLOR0;
    [[vk::location(3)]] float3 Normal : NORMAL0;
    [[vk::location(4)]] float3 Tangent : TEXCOORD1;
}


struct VSOutput
{
    float4 pos : SV_POSITION;
    [[vk::location(0)]] float2 UV : TEXCOORD0;
    [[vk::location(1)]] float3 Color : COLOR0;
    [[vk::location(2)]] float3 Normal : NORMAL0;
    [[vk::location(3)]] float3 Tangent : TEXCOORD1;
    [[vk::location(3)]] float3 WorldPos : POSITION0;
}
[[vk::binding(0)]] cbuffer ubo { UBO_MRT ubo; }
[[vk::binding(1)]]  Sampler2D samplerColor;
[[vk::binding(2)]]  Sampler2D samplerNormalMap;


struct FSOutput
{
    float4 position : SV_Target0;
    float4 normal : SV_Target1;
    float4 albedo : SV_Target2;
}

[shader("vertex")]
VSOutput mainVS(VSInput input, uint InstanceIndex: SV_InstanceID)
{
    VSOutput output;
    float4 tmpPos = input.Pos + ubo.instancePos[InstanceIndex];

    output.pos = mul(ubo.projection, mul(ubo.view, mul(ubo.model, tmpPos)));

    output.UV = input.UV;

    // Vertex position in world space
    output.WorldPos = mul(ubo.model, tmpPos).xyz;

    // Normal in world space
    output.Normal = normalize(input.Normal);
    output.Tangent = normalize(input.Tangent);

    // Currently just vertex color
    output.Color = input.Color;
    return output;
}

[shader("fragment")]
FSOutput mainFS(VSOutput input)
{
    FSOutput output;
    output.position = float4(input.WorldPos, 1.0f);
    var N = normalize(input.Normal);
    var T = normalize(input.Tangent);
    var B = cross(N, T);
    var TBN = float3x3(T, B, N);
    var tnorm = mul(normalize(texture(samplerNormalMap,input.UV).xyz*2.0 - vec3(1.0)), TBN);
    output.normal = float4(tnorm, 1.0f);
    output.albedo = texture(samplerColor, input.UV);
    return output;
}

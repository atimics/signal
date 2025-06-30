#include <metal_stdlib>
using namespace metal;

struct vs_uniforms {
    float4x4 mvp;
};

struct vs_in {
    float3 position [[attribute(0)]];
    float3 normal [[attribute(1)]];
    float2 texcoord [[attribute(2)]];
};

struct vs_out {
    float4 position [[position]];
    float3 normal;
    float2 texcoord;
};

vertex vs_out vs_main(vs_in in [[stage_in]], constant vs_uniforms& uniforms [[buffer(0)]]) {
    vs_out out;
    out.position = uniforms.mvp * float4(in.position, 1.0);
    out.normal = in.normal;
    out.texcoord = in.texcoord;
    return out;
}

#include <metal_stdlib>
using namespace metal;

struct vs_uniforms {
    float4x4 mvp;
    float4x4 model;
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
    float3 world_pos;
    float altitude;
};

vertex vs_out vs_main(vs_in in [[stage_in]], constant vs_uniforms& uniforms [[buffer(0)]]) {
    vs_out out;
    
    // Calculate world position
    float4 world_pos = uniforms.model * float4(in.position, 1.0);
    out.world_pos = world_pos.xyz;
    out.altitude = world_pos.y; // Y is up in our coordinate system
    
    // Standard MVP transformation
    out.position = uniforms.mvp * float4(in.position, 1.0);
    out.normal = in.normal; // Should be transformed by normal matrix in production
    out.texcoord = in.texcoord;
    
    return out;
}

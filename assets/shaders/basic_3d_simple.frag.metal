#include <metal_stdlib>
using namespace metal;

struct fs_in {
    float3 normal;
    float2 texcoord;
};

fragment float4 fs_main(fs_in in [[stage_in]], 
                       texture2d<float> diffuse_texture [[texture(0)]],
                       sampler diffuse_sampler [[sampler(0)]]) {
    // Simple solid red - no texture sampling, no lighting
    return float4(1.0, 0.0, 0.0, 1.0); // Bright red
}

#include <metal_stdlib>
using namespace metal;

struct fs_in {
    float3 normal;
    float2 texcoord;
};

fragment float4 fs_main(fs_in in [[stage_in]], 
                       texture2d<float> diffuse_texture [[texture(0)]],
                       sampler diffuse_sampler [[sampler(0)]]) {
    // For debugging: output bright red color to make triangle clearly visible
    return float4(1.0, 0.0, 0.0, 1.0); // Bright red
    
    // Original code (commented out for debugging):
    // float3 normal = normalize(in.normal);
    // float light = max(0.0, dot(normal, float3(0.0, -1.0, -0.5)));
    // float4 color = diffuse_texture.sample(diffuse_sampler, in.texcoord);
    // return float4(color.rgb * (0.3 + 0.7 * light), color.a);
}

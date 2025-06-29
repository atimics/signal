#include <metal_stdlib>
using namespace metal;

struct fs_in {
    float3 normal;
    float2 texcoord;
};

fragment float4 fs_main(fs_in in [[stage_in]], 
                       texture2d<float> diffuse_texture [[texture(0)]],
                       sampler diffuse_sampler [[sampler(0)]]) {
    float3 normal = normalize(in.normal);
    float light = max(0.0, dot(normal, float3(0.0, -1.0, -0.5)));
    float4 color = diffuse_texture.sample(diffuse_sampler, in.texcoord);
    return float4(color.rgb * (0.3 + 0.7 * light), color.a);
}

#include <metal_stdlib>
using namespace metal;

struct fs_uniforms {
    float3 light_dir;
    float glow_intensity;
    float time;
    float3 _pad;
};

struct fs_in {
    float3 normal;
    float2 texcoord;
};

fragment float4 fs_main(fs_in in [[stage_in]], 
                       constant fs_uniforms& uniforms [[buffer(0)]],
                       texture2d<float> diffuse_texture [[texture(0)]],
                       sampler diffuse_sampler [[sampler(0)]]) {
    float3 normal = normalize(in.normal);
    float light = max(0.0, dot(normal, -uniforms.light_dir));
    float4 color = diffuse_texture.sample(diffuse_sampler, in.texcoord);
    
    // Apply base lighting
    float3 final_color = color.rgb * (0.3 + 0.7 * light);
    
    // Add glow effect if intensity > 0
    if (uniforms.glow_intensity > 0.0) {
        // Create a pulsing glow effect based on time
        float pulse = 0.7 + 0.3 * sin(uniforms.time * 3.0); // Smooth pulsing
        float glow_strength = uniforms.glow_intensity * pulse;
        
        // Warm golden glow color
        float3 glow_color = float3(1.2, 1.0, 0.6);
        
        // Mix and additive glow
        final_color = mix(final_color, glow_color, glow_strength * 0.3);
        final_color += glow_color * glow_strength * 0.4; // Additive glow for brightness
    }
    
    return float4(final_color, color.a);
}

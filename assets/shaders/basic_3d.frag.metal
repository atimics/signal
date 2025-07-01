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
    
    // Add glow effect only if intensity > 0 (logo cube only)
    if (uniforms.glow_intensity > 0.0) {
        // Create a subtle pulsing glow effect
        float pulse_speed = 2.0;
        float pulse_amplitude = 0.3;
        float pulse_base = 0.7;
        
        float pulse = pulse_base + pulse_amplitude * (0.5 + 0.5 * sin(uniforms.time * pulse_speed));
        float glow_strength = uniforms.glow_intensity * pulse;
        
        // Calculate luminance of the texture to identify bright areas
        float luminance = dot(color.rgb, float3(0.299, 0.587, 0.114));
        
        // Create emissive mask for bright areas
        float emissive_mask = smoothstep(0.7, 0.95, luminance);
        
        // Subtle warm glow color (values <= 1.0 to prevent over-brightening)
        float3 glow_color = float3(1.0, 0.9, 0.7);
        
        // Apply glow effect - blend instead of add to prevent over-brightening
        float glow_factor = emissive_mask * glow_strength * 0.5;
        final_color = mix(final_color, final_color * glow_color, glow_factor);
    }
    
    return float4(final_color, color.a);
}

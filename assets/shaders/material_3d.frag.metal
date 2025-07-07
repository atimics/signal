#include <metal_stdlib>
using namespace metal;

struct fs_uniforms {
    // Legacy lighting
    float3 light_dir;
    float time;
    
    // Material properties
    float metallic;
    float roughness;
    float3 albedo;
    float3 emissive;
    float emissive_strength;
    float glow_intensity;
    
    // Lighting system
    float3 ambient_color;
    float ambient_intensity;
    float ground_effect_multiplier;
    
    // Padding for alignment
    float _pad1;
};

struct fs_in {
    float3 normal;
    float2 texcoord;
    float3 world_pos;
    float altitude;
};

fragment float4 fs_main(fs_in in [[stage_in]], 
                       constant fs_uniforms& uniforms [[buffer(0)]],
                       texture2d<float> diffuse_texture [[texture(0)]],
                       sampler diffuse_sampler [[sampler(0)]]) {
    
    float3 normal = normalize(in.normal);
    float4 texture_color = diffuse_texture.sample(diffuse_sampler, in.texcoord);
    
    // Calculate base material color
    float3 base_color = texture_color.rgb * uniforms.albedo;
    
    // Ambient lighting
    float3 ambient = uniforms.ambient_color * uniforms.ambient_intensity;
    float3 final_color = base_color * ambient;
    
    // Directional lighting (simplified)
    float ndotl = max(0.0, dot(normal, -uniforms.light_dir));
    float3 direct_light = float3(1.0, 0.95, 0.8) * 0.8 * ndotl;
    final_color += base_color * direct_light;
    
    // Emissive materials
    if (uniforms.emissive_strength > 0.0) {
        // Calculate luminance for selective emission
        float luminance = dot(texture_color.rgb, float3(0.299, 0.587, 0.114));
        float emissive_mask = smoothstep(0.6, 0.9, luminance);
        
        // Apply emissive glow
        float3 emissive_contribution = uniforms.emissive * uniforms.emissive_strength * emissive_mask;
        final_color += emissive_contribution;
    }
    
    // Special glow effect (for logo cube)
    if (uniforms.glow_intensity > 0.0) {
        float pulse_speed = 3.0;
        float pulse = 0.3 + 0.7 * (0.5 + 0.5 * sin(uniforms.time * pulse_speed));
        
        float luminance = dot(texture_color.rgb, float3(0.299, 0.587, 0.114));
        float glow_mask = smoothstep(0.7, 0.95, luminance);
        
        float3 glow_color = float3(1.5, 1.2, 0.8);
        final_color += glow_color * uniforms.glow_intensity * pulse * glow_mask;
    }
    
    // Ground effect lighting boost
    if (uniforms.ground_effect_multiplier > 1.0 && in.altitude < 50.0) {
        float ground_boost = mix(1.0, uniforms.ground_effect_multiplier, 
                               (50.0 - in.altitude) / 50.0);
        final_color *= ground_boost;
    }
    
    // Metallic/roughness effects (simplified)
    if (uniforms.metallic > 0.5) {
        // Make metallic surfaces more reflective
        final_color = mix(final_color, final_color * 1.5, uniforms.metallic * 0.3);
    }
    
    return float4(final_color, texture_color.a);
}

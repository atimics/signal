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
        // Create a more pronounced pulsing glow effect
        float pulse_speed = 3.0; // Faster pulse frequency for more noticeable effect
        float pulse_amplitude = 0.8; // Higher amplitude for stronger pulsing (0.2 to 1.0)
        float pulse_base = 0.2; // Lower minimum for more dramatic pulse
        
        float pulse = pulse_base + pulse_amplitude * (0.5 + 0.5 * sin(uniforms.time * pulse_speed));
        float glow_strength = uniforms.glow_intensity * pulse;
        
        // Calculate luminance of the texture to identify bright/white areas
        float luminance = dot(color.rgb, float3(0.299, 0.587, 0.114));
        
        // Create emissive mask for bright areas (white parts of the logo)
        float emissive_mask = smoothstep(0.7, 0.95, luminance); // White areas get strong emission
        float general_glow_mask = smoothstep(0.3, 0.8, luminance); // All bright areas get some glow
        
        // Bright warm emissive color for white parts
        float3 emissive_color = float3(2.0, 1.8, 1.2); // Very bright warm white
        float3 general_glow_color = float3(1.2, 1.0, 0.6); // Softer golden glow
        
        // Apply emissive effect to white areas (self-illuminated appearance)
        float emissive_strength = emissive_mask * glow_strength * 1.5; // Extra strong for white parts
        final_color += emissive_color * emissive_strength;
        
        // Apply general glow to all bright areas
        float general_glow = general_glow_mask * glow_strength * 0.8;
        final_color = mix(final_color, final_color * general_glow_color, general_glow * 0.3);
        final_color += general_glow_color * general_glow * 0.4;
        
        // Add a subtle rim lighting effect
        float rim = 1.0 - abs(dot(normal, float3(0.0, 0.0, 1.0))); // Fake view direction
        rim = smoothstep(0.5, 1.0, rim);
        final_color += general_glow_color * rim * glow_strength * 0.3;
    }
    
    return float4(final_color, color.a);
}

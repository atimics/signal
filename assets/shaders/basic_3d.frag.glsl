#version 330 core

uniform sampler2D diffuse_texture;
uniform vec3 light_dir;

in vec3 frag_normal;
in vec2 frag_texcoord;

out vec4 frag_color;

void main() {
    vec3 normal = normalize(frag_normal);
    float light = max(0.0, dot(normal, -light_dir));
    vec4 color = texture(diffuse_texture, frag_texcoord);
    frag_color = vec4(color.rgb * (0.3 + 0.7 * light), color.a);
}

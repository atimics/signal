/**
 * @file shader_sources.h
 * @brief Embedded shader sources for WASM builds
 * 
 * When building for WASM, we cannot load files from the filesystem,
 * so we embed the shader sources directly in the executable.
 */

#ifndef SHADER_SOURCES_H
#define SHADER_SOURCES_H

#ifdef WASM_BUILD

// Basic 3D vertex shader source
static const char* basic_3d_vert_glsl = 
    "#version 300 es\n"
    "precision highp float;\n"
    "\n"
    "uniform mat4 mvp;\n"
    "\n"
    "layout(location = 0) in vec3 position;\n"
    "layout(location = 1) in vec3 normal;\n"
    "layout(location = 2) in vec2 texcoord;\n"
    "\n"
    "out vec3 frag_normal;\n"
    "out vec2 frag_texcoord;\n"
    "\n"
    "void main() {\n"
    "    gl_Position = mvp * vec4(position, 1.0);\n"
    "    frag_normal = normal;\n"
    "    frag_texcoord = texcoord;\n"
    "}\n";

// Basic 3D fragment shader source  
static const char* basic_3d_frag_glsl = 
    "#version 300 es\n"
    "precision highp float;\n"
    "\n"
    "uniform sampler2D diffuse_texture;\n"
    "uniform vec3 light_dir;\n"
    "uniform float glow_intensity;\n"
    "uniform float time;\n"
    "\n"
    "in vec3 frag_normal;\n"
    "in vec2 frag_texcoord;\n"
    "\n"
    "out vec4 frag_color;\n"
    "\n"
    "void main() {\n"
    "    vec3 normal = normalize(frag_normal);\n"
    "    float light = max(0.0, dot(normal, -light_dir));\n"
    "    vec4 color = texture(diffuse_texture, frag_texcoord);\n"
    "    \n"
    "    // Apply glow effect if enabled\n"
    "    if (glow_intensity > 0.0) {\n"
    "        float pulse = sin(time * 3.0) * 0.5 + 0.5;\n"
    "        vec3 glow = color.rgb * glow_intensity * pulse;\n"
    "        color.rgb += glow;\n"
    "    }\n"
    "    \n"
    "    frag_color = vec4(color.rgb * (0.3 + 0.7 * light), color.a);\n"
    "}\n";

#endif // WASM_BUILD

#endif // SHADER_SOURCES_H
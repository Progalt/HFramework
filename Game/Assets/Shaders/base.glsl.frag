#version 450

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec4 vColour; 
layout(location = 1) in vec2 vTexCoord;

layout(set = 0, binding = 1) uniform sampler2D uAlbedo;

void main() 
{
    outColor = texture(uAlbedo, vTexCoord) * vColour;
}
#version 450

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec4 aColour;
layout(location = 2) in vec2 aTexCoord;


layout(location = 0) out vec4 vColour; 
layout(location = 1) out vec2 vTexCoord; 

layout(set = 0, binding = 0) uniform MainUBO
{
    mat4 vp;
} ubo;

void main()
{
    gl_Position = ubo.vp * vec4(aPosition, 1.0);
    vColour = aColour; 
    vTexCoord = aTexCoord;

    gl_Position.y = -gl_Position.y;
}
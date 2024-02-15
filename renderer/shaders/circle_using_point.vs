#version 330 core

layout (location = 0) in vec2 a_pos;
layout (location = 1) in float a_radius;
layout (location = 2) in vec3 a_color;

uniform mat4 projection;

out vec3 frag_color;
out vec2 center;
out float radius;

void main() {
    gl_Position = projection * vec4(a_pos, 0.0, 1.0);
    gl_PointSize = 2.0 * a_radius;
    
    frag_color = a_color; 
    center = a_pos;
    radius = a_radius;
}


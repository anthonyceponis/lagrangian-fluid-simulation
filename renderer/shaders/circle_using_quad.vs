#version 330 core

layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec2 a_offset;
layout (location = 2) in float a_radius;
layout (location = 3) in vec3 a_color;

uniform mat4 projection;

out vec3 frag_color;
out vec2 center;
out float radius;

void main() {
    gl_Position = projection * vec4(a_radius * a_pos + a_offset, 0.0, 1.0);

    frag_color = a_color;
    center = a_offset;
    radius = a_radius;
}

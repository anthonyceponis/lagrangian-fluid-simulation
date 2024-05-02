#version 430 core 

// RGB values in range 0-255 inclusive
in vec3 frag_color;
in vec2 center;
in float radius;

out vec4 out_color;

void main() {
    vec4 color = vec4(frag_color, 1.0);
    if (distance(gl_FragCoord.xy, center) > radius) {
        color = vec4(0.9, 0.9, 0.9, 1.0);
    }
    out_color = color;
    // out_color = vec4(1.0, 1.0, 1.0, 1.0);
}


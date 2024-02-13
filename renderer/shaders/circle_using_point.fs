#version 330 core 

// RGB values in range 0-255 inclusive
in vec3 frag_color;
in vec2 center;
in float radius;

out vec4 out_color;

void main() {
    vec3 color = frag_color;
    if (distance(gl_FragCoord.xy, center) > radius) {
        color = vec3(0, 0, 0);
    }
    out_color = vec4(color / 255.0, 1.0);
}


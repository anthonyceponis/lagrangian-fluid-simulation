#version 430 core

// OPTIMISE: Adjust work group sizes for warps (e.g. multiples of 64)
layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

layout(std430, binding = 0) buffer SSBO1 {
    float positions[];
} ssbo1;

layout(std430, binding = 1) readonly buffer SSBO2 {
    int count_arr[];
} ssbo2;

layout(std430, binding = 2) readonly buffer SSBO3 {
    int particles_grouped[];
} ssbo3;

layout(std430, binding = 3) readonly buffer SSBO4 {
   float cell_width;
   int cell_count_x;
   int cell_count_y;
   int particle_count;
} ssbo4;

void collideParticles(int p1_i, int p2_i);

void main() {
    
    int p_i = int(gl_GlobalInvocationID.x);

    // WARNING: Assuming that query wont go over 100.
    const int max_query_size = 100; 
    int query_ids[max_query_size];
    int query_size = 0;

    int x = int(ssbo1.positions[2 * p_i] / ssbo4.cell_width); 
    int y = int(ssbo1.positions[2 * p_i + 1] / ssbo4.cell_width);

    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int curr_x = x + dx;
            int curr_y = y + dy;
            int h = curr_y * ssbo4.cell_count_x + curr_x;

            // Check can be removed by pre computing the ranges using a radius
            if (curr_x < 0 || curr_y < 0 || curr_x >= ssbo4.cell_count_x || curr_y >= ssbo4.cell_count_y) {
                continue;
            }

            int start = ssbo2.count_arr[h];
            int end = ssbo2.count_arr[h+1];

            for (int i = start; i < end; i++) {
                query_ids[query_size] = ssbo3.particles_grouped[i];
                query_size++;
            }
        }
    }

    for (int i = 0; i < query_size; i++) {
        for (int j = i + 1; j < query_size; j++) {
            collideParticles(query_ids[i], query_ids[j]); 
        }
    }

}

void collideParticles(int p1_i, int p2_i) {
    // Potential to remove this check.
    // if (p1_i == p2_i) return; 

    // ASSUMPTION: Both particle radii are equal.
    float p1_radius = 3.0;
    float p2_radius = 3.0;

    vec2 p1_pos = vec2(ssbo1.positions[2 * p1_i], ssbo1.positions[2 * p1_i + 1]);
    vec2 p2_pos = vec2(ssbo1.positions[2 * p2_i], ssbo1.positions[2 * p2_i + 1]);

    float e = 0.5;
    float distance_between_centers = length(p1_pos - p2_pos);
    float sum_of_radii = p1_radius + p2_radius;

    if (distance_between_centers < sum_of_radii) {
        vec2 collision_axis = p1_pos - p2_pos;
        vec2 n = collision_axis / distance_between_centers;
        float mass_sum = p1_radius + p2_radius;
        float mass_ratio_1 = p1_radius / mass_sum;
        float mass_ratio_2 = p2_radius / mass_sum;
        float delta = e * (sum_of_radii - distance_between_centers);

        p1_pos += mass_ratio_2 * delta * n;
        p2_pos -= mass_ratio_1 * delta * n;
        
        ssbo1.positions[2 * p1_i] = p1_pos.x;
        ssbo1.positions[2 * p1_i + 1] = p1_pos.y;

        ssbo1.positions[2 * p2_i] = p2_pos.x;
        ssbo1.positions[2 * p2_i + 1] = p2_pos.y;

    }

}


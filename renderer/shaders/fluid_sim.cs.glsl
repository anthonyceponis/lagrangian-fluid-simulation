#version 430 core 

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

layout(std430, binding = 0) buffer ssbo1 {
    vec2 positions[];
};

layout(std430, binding = 1) buffer ssbo2 {
    vec2 velocities[];
};

layout(std430, binding = 2) buffer ssbo3 {
    vec2 forces[];
};

layout(std430, binding = 3) buffer ssbo4 {
    vec2 densities[];
};

layout(std430, binding = 4) buffer ssbo5 {
    int spatial_lookup[];
};

layout(std430, binding = 5) buffer ssbo6 {
    int spatial_indicies[];
};

// Determines which kernel function is actually executed.
uniform uint kernel_id;

uniform float dt;
uniform uint particle_count; 
uniform uint bucket_count;
const uint max_neighbour_query_size = 1024;
const float pi = 3.14159265359;

uniform float h; // smoothing_radius
float cell_width = h * 2;
float h2 = h * h;

uniform float particle_mass;
uniform float target_density;
uniform float pressure_multiplier;
uniform float near_pressure_multiplier;
uniform float viscosity_strength;

void calcDensity(int p_i);
void applyFluidForces(int p_i);

void main() {
    int p_i = int(gl_GlobalInvocationID.x); 
    // Since each work group has 64 local workers, must do range check because particle count is probably not a multiple of 64.
    if (p_i >= particle_count) 
        return;

    if (kernel_id == 0) {
        calcDensity(p_i);
    }
    else if (kernel_id == 1) {
        applyFluidForces(p_i);
    }
}

float poly6Kernel(float r) {
    return 4.0 / (pi * pow(h, 8)) * pow(h*h - r*r, 3);
}

float spikyGradKernel(float r) {
    return -10.0 / (pow(h, 5) * pi) * pow(h-r, 3); 
}

float laplacianKernel(float r) {
    return 40.0 / (pow(h, 5) * pi) * (h-r);
}

ivec2 posToCellCoord(vec2 pos) {
    return ivec2(pos / cell_width);
}

int cellCoordToHash(ivec2 cell_coord) {
    int prime1 = 15823;
    int prime2 = 9737333;

    int hash = abs((cell_coord.x * prime1) ^ (cell_coord.y * prime2));
    hash %= int(bucket_count);

    return hash;
}

void calcDensity(int p_i) {
    vec2 pos = positions[p_i];
    ivec2 cell_coord = posToCellCoord(pos);

    int query[max_neighbour_query_size];
    uint query_size = 0;

    // Get neighbours
    for (int y = cell_coord.y - 1; y <= cell_coord.y + 1; y++) {
        for (int x = cell_coord.x - 1; x <= cell_coord.x + 1; x++) {
            ivec2 curr_cell_coord = ivec2(x, y);
            int curr_hash = cellCoordToHash(curr_cell_coord);

            int start = spatial_lookup[curr_hash];
            int end = spatial_lookup[curr_hash + 1];

            for (int i = start; i < end; i++) {
                query[query_size] = spatial_indicies[i];
                query_size++;
            }
        }
    }

    float density =  0.0;
    float density_near = 0.0;

    for (int i = 0; i < query_size; i++) {
        const float r = distance(pos, positions[query[i]]);
        if (r < h) {
            density += particle_mass * poly6Kernel(r);
            // density_near += a * a * a * kern_near;
        }
    }

    // densities[p_i][0] = density;
    densities[p_i][0] = density;
    densities[p_i][1] = density_near;
}

vec2 densityToPressure(float density, float near_density) {
    float pressure = (density - target_density) * pressure_multiplier;
    float near_pressure = near_density * near_pressure_multiplier;
    return vec2(pressure, near_pressure);
}

void applyFluidForces(int p_i) {
    vec2 pos = positions[p_i];
    ivec2 cell_coord = posToCellCoord(pos);

    int query[max_neighbour_query_size]; 
    uint query_size = 0;

    // Get neighbours
    for (int y = cell_coord.y - 1; y <= cell_coord.y + 1; y++) {
        for (int x = cell_coord.x - 1; x <= cell_coord.x + 1; x++) {
            ivec2 curr_cell_coord = ivec2(x, y);
            int curr_hash = cellCoordToHash(curr_cell_coord);

            int start = spatial_lookup[curr_hash];
            int end = spatial_lookup[curr_hash + 1];

            for (int i = start; i < end; i++) {
                query[query_size] = spatial_indicies[i];
                query_size++;
            }
        }
    }


    vec2 pressure_force = vec2(0.0 ,0.0);
    vec2 visc_force = vec2(0.0, 0.0);

    float curr_density = densities[p_i][0];
    float curr_near_density = densities[p_i][1];
    vec2 curr_dual_pressure = densityToPressure(curr_density, curr_near_density);
    float curr_pressure = curr_dual_pressure[0];
    float curr_near_pressure = curr_dual_pressure[1];

    for (int i = 0; i < query_size; i++) {
        // Skip self
        if (query[i] == p_i)
            continue;

        const float r = distance(pos, positions[query[i]]);
        if (r < h) {
            float neighbour_density = densities[query[i]][0];    
            float neighbour_near_density = densities[query[i]][1];    

            vec2 neighbour_dual_pressure = densityToPressure(neighbour_density, neighbour_near_density);
            float neighbour_pressure = neighbour_dual_pressure[0];
            float neighbour_near_pressure = neighbour_dual_pressure[1];

            float shared_pressure = 0.5 * (curr_pressure + neighbour_pressure);
            float shared_near_pressure = 0.5 * (curr_near_pressure + neighbour_near_pressure);

            // vec2 rij = r == 0 ? vec2(0.0, 1.0) : normalize(positions[query[i]] - pos);
            vec2 rij = normalize(positions[query[i]] - pos);

            pressure_force += -rij * particle_mass * spikyGradKernel(r) * shared_pressure / neighbour_density;
            visc_force += particle_mass * laplacianKernel(r) * (velocities[query[i]] - velocities[p_i]) / neighbour_density;

        }
    }

    visc_force *= viscosity_strength;

    vec2 grav_force = vec2(0.0, -9.81) * particle_mass / curr_density;
    // vec2 grav_force = vec2(0.0, 0.0) * curr_density;
    forces[p_i] = pressure_force + visc_force + grav_force;
    // forces[p_i] = grav_force;
}

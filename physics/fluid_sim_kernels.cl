__constant float pi = 3.14159265f;

__constant float h = 50.0f; // Smoothing radius
__constant float h2 = h * h;
__constant float h3 = h2 * h;
__constant float h5 = h3 * h2;

__constant unsigned int particle_count = 1000;
__constant unsigned int spatial_bucket_count = particle_count+1;

__constant float target_density = 10.0f;
__constant float pressure_multiplier = 10.0f;

int smoothingWeight(float r) {
    float x = 1.0f - r * r / h2;
    return 315.0f / (64.0f * pi * h3) * x * x * x;
}

int smoothingWeightDerivative(float r) {
    float x = 1.0f - r * r / h2;
    return -945.0f / (32.0f * pi * h3 * h5) * r * x * x;
}

float2 gradient(float r, float2 dir_to_center) {
    return -smoothingWeightDerivative(r) * dir_to_center;
}

int2 positionToCellCoord(float2 pos) {
    const float cell_width = 2 * h; 
    int2 cell_coord; 

    cell_coord.x = convert_int(pos.x / cell_width); 
    cell_coord.y = convert_int(pos.x / cell_width);

    return cell_coord;
}

int cellCoordToHash(int2 cell_coord) {
    int prime1 = 15823;
    int prime2 = 9737333;

    int hash = (cell_coord.x * prime1) ^ (cell_coord.y * prime2);
    
    // Subtract 1 for the overflow cell.
    return hash % (spatial_bucket_count - 1);
}

__kernel void calcDensity(__global float2 *positions, __global float *densities, __global int *spatial_lookup, __global int *spatial_indicies) {  
    int query[particle_count]; 
    unsigned int query_size = 0;

    unsigned int p_i = get_global_id(0);
    int2 cell_coord = positionToCellCoord(positions[p_i]); 
    int hash = cellCoordToHash(cell_coord);

    // Get neighbours
    for (int y = cell_coord.y - 1; y <= cell_coord.y + 1; y++) {
        for (int x = cell_coord.x - 1; x <= cell_coord.x + 1; x++) {
            int2 curr_cell_coord = (int2)(x, y);
            int curr_hash = cellCoordToHash(curr_cell_coord);

            int start = spatial_lookup[curr_hash];
            int end = spatial_lookup[curr_hash+1];

            for (int i = start; i <= end; i++) {
                 query[query_size] = spatial_indicies[i];
                 query_size++;
            }
        }
    }

    float density = 0.0f;

    // Note that mass = 1
    for (int i = 0; i < query_size; i++) {
        const float dist = distance(positions[p_i], positions[query[i]]);
        if (dist < h) {
            density += densities[query[i]] * smoothingWeight(dist); 
        }
    }

    densities[p_i] = density;
}

float densityToPressure(float density) {
    return (density - target_density) * pressure_multiplier; 
}

__kernel void applyPressureForce(__global float2 *positions, __global float2 *velocities, __global float *densities, __global int *spatial_lookup, __global int *spatial_indicies, float dt) {
    int query[particle_count]; 
    unsigned int query_size = 0;

    unsigned int p_i = get_global_id(0);
    int2 cell_coord = positionToCellCoord(positions[p_i]); 
    int hash = cellCoordToHash(cell_coord);

    // Get neighbours
    for (int y = cell_coord.y - 1; y <= cell_coord.y + 1; y++) {
        for (int x = cell_coord.x - 1; x <= cell_coord.x + 1; x++) {
            int2 curr_cell_coord = (int2)(x, y);
            int curr_hash = cellCoordToHash(curr_cell_coord);

            int start = spatial_lookup[curr_hash];
            int end = spatial_lookup[curr_hash+1];

            for (int i = start; i <= end; i++) {
                 query[query_size] = spatial_indicies[i];
                 query_size++;
            }
        }
    }

    float2 pressure_force = (float2)(0.0f, 0.0f);

    // Note that mass = 1
    for (int i = 0; i < query_size; i++) {
        const float dist = distance(positions[p_i], positions[query[i]]);
        if (dist < h) {
            float shared_pressure = (densityToPressure(densities[query[i]]) + densityToPressure(densities[p_i])) / 2.0f; 
            pressure_force += shared_pressure * smoothingWeightDerivative(dist) / densities[query[i]]; 
        }
    }

    // F = ma but here we are finding the force of small volumes of water so we use density (which is the mass per volume) as mass here instead.
    velocities[p_i] += pressure_force / densities[p_i] * dt;
}


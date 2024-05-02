__constant float pi = 3.14159265f;

__constant float h = 50.0f; // Smoothing radius
__constant float h2 = h * h;
__constant float h3 = h2 * h;
__constant float h5 = h3 * h2;

__constant float cell_width = 2 * h;
__constant int prime1 = 15823;
__constant int prime2 = 9737333;

__constant unsigned int particle_count = 1024;
__constant unsigned int spatial_bucket_count = particle_count+1;

__constant float target_density = 10.0f;
__constant float pressure_multiplier = 10.0f;

#define HASH(cell_coord) ((cell_coord.x * prime1) ^ (cell_coord.y * prime2));

__kernel void calcDensity(__global float2 *positions, __global float *densities, __global int *spatial_lookup, __global int *spatial_indicies) {  
    int query[particle_count]; 
    unsigned int query_size = 0;

    unsigned int p_i = get_global_id(0);

    int2 cell_coord; 

    cell_coord.x = convert_int(positions[p_i].x / cell_width); 
    cell_coord.y = convert_int(positions[p_i].x / cell_width);

    int hash = HASH(cell_coord);
    // Subtract 1 for the overflow cell.
    hash %= (spatial_bucket_count - 1);    
    
    printf("%d\n", hash);
}

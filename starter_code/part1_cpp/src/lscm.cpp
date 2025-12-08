/**
 * @file lscm.cpp
 * @brief LSCM (Least Squares Conformal Maps) parameterization
 *
 * SKELETON - YOU IMPLEMENT THIS
 *
 * This is the MOST COMPLEX part of the assignment.
 *
 * IMPORTANT:
 * - See reference/lscm_matrix_example.cpp for matrix assembly example
 * - See reference/algorithms.md for mathematical background
 * - Use Eigen library for sparse linear algebra
 *
 * Algorithm:
 * 1. Build local vertex mapping (global → local indices)
 * 2. Assemble LSCM sparse matrix
 * 3. Set boundary conditions (pin 2 vertices)
 * 4. Solve sparse linear system
 * 5. Normalize UVs to [0,1]²
 */

#include "lscm.h"
#include "math_utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <map>
#include <vector>
#include <set>

// Eigen library for sparse matrices
#include <Eigen/Sparse>
#include <Eigen/SparseLU>
// Alternative: #include <Eigen/IterativeLinearSolvers>

struct Vec3{
    double x, y, z;
    Vec3(double x= 0, double y= 0, double z= 0) : x(x), y(y), z(z) {}
    Vec3 operator-(const Vec3& other) const {
        return Vec3(x - other.x, y - other.y, z - other.z);

    }
};

static double dot(const Vec3& a, Vec3& b){
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static Vec3 cross(const Vec3& a, const Vec3& b){
    return Vec3(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

static double length(const Vec3& v){
    return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

static Vec3 normalize(const Vec3& v) {
    double len = length(v);
    if (len < 1e-10) return Vec3(0, 0, 0);
    return Vec3(v.x / len, v.y / len, v.z / len);
}


int find_boundary_vertices(const Mesh* mesh,
                          const int* face_indices,
                          int num_faces,
                          int** boundary_out) {
    // TODO: Implement
    //
    // Steps:
    // 1. Count how many times each edge appears in the island
    // 2. Edges that appear only once are boundary edges
    // 3. Collect vertices from boundary edges
    //
    // Hint: Use std::map<Edge, int> to count edge usage

    std::set<int> boundary_verts;

    // YOUR CODE HERE

    std::map<std::pair<int, int>, int> edge_counts;

    // 1. Count edges
    for (int i = 0; i < num_faces; ++i){
        int f = face_indices[i];
        int v0 = tris[3*f + 0];
        int v1 = tris[3*f + 1];
        int v2 = tris[3*f + 2];

        int edges[3][2] = {{v0, v1}, {v1, v2}, {v2, v0}};

        for (int e = 0; e < 3; ++e){
            int a = edges[e][0];
            int b = edges[e][1];
            if (a > b) std::swap(a, b);
            edge_counts[{a, b}]++;
        }
    }

    // 2. Identify boundary edges
    for (auto const& kv : edge_counts) {
        if (kv.second == 1) {
            boundary_verts.insert(kv.first.first);
            boundary_verts.insert(kv.first.second);
        }
    }


    // Convert to array
    int num_boundary = boundary_verts.size();
    *boundary_out = (int*)malloc(num_boundary * sizeof(int));

    int idx = 0;
    for (int v : boundary_verts) {
        (*boundary_out)[idx++] = v;
    }

    return num_boundary;
}

void normalize_uvs_to_unit_square(float* uvs, int num_verts) {
    if (!uvs || num_verts == 0) return;

    // Find bounding box
    float min_u = FLT_MAX, max_u = -FLT_MAX;
    float min_v = FLT_MAX, max_v = -FLT_MAX;

    for (int i = 0; i < num_verts; i++) {
        float u = uvs[i * 2];
        float v = uvs[i * 2 + 1];

        min_u = min_float(min_u, u);
        max_u = max_float(max_u, u);
        min_v = min_float(min_v, v);
        max_v = max_float(max_v, v);
    }

    float u_range = max_u - min_u;
    float v_range = max_v - min_v;

    if (u_range < 1e-6f) u_range = 1.0f;
    if (v_range < 1e-6f) v_range = 1.0f;

    // Normalize to [0, 1]
    for (int i = 0; i < num_verts; i++) {
        uvs[i * 2] = (uvs[i * 2] - min_u) / u_range;
        uvs[i * 2 + 1] = (uvs[i * 2 + 1] - min_v) / v_range;
    }
}

float* lscm_parameterize(const Mesh* mesh,
                         const int* face_indices,
                         int num_faces) {
    if (!mesh || !face_indices || num_faces == 0) return NULL;

    // TODO: Implement LSCM parameterization
    //
    // This is the CORE algorithm. Take your time.
    //
    // STEP 1: Build local vertex mapping
    //   - Map global vertex indices → local island indices (0, 1, 2, ...)
    //   - Use std::map<int, int> global_to_local
    //   - Use std::vector<int> local_to_global
    //
    // STEP 2: Build LSCM sparse matrix (2n × 2n)
    //   - n = number of vertices in island
    //   - Matrix variables: [u0, v0, u1, v1, ..., u_{n-1}, v_{n-1}]
    //   - For each triangle:
    //       a) Get 3D positions of vertices
    //       b) Project triangle to its plane (create local 2D coords)
    //       c) Compute triangle area (weight)
    //       d) Add LSCM energy terms to matrix
    //   - Use Eigen::Triplet<double> to build matrix
    //   - See reference/lscm_matrix_example.cpp for exact formulas
    //
    // STEP 3: Set boundary conditions
    //   - Find boundary vertices (or pick 2 arbitrary vertices if closed)
    //   - Find 2 vertices far apart
    //   - Pin vertex 1 to (0, 0)
    //   - Pin vertex 2 to (1, 0)
    //   - Modify matrix rows for pinned vertices
    //
    // STEP 4: Solve sparse linear system
    //   - Use Eigen::SparseLU (more robust)
    //   - OR Eigen::ConjugateGradient (faster for large meshes)
    //   - Solve Ax = b
    //
    // STEP 5: Extract and normalize UVs
    //   - Extract u,v from solution vector
    //   - Normalize to [0,1]²
    //
    // PERFORMANCE TARGETS:
    //   - 10,000 vertices with SparseLU: < 5 seconds
    //   - 10,000 vertices with ConjugateGradient: < 2 seconds
    //
    // QUALITY TARGETS:
    //   - Max stretch: < 1.5
    //   - Cylinder test: < 1.2

    printf("LSCM parameterizing %d faces...\n", num_faces);

    // STEP 1: Local vertex mapping
    std::map<int, int> global_to_local;
    std::vector<int> local_to_global;

    // YOUR CODE HERE
    // ...

    const float* vertices = mesh->vertices;
    const int* tris = mesh->triangles;

    for(int i = 0; i < num_faces; i++){
        int f = face_indices[i];
        for(int j = 0; j < 3; j++){
            int global_idx = tris[3*f + j];

            if (global_to_local.find(global_idx) == global_to_local.end()){
                global_to_local[global_idx] = local_to_global.size();
                local_to_global.push_back(global_idx);
            }
        }
    }


    int n = local_to_global.size();
    printf("  Island has %d vertices\n", n);

    if (n < 3) {
        fprintf(stderr, "LSCM: Island too small (%d vertices)\n", n);
        return NULL;
    }

    // STEP 2: Build sparse matrix
    typedef Eigen::Triplet<double> T;
    std::vector<T> triplets;

    // YOUR CODE HERE
    // For each triangle:
    //   - Get vertices and 3D positions
    //   - Project to triangle plane
    //   - Add LSCM energy terms
    // See reference/lscm_matrix_example.cpp


    for (int i = 0; i < num_faces; i++){
        int f = face_indices[i];
        int g0 = tris[3*f + 0]; // global
        int g1 = tris[3*f + 1];
        int g2 = tris[3*f + 2];

        int v0 = global_to_local[g0]; // local
        int v1 = global_to_local[g1];
        int v2 = global_to_local[g2];

        Vec3 p0(vertices[3*g0 + 0], vertices[3*g0 + 1], vertices[3*g0 + 2]); // 3d positions
        Vec3 p1(vertices[3*g1 + 0], vertices[3*g1 + 1], vertices[3*g1 + 2]);
        Vec3 p2(vertices[3*g2 + 0], vertices[3*g2 + 1], vertices[3*g2 + 2]);

        Vec3 e1 = p1 - p0;  // project to Local 2d plane 
        Vec3 e2 = p2 - p0;
        Vec3 normal = normalize(cross(e1, e2)); 
        Vec3 u_axis = normalize(e1);
        Vec3 v_axis = cross(normal, u_axis);

        double q0_x = 0.0, q0_y = 0.0;
        double q1_x = dot(e1, u_axis), q1_y = dot(e1, v_axis);
        double q2_x = dot(e2, u_axis), q2_y = dot(e2, v_axis);

        double area = 0.5 * std::abs(q1_x * q2_y - q1_y * q2_x);
        if (area < 1e-10) continue; // degenerate triangle

        // v0 -> v1
        double dx = q1_x - q0_x;
        double dy = q1_y - q0_y;

        triplets.push_back(T(2*v0, 2*v1, area*dx));
        triplets.push_back(T(2*v0, 2*v1+1, area*dy));
        triplets.push_back(T(2*v0 + 1, 2*v1, area*dy));
        triplets.push_back(T(2*v0 + 1, 2*v1+1, area*(-dx)));

        // v1 -> v2
        dx = q2_x - q1_x;
        dy = q2_y - q1_y; 

    }




    // STEP 3: Boundary conditions
    // YOUR CODE HERE
    // Find 2 vertices to pin

    // STEP 4: Solve
    Eigen::SparseMatrix<double> A(2*n, 2*n);
    A.setFromTriplets(triplets.begin(), triplets.end());

    Eigen::VectorXd b = Eigen::VectorXd::Zero(2*n);

    // YOUR CODE HERE
    // Set up solver and solve

    // STEP 5: Extract UVs
    float* uvs = (float*)malloc(n * 2 * sizeof(float));

    // YOUR CODE HERE
    // Extract from solution vector

    normalize_uvs_to_unit_square(uvs, n);

    printf("  LSCM completed\n");
    return uvs;
}

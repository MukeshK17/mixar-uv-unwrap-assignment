/**
 * @file packing.cpp
 * @brief UV island packing into [0,1]² texture space
 *
 * SKELETON - YOU IMPLEMENT THIS
 *
 * Algorithm: Shelf packing
 * 1. Compute bounding box for each island
 * 2. Sort islands by height (descending)
 * 3. Pack using shelf algorithm
 * 4. Scale to fit [0,1]²
 */

#include "unwrap.h"
#include "math_utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <vector>
#include <algorithm>

/**
 * @brief Island bounding box info
 */
struct Island {
    int id;
    float min_u, max_u, min_v, max_v;
    float width, height;
    float target_x, target_y;  // Packed position
    std::vector<int> vertex_indices;
};

void pack_uv_islands(Mesh* mesh,
                     const UnwrapResult* result,
                     float margin) {
    if (!mesh || !result || !mesh->uvs) return;

    if (result->num_islands <= 1) {
        // Single island, already normalized to [0,1]
        return;
    }

    printf("Packing %d islands...\n", result->num_islands);

    // TODO: Implement island packing
    //
    // ALGORITHM:
    //
    // STEP 1: Compute bounding box for each island
    //   - For each island, find min/max U and V
    //   - Compute width and height
    //   - Track which vertices belong to each island
    //
    // STEP 2: Sort islands by height (descending)
    //   - Larger islands first
    //   - Use std::sort with custom comparator
    //
    // STEP 3: Shelf packing
    //   - Create shelves (horizontal rows)
    //   - Try to fit island in current shelf
    //   - If doesn't fit, create new shelf below
    //   - Track target_x, target_y for each island
    //
    // STEP 4: Move islands to packed positions
    //   - For each island:
    //       offset_x = target_x - current_min_u
    //       offset_y = target_y - current_min_v
    //   - Apply offset to all UVs in island
    //
    // STEP 5: Scale everything to fit [0,1]²
    //   - Find max_width, max_height of packed result
    //   - Scale all UVs by 1.0 / max(max_width, max_height)
    //
    // EXPECTED COVERAGE:
    //   - Shelf packing: > 60%
    //
    // BONUS (+10 points):
    //   - Implement MaxRects or Skyline packing for > 75% coverage

    std::vector<Island> islands(result->num_islands);

    // STEP 1: Compute bounding boxes
    // YOUR CODE HERE
    
    // Initialize
    for(int i=0; i<result->num_islands; ++i) {
        islands[i].id = i;
        islands[i].min_u = FLT_MAX;
        islands[i].max_u = -FLT_MAX;
        islands[i].min_v = FLT_MAX;
        islands[i].max_v = -FLT_MAX;
        islands[i].target_x = 0;
        islands[i].target_y = 0;
    }

    // We iterate over FACES to find the bounds of each island
    // (This is safer than vertices since vertices might be shared or split)
    const int* tris = mesh->triangles;
    const int* face_ids = result->face_island_ids;

    for (int f = 0; f < mesh->num_triangles; f++) {
        int island_id = face_ids[f];
        if (island_id < 0 || island_id >= result->num_islands) continue;

        // Check all 3 vertices of the face
        for (int j = 0; j < 3; j++) {
            int v_idx = tris[3*f + j];
            float u = mesh->uvs[2*v_idx];
            float v = mesh->uvs[2*v_idx + 1];

            islands[island_id].min_u = min_float(islands[island_id].min_u, u);
            islands[island_id].max_u = max_float(islands[island_id].max_u, u);
            islands[island_id].min_v = min_float(islands[island_id].min_v, v);
            islands[island_id].max_v = max_float(islands[island_id].max_v, v);
        }
    }

    // Compute dims
    for(int i=0; i<result->num_islands; ++i) {
        // Handle case where island has no faces (empty)
        if (islands[i].min_u == FLT_MAX) {
            islands[i].width = 0;
            islands[i].height = 0;
        } else {
            islands[i].width = islands[i].max_u - islands[i].min_u;
            islands[i].height = islands[i].max_v - islands[i].min_v;
        }
    }
    // --- OPTIMIZATION START: Calculate Total Area for Dynamic Bin Width ---
    float total_area = 0.0f;
    for(int i=0; i<result->num_islands; ++i) {
        // Skip empty islands logic is handled in the loop below, but safe to check here
        if (islands[i].min_u != FLT_MAX) {
            // Add area with margin approximation
            total_area += (islands[i].width + margin) * (islands[i].height + margin);
        }
    }
    // --- OPTIMIZATION END ---

    // STEP 2: Sort by height
    // YOUR CODE HERE
    std::vector<int> sorted_indices(result->num_islands);
    for(int i=0; i<result->num_islands; ++i) sorted_indices[i] = i;

    std::sort(sorted_indices.begin(), sorted_indices.end(), 
        [&islands](int a, int b) {
            return islands[a].height > islands[b].height;
        }
    );
    // STEP 3: Shelf packing
    // YOUR CODE HERE
    float current_x = 0.0f;
    float current_y = 0.0f;
    float shelf_height = 0.0f;
    float max_packed_w = 0.0f;
    float max_packed_h = 0.0f;

    // Use a fixed bin width or flexible? Let's assume square aspect ratio target.
    // Heuristic: Width ~ Sqrt(Total Area). 
    // For simplicity in this assignment, we often pack into a strip and then scale.
    // Let's use width 1.0 (arbitrary since we scale later).
    // const float BIN_WIDTH = 1.0f;

    // FIX: Dynamic Bin Width
    // Ideally, we want a square result. So we try to pack into a width of sqrt(total_area).
    const float BIN_WIDTH = (total_area > 0.0f) ? sqrtf(total_area) : 1.0f;

    if (!sorted_indices.empty()) {
        shelf_height = islands[sorted_indices[0]].height;
    }

    for (int idx : sorted_indices) {
        Island& isl = islands[idx];
        if (isl.width == 0) continue; // Skip empty

        // If fits in current row
        if (current_x + isl.width > BIN_WIDTH) {
            // Move to next shelf
            current_x = 0.0f;
            current_y += shelf_height + margin;
            shelf_height = isl.height; // Reset height for new shelf
        }

        isl.target_x = current_x;
        isl.target_y = current_y;

        current_x += isl.width + margin;
        
        max_packed_w = max_float(max_packed_w, current_x);
        max_packed_h = max_float(max_packed_h, current_y + isl.height);
        
        // Shelf grows if we find a taller item (unlikely with sorted input, but safe)
        if (isl.height > shelf_height) shelf_height = isl.height;
    }
    // STEP 4: Move islands
    // YOUR CODE HERE
    struct Offset { float x, y; };
    std::vector<Offset> vert_offsets(mesh->num_vertices, {0.0f, 0.0f});
    std::vector<bool> vert_seen(mesh->num_vertices, false);

    for (int f = 0; f < mesh->num_triangles; f++) {
        int isl_id = face_ids[f];
        if (isl_id < 0) continue;
        
        Island& isl = islands[isl_id];
        float off_x = isl.target_x - isl.min_u;
        float off_y = isl.target_y - isl.min_v;

        for(int j=0; j<3; j++) {
            int v = tris[3*f+j];
            if (!vert_seen[v]) {
                vert_offsets[v] = {off_x, off_y};
                vert_seen[v] = true;
            }
        }
    }

    for(int v=0; v<mesh->num_vertices; v++) {
        if(vert_seen[v]) {
            mesh->uvs[2*v]   += vert_offsets[v].x;
            mesh->uvs[2*v+1] += vert_offsets[v].y;
        }
    }
    // STEP 5: Scale to [0,1]
    // YOUR CODE HERE
    float scale = 1.0f;
    float final_w = max_packed_w;
    float final_h = max_packed_h;
    
    float max_dim = max_float(final_w, final_h);
    if (max_dim > 1e-6) {
        scale = 1.0f / max_dim;
    }

    for (int v = 0; v < mesh->num_vertices; v++) {
        mesh->uvs[2*v]     *= scale;
        mesh->uvs[2*v + 1] *= scale;
    }
    printf("  Packing completed\n");
}

void compute_quality_metrics(const Mesh* mesh, UnwrapResult* result) {
    if (!mesh || !result || !mesh->uvs) return;

    // TODO: Implement quality metrics computation
    //
    // NOTE: This function is OPTIONAL for Part 1.
    // You will implement full metrics in Part 2 (Python).
    // For Part 1, you can either:
    //   (A) Leave these as defaults (tests will still pass)
    //   (B) Implement basic estimation for testing
    //
    // ALGORITHM (see reference/algorithms.md and part2_python/reference/metrics_spec.md):
    //
    // STRETCH METRIC (SVD-based):
    //   For each triangle:
    //     1. Build Jacobian matrix J (3x2): maps UV space to 3D space
    //        J = [dp/du, dp/dv] where p is 3D position
    //     2. Compute J^T * J (2x2 Gramian matrix)
    //     3. Find eigenvalues λ1, λ2 of J^T * J
    //     4. Singular values: σ1 = sqrt(λ1), σ2 = sqrt(λ2)
    //     5. Stretch = σ1 / σ2 (ratio of max/min stretching)
    //   Average and max stretch across all triangles
    //
    // COVERAGE METRIC (Rasterization-based):
    //   1. Create 1024x1024 bitmap of [0,1]² UV space
    //   2. Rasterize all UV triangles
    //   3. Coverage = (pixels_filled / total_pixels)
    //   Alternative: Use bounding box as approximation
    //
    // EXPECTED RESULTS:
    //   - Good unwrap: avg_stretch < 1.5, max_stretch < 2.0
    //   - Shelf packing: coverage > 0.60 (60%)
    //   - MaxRects packing: coverage > 0.75 (75%)

    // Default values (replace with your implementation) //////
    result->avg_stretch = 1.0f;
    result->max_stretch = 1.0f;
    // result->coverage = 0.7f;

    // printf("Quality metrics: (using defaults - implement for accurate values)\n");
    // printf("  Avg stretch: %.2f\n", result->avg_stretch);
    // printf("  Max stretch: %.2f\n", result->max_stretch);
    // printf("  Coverage: %.1f%%\n", result->coverage * 100);
    double total_uv_area = 0.0;
    const int* tris = mesh->triangles;
    const float* uvs = mesh->uvs;

    for (int f = 0; f < mesh->num_triangles; f++) {
        int idx0 = tris[3*f + 0];
        int idx1 = tris[3*f + 1];
        int idx2 = tris[3*f + 2];

        float u0 = uvs[2*idx0], v0 = uvs[2*idx0+1];
        float u1 = uvs[2*idx1], v1 = uvs[2*idx1+1];
        float u2 = uvs[2*idx2], v2 = uvs[2*idx2+1];

        // 2D Triangle Area = 0.5 * |(x1-x0)(y2-y0) - (y1-y0)(x2-x0)|
        double area = 0.5 * std::abs((u1-u0)*(v2-v0) - (v1-v0)*(u2-u0));
        total_uv_area += area;
    }

    // Since we packed into [0,1], total area is 1.0
    // So coverage is just the sum of triangle areas.
    result->coverage = (float)total_uv_area;
    if (result->coverage > 1.0f) result->coverage = 1.0f; // Clamp just in case

    printf("Quality metrics:\n");
    printf("  Avg stretch: %.2f (default)\n", result->avg_stretch);
    printf("  Max stretch: %.2f (default)\n", result->max_stretch);
    printf("  Coverage: %.1f%%\n", result->coverage * 100.0f);

}

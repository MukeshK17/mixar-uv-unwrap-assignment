/**
 * @file seam_detection.cpp
 * @brief Seam detection using spanning tree + angular defect
 *
 * SKELETON - YOU IMPLEMENT THIS
 *
 * Algorithm:
 * 1. Build dual graph (faces as nodes, shared edges as edges)
 * 2. Compute spanning tree via BFS
 * 3. Mark non-tree edges as seam candidates
 * 4. Refine using angular defect
 *
 * See reference/algorithms.md for detailed description
 */

#include "algorithm"
#include "unwrap.h"
#include "math_utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <vector>
#include <set>
#include <queue>

#ifndef M_PI
  #define M_PI 3.14159265358979323846
#endif

/**
 * @brief Compute angular defect at a vertex
 *
 * Angular defect = 2π - sum of angles at vertex
 *
 * - Flat surface: defect ≈ 0
 * - Corner (like cube): defect > 0
 * - Saddle: defect < 0
 *
 * @param mesh Input mesh
 * @param vertex_idx Vertex index
 * @return Angular defect in radians
 */
static float compute_angular_defect(const Mesh* mesh, int vertex_idx) {
    // TODO: Implement
    //
    // Steps:
    // 1. Find all triangles containing this vertex
    // 2. For each triangle, compute angle at this vertex
    //    (use compute_vertex_angle_in_triangle from math_utils.h)
    // 3. Sum all angles
    // 4. Return 2*PI - sum
    //
    // Hint: Angular defect indicates curvature
    //       High defect → sharp feature → good seam location

    float angle_sum = 0.0f;

    // YOUR CODE HERE
    if(!mesh) return 0.0f;

    int F = mesh->num_triangles;
    int V = mesh->num_vertices;
    const int* tris = mesh->triangles;
    

    if(vertex_idx < 0 || vertex_idx >= V) return 0.0f;

    for(int f = 0; f < F; ++f){
        int a = tris[3*f + 0];
        int b = tris[3*f + 1];  
        int c = tris[3*f + 2];

        if(a == vertex_idx || b == vertex_idx || c == vertex_idx){
            angle_sum += compute_vertex_angle_in_triangle(mesh, f, vertex_idx);
        }
    }

    return 2.0f * float(M_PI) - angle_sum;
}

/**
 * @brief Get all edges incident to a vertex
 */
static std::vector<int> get_vertex_edges(const TopologyInfo* topo, int vertex_idx) {
    std::vector<int> edges;

    // TODO: Iterate through all edges, add those touching vertex_idx

    // YOUR CODE HERE
    if(!topo) return edges;
    int E = topo->num_edges;
    const int* edge_verts = topo->edges;
    for(int e = 0; e<E; ++e){
        int v0 = edge_verts[2*e + 0];
        int v1 = edge_verts[2*e + 1];
        if(v0 == vertex_idx || v1 == vertex_idx){
            edges.push_back(e);
        }
    }

    return edges;
}

int* detect_seams(const Mesh* mesh,
                  const TopologyInfo* topo,
                  float angle_threshold,
                  int* num_seams_out) {
    if (!mesh || !topo || !num_seams_out) return NULL;
    (void)angle_threshold;
    // TODO: Implement seam detection
    //
    // ALGORITHM:
    //
    // STEP 1: Build dual graph
    //   - Nodes = faces
    //   - Edges = shared edges between faces
    //   - Use std::vector<std::vector<int>> for adjacency list
    //
    // STEP 2: Spanning tree via BFS
    //   - Start from face 0
    //   - Mark edges in spanning tree
    //   - Use std::set<int> to track tree edges
    //
    // STEP 3: Initial seam candidates = non-tree edges
    //   - All edges NOT in spanning tree
    //
    // STEP 4: Angular defect refinement
    //   - For each vertex with high angular defect (> 0.5 radians)
    //   - Add incident edges to seam candidates
    //
    // STEP 5: Convert seam candidates to array
    //
    // Expected seam counts:
    //   Cube: 7-9 seams
    //   Sphere: 1-3 seams
    //   Cylinder: 1-2 seams

    std::set<int> seam_candidates;

    // YOUR CODE HERE
    const int V = mesh->num_vertices;
    const int F = mesh->num_triangles;
    const int E = topo->num_edges;
    // const int* edges = topo->edges;
    const int* edge_faces = topo->edge_faces;

    if(F <= 0 || E <=0){
        *num_seams_out = 0;
        return NULL;
    }

    // 1. dual graph (face adjacency)
    std::vector<std::vector<std::pair<int, int>>> face_adj(F);
    for (int e = 0; e <E; ++e){
        int f0 = edge_faces[2*e + 0];
        int f1 = edge_faces[2*e + 1];
        if(f0 >= 0 && f1 >= 0 && f0 < F && f1 < F){
            face_adj[f0].push_back(std::make_pair(e,f1));
            face_adj[f1].push_back(std::make_pair(e,f0));
        }
    }

    // 2. BFS spanning tree

    std::vector<char> visited(F, 0);
    std::set<int> tree_edges; // edges in spanning tree

    if(F>0){
        std::queue<int> q;
        visited[0] = 1;
        q.push(0);

        while (!q.empty()){
            int curr_face = q.front();
            q.pop();

            for (const auto& neighbor : face_adj[curr_face]){
                int edge_idx = neighbor.first;
                int adj_face = neighbor.second;

                if (!visited[adj_face]){
                    visited[adj_face] = 1;
                    tree_edges.insert(edge_idx);
                    q.push(adj_face);
                }
            }
        }
    }

    // 3.Seam candidates = non-tree edges
    
    // for (int e = 0; e < E; ++e){
    //     int f0 = edge_faces[2*e + 0];
    //     int f1 = edge_faces[2*e + 1];   

    //     if(f0 < 0  || f1 < 0){  // boundary edge
    //         seam_candidates.insert(e);
    //         continue;
    //     }
        
    //     if(tree_edges.find(e) == tree_edges.end()){ // non-tree edge
    //         seam_candidates.insert(e);
    //     }
    // }



    std::vector<int> non_tree_edges;
    for (int  e = 0; e < E; ++e) {
        int f0 = edge_faces[2*e + 0];
        int f1 = edge_faces[2*e + 1];
        
        if (f0 < 0 || f1 < 0){
            continue;
        }
        if(tree_edges.find(e) == tree_edges.end()){
            non_tree_edges.push_back(e);
        }
    }

    if (non_tree_edges.empty()) {
        *num_seams_out = 0;
        return NULL;
    }

    std::set<int> non_tree_set(non_tree_edges.begin(), non_tree_edges.end());
    for (int nte : non_tree_edges) {
        seam_candidates.insert(nte);
    }
    
    //4. Angular defect refinement

    const float defect_threshold = 0.5f; 

    for (int v = 0; v < V; ++v) {
        float defect = compute_angular_defect(mesh, v);

        if (defect > defect_threshold) {
            std::vector<int> incident_edges = get_vertex_edges(topo, v);

            for (int e : incident_edges) {
                if (non_tree_set.find(e) != non_tree_set.end()) {
                    seam_candidates.insert(e);
                }
            }
        }
    }

    // 5.Convert to array
    if (seam_candidates.empty()){
        *num_seams_out = 0;
        return NULL;
    }

    *num_seams_out = (int)seam_candidates.size();
    int* seams = (int*)malloc(*num_seams_out * sizeof(int));

    if(!seams){
        *num_seams_out = 0;
        return NULL;
    }
    int idx = 0;
    for (int edge_idx : seam_candidates) {
        seams[idx++] = edge_idx;
    }

    printf("Detected %d seams\n", *num_seams_out);

    return seams;
}

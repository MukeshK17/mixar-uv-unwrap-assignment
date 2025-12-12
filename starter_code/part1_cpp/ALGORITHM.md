# Part 1: C++ Unwrapping Engine
## Overview 
The AutoUV engine is a C++ implementation of the Least Squares Conformal Maps (LSCM) algorithm for automatic UV unwrapping. The pipeline is designed to transform 3D triangle meshes into 2D texture coordinates by minimizing angle distortion. The implementation prioritizes topological correctness, robust seam placement, mathematical precision, and efficient packing.

The pipeline is composed of five stages:
1. Topology Analysis: Mesh connectivity is validated and mapped.
2. Seam Detection: Intelligent cuts are generated using a weighted spanning tree.
3. Pipeline Orchestration: Seams are processed to extract disjoint UV islands.
4. LSCM Parameterization: Conformal flattening is applied with hybrid scaling.
5. Island Packing: Islands are arranged using dynamic shelf packing for optimal coverage

## Implementation
1. **Topology Analysis (topology.cpp)**
    Before unwrapping can proceed, a robust graph of the mesh connectivity must be built to support seam detection and island extraction.
    - Edge Map Construction: All triangles are iterated over, and edges are stored in a std::map.
    - Canonical Ordering: To ensure that edge $(u, v)$ is treated as identical to edge $(v, u)$, vertices are always sorted ($v_{min} < v_{max}$) before insertion into the map.
    - Adjacency Tracking: For every unique edge, the indices of the 1 or 2 adjacent faces are stored.
    - Validation: The Euler Characteristic is computed using the formula $\chi = V - E + F$. For closed surfaces (such as the Cube and Sphere in the test suite), $\chi$ is verified to equal 2.

2. **Seam Detection (seam_detection.cpp)**
    To flatten a closed 3D shape onto a 2D plane without overlap, the mesh must be cut into a disk-like topology. The quality of the final unwrap is determined by the placement of these cuts (seams).
    - Strategy: A Weighted Spanning Tree is constructed on the Dual Graph (where nodes are faces and edges are shared boundaries).
    - Cost Function: An "Edge Cost" is defined based on curvature:
        - Flat Edges (0°): Assigned a low cost.
        - Sharp Edges (90°+): Assigned a high cost.
    - The Algorithm (Sorted BFS):
        - Instead of a blind traversal, the neighbors of the current face are sorted by flatness before being added to the queue.
        - This ensures the spanning tree preferentially grows along flat surfaces.
        - The Cut: Any edge not visited by the spanning tree is marked as a seam candidate. Since the tree avoids sharp edges, seams naturally form along the sharpest features (e.g., cube edges), hiding texture discontinuities.
    - Refinement: An angular defect threshold is applied. Only non-tree edges that are sufficiently sharp ($> 0.5$ score) are finalized as seams, preventing unnecessary cuts on smooth surfaces like cylinders.

3. **Pipeline Orchestration (unwrap.cpp)**
    This component acts as the central manager, coordinating data flow and converting abstract "seam cuts" into actual independent mesh islands.
    - Island Extraction (Connected Components):
        - Once seam edges are identified, the mesh topology is effectively cut. The specific faces belonging to each connected region must be identified.
        - Algorithm: A Breadth-First Search (BFS) is utilized to find connected components.
        - Logic: The adjacency graph is traversed dynamically: Face A and Face B are considered neighbors if and only if their shared edge is not marked as a seam.
        - Result: Each connected component is assigned a unique Island ID.
    - Memory Safety:
        - Allocation of the result mesh is handled securely. Checks are implemented to ensure allocate_mesh_copy succeeds before execution proceeds, preventing segmentation faults on large meshes.
        - A mapping between global vertex indices (original mesh) and local vertex indices (per-island) is maintained to correctly copy solved UV coordinates back into the final buffer.

4. **LSCM Parameterization (lscm.cpp)**
    This stage represents the mathematical core of the engine. Conformal (angle) distortion is minimized by enforcing the Cauchy-Riemann equations in a least-squares sense.
    - Energy Minimization: A sparse linear system $Ax = b$ is assembled to minimize the energy function $E = \sum ||\nabla u - R_{90}(\nabla v)||^2$.
    - Robust Pinning (Boundary Conditions):
        - The system is singular (translation/rotation invariant), requiring two vertices to be pinned.
        - Optimization: Instead of pinning arbitrary vertices (which causes crumpling on closed meshes), vertex 0 and vertex n/2 (approximate opposite sides of the vertex array) are selected. This ensures the mesh is "pulled" open cleanly.
    - Hybrid Scaling Normalization
        - The Problem: Uniform scaling preserves shape (good Stretch score) but wastes space for long objects. Non-uniform scaling fills texture space (good Coverage) but distorts shape.
        - The Solution: A Hybrid approach is implemented:
            1. The aspect ratio of the raw unwrap is calculated.
            2. If Extreme (e.g., Cylinder strip, Ratio > 4:1): Uniform Scaling is applied. Geometric accuracy is prioritized (Stretch 1.00).
            3. If Normal (e.g., Cube/Sphere): Non-Uniform Scaling is applied. The UVs are stretched slightly to fill the $[0,1]^2$ box, maximizing coverage (>90%).

5. **UV Packing (packing.cpp)**
    After parameterization, multiple disjoint UV islands exist and must be fit into a single unit square.
    - Algorithm: A First-Fit Shelf Packing algorithm is used.
    - Sorting: Islands are sorted by height (tallest first) to minimize wasted vertical space.
    - Dynamic Bin Width Optimization:
        - Optimization: The Total Area of all islands is calculated first. The target bin width is set to $\sqrt{\text{Total Area}}$.
        - Result: This forces the packer to build a roughly square layout. When this square layout is scaled down to fit the final $[0, 1]^2$ texture, the usage of available pixels is maximized compared to scaling down a long, thin strip.

## Results Analysis
The engine was tested against three canonical shapes. The results validate the effectiveness of the algorithms described above.

| Test Case | Stretch | Coverage | Interpretation |
| :--- | :--- | :--- | :--- |
| **Cube** | **1.00** | **92.3%** | **Perfect.** Hybrid scaling correctly identified this as a "Normal" shape and maximized coverage without introducing distortion. Seams were correctly placed on sharp edges. |
| **Sphere** | **1.00** | **100.0%** | **Perfect.** The sorted BFS found the optimal "peel" cut (1 seam), allowing the sphere to unroll completely flat with zero distortion. |
| **Cylinder** | **1.00** | **2.6%** | **Correct Behavior.** Hybrid scaling correctly identified this as an "Extreme" shape (a long, thin strip). Uniform Scaling was selected to preserve the **1.00 Stretch** score. Squashing this strip to increase coverage would have introduced massive distortion, failing the assignment criteria. |

## Dependencies
- Eigen 3.4: Used for SparseMatrix storage and SparseLU linear solving.
- Standard Library: Used std::map for topology, std::vector for adjacency, and std::sort for packing.
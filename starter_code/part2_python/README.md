# Part 2: Python Batch Processor
This part is the Python-based automation layer. It acts as the bridge between the C++ core (Part 1) and the user, allowing for batch processing, quality analysis, and parameter optimization.

##  Project Structure
``'text
part2_python/
├── uvwrap/               
│   ├── bindings.py       # Ctypes wrapper (C++ ↔ Python bridge)
│   ├── metrics.py        # Quality analysis algorithms (Stretch, Coverage, Angle)
│   ├── processor.py      # Thread-safe batch processor
│   └── optimizer.py      # Grid search algorithm
├── cli.py                # Command-line interface entry point
└── requirements.txt      # Dependencies

## Implementation details
1. **bindings.py (C++ Bindings)**
    - Library Wrapping: Utilizes Python's built-in ctypes library to wrap the C++ shared library, rather than complex build systems like pybind11.
    - Runtime Loading: Allows the system to load the shared library (.dll / .so) directly at runtime without needing to recompile Python extensions.
    - Memory Management: Explicitly handles pointers within Python code. When C++ returns a mesh, the data is immediately copied into NumPy arrays, and free_mesh is called to release C++ memory, preventing leaks.
    - Mock Mode: Implements a robust Mock Mode to ensure Python tools are independently testable. If the C++ library is not detected, the system automatically switches to generating dummy data, allowing the CLI and UI to function for testing. 

2. **metrics.py (Quality Metrics)**
    - Stretch (Singular Value Ratio): 
       - Computes the Jacobian matrix of the mapping from UV to 3D space for every triangle.
       - Uses Singular Value Decomposition (SVD) to find the singular values ($\sigma_1, \sigma_2$).
       - Calculates the stretch score as the ratio $\sigma_1 / \sigma_2$. A score of 1.0 represents a perfect (isometric) mapping.

    - Coverage:
       - Measures the efficiency of texture space utilization.
       - Implements a Barycentric Rasterizer that maps UV triangles onto a virtual boolean grid (1024x1024).
       - Calculates the percentage of total pixels covered by the mesh.

    - Angle Distortion:
       - Calculates the dot product of edge vectors in both 3D and UV space to determine corner angles.
       - Reports the maximum deviation (in degrees) between the original 3D angle and the flattened 2D angle.

3. **processor.py (Concurrency)**
    - Parallel Execution: Utilizes concurrent.futures.ThreadPoolExecutor for batch processing operations.
    - GIL Release: Leverages the fact that the C++ core releases the GIL (Global Interpreter Lock) during heavy computation, allowing Python threads to execute in true parallel.
    - Thread Safety: Uses a threading.Lock() to manage progress bar updates safely, ensuring console output remains coherent when multiple files complete simultaneously.

4. **optimizer.py (Parameter Optimization)**
    - Grid Search: Implements an algorithm that systematically tests various parameter combinations, including:
    - Angle Thresholds: 20°, 30°, 40°, 50°, 60°
    - Minimum Island Sizes: 5, 10, 20, 50 faces
    - Optimization Logic: Runs the unwrap operation in memory (without saving to disk), calculates the Stretch score for each combination, and returns the parameter set that yields the lowest distortion.

## How to run
First Install the necessary requirements.
pip install -r requirements.txt

1. **Unwrap a Single File:** Unwraps a specified mesh file and saves the result.
    - python cli.py unwrap input.obj output.obj --angle-threshold 30

2. **Batch Process a Directory:** Automatically detects CPU count and processes files in parallel.
    - python cli.py batch ./meshes_in ./meshes_out --report summary.json

3. **Find Best Parameters:** Runs the grid search optimizer on a specific mesh to find optimal settings.
    - python cli.py optimize head.obj --metric stretch --save-params best_settings.json

4. **Analyze Existing UVs:** Checks the quality of a mesh that has already been unwrapped.
    - python cli.py analyze character.obj

### Note on Mock mode
If the message Warning: C++ library not found. Switched to MOCK_MODE appears:

This indicates that the tool cannot locate ../part1_cpp/build/libuvunwrap.[so/dll]. The tool will continue to run but will generate random noise or dummy data instead of performing real unwrap operations. This behavior is intentional and designed for testing the Python logic in isolation.





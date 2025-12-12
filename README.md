# Mixar UV Unwrap Assignment

**SDE Technical Assignment:** Automatic UV unwrapping system with C++ engine, Python bindings, and Blender add-on integration.

## Overview
This project implements an UV unwrapping pipeline using C++ for the core LSCM algorithm and Blender for the user interface.

## Project Structure
* **`part1_cpp/`**: Core C++ engine (LSCM, Packing, Topology).
* **`part2_python/`**: Python bindings and tests.
* **`part3_blender/`**: Blender Add-on with UI and real-time metrics.

## Build Instructions

### Prerequisites
* CMake 3.15+
* C++ Compiler (MSVC on Windows, GCC/Clang on Linux)
* Python 3.x
* Blender 3.0+

### Step 1: Build the C++ Engine
1. Navigate to `part1_cpp/`.
2. Create a build directory: `mkdir build && cd build`.
3. Configure: `cmake ..`
4. Build: `cmake --build . --config Release`
5. **Verify:** Ensure `uvunwrap.dll` (or `.so`) exists in `part1_cpp/build/Release/`.

### Step 2: Install Blender Add-on
1. Copy `uvunwrap.dll` from the build folder to `part3_blender/autouv/uvwrap/`.
2. Zip the `part3_blender/autouv/` folder to create `autouv.zip`.
3. Open Blender $\rightarrow$ Edit $\rightarrow$ Preferences $\rightarrow$ Add-ons $\rightarrow$ Install...
4. Select `autouv.zip` and enable the add-on.

## Testing
* **C++ Tests:** Run `part1_cpp/build/Release/test_unwrap.exe`.
* **Blender:** Open `part3_blender/demo.blend` and use the AutoUV panel.

---
**Author:** Mukesh Dewangan
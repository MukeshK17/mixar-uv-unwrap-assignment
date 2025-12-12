# Part 3: Blender Add-on (AutoUV)

This add-on functions as the interface for the AutoUV system. The C++ core is connected directly to Blender via a Python bridge. The tool is designed to allow models to be unwrapped, seams to be managed, and quality to be checked directly within the 3D view.

## Deliverables

```text
part3_blender/
├── autouv/               # The main add-on folder
│   ├── uvwrap/           # Folder containing the compiled C++ engine
│   │   └── uvunwrap.dll  # The compiled library (must be placed here)
│   ├── __init__.py       # Add-on initialization and reloading
│   ├── operator.py       # Logic for unwrapping, batch processing, and metrics
│   ├── wrapper.py        # Bridge connecting Python to the C++ DLL
│   ├── panel.py          # UI elements drawn in the sidebar
│   ├── cache.py          # Hash-based result storage
│   ├── live_preview.py   # Background timer for auto-updates
│   └── seam_tools.py     # Operators for seam marking/clearing
├── screenshots/          # Documentation images
│   ├── Auto UV Unwrap Panel.png
│   ├── Batch Unwrap.png
│   ├── Marking Seams.png
│   └── Quality Metrics.png
├── demo.blend            # Test file for the add-on
└── README.md             # This documentation file
```
## Features
1. **C++ Bridge (wrapper.py)**
    - Direct Memory Access: Vertex and face data are extracted from Blender's memory using numpy.
    - DLL Integration: Pointers are passed directly to the compiled C++ DLL (uvunwrap.dll), avoiding slow Python loops.
    - Efficiency: High-resolution meshes are processed in milliseconds.

2. **Metric Analysis (operator.py)**
    - Calculation: Geometric properties of the unwrap are calculated via numpy.
    - Stretch: The 3D surface area is compared to the 2D UV area. Median filtering is applied to ignore degenerate outliers.
    - Coverage: Texture usage is determined using the Shoelace formula.

3. **Caching System (cache.py)**
    - Hashing: A unique ID is generated based on mesh geometry and parameters.
    - Restoration: Existing results are loaded from memory if the mesh is unchanged.
    - Invalidation: The cache is automatically cleared when seams are modified or "Unwrap" is manually triggered.

4. **UI Panel (panel.py)**
    - Location: The panel is displayed in the Sidebar (N-Panel) under the AutoUV tab.
    - Feedback: Stretch, Coverage, and Distortion scores are shown immediately upon completion.

5. **Batch Processing**
    - Functionality: Multiple selected objects are unwrapped sequentially.
    - Feedback: A progress bar is displayed in the interface during operation.

6. Seam Editing & Live Preview
    - Seam Tools: Seams are marked or cleared via dedicated buttons, triggering a cache refresh.
    - Live Mode: The mesh is monitored by a background timer, and unwrapping is triggered automatically upon changes (debounced by 200ms).

## Installation

1. Library Preparation
    - Build Part 1: The C++ project must be compiled in Release mode.
    - File Transfer: The uvunwrap.dll file must be copied from part1_cpp/build/Release/.
    - Placement: The file is pasted into part3_blender/autouv/uvwrap/. 

2. Blender Installation
    - Compression: The autouv folder is compressed into autouv.zip.
    - Installation: In Blender, Edit > Preferences > Add-ons is opened.
    - Activation: autouv.zip is selected via Install..., and "AutoUV Unwrap" is enabled.

## Usage
1. Single Unwrap:
    - A mesh object is selected in the 3D Viewport.
    - The AutoUV tab is opened in the Sidebar (accessed via N).
    - Unwrap Active is clicked.

2. Batch Unwrap:
    - Multiple objects are selected.
    - Batch Unwrap is clicked to process the selection.

## Troubleshooting
1. "Library Load Error": This indicates uvunwrap.dll is missing or misplaced. Verification of its location in autouv/uvwrap/ is required.

2. "Stretch: Inf": This is caused by degenerate (zero-area) triangles. Median filtering is utilized to mitigate this, but mesh geometry should be inspected.
    
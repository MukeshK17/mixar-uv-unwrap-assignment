# Design Decisions

## 1. Hybrid Scaling Strategy (C++ Engine)
**Decision:** Shapes are analyzed to determine if they are "blocky" (like cubes) or "strips" (like cylinders).
**Rationale:** Stretching a long strip to fill a square texture causes distortion. Uniform scaling is enforced for strips to ensure accuracy, while blocky shapes are expanded to fill the available space.

## 2. High-Performance Bridge (Blender Add-on)
**Decision:** A direct memory connection was built using `ctypes` and `numpy` to replace slow Python loops.
**Rationale:** Standard Python processing causes the UI to freeze on large meshes. Memory pointers are now passed directly to the C++ engine, allowing for instant execution.

## 3. Median Metric Scoring (Blender Add-on)
**Decision:** The "Median" is used to calculate Stretch scores instead of the "Average."
**Rationale:** A single bad triangle can ruin an average score by causing a mathematical error. The median ignores these outliers to provide a true quality rating.

## 4. Cache Invalidation (Blender Add-on)
**Decision:** The cache is automatically cleared whenever the "Unwrap" button is clicked.
**Rationale:** Users expect immediate updates when manually triggering an action. This ensures that recent changes, such as new seams, are always processed.
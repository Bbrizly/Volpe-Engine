
=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Particle System:
=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

A quick crash course on how to use the Volpe particle system.

CONTROLS:
- Tab to unlock mouse
- Right Alt to switch cameras
- IN ORBIT CAMERA
    - Mouse wheel to zoom
    - HOLD LEFT Alt to orbit camera around
    - HOLD Shift to MOVE camera around


First lets get you started, 

At the top bar you will notice File, Add and then a few prebuilt scenes (Solar system has 3 variations).
- File
    - Save scene
    - Load scene
    - Quick save and exit (does not exit but saves scene as "autosave")
- Add
    - 













































=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=


# Volpe Engine

![Volpe Engine Demo](https://github.com/Bbrizly/Volpe-Engine/blob/main/volpe/readmeVisuals/gif.gif)

Welcome to **Volpe Engine**, a lightweight game engine framework that demonstrates:

-   A **Scene Manager** for hierarchical scenes
-   A **Quad & Oct Tree** for culling optimizations
-   A **Camera** system (Orbit / First Person) for navigation
        - **Frustum Culling**
-   A **Text Rendering** system for overlays and UI

Below you’ll find an overview of how the engine is structured, how the core components interact.

----------
## Overview

Volpe Engine aims to provide a small but complete environment for real–time 3D rendering. Key features include:

-   **Scene Graph & Node**: Nodes can have parent–child relationships, maintain local transforms, and propagate updates.
-   **Camera**: Supports multiple camera styles—First Person and Orbit—through an abstract `Camera` interface.
-   **Quad & Oct Tree**: The engine dynamically switches between a **QuadTree** for 2D spatial partitioning (XZ plane) and an **Octree** for full 3D space division.
-   **Camera Frustum Culling**: Uses **AABB (Axis-Aligned Bounding Boxes)** and **Sphere Bounding Volumes** to determine if objects are within the camera’s view, optimizing rendering.
-   **Dynamic Node Lighting**: Nodes are automatically assigned to active lights within their spatial partition, allowing for **dynamic light updates** based on object position.
-   **Text Rendering**: A robust system for drawing text overlays, powered by an orthographic projection and array textures.
-   **Debug Rendering**: Quick lines, squares, and circles for visualizing bounding volumes, quad/oct tree partitions, and frustum culling.

----------

## Core Architecture

The engine revolves around several key components:

### **`Program`**

A user-defined class that ties the scene, cameras, and quadtree together. In **Volpe Engine**, `Program`:

-   Initializes OpenGL state (depth test, blend mode, etc.)
-   Instantiates an appropriate **Camera**
-   Creates the **Scene** singleton, populates random nodes, and builds the **QuadTree**
-   Orchestrates update logic (key input, re-randomizing the scene, toggling quadtree view)

### **`Scene` Singleton**

Central to Volpe Engine is the **Scene**. It maintains a list of top–level scene `Node` objects, an active camera, and a `QuadTree` for culling. Typical usage:

1.  **Populate** the scene by adding `DebugCube` (or other) nodes.
2.  **BuildQuadTree** once the scene is initialized.
3.  **Update** calls propagate transforms, recalculate bounding volumes, and cull using frustum checks.
4.  **Render** calls draw the visible nodes and optionally show quad tree boundaries.

----------

## Scene Management and Culling

1.  **Scene Graph**
    
    -   Each `Node` can have children, forming a hierarchy (parent transforms affect children) (ex. Solar-System Scene.)
    -   **update**: Each node’s local and world transformations are recalculated.
    -   **draw**: Each node issues draw calls for itself (e.g. a `DebugCube` can bind a shader and buffer, then call `glDrawArrays`).

2.  **Quad & Oct Tree**
    
    -   The engine organizes objects in **XZ** space using a **QuadTree** but switches to an **Octree** when handling 3D spatial partitioning for more complex scenes.
    -   **AABB and Sphere Bounding Volumes** are used for fast collision and culling checks.
    -   On **Scene::Update**, it extracts the camera frustum and queries the **QuadTree (for terrain & 2D elements)** or **Octree (for 3D spatial objects)**, gathering only the objects that intersect the frustum.
    -   This culling significantly reduces the rendering load for large scenes.
    -   **Dynamic light system** queries efficently using whichever tree you choose.

3.  **Camera Frustum Culling**
    
    -   The camera calculates **six planes** defining its viewing frustum.
    -   **Bounding Volumes (AABB & Sphere Tests)** are used to quickly discard objects that lie outside the view.
    -   Objects fully outside the frustum are skipped, while partially visible objects undergo finer checks.

4. **Dynamic Lighting Assignment**
    
    -   Each node dynamically assigns itself to the **nearest active lights** based on its position in the **QuadTree/Octree**.
    -   The engine updates the **list of affecting lights** per frame to ensure optimal real-time lighting performance.
    -   Lighting calculations are optimized by limiting the number of lights per node based on proximity and importance.
    

----------

## Text Rendering

Volpe Engine also has a **Text Renderer**, which draws 2D overlays (FPS counters, instructions, debug text) on top of the 3D scene. Key components:

1.  **`Font`**
    
    -   Loads a set of `.fnt` and `.tga` files (commonly exported by bmFont).
    -   Handles multiple font pages (e.g. `Arial0.fnt`, `Arial1.fnt`, etc.) for different sizes.
    -   Merges them into an **Array Texture** for efficient binding.
2.  **`TextBox`**
    
    -   Represents a rectangle containing text with dynamic scaling, word-wrapping, alignment, optional background, etc.
    -   Has auto‐truncation if the text overflows the box’s height, optionally adding ellipses (“…”).
    -   Renders as quads in an orthographic projection.
3.  **`TextRenderer`**
    
    -   Manages one or more TextBoxes.
    -   Maintains a simple shader (`2d.vsh`, `2d.fsh`) for drawing text in screen‐space.
    - Have the ability to edit colours, alignments, etc.

### Localization / `TextTable`

-   (Optional) The engine provides a CSV-based system for storing multiple languages or dynamic placeholders (like `{scoreValue}`).
-   `TextTable` can do runtime substitution so your UI text automatically updates if you switch languages or variables.

----------

## Building and Running

1.  **Clone or Download** the project.
2.  **Dependencies**: Make sure you have OpenGL, GLEW, GLFW, GLM, and your favorite compiler toolchain (e.g. MSVC, Clang, or GCC).
3.  **CMake**: Update your `CMakeLists.txt` or build system to include all source files (`Scene.cpp`, `QuadTree.cpp`, `TextRenderer.cpp`, etc.).
4.  **Compile**:

    ```bash
    cmake -B build
    cmake --build build
    
    ```
    
5.  **Run** the resulting executable. You should see a window with random cubes, a grid, and instructions on top of the screen.

----------

## Roadmap / Next Steps

-   **Physics / Collision**: Integrate a physics engine or broad‐phase approach for collisions.
-   **GUI**: Extend the text renderer to handle buttons, sliders, or advanced GUI elements.
-   **Networking**: Potentially add a netcode layer for multiplayer scenarios.
-   **Asset Management**: Create a formal pipeline for loading models, textures, and prefabs.


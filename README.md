
# Volpe Engine

Welcome to **Volpe Engine**, a lightweight game engine framework that demonstrates:

-   A **Scene Manager** for hierarchical scenes
-   A **Quad Tree** for culling optimizations
-   A **Camera** system (Orbit / First Person) for navigation
-   A **Text Rendering** system for overlays and UI

Below you’ll find an overview of how the engine is structured, how the core components interact.

----------
## Overview

Volpe Engine aims to provide a small but complete environment for real–time 3D rendering. Key features include:

-   **Scene Graph & Node**: Nodes can have parent–child relationships, maintain local transforms, and propagate updates.
-   **Camera**: Supports multiple camera styles—First Person and Orbit—through an abstract `Camera` interface.
-   **Quad Tree**: Divides space into quadrants to skip drawing objects that lie outside the camera frustum.
-   **Text Rendering**: A robust system for drawing text overlays, powered by an orthographic projection and array textures.
-   **Debug Rendering**: Quick lines, squares, and circles for visualizing bounding volumes or quad tree partitions.

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
    
    -   Each `Node` can have children, forming a hierarchy (parent transforms propagate to children).
    -   **update**: Each node’s local and world transformations are recalculated.
    -   **draw**: Each node issues draw calls for itself (e.g. a `DebugCube` can bind a shader and buffer, then call `glDrawArrays`).
2.  **Quad Tree**
    
    -   The engine organizes objects in **XZ** space, subdividing until a max level or until each node holds fewer than some threshold of objects (`MAX_OBJECTS`).
    -   On **Scene::Update** it extracts the camera frustum, queries the Quad Tree, and gathers only the objects that intersect this frustum.
    -   This culling significantly reduces the rendering load for large scenes.
3.  **Active Camera**
    
    -   The **Scene** holds a pointer to a `Camera` interface.
    -   Two built-in cameras:
        -   **OrbitCamera** (rotates around a target, good for debugging)
        -   **FirstPersonCamera** (typical WASD + mouse–look approach)
    -   The camera supplies a **view** matrix (`getViewMatrix()`) and a **projection** matrix (`getProjMatrix()`) for culling and rendering.

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



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
    - Cube
    - Sphere
    - Light node
    - Particle node
    - Effect node
    - BOIDS (WIP)

- Scenes
    - New Scene
    - Solar system (3 variations)
    - Random Scene (random cubes and spheres)
    - Particle scene (basic particle node with random textures)
    - Fire scene (Requirement & beautiful)
    - Rain Scene
    - Magic Spell Scene

Moving on to the Scene hierarchy:
Simply shows what nodes are in the scene and allows you to show grid, or rebuild the tree every update.
You can select the nodes and move them in the inspector

Inspector (VERY DENSE)

You will see the objects name, delete, transform and reparenting properties
For the particle node you can click load texture and write the name.png of any image THAT IS IN THE data/textures folder.
Very responsive and quick.

Continuous: Spawns N particles per second indefinitely.
Burst: Uses a list of times and spawn counts to quickly emit a chunk of particles at certain times. it loops if the duration is infinite.
Max Particles
The internal cap; the emitter never holds more than this many active particles at once.

Duration
How long the emitter should keep spawning (if set to a positive number). If you set it to -1, it can keep going forever.
Emission Rate

Stretch (X, Y)

A multiplier that can stretch each billboard quad horizontally or vertically, so particles might appear elongated.

Affectors

In the Inspector, you’ll see:

    A dropdown to pick from known affector types (e.g., Acceleration, FadeOverLife, ScaleOverLife, TowardsPoint, AwayFromPoint, DiePastAxis).
    An “Add Affector” button that adds the chosen affector to the emitter’s list.

For each existing affector, the inspector shows relevant fields:

    AccelerationAffector
        VelocityToAdd (3D vector)
        Local to Node? (checkbox)
        (If true, it transforms that acceleration by the emitter’s local rotation.)
    FadeOverLifeAffector
        startAlpha / endAlpha
        Over the particle’s lifetime, alpha linearly transitions from start to end.
    ScaleOverLifeAffector
        startScale / endScale
        Over the particle’s lifetime, size transitions from start to end (multiplied by the particle’s initial size).
    TowardsPointAffector / AwayFromPointAffector
        A 3D position (target or center)
        A strength (float) that sets how forcefully the particle is pushed or pulled.
        “Local to Node?” for whether that point is in local or world coords.
    DiePastAxisAffector
        Axis: X, Y, or Z
        Threshold: the coordinate value at which the particle is killed
        GreaterThan: if true, kill if position > threshold; if false, kill if position < threshold.


For the BOIDS

Max Boids
The largest number of boids allowed in this node.

World Bounds
The region in which the boids wrap around or remain.

Wrap Around
If checked, boids wrapping from one edge of that region to the other.

Alignment / Cohesion / Separation
Toggles for the three flocking behaviors:

    Alignment: boids steer to match velocity of neighbors.
    Cohesion: boids steer toward the group center.
    Separation: boids steer away if too close.

Current Boids
How many boids are actively stored in the node.

Spawn Count
How many to spawn (or respawn) at once.

Respawn Boids
Clears existing boids and spawns fresh ones.

Particles per second for continuous mode.


# WHAT THE CODE DOES:


## ParticleNode (Emitter)

Represents an emitter in the scene. It holds an optimized array of active particles, and spawns new ones according to rules you choose:

    Emission settings (how many particles per second, maximum particles allowed, for how long to emit).
    Shapes (point, sphere, box, cone, donut) which define where exactly in space each new particle is placed.
    Lifetime ranges (minimum and maximum possible lifetime for each particle).
    Size, rotation, and alpha ranges to randomize each new particle’s look.
    Gradients over time (for color fading and cool effects).
    

Under the hood, each ParticleNode runs a spawn-update-render loop:

    Spawning: Checks if it needs new particles this frame, generates them, randomizing their initial properties within the given ranges.
    Updating: For each existing particle:
        Increments its age; if it has outlived its lifetime, it is removed.
        Applies affectors, which might alter velocity, color, scale, etc.
        Moves the particle by updating position or rotation.
    Rendering: Sorts the particles (for correct blending) and draws each one as a billboard quad, meaning it always faces the camera. This is done by calculating “right” and “up” vectors from the camera to orient the particles. The system supports normal alpha blending or additive/glow-style blending.

### Particle

This is just a record of one particle’s position, velocity, color, size, lifetime, etc. These particles come and go according to the emitter’s rules.
### Affector

An affector is a plug-in module that modifies each particle over time. Common examples:

    Constant acceleration (like gravity).
    Fade-out by reducing alpha as it ages.
    Scale from small to large from birth to death.
    Towards or Away from a point (attract or repel).
    DiePastAxis (kill a particle once it crosses a certain boundary).

Each affector runs its logic on every particle during the update phase. It can be combined with others for complex behaviors. Under the hood, each affector simply changes particle fields like velocity, alpha, or position.
## Workflow and Configuration

To use this system:

    Create an emitter node (the system’s primary interface).
    Set emitter parameters—such as how quickly to spawn particles, what shapes they appear in, and how big or colorful they are at birth.
    Attach any number of affectors to shape their motion or color over time.
    Add the emitter node into your scene so it updates and renders.
    Start the emitter, or “play” it. Depending on settings, it will either continuously emit new particles (continuous mode) or spawn large bursts at specific times (burst mode).

Beneath the surface, your emitter’s transformation (position, rotation, scale) can be applied to new or existing particles if you choose local space. If you keep it in world space, the emitter’s own movement only affects future particles.
## Under the Hood Mechanics

    Spawning
        A small accumulator tracks how many particles should be spawned based on the emission rate and the time step.
        Once enough “spawn budget” accumulates, the system creates new particles in the chosen shape.

    Particle Array
        Maintains a limit (maxParticles) to prevent excessive growth.
        The system reuses the array, removing “dead” particles that exceed their lifetime.

    Sorting & Billboard Quads
        Just before drawing, the emitter sorts particles by distance to the camera. This ensures the correct blending order so that transparent particles look correct.
        A small chunk of vertex data is built for each particle: two triangles forming a quad, oriented to face the camera.

    Affectors
        Each affector’s Apply method is called per particle. They have direct access to the particle fields (position, velocity, color). They might combine the emitter’s world matrix to handle local transformations.

    Rendering
        Uses a custom or shared material that can reference a texture (like a smoke texture, flame texture, etc.).
        If “glow” is on, it switches to additive blending, making each particle’s color add up brightly.
        If “glow” is off, it uses alpha blending so particles fade with a typical see-through effect.

## Loading and Saving

The system supports loading and saving configurations in YAML. An emitter or entire “effect” (with multiple emitters) can be stored in a .yaml file describing:

    Emitter name, shape, rates, color keys, etc.
    Attached affectors (with their relevant fields).
    Optional texture paths.

When you load such YAML files at runtime, the system reconstructs the emitter or effect. This reduces the need to hardcode particle setups in your project.

## EffectNode for Multiple Emitters

Sometimes you need multiple particle emitters that work together—like an explosion that has sparks, smoke, and shockwave rings. The EffectNode:

    Collects child ParticleNodes under one parent.
    Let’s you control them all at once (Play, Stop, Pause, etc.).
    Can load from an .effect.yaml file that has a list of child emitters with offsets.

Under the hood, EffectNode is just a scene Node with children that happen to be ParticleNodes. Its “Play” method simply calls “Play” on every child emitter. You can also transform the entire effect as a group.





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


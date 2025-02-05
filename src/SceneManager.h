#pragma once
#include <vector>
#include "Node.h"
#include "DebugCube.h"
#include "Light.h"
#include "../samplefw/OrbitCamera.h"
#include "../samplefw/Grid3D.h"

class SceneManager
{
private:
    volpe::App* m_pApp = nullptr;
    Node*              m_root;       // top-level “scene root”
    std::vector<Light> m_lights;     // all lights in the scene
    std::vector<Node*> m_allNodes;   // convenience to keep track of every Node
    OrbitCamera* m_pOrbitCam = nullptr;
    Grid3D* m_pGrid = nullptr;

    // Helper to pick which lights (1 or 2 or up to N) are relevant to a given node
    // This example returns up to 2 closest lights for demonstration
    std::vector<Light> pickLightsForNode(const Node* node);

public:
    SceneManager(volpe::App* pApp);
    ~SceneManager();

    void initScene();           // set up cubes and lights
    void update(float dt);      // animate or move stuff each frame
    void draw(int width, int height);

    // In a more advanced engine, you'd set up a multi-light shader. 
    // For demonstration, we do a “2-lights-per-object” approach or a “single-light” approach.
    void drawSingleLight(const glm::mat4& viewProj);
    void drawTwoLightsPerObject(const glm::mat4& viewProj);
};

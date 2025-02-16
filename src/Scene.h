#pragma once
#include <vector>
#include "Node.h"
#include "DebugCube.h"
#include "DebugSphere.h"
#include "QuadTree.h"
#include "OctTree.h"
#include "Light.h"
#include "../samplefw/Camera.h"
#include "../samplefw/Grid3D.h"
#include "../samplefw/Sphere.h"

#include "DebugRender.h"
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <chrono> 
#include <random>

#include "../volpe/TextRendering/TextRenderer.h"
#include "../volpe/TextRendering/TextBox.h"

using namespace std;
class Scene
{
private:

    Scene();
    ~Scene();
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

    vector<Node*> m_nodes;
    vector<Node*> m_nodesToRender;
    vector<Light> m_lights; 
    Camera* m_activeCamera;
    Camera* m_movemenetCamera;
    QuadTree* m_quadTree;
    OctTree* m_octTree;
    bool m_ShowDebug = true;

    volpe::Program* m_unlitProgram = nullptr;    // "Unlit3d.vsh" / "Unlit3d.fsh"
    volpe::Program* m_pointProgram = nullptr;    // "PointLight.vsh" / "PointLight.fsh"


    Grid3D* m_pGrid = nullptr;

    TextRenderer* m_textRenderer = nullptr;
    TextBox* textBox = nullptr;
    TextBox* debugTextBox = nullptr;

    vector<Light> pickLightsForNode(const Node* node);

    Frustum m_debugFrustum;
    bool m_useDebugFrustum = false;
    bool m_useQuadTreeOrOct = true; //true quadtree, false octree

    Camera* m_debugCamera;

    //STATISTICSSS

    // Track the last time (in ms) it took to build the QuadTree.
    float m_lastQuadTreeBuildTimeMs = 0.0f;

    // We'll keep exponential-moving-average for each update time:
    float m_avgCreation             = 0.0f;
    float m_avgCameraUpdateMs       = 0.0f;
    float m_avgNodeUpdateMs         = 0.0f;
    float m_avgBoundingVolumeMs     = 0.0f;
    float m_avgFrustumExtractMs     = 0.0f;
    float m_avgQuadTreeQueryMs      = 0.0f;
    float m_avgLightQuery           = 0.0f;

    //BOUNDS
    float m_bounds = 10.0f;

    bool reDebug = false;
    
    const float m_smoothAlpha = 0.0001f;

public:

    static Scene& Instance();
    
    void AddNode(Node* node);

    void RandomInitScene(int amount);

    void SetActiveCamera(Camera* cam) { m_activeCamera = cam; }
    void BuildQuadTree();
    void BuildOctTree();
    void Update(float dt, int screenWidth, int screenHeight);
    void Render(int screenWidth, int screenHeight);

    void AddLight(Light l);
    void InitLights();         // rando lgihts
    void UpdateLighting();     // choose which cubes get which program
    void MoveLights();
    

    void setTextBoxPos(float x, float y)
    {
        textBox->SetPosition(x,y);
    }

    void createGrid(int bounds) {m_pGrid = new Grid3D(bounds);}

    void SetBounds(int b) { m_bounds = b; createGrid(b);}

    void ShowDebugText();
    void DebugDrawFrustum(const Frustum& frustum);

    void ToggleQuadTreeRender() { m_ShowDebug = !m_ShowDebug; }
    void ToggleUseDebugFrustum(Camera* c);

    bool getWhichTree() {return m_useQuadTreeOrOct;} //true quadtree, false octree

    //TEMPORARYYYY
    vector<Node*> GetNodes() { return m_nodes; }

    void Clear();

    bool debugLocation = true;
    
    void drawSingleLight(const glm::mat4& viewProj);
    void drawTwoLightsPerObject(const glm::mat4& viewProj);
};

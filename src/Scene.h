#pragma once
#include <vector>
#include "Node.h"
#include "DebugCube.h"
#include "DebugSphere.h"
#include "QuadTree.h"
#include "OctTree.h"
#include "LightNode.h"
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
using namespace glm;
class Scene
{
private:

    Scene();
    ~Scene();
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

    vector<Node*> m_nodes;
    vector<Node*> m_nodesToRender;
    // vector<Light> m_lights; 
    Camera* m_activeCamera;
    Camera* m_movemenetCamera;
    QuadTree* m_quadTree;
    OctTree* m_octTree;
    bool m_ShowDebug = true;
    bool m_ShowBoundingVolumes = false;
    
    volpe::Material* m_matUnlit     = nullptr; // For zero-lights  "Unlit3d.vsh" / "Unlit3d.fsh"
    volpe::Material* m_matPoint     = nullptr; // For 1+ lights    "PointLight.vsh" / "PointLight.fsh"


    Grid3D* m_pGrid = nullptr;

    TextRenderer* m_textRenderer = nullptr;
    TextBox* textBox = nullptr;
    TextBox* debugTextBox = nullptr;

    // vector<Light> pickLightsForNode(const Node* node);

    Frustum m_debugFrustum;
    bool m_useDebugFrustum = false;
    bool m_useQuadTreeOrOct = true; //true quadtree, false octree

    Camera* m_debugCamera;

    //STATISTICSSS

    float m_lastQuadTreeBuildTimeMs = 0.0f;

    float m_avgCreation             = 0.0f;
    float m_avgCameraUpdateMs       = 0.0f;
    float m_avgNodeUpdateMs         = 0.0f;
    float m_avgBoundingVolumeMs     = 0.0f;
    float m_avgFrustumExtractMs     = 0.0f;
    float m_avgQuadTreeQueryMs      = 0.0f;
    float m_avgLightQuery           = 0.0f;

    //BOUNDS
    float m_bounds = 10.0f;

    bool shownLights = false;
    bool reDebug = false;
    
    const float m_smoothAlpha = 0.01f;

    float m_lastKnownFps = 0.0f;

public:

    bool showGrid = true;

    #pragma region GETTERS FOR IMGUI

    float getFPS() const { return m_lastKnownFps; }  // m_lastKnownFps should be updated in Update()
    bool getShowDebug() const { return m_ShowDebug; }
    bool getShowBoundingVolumes() const { return m_ShowBoundingVolumes; }
    bool getUseDebugFrustum() const { return m_useDebugFrustum; }
    std::string getActiveTreeName() const { return m_useQuadTreeOrOct ? "Quad" : "Oct"; }
    float getSceneCreationTime() const { return m_avgCreation; }
    float getTreeBuildTime() const { return m_lastQuadTreeBuildTimeMs; }
    float getAvgCameraUpdateMs() const { return m_avgCameraUpdateMs; }
    float getAvgNodeUpdateMs() const { return m_avgNodeUpdateMs; }
    float getAvgBoundingVolumeMs() const { return m_avgBoundingVolumeMs; }
    float getAvgFrustumExtractMs() const { return m_avgFrustumExtractMs; }
    float getAvgQuadTreeQueryMs() const { return m_avgQuadTreeQueryMs; }
    float getAvgLightQuery() const { return m_avgLightQuery; }
    size_t getNodesAffectedByLight() const
    {
        size_t count = 0;
        for (const auto* node : m_nodes)
        {
            if (node && !node->m_affectingLights.empty())
                count++;
        }
        return count;
    }

    // const std::vector<Light>& GetLights() const { return m_lights; }
    const std::vector<Node*>& GetNodes() const { return m_nodes; }
    const std::vector<Node*>& GetNodesToRender() const { return m_nodesToRender; }

    #pragma endregion


    static Scene& Instance();
    void AddLight(const glm::vec3& position, const glm::vec3& color, float intensity, float radius);
    void AddNode(Node* node);
    void RemoveNode(Node* node);

    void RandomInitScene(int amount);

    void SetActiveCamera(Camera* cam) { m_activeCamera = cam; }
    Camera* GetActiveCamera() { return m_activeCamera; }
    void BuildQuadTree();
    void BuildOctTree();
    void Update(float dt, int screenWidth, int screenHeight);
    void Render(int screenWidth, int screenHeight);

    void InitLights();         // rando lgihts
    void UpdateLighting();     // choose which cubes get which program

    void setTextBoxPos(float x, float y)
    {
        textBox->SetPosition(x,y);
    }

    void createGrid(int bounds) {m_pGrid = new Grid3D(bounds);}

    void SetBounds(int b, bool x= false) { m_bounds = b; if(x)createGrid(b);}

    void ShowDebugText();
    void DebugDrawFrustum(const Frustum& frustum);

    void ToggleBoundingVolumeDebug() { m_ShowBoundingVolumes = !m_ShowBoundingVolumes; }
    void ToggleQuadTreeRender() { m_ShowDebug = !m_ShowDebug; DebugRender::Instance().ClearLayer("BoundingVolumes");}
    void ToggleUseDebugFrustum(Camera* c);

    bool getWhichTree() {return m_useQuadTreeOrOct;} //true quadtree, false octree
    void ReBuildTree(); 

    //TEMPORARYYYY
    vector<Node*> GetNodes() { return m_nodes; }
    vector<Node*> GetNodesToRender() { return m_nodesToRender; }

    void Clear();

    bool debugLocation = true;
    
    void drawSingleLight(const glm::mat4& viewProj);
    void drawTwoLightsPerObject(const glm::mat4& viewProj);
};

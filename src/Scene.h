#pragma once
#include <vector>
#include "Node.h"
#include "DebugCube.h"
#include "QuadTree.h"
#include "Light.h"
#include "../samplefw/Camera.h"
#include "../samplefw/Grid3D.h"

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
    QuadTree* m_quadTree;
    bool m_renderQuadTree = true;

    Grid3D* m_pGrid = nullptr;

    TextRenderer* m_textRenderer = nullptr;
    TextBox* textBox = nullptr;

    vector<Light> pickLightsForNode(const Node* node);

    Frustum m_debugFrustum;
    bool m_useDebugFrustum = false;

    //STATISTICSSS

    // Track the last time (in ms) it took to build the QuadTree.
    float m_lastQuadTreeBuildTimeMs = 0.0f;

    // We'll keep exponential-moving-average for each update time:
    float m_avgCameraUpdateMs       = 0.0f;
    float m_avgNodeUpdateMs         = 0.0f;
    float m_avgBoundingVolumeMs     = 0.0f;
    float m_avgFrustumExtractMs     = 0.0f;
    float m_avgQuadTreeQueryMs      = 0.0f;
    
    // Used for the smoothing factor. 
    // e.g., alpha=0.1 means 90% old + 10% new every frame
    const float m_smoothAlpha = 0.000001f;

public:

    static Scene& Instance();
    
    void AddNode(Node* node);

    void RandomInitScene(int amount);

    void SetActiveCamera(Camera* cam) { m_activeCamera = cam; }
    void BuildQuadTree();
    void Update(float dt, int screenWidth, int screenHeight);
    void Render(int screenWidth, int screenHeight);

    void setTextBoxPos(float x, float y)
    {
        textBox->SetPosition(x,y);
    }

    void ShowDebugText();
    void DebugDrawFrustum(const Frustum& frustum);

    void ToggleQuadTreeRender() { m_renderQuadTree = !m_renderQuadTree; }
    void ToggleUseDebugFrustum();

    //TEMPORARYYYY
    vector<Node*> GetNodes() { return m_nodes; }

    void Clear();

    
    void drawSingleLight(const glm::mat4& viewProj);
    void drawTwoLightsPerObject(const glm::mat4& viewProj);
};

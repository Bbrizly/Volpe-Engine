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

public:

    static Scene& Instance();
    
    void AddNode(Node* node);

    void RandomInitScene();

    void SetActiveCamera(Camera* cam) { m_activeCamera = cam; }
    void BuildQuadTree();
    void Update(float dt, int screenWidth, int screenHeight);
    void Render(int screenWidth, int screenHeight);

    void setTextBoxPos(float x, float y)
    {
        textBox->SetPosition(x,y);
    }

    void ShowDebugText();

    void ToggleQuadTreeRender() { m_renderQuadTree = !m_renderQuadTree; }

    //TEMPORARYYYY
    vector<Node*> GetNodes() { return m_nodes; }

    void Clear();

    
    void drawSingleLight(const glm::mat4& viewProj);
    void drawTwoLightsPerObject(const glm::mat4& viewProj);
};

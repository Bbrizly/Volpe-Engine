#pragma once
#include <vector>
#include "Node.h"
#include "DebugCube.h"
#include "Light.h"
#include "../samplefw/Camera.h"
#include "../samplefw/Grid3D.h"
using namespace std;
class Scene
{
private:

    Scene();
    ~Scene();
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

    vector<Node*> m_nodes;
    vector<Node*> m_objectsToRender;
    vector<Light> m_lights; 
    Camera* m_activeCamera;

    Grid3D* m_pGrid = nullptr;

    vector<Light> pickLightsForNode(const Node* node);

public:

    static Scene& Instance();
    
    void RandomInitScene();

    void AddNode(Node* node);
    void SetActiveCamera(Camera* cam);
    void BuildQuadTree();
    void Update(float dt);//, int screenWidth, int screenHeight);
    void Render(int screenWidth, int screenHeight);

    void ToggleQuadTreeRender();

    //TEMPORARYYYY
    vector<Node*> GetNodes() { return m_nodes; }

    
    void drawSingleLight(const glm::mat4& viewProj);
    void drawTwoLightsPerObject(const glm::mat4& viewProj);
};

#include "Scene.h"
#include "DebugRender.h"
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <random>

// Helper function to extract a frustum from a clip matrix.
static Frustum ExtractFrustum(const glm::mat4& clip) {
    Frustum frustum;
    frustum.planes[0] = glm::vec4(clip[0][3] + clip[0][0],
                                  clip[1][3] + clip[1][0],
                                  clip[2][3] + clip[2][0],
                                  clip[3][3] + clip[3][0]);
    frustum.planes[1] = glm::vec4(clip[0][3] - clip[0][0],
                                  clip[1][3] - clip[1][0],
                                  clip[2][3] - clip[2][0],
                                  clip[3][3] - clip[3][0]);
    frustum.planes[2] = glm::vec4(clip[0][3] + clip[0][1],
                                  clip[1][3] + clip[1][1],
                                  clip[2][3] + clip[2][1],
                                  clip[3][3] + clip[3][1]);
    frustum.planes[3] = glm::vec4(clip[0][3] - clip[0][1],
                                  clip[1][3] - clip[1][1],
                                  clip[2][3] - clip[2][1],
                                  clip[3][3] - clip[3][1]);
    frustum.planes[4] = glm::vec4(clip[0][3] + clip[0][2],
                                  clip[1][3] + clip[1][2],
                                  clip[2][3] + clip[2][2],
                                  clip[3][3] + clip[3][2]);
    frustum.planes[5] = glm::vec4(clip[0][3] - clip[0][2],
                                  clip[1][3] - clip[1][2],
                                  clip[2][3] - clip[2][2],
                                  clip[3][3] - clip[3][2]);
    for (int i = 0; i < 6; ++i) {
        float length = glm::length(glm::vec3(frustum.planes[i]));
        frustum.planes[i] /= length;
    }
    return frustum;
}

Scene& Scene::Instance() {
    static Scene instance;
    return instance;
}

Scene::Scene() : m_activeCamera(nullptr), m_quadTree(nullptr), m_renderQuadTree(false) {}

Scene::~Scene() {
    // for (auto node : m_nodes)
    //     delete node;
    m_nodes.clear();
    if(m_quadTree) {
        delete m_quadTree;
        m_quadTree = nullptr;
    }
}

void Scene::AddNode(Node* node) {
    if(node)
        m_nodes.push_back(node);
}


void Scene::BuildQuadTree() {
    AABB2D sceneBounds;
    sceneBounds.min = glm::vec2(-30.0f, -30.0f);
    sceneBounds.max = glm::vec2(30.0f, 30.0f);
    if(m_quadTree) {
        delete m_quadTree;
    }
    m_quadTree = new QuadTree(sceneBounds);

    for(auto node : m_nodes)
    {
        m_quadTree->Insert(node);
    }
}

void Scene::RandomInitScene()
{
    m_nodes.clear();
    m_pGrid = new Grid3D(30);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> distPos(-10.0f, 10.0f); // Random positions
    std::uniform_int_distribution<int> distChildren(0, 3); // Random number of children per cube

    // DebugCube* parentCube = new DebugCube("parentCube");
    // parentCube->setTransform(glm::translate(glm::mat4(1.0f), glm::vec3(distPos(gen), 0, distPos(gen))));
    // AddNode(parentCube);

    // std::vector<Node*> allCubes;
    // allCubes.push_back(parentCube);

    // Generate child cubes with random positions and parent-child relationships
    for (int i = 1; i <= 200; ++i)
    {
        DebugCube* newCube = new DebugCube("cube_" + std::to_string(i));
        glm::vec3 randomPos(distPos(gen), 0.0f, distPos(gen));
        newCube->setTransform(glm::translate(glm::mat4(1.0f), randomPos));

        // Randomly parent from existing cubes
        // int parentIndex = std::uniform_int_distribution<int>(0, allCubes.size() - 1)(gen);
        // allCubes[parentIndex]->addChild(newCube);

        AddNode(newCube);
        // return;5
        // allCubes.push_back(newCube);
    }

    m_lights.push_back( Light(glm::vec3(0, 5, 5),  glm::vec3(1,1,1), 1.0f) );
    m_lights.push_back( Light(glm::vec3(5, 5, 0),  glm::vec3(1,0,0), 1.0f) );
    m_lights.push_back( Light(glm::vec3(-5, 5, 0), glm::vec3(0,1,0), 1.0f) );
    m_lights.push_back( Light(glm::vec3(0, 5, -5), glm::vec3(0,0,1), 1.0f) );

}

void Scene::ShowDebugText()
{
    m_textRenderer = new TextRenderer();
    m_textRenderer->init();
    Font* fontArial   = m_textRenderer->createFont("Arial");
    TextBox* textBoc   = m_textRenderer->createTextBox(fontArial,"Press Q to visualize Quad Tree\n Press R To randomly Generate Nodes", 640.0f - 400, 360.0f, 400, 200);
    textBoc->SetColor(0,0,0,255);
    textBoc->SetVisualization(false); //remove bg box
    textBoc->SetAlignment(2);

    textBox  = m_textRenderer->createTextBox(fontArial,"FPS, Each Process's MS, Other important values", -640.0f, 360.0f, 200, 400);
    textBox->SetColor(0, 0, 0, 255);
    textBox->SetVisualization(false); //remove bg box
    m_textRenderer->setTextBox(textBoc);
    m_textRenderer->setTextBox(textBox);
}

void Scene::Update(float dt, int screenWidth, int screenHeight) {
    #pragma region  Get FPS
    static int frameCounter = 0;
    static float accumulatedTime = 0.0f;
    
    m_activeCamera->update(dt);

    // Only update FPS text every 10 frames
    frameCounter++;
    accumulatedTime += dt;
    if (frameCounter >= 60) { 
        float avgFps = frameCounter / accumulatedTime;  // Calculate smoothed FPS
        textBox->SetText("FPS: " + std::to_string((int)avgFps));
        
        frameCounter = 0;
        accumulatedTime = 0.0f;
    }

    #pragma endregion

    m_activeCamera->update(dt);

    // // Update all top-level nodes.
    // for (auto node : m_nodes)
    //     node->update(dt);
    // for (auto node : m_nodes)
    //     node->updateBoundingVolume();

    // // Get frustum from active camera
    // glm::mat4 proj = m_activeCamera->getProjMatrix(screenWidth, screenHeight);
    // glm::mat4 view = m_activeCamera->getViewMatrix();
    // glm::mat4 clip = proj * view;
    // Frustum frustum = ExtractFrustum(clip);
    
    // m_objectsToRender.clear();
    // if (m_activeCamera && m_quadTree) {
    //     m_quadTree->Query(frustum, m_objectsToRender);
    // } else {
        m_nodesToRender = m_nodes;
    //     // std::cout<<"WHAT?";
    // }
}
void Scene::Clear()
{
    m_nodes.clear();
    m_nodesToRender.clear();
    DebugRender::Instance().Clear();
}

void Scene::Render(int screenWidth, int screenHeight) {

    if (!m_activeCamera)
        return;
    glm::mat4 proj = m_activeCamera->getProjMatrix(screenWidth, screenHeight);
    glm::mat4 view = m_activeCamera->getViewMatrix();
    
    m_pGrid->render(view,proj);

    //COMMENT OUT TO DEBUG QUADTREE LOGICCCC
    //MAKE SURE TO ADD DEBUG CIRCLE TO EVERY NODE BEFOREHAND
    for (auto node : m_nodesToRender)
        // node->draw(proj, view);
        
    if(m_renderQuadTree)
        m_quadTree->Render(proj, view);

    // DebugRender::Instance().Render(proj,view);
    
    //Render text
    if(m_textRenderer && textBox)
    {
        glDisable(GL_DEPTH_TEST);
        m_textRenderer->render(proj,view);
        glEnable(GL_DEPTH_TEST);
    }
}


#include "Scene.h"
#include "DebugRender.h"
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <chrono> 
#include <random>

using namespace std;

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

void Scene::DebugDrawFrustum(const Frustum& frustum)
{
    // For demonstration, let’s just show each plane as a square.
    // Each plane = (A,B,C,D) in Ax+By+Cz+D=0, normal = (A,B,C), distance = -D
    // We'll pick an arbitrary “square size” in that plane.

    float planeSize = 5.0f; // half-extent

    for (int i = 0; i < 6; ++i)
    {
        const glm::vec4& p = frustum.planes[i];
        glm::vec3 normal(p.x, p.y, p.z);

        // Distance from origin
        float dist = p.w;  // recall plane eq is A*x + B*y + C*z + D = 0
                           // if plane is normalized, w = D => distance is -D
        // For clarity, let's invert sign if we used D=+someValue:
        // float d = -dist; 
        float d = -dist;

        // Plane center in world space is normal * distance
        glm::vec3 planeCenter = normal * d;

        // Now we need 2 perpendicular vectors in the plane
        // Let's pick an arbitrary up vector to cross with normal:
        glm::vec3 up(0,1,0);
        if (fabs(glm::dot(up, normal)) > 0.99f) {
            up = glm::vec3(1,0,0); // if normal is near vertical, choose a different "up"
        }
        glm::vec3 right = glm::normalize(glm::cross(normal, up));
        glm::vec3 planeUp = glm::normalize(glm::cross(right, normal));

        // Compute 4 corners around planeCenter
        glm::vec3 corner1 = planeCenter + ( right*planeSize) + ( planeUp*planeSize);
        glm::vec3 corner2 = planeCenter + (-right*planeSize) + ( planeUp*planeSize);
        glm::vec3 corner3 = planeCenter + (-right*planeSize) + (-planeUp*planeSize);
        glm::vec3 corner4 = planeCenter + ( right*planeSize) + (-planeUp*planeSize);

        // Use DebugRender to draw lines around that square
        DebugRender::Instance().DrawLine(corner1, corner2, glm::vec3(1,0,0));
        DebugRender::Instance().DrawLine(corner2, corner3, glm::vec3(0,1,0));
        DebugRender::Instance().DrawLine(corner3, corner4, glm::vec3(0,0,1));
        DebugRender::Instance().DrawLine(corner4, corner1, glm::vec3(1,1,0));
    }
}

void Scene::ToggleUseDebugFrustum()
{
    //Left, Right, Bottom, Top, Near, Far
    m_debugFrustum.planes[0] = glm::vec4( 1, 0, 0,  10.0f);  //   x + 0*y + 0*z + 10 = 0
    m_debugFrustum.planes[1] = glm::vec4(-1, 0, 0,  10.0f);  //  -x + 0*y + 0*z + 10 = 0
    m_debugFrustum.planes[2] = glm::vec4( 0, 1, 0,   5.0f);  //   0*x +  y + 0*z +  5 = 0
    m_debugFrustum.planes[3] = glm::vec4( 0,-1, 0,   5.0f);  //   0*x + -y + 0*z +  5 = 0
    m_debugFrustum.planes[4] = glm::vec4( 0, 0, 1,   1.0f);  //   0*x + 0*y +  z +  1 = 0
    m_debugFrustum.planes[5] = glm::vec4( 0, 0,-1, 100.0f);  //   0*x + 0*y + -z +100 = 0

    // Make sure each plane is normalized
    for (int i = 0; i < 6; ++i) {
        float length = glm::length(glm::vec3(m_debugFrustum.planes[i]));
        m_debugFrustum.planes[i] /= length;
    }

    m_useDebugFrustum  = !m_useDebugFrustum;

    // DebugRender::Instance().Clear();
}

void Scene::BuildQuadTree() {
    using namespace chrono;
    auto t0 = high_resolution_clock::now();

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
    
    auto t1 = high_resolution_clock::now();
    float buildTimeMs = duration<float, milli>(t1 - t0).count();
    m_lastQuadTreeBuildTimeMs = buildTimeMs;
}

void Scene::RandomInitScene(int amount)
{
    m_nodes.clear();
    m_pGrid = new Grid3D(30);

    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<float> distPos(-30.0f, 30.0f);

    for (int i = 1; i <= amount; ++i)
    {
        DebugCube* newCube = new DebugCube("cube_" + to_string(i));
        glm::vec3 randomPos(distPos(gen), 0.0f, distPos(gen));
        newCube->setTransform(glm::translate(glm::mat4(1.0f), randomPos));

        AddNode(newCube);
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
    TextBox* textBoc   = m_textRenderer->createTextBox(fontArial,"Press Q to visualize Quad Tree.\nPress R To randomly Generate Nodes.\nPress F to toggle Debug Frustum.", 640.0f - 400, 360.0f, 400, 200);
    textBoc->SetColor(0,0,0,255);
    textBoc->SetVisualization(false); //remove bg box
    textBoc->SetAlignment(2);

    textBox  = m_textRenderer->createTextBox(fontArial,"FPS, Each Process's MS, Other important values", -640.0f, 360.0f, 350, 720);
    textBox->SetColor(0, 0, 0, 255);
    textBox->SetVisualization(false); //remove bg box
    m_textRenderer->setTextBox(textBoc);
    m_textRenderer->setTextBox(textBox);
}



void Scene::Update(float dt, int screenWidth, int screenHeight) {
    using namespace chrono;
    auto EMA = [this](float oldValue, float newValue) {
        return oldValue * (1.0f - m_smoothAlpha) + newValue * m_smoothAlpha;
    };

    
    if(m_useDebugFrustum)
        DebugDrawFrustum(m_debugFrustum);

    /************************************
     *FPS
     ************************************/
    static int   frameCounter = 0;
    static float accumulatedTime = 0.0f;
    static float lastKnownFps   = 0.0f;

    frameCounter++;
    accumulatedTime += dt;
    if (frameCounter >= 60) {
        lastKnownFps = frameCounter / accumulatedTime;  // smoothed FPS
        frameCounter = 0;
        accumulatedTime = 0.0f;
    }

    /*********************************************
     *runtime in ms
     *********************************************/

    // Grab current time at start
    auto t0 = high_resolution_clock::now();

    // ========== Camera Update ==========
    m_activeCamera->update(dt);

    auto t1 = high_resolution_clock::now();
    float cameraUpdateMS = duration<float, milli>(t1 - t0).count();

    // ========== Node Updates ==========
    t0 = high_resolution_clock::now();
    for (auto node : m_nodes) {
        node->update(dt);
    }
    t1 = high_resolution_clock::now();
    float nodeUpdateMS = duration<float, milli>(t1 - t0).count();

    // ========== Bounding Volume Updates ==========
    t0 = high_resolution_clock::now();
    for (auto node : m_nodes) {
        node->updateBoundingVolume();
    }
    t1 = high_resolution_clock::now();
    float boundingVolumeMS = duration<float, milli>(t1 - t0).count();

    // ========== Frustum Extraction ==========
    t0 = high_resolution_clock::now();
    glm::mat4 proj = m_activeCamera->getProjMatrix(screenWidth, screenHeight);
    glm::mat4 view = m_activeCamera->getViewMatrix();
    glm::mat4 clip = proj * view;
    Frustum realFrustum = ExtractFrustum(clip);
    Frustum frustumToUse = m_useDebugFrustum ? m_debugFrustum : realFrustum;
    t1 = high_resolution_clock::now();
    float frustumExtractMS = duration<float, milli>(t1 - t0).count();

    // ========== QuadTree Query ==========
    t0 = high_resolution_clock::now();
    m_nodesToRender.clear();
    if (m_activeCamera && m_quadTree) {
        m_quadTree->Query(frustumToUse, m_nodesToRender);
    } else {
        m_nodesToRender = m_nodes;
    }
    t1 = high_resolution_clock::now();
    float quadTreeQueryMS = duration<float, milli>(t1 - t0).count();

    m_avgCameraUpdateMs   = EMA(m_avgCameraUpdateMs,   cameraUpdateMS);
    m_avgNodeUpdateMs     = EMA(m_avgNodeUpdateMs,     nodeUpdateMS);
    m_avgBoundingVolumeMs = EMA(m_avgBoundingVolumeMs, boundingVolumeMS);
    m_avgFrustumExtractMs = EMA(m_avgFrustumExtractMs, frustumExtractMS);
    m_avgQuadTreeQueryMs  = EMA(m_avgQuadTreeQueryMs,  quadTreeQueryMS);

    string info;
    info += "FPS: " + to_string((int)lastKnownFps) + "\n";
    info += "\n";
    info += "QuadTree Render: "  + string(m_renderQuadTree   ? "ON" : "OFF") + "\n";
    info += "Debug Frustum  : "  + string(m_useDebugFrustum  ? "ON" : "OFF") + "\n";
    info += "QuadTree Build (last): " + to_string(m_lastQuadTreeBuildTimeMs) + " ms\n";
    info += "Cubes Visible: " + to_string(m_nodesToRender.size()) + "\n";
    info += "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
    info += "Camera Update      : " + to_string(cameraUpdateMS)    + " ms\n";
    info += "Nodes Update       : " + to_string(nodeUpdateMS)       + " ms\n";
    info += "Bounding Vol Update: " + to_string(boundingVolumeMS)   + " ms\n";
    info += "Extract Frustum    : " + to_string(frustumExtractMS)   + " ms\n";
    info += "QuadTree Query     : " + to_string(quadTreeQueryMS)    + " ms\n";

    // Now update your textBox
    // (You can set this every frame or only every 60 frames; up to you.)
    if (textBox) {
        textBox->SetText(info);
    }
    /*
    
    #pragma region  Get FPS
    static int frameCounter = 0;
    static float accumulatedTime = 0.0f;
    
    m_activeCamera->update(dt);

    // Only update FPS text every 10 frames
    frameCounter++;
    accumulatedTime += dt;
    if (frameCounter >= 60) { 
        float avgFps = frameCounter / accumulatedTime;  // Calculate smoothed FPS
        textBox->SetText("FPS: " + to_string((int)avgFps));
        
        frameCounter = 0;
        accumulatedTime = 0.0f;
    }

    #pragma endregion

    m_activeCamera->update(dt);

    // Update all top-level nodes.
    for (auto node : m_nodes)
        node->update(dt);
    for (auto node : m_nodes)
        node->updateBoundingVolume();

    // Get frustum from active camera
    glm::mat4 proj = m_activeCamera->getProjMatrix(screenWidth, screenHeight);
    glm::mat4 view = m_activeCamera->getViewMatrix();
    glm::mat4 clip = proj * view;
    Frustum frustum = ExtractFrustum(clip);
    
    Frustum frustumToUse = m_useDebugFrustum ? m_debugFrustum : frustum;

    m_nodesToRender.clear();
    if (m_activeCamera && m_quadTree) {
        m_quadTree->Query(frustumToUse, m_nodesToRender);
    } else {
        m_nodesToRender = m_nodes;
    }

    // if(m_useDebugFrustum)
    //     DebugDrawFrustum(m_debugFrustum);
    */
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
        node->draw(proj, view);
        
    if(m_renderQuadTree)
        m_quadTree->Render(proj, view);

    // if(m_useDebugFrustum)
    //     DebugRender::Instance().Render(proj,view);
    
    //Render text
    if(m_textRenderer && textBox)
    {
        glDisable(GL_DEPTH_TEST);
        m_textRenderer->render(proj,view);
        glEnable(GL_DEPTH_TEST);
    }
}


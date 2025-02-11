#include "Scene.h"

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

Scene::Scene() : m_activeCamera(nullptr), m_quadTree(nullptr), m_octTree(nullptr), m_renderTree(false) {}

Scene::~Scene() {
    // for (auto node : m_nodes)
    //     delete node;
    m_nodes.clear();
    m_lights.clear();
    if(m_quadTree) {
        delete m_quadTree;
        m_quadTree = nullptr;
    }
    if(m_octTree)
    {
        delete m_octTree;
        m_octTree = nullptr;
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

void Scene::BuildOctTree()
{
    using namespace chrono;
    auto t0 = high_resolution_clock::now();

    m_useQuadTreeOrOct = false;
    AABBVolume sceneBounds3D;
    sceneBounds3D.min = glm::vec3(-bounds, -bounds, -bounds);
    sceneBounds3D.max = glm::vec3(bounds, bounds, bounds);
    if(m_octTree)
        delete m_octTree;

    m_octTree = new OctTree(sceneBounds3D);

    DebugRender::Instance().Clear(); 
    for (Node* n : m_nodes) {
        m_octTree->Insert(n);
    }
    
    auto t1 = high_resolution_clock::now();
    float buildTimeMs = duration<float, milli>(t1 - t0).count();
    m_lastQuadTreeBuildTimeMs = buildTimeMs;
    
    reDebug = true;
}

void Scene::BuildQuadTree() {
    using namespace chrono;
    auto t0 = high_resolution_clock::now();

    m_useQuadTreeOrOct = true;
    AABB2D sceneBounds;
    sceneBounds.min = glm::vec2(-bounds, -bounds);
    sceneBounds.max = glm::vec2(bounds, bounds);
    if(m_quadTree) {
        delete m_quadTree;
    }
    m_quadTree = new QuadTree(sceneBounds);

    DebugRender::Instance().Clear(); 
    for(auto node : m_nodes)
    {
        m_quadTree->Insert(node);
    }
    
    auto t1 = high_resolution_clock::now();
    float buildTimeMs = duration<float, milli>(t1 - t0).count();
    m_lastQuadTreeBuildTimeMs = buildTimeMs;
    
    reDebug = true;
}

void Scene::RandomInitScene(int amount)
{
    Clear();
    m_pGrid = new Grid3D(bounds);
    /*int gridSize = std::ceil(std::cbrt(amount)); // Determine the grid dimensions (N x N x N)
    float spacing = (2.0f * bounds) / gridSize; // Adjust spacing to fit within bounds

    int cubeCount = 0;
    for (int     x = 0; x < gridSize && cubeCount < amount; x++) {
        for (int y = 0; y < gridSize && cubeCount < amount; y++) {
            for (int z = 0; z < gridSize && cubeCount < amount; z++) {
                // Map (x, y, z) to a position within -bounds to bounds
                float posX = -bounds + x * spacing;
                float posY = -bounds + y * spacing;
                float posZ = -bounds + z * spacing;

                glm::vec3 position = glm::vec3(posX, posY, posZ);

                DebugCube* cube = new DebugCube("Cube_" + to_string(cubeCount));
                cube->setTransform(glm::translate(glm::mat4(1.0f), position));

                AddNode(cube); // Add cube to scene
                cubeCount++;
            }
        }
    }
    */
    
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<float> distPos(-bounds, bounds);
    uniform_real_distribution<float> rgb(0.0f, 255.0f);
    // rgb = new vec3(rgb(gen),rgb(gen),rgb(gen));
    

    for (int i = 1; i <= amount; ++i)
    {
        DebugCube* newCube = new DebugCube("cube_" + to_string(i));
        glm::vec3 randomPos(distPos(gen), distPos(gen), distPos(gen));
        newCube->setTransform(glm::translate(glm::mat4(1.0f), randomPos));
        GLubyte r = rgb(gen)
               ,g = rgb(gen)
               ,b = rgb(gen);
        newCube->setColor(r,g,b);
        
        // DebugRender::Instance().DrawCircle(randomPos, 0.2f, glm::vec3(1.0f));
        AddNode(newCube);
    }
    for (int i = 0; i < 2; i++)
    {
        glm::vec3 pos = glm::vec3(distPos(gen), distPos(gen), distPos(gen));
        m_lights.push_back( Light(pos,  glm::vec3(1,1,1), 1.0f, 10.0f));
        // DebugRender::Instance().DrawCircle(pos, 0.2f, glm::vec3(1.0f));
    }
    

    // DebugCube* cube = new DebugCube("Cube_");// + to_string(cubeCount));
    // cube->setTransform(glm::translate(glm::mat4(1.0f), glm::vec3(0,2,0)));
    // AddNode(cube); // Add cube to scene

    // m_lights.push_back( Light(glm::vec3(0, 5, 0),  glm::vec3(1,1,1), 10.0f, 10.0f));

    // m_lights.push_back( Light(glm::vec3(-5, 0, 0), glm::vec3(0,1,0), 1.0f, 10.0f));
    // m_lights.push_back( Light(glm::vec3(0, 0, 5), glm::vec3(0,0,1), 1.0f, 10.0f));

}

void Scene::ShowDebugText()
{
    m_textRenderer = new TextRenderer();
    m_textRenderer->init();
    Font* fontArial   = m_textRenderer->createFont("Arial");
    TextBox* textBoc   = m_textRenderer->createTextBox(fontArial,"Press Q to visualize Quad Tree.\nPress R To randomly Generate Nodes.\nPress F to toggle Debug Frustum.\nPress A to Switch Trees", 640.0f - 400, 360.0f, 400, 200);
    textBoc->SetColor(0,0,0,255);
    textBoc->SetVisualization(false); //remove bg box
    textBoc->SetAlignment(2);

    textBox  = m_textRenderer->createTextBox(fontArial,"FPS, Each Process's MS, Other important values", -640.0f, 360.0f, 400, 720);
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

    //time at start
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
        node->UpdateBoundingVolume();
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

    if(m_useQuadTreeOrOct)
    {
        if (m_activeCamera && m_quadTree) {
            m_quadTree->Query(frustumToUse, m_nodesToRender);
        } else {
            m_nodesToRender = m_nodes;
        }
    }
    else
    {
        if (m_activeCamera && m_octTree) {
            m_octTree->Query(frustumToUse, m_nodesToRender);
        } else {
            m_nodesToRender = m_nodes;
        }
    }
    t1 = high_resolution_clock::now();
    float quadTreeQueryMS = duration<float, milli>(t1 - t0).count();

    //LIGHT QUERY
    t0 = high_resolution_clock::now();
    UpdateLighting();

    t1 = high_resolution_clock::now();
    float lightQueryMS = duration<float, milli>(t1 - t0).count();

    m_avgCameraUpdateMs   = EMA(m_avgCameraUpdateMs,   cameraUpdateMS);
    m_avgNodeUpdateMs     = EMA(m_avgNodeUpdateMs,     nodeUpdateMS);
    m_avgBoundingVolumeMs = EMA(m_avgBoundingVolumeMs, boundingVolumeMS);
    m_avgFrustumExtractMs = EMA(m_avgFrustumExtractMs, frustumExtractMS);
    m_avgQuadTreeQueryMs  = EMA(m_avgQuadTreeQueryMs,  quadTreeQueryMS);

    string activeTreeName = m_useQuadTreeOrOct ? "Quad" : "Oct";
    int cubesAffectedByLight = 0;
    for(auto* node : m_nodes) {
        DebugCube* c = dynamic_cast<DebugCube*>(node);
        if(c->m_affectingLights.size() > 0)
            cubesAffectedByLight++; 
    }

    string info;
    info += "FPS: " + to_string((int)lastKnownFps) + "\n";
    info += "\n";
    info += "QuadTree Render: "  + string(m_renderTree   ? "ON" : "OFF") + "\n";
    info += "Debug Frustum  : "  + string(m_useDebugFrustum  ? "ON" : "OFF") + "\n";
    info += "CURRENT TREE   : " + activeTreeName + "\n";
    info += "Tree Build Time: " + to_string(m_lastQuadTreeBuildTimeMs) + " ms\n";
    info += "Cubes Visible  : " + to_string(m_nodesToRender.size()) + "\n";
    info += "Lights In Scene: " + to_string(m_lights.size()) + "\n";
    info += "Cubes Affected by light: " + to_string(cubesAffectedByLight) + "\n";
    info += "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
    info += "Camera Update      : " + to_string(cameraUpdateMS)    + " ms\n";
    info += "Nodes Update       : " + to_string(nodeUpdateMS)       + " ms\n";
    info += "Bounding Vol Update: " + to_string(boundingVolumeMS)   + " ms\n";
    info += "Extract Frustum    : " + to_string(frustumExtractMS)   + " ms\n";
    info += "QuadTree Query     : " + to_string(quadTreeQueryMS)    + " ms\n";
    info += "Light Query     : " + to_string(lightQueryMS)    + " ms\n";

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

void Scene::InitLights()
{

    // load the GPU programs
    m_unlitProgram  = volpe::ProgramManager::CreateProgram("data/Unlit3d.vsh",  "data/Unlit3d.fsh");
    // m_pointProgram  = volpe::ProgramManager::CreateProgram("data/Unlit3d.vsh","data/Unlit3d.fsh");
    m_pointProgram  = volpe::ProgramManager::CreateProgram("data/PointLights.vsh","data/PointLights.fsh");
}

void Scene::UpdateLighting()
{
    // reset cubes
    for(auto* node : m_nodes) {
        DebugCube* c = dynamic_cast<DebugCube*>(node);
        if(c) {
            c->m_numLightsAffecting = 0;
        }
    }

    // For each light, we do QueryLight
    // for(auto& L : m_lights)
    for(int i = 0; i < m_lights.size(); i++)
    {
        auto& L = m_lights[i];
        // DebugRender::Instance().DrawSphere(L.position, L.radius, glm::vec3(1.0f));  //CAUSES CRASHESS WATCH OUTTTTT
        float range = L.intensity * 10.f; 
        vector<Node*> inRange;
        if(m_useQuadTreeOrOct && m_quadTree)
            m_quadTree->QueryLight(L.position, range, inRange);
        else if(!m_useQuadTreeOrOct && m_octTree)
            m_octTree->QueryLight(L.position, range, inRange);

        for(auto* n : inRange){
            DebugCube* c = dynamic_cast<DebugCube*>(n);
            if(!c) continue;
            
            c->m_affectingLights.push_back(i);  // storing the LIGHT’S INDEX
            c->m_numLightsAffecting++;
        }
    }

    // Switch shader
    for(auto* node : m_nodes)
    {
        DebugCube* c = dynamic_cast<DebugCube*>(node);
        if(!c) continue;

        if(c->m_numLightsAffecting == 0) {
            c->SetProgram(m_unlitProgram);
        } else {
            c->SetProgram(m_pointProgram); //m_pointProgram

        }
    }
}

void Scene::MoveLights()
{
    // DebugRender::Instance().Clear();
    // Move all existing lights randomly
    if(m_lights.empty()) return;
    // m_lights.clear();

    std::random_device rd;
    std::mt19937 gen(rd());
    /*
    std::uniform_real_distribution<float> distDelta(-0.05f, 0.05f);

    for(auto& light : m_lights)
    {
        // Add a small random offset to each coordinate.
        light.position.x += distDelta(gen);
        light.position.y += distDelta(gen);
        light.position.z += distDelta(gen);
        
        // (Optionally: Clamp or wrap the position if it goes out of desired bounds.)
    }
    */
    // /*
    std::uniform_real_distribution<float> distPos(-bounds, bounds);

    for(auto& light : m_lights)
    {
        light.position.x = distPos(gen);
        light.position.y = distPos(gen);
        light.position.z = distPos(gen);
    }
    // */
}

void Scene::Clear()
{
    if(!m_nodes.empty())
        m_nodes.clear();
    
    if(!m_lights.empty())
        m_lights.clear();

    if(!m_nodesToRender.empty())
        m_nodesToRender.clear();
        
    DebugRender::Instance().Clear();
}

void Scene::Render(int screenWidth, int screenHeight) {

    if (!m_activeCamera)
        return;
    glm::mat4 proj = m_activeCamera->getProjMatrix(screenWidth, screenHeight);
    glm::mat4 view = m_activeCamera->getViewMatrix();
    
    m_pGrid->render(view,proj);
    
    for(auto* n : m_nodesToRender)
    {
        DebugCube* c = dynamic_cast<DebugCube*>(n);
        if(!c) {
            // If not a DebugCube, draw normally.
            n->draw(proj, view);
            continue;
        }

        volpe::Program* prog = c->GetProgram();
        if(prog == m_pointProgram)
        {
            // Bind the point–light shader and push the lighting uniforms.
            prog->Bind();
            ////////////////////////////////////////////////////////////////
            int lightCount = c->m_numLightsAffecting; 
            int maximumLights = 5;
            if(lightCount > maximumLights)
                lightCount = maximumLights;  // clamp

            // Set how many we actually pass
            prog->SetUniform("lightsInRange", lightCount);

            // fill up test[ i ] for each light in c->m_affectingLights
            for(int i=0; i<lightCount; i++)
            {
                int lightIdx = c->m_affectingLights[i]; 
                Light& L     = m_lights[lightIdx];

                std::string base = "pointLights[" + std::to_string(i) + "]";

                glm::vec3 pos  =  L.position;
                float radius   =  L.radius;
                glm::vec3 col  =  L.color;
                float strength =  L.intensity;

                prog->SetUniform(base+".PositionRange", glm::vec4(pos, radius));
                prog->SetUniform(base+".Color",         col);
                prog->SetUniform(base+".Strength",      strength);
            }

            prog->SetUniform("fade", 1.0f);
            ////////////////////////////////////////////////////////////////
            
            //Make it so it only sets Uniform for the Lights that affect the cubes
            //These lights are found during the UpdateLighting()

            // prog->SetUniform(("test[0].PositionRange"), glm::vec4(m_lights[0].position, m_lights[0].radius));
            // prog->SetUniform(("test[0].Color"), m_lights[0].color);
            // prog->SetUniform(("test[0].Strength"), m_lights[0].intensity);
            // prog->SetUniform(("lightsInRange"), 3);
            // prog->SetUniform("fade", 1.0f); // normal shit

            // Draw the cube WITHOUT rebinding the shader.
            c->draw(proj, view, true);
        }
        else
        {
            // For cubes using the unlit shader, just draw normally.
            c->draw(proj, view);
        }
    }
    
    if(m_renderTree)
    {
        if(reDebug) // if there was a change
        {
            reDebug = false;
            if(m_useQuadTreeOrOct)
            {
                DebugRender::Instance().Clear();
                m_quadTree->BuildDebugLines();
            }
            else
            {
                std::cout << "Clear and fuck off?\n";
                DebugRender::Instance().Clear();
                m_octTree->BuildDebugLines();
            }
        }
        glDisable(GL_DEPTH_TEST);
        DebugRender::Instance().Render(proj, view);
        glEnable(GL_DEPTH_TEST);
    }

    // Render text
    if(m_textRenderer && textBox)
    {
        glDisable(GL_DEPTH_TEST);
        m_textRenderer->render(proj,view);
        glEnable(GL_DEPTH_TEST);
    }
}


// void Scene::Render(int screenWidth, int screenHeight) {
//     if (!m_activeCamera)
//         return;
//     glm::mat4 proj = m_activeCamera->getProjMatrix(screenWidth, screenHeight);
//     glm::mat4 view = m_activeCamera->getViewMatrix();
//     m_pGrid->render(view,proj);
//     //COMMENT OUT TO DEBUG QUADTREE LOGICCCC
//     //MAKE SURE TO ADD DEBUG CIRCLE TO EVERY NODE BEFOREHAND
//     // for (auto node : m_nodesToRender)
//         // node->draw(proj, view);
//     for(auto* n : m_nodesToRender)
//     {
//         DebugCube* c = dynamic_cast<DebugCube*>(n);
//         if(!c) {
//             // if not a DebugCube, just draw
//             n->draw(proj, view);
//             continue;
//         }
//         volpe::Program* prog = c->GetProgram();
//         if(prog == m_pointProgram)
//         {
//             // Bind the program and pass the array of lights
//             prog->Bind();
//             // We'll pass up to 8 lights:
//             int lightCount = std::min((int)m_lights.size(), 8);
//             prog->SetUniform("u_lightCount", lightCount);
//             for(int i=0; i<lightCount; i++)
//             {
//                 // name in the shader
//                 string base = "u_pointLights[" + to_string(i) + "]";
//                 // define the 4D "PositionRange"
//                 glm::vec3 pos = m_lights[i].position;
//                 float range   = m_lights[i].radius;   // or m_lights[i].intensity * 10.0f
//                 glm::vec3 col = m_lights[i].color;
//                 float strength= m_lights[i].intensity;
//                 prog->SetUniform(base+".PositionRange", glm::vec4(pos, range));
//                 prog->SetUniform(base+".Color",         col);
//                 prog->SetUniform(base+".Strength",      strength);
//             }
//             // also set "fade" or "texture1" if needed
//             prog->SetUniform("fade", 1.0f);
//             // etc.
//         }
//         // Now call the cube's draw:
//         c->draw(proj, view);
//     }
//     /*for(auto* n: m_nodesToRender)
//     {
//         DebugCube* c = dynamic_cast<DebugCube*>(n);
//         if(!c) {
//             n->draw(proj, view);
//             continue;
//         }
//         volpe::Program* prog = c->GetProgram();
//         if(prog == m_pointProgram) {
//             prog->Bind();
//             // We'll pass up to 8 lights or however many we have
//             int lightCount = std::min((int)m_lights.size(), 8);
//             // Optionally skip if c->m_numLightsAffecting==0 => but we already set unlit.
//             prog->SetUniform("u_lightCount", lightCount);
//             for(int i=0; i<lightCount; i++){
//                 // define uniform name
//                 std::string base = "u_pointLights[" + std::to_string(i) + "]";
//                 glm::vec3 pos   = m_lights[i].position;
//                 float range     = m_lights[i].intensity * 10.f;
//                 glm::vec3 color = m_lights[i].color;
//                 float str       = m_lights[i].intensity;
//                 prog->SetUniform(base+".PositionRange", glm::vec4(pos, range));
//                 prog->SetUniform(base+".Color", color);
//                 prog->SetUniform(base+".Strength", str);
//             }
//             // also set fade or texture if needed:
//             prog->SetUniform("fade", 1.0f);
//             // ...
//         }
//         // now call the cube's draw
//         c->draw(proj, view);
//     }
//     */
//     if(m_renderTree)
//     {
//         if(reDebug) //if change change = false
//         {
//             reDebug = false;
//             if(m_useQuadTreeOrOct)
//             {
//                 // m_quadTree->Render(proj, view);
//                 DebugRender::Instance().Clear();
//                 m_quadTree->BuildDebugLines();
//                 // DebugRender::Instance().Render(proj,view);
//             }
//             else
//             {
//                 cout<<"Clear and fuck off?\n";
//                 // m_octTree->Render(proj, view);
//                 DebugRender::Instance().Clear();
//                 m_octTree->BuildDebugLines();
//                 // DebugRender::Instance().Render(proj,view);
//             }
//         }
//         glDisable(GL_DEPTH_TEST);
//         DebugRender::Instance().Render(proj, view);
//         glEnable(GL_DEPTH_TEST);
//     }
//     // if(m_useDebugFrustum)
//     //     DebugRender::Instance().Render(proj,view);
//     //Render text
//     if(m_textRenderer && textBox)
//     {
//         glDisable(GL_DEPTH_TEST);
//         m_textRenderer->render(proj,view);
//         glEnable(GL_DEPTH_TEST);
//     }
// }

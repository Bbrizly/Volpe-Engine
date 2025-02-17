#include "Scene.h"

//REMOVE LATERRRR RMEOV EMRO ERMO
#include <string>
#include <sstream>
#include <iomanip>

using namespace chrono;
using namespace std;

Scene& Scene::Instance() {
    static Scene instance;
    return instance;
}

Scene::Scene() : m_activeCamera(nullptr), m_quadTree(nullptr), m_octTree(nullptr), m_ShowDebug(false) {}

Scene::~Scene() {
    // for (auto node : m_nodes)
    //     delete node;
    m_nodes.clear();
    m_lights.clear();
    delete m_pGrid;
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

void Scene::AddLight(Light l)
{
    
    m_lights.push_back(l);
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

void Scene::ToggleUseDebugFrustum(Camera* c)
{
    m_debugCamera = c;
    //Left, Right, Bottom, Top, Near, Far
    // m_debugFrustum.planes[0] = glm::vec4( 1, 0, 0,  10.0f);  //   x + 0*y + 0*z + 10 = 0
    // m_debugFrustum.planes[1] = glm::vec4(-1, 0, 0,  10.0f);  //  -x + 0*y + 0*z + 10 = 0
    // m_debugFrustum.planes[2] = glm::vec4( 0, 1, 0,   5.0f);  //   0*x +  y + 0*z +  5 = 0
    // m_debugFrustum.planes[3] = glm::vec4( 0,-1, 0,   5.0f);  //   0*x + -y + 0*z +  5 = 0
    // m_debugFrustum.planes[4] = glm::vec4( 0, 0, 1,   1.0f);  //   0*x + 0*y +  z +  1 = 0
    // m_debugFrustum.planes[5] = glm::vec4( 0, 0,-1, 100.0f);  //   0*x + 0*y + -z +100 = 0

    // for (int i = 0; i < 6; ++i) {
    //     float length = glm::length(glm::vec3(m_debugFrustum.planes[i]));
    //     m_debugFrustum.planes[i] /= length;
    // }

    m_useDebugFrustum  = !m_useDebugFrustum;
}

void Scene::BuildOctTree()
{
    using namespace chrono;
    auto t0 = high_resolution_clock::now();

    m_useQuadTreeOrOct = false;
    AABBVolume sceneBounds3D;
    sceneBounds3D.min = glm::vec3(-m_bounds, -m_bounds, -m_bounds);
    sceneBounds3D.max = glm::vec3(m_bounds, m_bounds, m_bounds);
    if(m_octTree)
        delete m_octTree;

    m_octTree = new OctTree(sceneBounds3D);

    DebugRender::Instance().ClearLayer("Tree");
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
    sceneBounds.min = glm::vec2(-m_bounds, -m_bounds);
    sceneBounds.max = glm::vec2(m_bounds, m_bounds);
    if(m_quadTree) {
        delete m_quadTree;
    }
    m_quadTree = new QuadTree(sceneBounds);

    DebugRender::Instance().ClearLayer("Tree");
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
    auto t0 = high_resolution_clock::now();
    Clear();
    m_pGrid = new Grid3D(m_bounds);
    // /* GRID 
    int gridSize = std::ceil(std::cbrt(amount)); // Determine the grid dimensions (N x N x N)
    float spacing = (2.0f * m_bounds) / gridSize; // Adjust spacing to fit within bounds

    random_device rd;
    // mt19937 gen(rd());
    mt19937 gen(100);
    uniform_real_distribution<float> distPos(-m_bounds, m_bounds);
    uniform_real_distribution<float> rgb(0.0f, 255.0f);

    int cubeCount = 0;
    for (int     x = 0; x < gridSize && cubeCount < amount; x++) {
        for (int y = 0; y < gridSize && cubeCount < amount; y++) {
            for (int z = 0; z < gridSize && cubeCount < amount; z++) {
                // Map (x, y, z) to a position within -bounds to bounds
                float posX = -m_bounds + x * spacing;
                float posY = -m_bounds + y * spacing;
                float posZ = -m_bounds + z * spacing;

                glm::vec3 position = glm::vec3(posX, posY, posZ);

                DebugCube* cube = new DebugCube("Cube_" + to_string(cubeCount));
                cube->setTransform(glm::translate(glm::mat4(1.0f), position));

                glm::vec4 trans = cube->getTransform()[3];
                std::cout << "Cube_" << cubeCount << " location: ("
                      << trans.x << ", " << trans.y << ", " << trans.z << ")\n";
                
                GLubyte r = rgb(gen)
                ,g = rgb(gen)
                ,b = rgb(gen);
                cube->setColor(r,g,b);

                AddNode(cube); // Add cube to scene
                cubeCount++;
            }
        }
    }
    // */
    
    /*
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
    
    */
    // DebugCube* cube = new DebugCube("Cube_");// + to_string(cubeCount));
    // cube->setTransform(glm::translate(glm::mat4(1.0f), glm::vec3(0,2,0)));
    // AddNode(cube); // Add cube to scene
    // m_lights.push_back( Light(glm::vec3(0, 5, 0),  glm::vec3(1,1,1), 10.0f, 10.0f));
    // m_lights.push_back( Light(glm::vec3(-5, 0, 0), glm::vec3(0,1,0), 1.0f, 10.0f));
    // m_lights.push_back( Light(glm::vec3(0, 0, 5), glm::vec3(0,0,1), 1.0f, 10.0f));

    
    auto t1 = high_resolution_clock::now();
    m_avgCreation = duration<float, milli>(t1 - t0).count();
}

void Scene::ShowDebugText()
{
    m_textRenderer = new TextRenderer();
    m_textRenderer->init();

    const GLubyte* renderer = glGetString(GL_RENDERER);
    std::string rendererStr = reinterpret_cast<const char*>(renderer);
    const GLubyte* vendor = glGetString(GL_VENDOR);
    std::string vendorStr = reinterpret_cast<const char*>(vendor);

    Font* fontArial   = m_textRenderer->createFont("Arial");
    TextBox* textBoc = m_textRenderer->createTextBox(fontArial,
        "Press Q to visualize Quad Tree.\n"
        "Press G to visualize Bounding volumes.\n"
        "Press R To Generate random Nodes.\n"
        "Press B To Generate Solar System.\n"
        "Press F to toggle FPS Camera's Frustum.\n"
        "Press E to Switch between Trees\n"
        "Press C to Switch between CAMERAS\n"
        "Press L to randomize Light Locations.\n"
        "Press TAB to unlock mouse.\n"
        "Renderer: " + rendererStr + "\nVendor: " + vendorStr,
        640.0f - 400, 360.0f, 400, 265);
    
    TextBox* solarSysTextBox = m_textRenderer->createTextBox(fontArial,
        "Solar System Control Scheme:.\n"
        "Press I to toggle X axis.\n"
        "Press O To toggle Y axis.\n"
        "Press P to toggle Z axis.\n"
        "Press J to Decrement Speed\n"
        "Press K to Increment Speed\n",
        640.0f - 400, 0.0f - 200, 400, 200);
    solarSysTextBox->SetColor(0,0,0,255);
         
    textBoc->SetColor(0,0,0,255);
    // textBoc->SetVisualization(false); //remove bg box
    textBoc->SetAlignment(2);

    textBox  = m_textRenderer->createTextBox(fontArial,"FPS, Each Process's MS, Other important values", -640.0f, 360.0f, 300, 720);
    textBox->SetColor(0, 0, 0, 255);
    // textBox->SetVisualization(false); //remove bg box
    debugTextBox =  m_textRenderer->createTextBox(fontArial,"FPS, Each Process's MS, Other important values", 640-200.0f, 95.0f, 200, 200);
    debugTextBox->SetColor(0, 0, 0, 255);
    
    m_textRenderer->setTextBox(solarSysTextBox);
    m_textRenderer->setTextBox(debugTextBox);
    m_textRenderer->setTextBox(textBoc);
    m_textRenderer->setTextBox(textBox);
}

void Scene::Update(float dt, int screenWidth, int screenHeight) {
    #pragma region Normal functions
    auto EMA = [this](float oldValue, float newValue) {
        return oldValue * (1.0f - m_smoothAlpha) + newValue * m_smoothAlpha;
    };
    
    // if(m_useDebugFrustum)
    //     DebugDrawFrustum(m_debugFrustum);

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
        node->update(dt);                                    /////////////////////////////                                    /////////////////////////////                                    /////////////////////////////
    }
    t1 = high_resolution_clock::now();
    float nodeUpdateMS = duration<float, milli>(t1 - t0).count();

    // ========== Bounding Volume Updates ==========
    t0 = high_resolution_clock::now();
    for (auto node : m_nodes) {
        node->UpdateBoundingVolume();                                    /////////////////////////////                                    /////////////////////////////                                    /////////////////////////////
    }
    t1 = high_resolution_clock::now();
    float boundingVolumeMS = duration<float, milli>(t1 - t0).count();

    // ========== Frustum Extraction ==========
    t0 = high_resolution_clock::now();
    // glm::mat4 proj = m_activeCamera->getProjMatrix(screenWidth, screenHeight);
    // glm::mat4 view = m_activeCamera->getViewMatrix();
    // glm::mat4 clip = proj * view;
    // Frustum realFrustum = ExtractFrustum(clip);

    Frustum realFrustum = m_activeCamera->getFrustum(screenWidth, screenHeight);
    if(m_debugCamera != NULL && m_useDebugFrustum)
    {
        m_debugFrustum = m_debugCamera->getFrustum(screenWidth, screenHeight);
        m_debugCamera->update(dt);
    }
        
    Frustum frustumToUse = m_useDebugFrustum ? m_debugFrustum : realFrustum;
    t1 = high_resolution_clock::now();
    float frustumExtractMS = duration<float, milli>(t1 - t0).count();

    // ========== QuadTree Query ==========
    t0 = high_resolution_clock::now();
    // for(auto& x : m_nodes) //Making sure bounding volumes function
    // {
    //     if(x->GetBoundingVolume()->IntersectsFrustum(frustumToUse))
    //     {    m_nodesToRender.push_back(x);   }
    // }
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

    std::string shapes = "";

    for (auto& object : m_nodesToRender) {
        vec3 x = object->getWorldTransform()[3];

        std::ostringstream oss;
        oss << std::fixed << std::setprecision(0) << x.x << ", " 
            << std::fixed << std::setprecision(0) << x.y << ", " 
            << std::fixed << std::setprecision(0) << x.z;

        shapes += object->getName() + " | " + oss.str() + "\n";
    }

    // Set text in the debug UI
    debugTextBox->SetText(shapes);
    
    t1 = high_resolution_clock::now();
    float quadTreeQueryMS = duration<float, milli>(t1 - t0).count();

    //LIGHT QUERY
    t0 = high_resolution_clock::now();
    UpdateLighting();

    t1 = high_resolution_clock::now();
    float lightQueryMS = duration<float, milli>(t1 - t0).count();
    #pragma endregion

    // for (auto* node : m_nodes)
    // {
    //     std::cout << node->getName() << " is a " 
    //               << (dynamic_cast<DebugCube*>(node) ? "Cube" : "Sphere") 
    //               << " with bounding volume: " 
    //               << (dynamic_cast<AABBVolume*>(node->GetBoundingVolume()) ? "AABB" : "Sphere")
    //               << std::endl;
    // }

    #pragma region Statistics

    m_avgCameraUpdateMs   = EMA(m_avgCameraUpdateMs,   cameraUpdateMS);
    m_avgNodeUpdateMs     = EMA(m_avgNodeUpdateMs,     nodeUpdateMS);
    m_avgBoundingVolumeMs = EMA(m_avgBoundingVolumeMs, boundingVolumeMS);
    m_avgFrustumExtractMs = EMA(m_avgFrustumExtractMs, frustumExtractMS);
    m_avgQuadTreeQueryMs  = EMA(m_avgQuadTreeQueryMs,  quadTreeQueryMS);
    m_avgLightQuery  = EMA(m_avgLightQuery,  lightQueryMS);
    // m_avgQuadTreeQueryMs  = EMA(m_avgQuadTreeQueryMs,  quadTreeQueryMS);

    string activeTreeName = m_useQuadTreeOrOct ? "Quad" : "Oct";

    int nodesAffectedByLight = 0;
    for(auto* node : m_nodes) { //ONLY WORKS FOR DEBUG CUBE, NOT GOOD. FIX NOWWW
        // DebugCube* c = dynamic_cast<DebugCube*>(node);
        if(node->m_affectingLights.size() > 0)
            nodesAffectedByLight++; 
    }

    string info;
    info += "FPS: " + to_string((int)lastKnownFps) + "\n";
    info += "\n";
    info += "QuadTree Render: "  + string(m_ShowDebug   ? "ON" : "OFF") + "\n";
    info += "Visual Bounding Vols: "  + string(m_ShowBoundingVolumes   ? "ON" : "OFF") + "\n";
    info += "Debug Frustum  : "  + string(m_useDebugFrustum  ? "ON" : "OFF") + "\n";
    info += "CURRENT TREE   : " + activeTreeName + "\n";
    info += "Scene Creation Time: " + to_string(m_avgCreation) + " ms\n";
    info += "Tree Build Time: " + to_string(m_lastQuadTreeBuildTimeMs) + " ms\n";
    info += "Existing Nodes : " + to_string(m_nodes.size()) + "\n";
    info += "Nodes Visible  : " + to_string(m_nodesToRender.size()) + "\n";
    info += "Lights In Scene: " + to_string(m_lights.size()) + "\n";
    info += "Nodes Affected by light: " + to_string(nodesAffectedByLight) + "\n";
    info += "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
    info += "Camera Update      : " + to_string(m_avgCameraUpdateMs)    + " ms\n";
    info += "Nodes Update       : " + to_string(m_avgNodeUpdateMs)       + " ms\n";
    info += "Bounding Vol Update: " + to_string(m_avgBoundingVolumeMs)   + " ms\n";
    info += "Extract Frustum    : " + to_string(m_avgFrustumExtractMs)   + " ms\n";
    info += "QuadTree Query     : " + to_string(m_avgQuadTreeQueryMs)    + " ms\n";
    info += "Light Query        : " + to_string(m_avgLightQuery)    + " ms\n";

    // updating textBox
    if (textBox) {
        textBox->SetText(info);
    }

    #pragma endregion
}

void Scene::InitLights()
{
    m_matUnlit = volpe::MaterialManager::CreateMaterial("UnlitMaterial");
    m_matUnlit->SetProgram("data/Unlit3d.vsh", "data/Unlit3d.fsh");
    m_matUnlit->SetDepthTest(true);
    m_matUnlit->SetDepthWrite(true);

    m_matPoint = volpe::MaterialManager::CreateMaterial("PointLightMaterial");
    m_matPoint->SetProgram("data/PointLights.vsh", "data/PointLights.fsh"); 
    m_matPoint->SetDepthTest(true);
    m_matPoint->SetDepthWrite(true);
}

void Scene::UpdateLighting()
{
    for(auto* node : m_nodes)
    {
        node->m_affectingLights.clear(); 
    }

    for(int i=0; i < (int)m_lights.size(); i++)
    {
        Light& L = m_lights[i];
        float range = L.radius;

        std::vector<Node*> inRange;
        if(m_useQuadTreeOrOct && m_quadTree)
        {
            m_quadTree->QueryLight(L.position, range, inRange);
        }
        else if(!m_useQuadTreeOrOct && m_octTree)
        {
            m_octTree->QueryLight(L.position, range, inRange);
        }
        else
        {
            for(auto* nd : m_nodes)
            {
                if(nd->GetBoundingVolume() 
                   && SphereVolume(L.position, range).Overlaps(*nd->GetBoundingVolume()))
                {
                    inRange.push_back(nd);
                }
            }
        }

        for(auto* n : inRange)
        {
            n->m_affectingLights.push_back(i);
        }
    }
}

void Scene::MoveLights()
{
    if(m_lights.empty()) return;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> distPos(-m_bounds, m_bounds);

    for(auto& light : m_lights)
    {
        light.position.x = distPos(gen);
        light.position.y = distPos(gen);
        light.position.z = distPos(gen);
    }
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
    if(m_pGrid)
        m_pGrid->render(view,proj);

    for(auto* n : m_nodesToRender) //m_nodesToRender //m_nodes
    {   
        if(n->m_affectingLights.size() > 0 && n->GetReactToLight())
        {
            n->SetMaterial(m_matPoint);
            volpe::Material* mat = n->GetMaterial();

            int maxLights = 5;
            int lightCount = std::min(static_cast<int>(n->m_affectingLights.size()), maxLights);

            // if(lightCount > maxLights)
            //     lightCount = maxLights;
            
            mat->SetUniform("lightsInRange", lightCount);
            // cout<<lightCount<<", lights in range\n";
            
            for(int i=0; i<lightCount; i++)
            {
                int lightIdx = n->m_affectingLights[i]; 
                Light& L     = m_lights[lightIdx];
                
                std::string base = "pointLights[" + std::to_string(i) + "]";

                glm::vec3 pos  =  L.position;
                float radius   =  L.radius;
                glm::vec3 col  =  L.color;
                float strength =  L.intensity;

                mat->SetUniform(base+".PositionRange", vec4(pos, radius));
                mat->SetUniform(base+".Color",              col);
                mat->SetUniform(base+".Strength",           strength);
            }
            mat->SetUniform("fade", 1.0f);
        }
        // else        
            // n->SetMaterial(m_matUnlit);
        
        n->draw(proj, view);
    }
    
    #pragma region debug
    
    if(m_debugCamera != NULL && m_useDebugFrustum)
    {
        DebugRender::Instance().ClearLayer("frustum");
        DebugRender::Instance().DrawFrustumFromCamera(m_debugCamera, screenWidth, screenHeight, "frustum");
    }

    if(m_ShowDebug)
    {
        if(m_ShowBoundingVolumes)
        {
            for(auto* n : m_nodesToRender)
            {
                if(n->GetBoundingVolume())
                {  n->GetBoundingVolume()->DrawMe();  }
            }
        }
        for (Light l : m_lights)
        {
            DebugRender::Instance().DrawSphere(l.position,l.radius,vec3(1));
        }

        if(reDebug) // if there was a change
        {
            reDebug = false;
            DebugRender::Instance().ClearLayer("Tree");

            if(m_useQuadTreeOrOct)
            {
                m_quadTree->BuildDebugLines();
            }
            else
            {
                m_octTree->BuildDebugLines();
            }
        }
        
        glDisable(GL_DEPTH_TEST);
        DebugRender::Instance().Render(proj, view);
        glEnable(GL_DEPTH_TEST);
    }
    #pragma endregion
    
    // Render text
    if(m_textRenderer && textBox)
    {
        glDisable(GL_DEPTH_TEST);
        m_textRenderer->setScreenSize(screenWidth,screenHeight);
        m_textRenderer->render(proj,view);
        glEnable(GL_DEPTH_TEST);
    }
}

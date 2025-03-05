#include "Program.h"

using namespace std;

Program::Program(volpe::App* pApp)
: m_pApp(pApp)
{
    // Scene* scene = new Scene(pApp);
}

Program::~Program()
{
    delete orbitCamera;
    delete fpsCamera;
    
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}


int amount = 10; //AMOUTN TEMPORORARY DELETE LATEERRRRR 
int bounds = 10;

static DebugSphere* gSun   = nullptr;
static DebugSphere* gEarth = nullptr;
static DebugSphere* gMoon  = nullptr;
static DebugSphere* gVenus = nullptr;
static DebugSphere* gJupiter = nullptr;

static Node* gEarthOrbit = nullptr;
static Node* gMoonOrbit  = nullptr;
static Node* gVenusOrbit = nullptr;
static Node* gJupiterOrbit = nullptr;

vec3 OrbitAxis = vec3(0,1,0);
float speedMultipler = 1.0f;

#pragma region SCENE UI


static Node* g_selectedNode = nullptr;



static glm::vec3 ExtractTranslation(const glm::mat4& m)
{
    return glm::vec3(m[3][0], m[3][1], m[3][2]);
}

static glm::vec3 ExtractScale(const glm::mat4& m)
{
    
    float sx = glm::length(glm::vec3(m[0][0], m[1][0], m[2][0]));
    float sy = glm::length(glm::vec3(m[0][1], m[1][1], m[2][1]));
    float sz = glm::length(glm::vec3(m[0][2], m[1][2], m[2][2]));
    return glm::vec3(sx, sy, sz);
}


static glm::mat4 MakeTransform(const glm::vec3& position, const glm::vec3& scale)
{
    glm::mat4 T = glm::translate(glm::mat4(1.0f), position);
    glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);
    return T * S;
}

float debugWindowHeight = 500.0f;
bool culled = false;
void Program::DrawSceneHierarchy()
{
    float sceneWidth = 230.0f;
    float topOffset = 30.0f;
    float sceneHeight = ImGui::GetIO().DisplaySize.y - (topOffset + debugWindowHeight);
    ImGui::SetNextWindowPos(ImVec2(0, topOffset));
    ImGui::SetNextWindowSize(ImVec2(sceneWidth, sceneHeight));
    ImGui::Begin("Scene Hierarchy", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    ImGui::Checkbox("Culled", &culled);
    vector<Node*> nodes;
    if(culled) nodes = Scene::Instance().GetNodesToRender();
    else       nodes = Scene::Instance().GetNodes();
        
    for (auto* node : nodes)
    {
        bool isSelected = (node == g_selectedNode);
        if (ImGui::Selectable(node->getName().c_str(), isSelected))
        {
            g_selectedNode = node;
        }
    }

    ImGui::End();
}

void Program::DrawDebugWindow()
{
    float debugWidth = 230;
    float debugPosY = ImGui::GetIO().DisplaySize.y - debugWindowHeight; //bottom-left

    ImGui::SetNextWindowPos(ImVec2(0, debugPosY));
    ImGui::SetNextWindowSize(ImVec2(debugWidth, debugWindowHeight));
    ImGui::Begin("Debug", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    if (ImGui::BeginTabBar("DebugTabs"))
    {
        if (ImGui::BeginTabItem("Statistics"))
        {
            ImGui::Separator();
            ImGui::Text("FPS: %d", (int)Scene::Instance().getFPS());
            ImGui::Separator();
            ImGui::Text("QuadTree Render: %s", Scene::Instance().getShowDebug() ? "ON" : "OFF");
            ImGui::Text("Visual Bounding Vols: %s", Scene::Instance().getShowBoundingVolumes() ? "ON" : "OFF");
            ImGui::Text("Debug Frustum: %s", Scene::Instance().getUseDebugFrustum() ? "ON" : "OFF");
            ImGui::Text("CURRENT TREE: %s", Scene::Instance().getActiveTreeName().c_str());
            ImGui::Separator();
            ImGui::Text("Scene Creation Time: %.2f ms", Scene::Instance().getSceneCreationTime());
            ImGui::Text("Tree Build Time: %.2f ms", Scene::Instance().getTreeBuildTime());
            ImGui::Text("Existing Nodes: %d", (int)Scene::Instance().GetNodes().size());
            ImGui::Text("Nodes Visible: %d", (int)Scene::Instance().GetNodesToRender().size());
            ImGui::Text("Lights In Scene: %d", (int)Scene::Instance().GetLights().size());
            ImGui::Text("Nodes Affected by Light: %d", (int)Scene::Instance().getNodesAffectedByLight());
            ImGui::Separator();
            ImGui::Text("Camera Update: %.2f ms", Scene::Instance().getAvgCameraUpdateMs());
            ImGui::Text("Nodes Update: %.2f ms", Scene::Instance().getAvgNodeUpdateMs());
            ImGui::Text("Bounding Vol Update: %.2f ms", Scene::Instance().getAvgBoundingVolumeMs());
            ImGui::Text("Extract Frustum: %.2f ms", Scene::Instance().getAvgFrustumExtractMs());
            ImGui::Text("QuadTree Query: %.2f ms", Scene::Instance().getAvgQuadTreeQueryMs());
            ImGui::Text("Light Query: %.2f ms", Scene::Instance().getAvgLightQuery());
            ImGui::Separator();
            
            const GLubyte* renderer = glGetString(GL_RENDERER);
            std::string rendererStr = reinterpret_cast<const char*>(renderer);
            const GLubyte* vendor = glGetString(GL_VENDOR);
            std::string vendorStr = reinterpret_cast<const char*>(vendor);
            ImGui::PushTextWrapPos();
                ImGui::Text("Renderer: %s", rendererStr.c_str());
                ImGui::Text("Vendor: %s", vendorStr.c_str());
            ImGui::PopTextWrapPos();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Controls"))
        {
            if (ImGui::Button("Toggle Debug Frustum"))
            {
                Scene::Instance().ToggleUseDebugFrustum(fpsCamera);
            }
            if (ImGui::Button("Toggle Tree Render"))
            {
                Scene::Instance().ToggleQuadTreeRender();
            }
            if (ImGui::Button("Toggle Bounding Vol Debug"))
            {
                Scene::Instance().ToggleBoundingVolumeDebug();
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

void Program::DrawInspector()
{
    float inspectorWidth = 300.0f;
    float topOffset = 30.0f;
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - inspectorWidth, topOffset));
    ImGui::SetNextWindowSize(ImVec2(inspectorWidth, ImGui::GetIO().DisplaySize.y - topOffset));
    ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    if (!g_selectedNode)
    {
        ImGui::Text("No node selected");
        ImGui::End();
        return;
    }

    char nameBuf[256];
    strcpy(nameBuf, g_selectedNode->getName().c_str());
    if (ImGui::InputText("Name", nameBuf, IM_ARRAYSIZE(nameBuf)))
    {
        string nameSet = nameBuf;
        if(nameBuf[0] == '\0') {nameSet = "NULL";} //needed cuz deleting whats in textbox crashes program
        g_selectedNode->setName(std::string(nameSet));
    }

    glm::mat4 localT = g_selectedNode->getTransform();
    glm::vec3 pos = ExtractTranslation(localT);
    glm::vec3 scl = ExtractScale(localT);

    if (ImGui::DragFloat3("Position", (float*)&pos, 0.1f))
    {
        glm::mat4 newTrans = MakeTransform(pos, scl);
        g_selectedNode->setTransform(newTrans);
    }
    if (ImGui::DragFloat3("Scale", (float*)&scl, 0.01f, 0.01f, 100.0f))
    {
        glm::mat4 newTrans = MakeTransform(pos, scl);
        g_selectedNode->setTransform(newTrans);
    }

    // Node specific BUT once I convert codebase to entity system, this will muchhh simpler
    if (auto* cube = dynamic_cast<DebugCube*>(g_selectedNode))
    {
        glm::vec3 c = cube->getColor();
        float color[3] = { c.r, c.g, c.b };
        if (ImGui::ColorEdit3("Cube Color", color))
        {
            GLubyte rr = (GLubyte)(color[0] * 255.0f);
            GLubyte gg = (GLubyte)(color[1] * 255.0f);
            GLubyte bb = (GLubyte)(color[2] * 255.0f);
            cube->setColor(rr, gg, bb);
        }
    }
    else if (auto* sphere = dynamic_cast<DebugSphere*>(g_selectedNode))
    {
        float radius = sphere->getRadius();
        if (ImGui::DragFloat("Radius", &radius, 0.1f, 0.01f, 100.0f))
        {
            sphere->setRadius(radius);
        }

        glm::vec3 sc = sphere->getColor();
        float col[3] = {sc.r, sc.g, sc.b};
        if(ImGui::ColorEdit3("Sphere Color", col))
        {
            GLubyte rr = (GLubyte)(col[0] * 255.0f);
            GLubyte gg = (GLubyte)(col[1] * 255.0f);
            GLubyte bb = (GLubyte)(col[2] * 255.0f);
            sphere->setColor(rr, gg, bb);
        }
    }
    // /*
    else if (auto* emitter = dynamic_cast<ParticleNode*>(g_selectedNode))
    {
        ImGui::Separator();
        ImGui::Text("Particle System Controls");
        if(ImGui::Button("Play")) {
            emitter->Play();
        }
        ImGui::SameLine();
        if(ImGui::Button("Pause")) {
            emitter->Pause();
        }
        ImGui::SameLine();
        if(ImGui::Button("Stop")) {
            emitter->Stop();
        }
        ImGui::SameLine();
        if(ImGui::Button("Restart")) {
            emitter->Restart();
        }
    
        ImGui::Text("Current State: %s",
           (emitter->systemState == ParticleSystemState::Playing) ? "Playing" :
           (emitter->systemState == ParticleSystemState::Paused)  ? "Paused"  : 
                                                                   "Stopped");

        ImGui::Text("Active Particles: %d", emitter->getParticles().size());

        ImGui::DragFloat("Emission Rate", &emitter->emissionRate, 0.1f, 0.f, 9999.f);
        ImGui::Checkbox("Local Space", &emitter->localSpace);
        ImGui::DragInt("Max Particles", &emitter->maxParticles, 1, 1, 100000);
        
        // Emitter shape
        static const char* shapeNames[] = {"Point","Sphere","Cone","Box","Mesh"};
        int shapeIdx = (int)emitter->shape;
        if(ImGui::Combo("Shape", &shapeIdx, shapeNames, IM_ARRAYSIZE(shapeNames))) {
            emitter->shape = (EmitterShape)shapeIdx;
        }
        ImGui::PushItemWidth(50);
        ImGui::DragFloat(" - ##size", (float*)&emitter->startSizeMin, 0.05f); ImGui::SameLine();
        ImGui::DragFloat("Spawn Size range", (float*)&emitter->startSizeMax, 0.05f);
        
        ImGui::DragFloat(" - ##lifetime", (float*)&emitter->lifetimeMin, 0.05f); ImGui::SameLine();
        ImGui::DragFloat("Lifetime range", (float*)&emitter->lifetimeMax, 0.05f);
        
        // ImGui::DragFloat(" - ##rot", (float*)&emitter->rotationMin, 0.05f); ImGui::SameLine();
        // ImGui::DragFloat("Rotation range", (float*)&emitter->rotationMax, 0.05f);
        
        ImGui::DragFloat(" - ##rotSpeed", (float*)&emitter->rotationSpeedMin, 0.05f); ImGui::SameLine();
        ImGui::DragFloat("Rotation Speed range", (float*)&emitter->rotationSpeedMax, 0.05f);
        
        ImGui::DragFloat(" - ##vel", (float*)&emitter->velocityScaleMin, 0.05f); ImGui::SameLine();
        ImGui::DragFloat("Velocity scale range", (float*)&emitter->velocityScaleMax, 0.05f);
        
        ImGui::DragFloat(" - ##alpha", (float*)&emitter->startAlphaMin, 0.05f); ImGui::SameLine();
        ImGui::DragFloat("Alpha range", (float*)&emitter->startAlphaMax, 0.05f);

        ImGui::PopItemWidth();

        ImGui::DragFloat3("Spawn Pos", (float*)&emitter->spawnPosition, 0.05f);
        ImGui::DragFloat3("Spawn Vel", (float*)&emitter->spawnVelocity, 0.05f);
        ImGui::DragFloat3("Acceleration", (float*)&emitter->globalAcceleration, 0.1f);
    
        // Over-lifetime
        ImGui::Separator();
        ImGui::Text("Over Lifetime");
        // ImGui::DragFloat("Lifetime", (float*)&emitter->life, 0.05f);
        ImGui::DragFloat("Size Over Life",  &emitter->sizeOverLife,  0.01f, 0.0f, 999.f);
        ImGui::DragFloat("Alpha Over Life", &emitter->alphaOverLife, 0.01f, 0.0f, 999.f);
    
        //COlor gradient
        if (ImGui::TreeNode("Color Gradient"))
        {
            // Loop through current keys
            for (size_t i = 0; i < emitter->colorKeys.size(); i++)
            {
                ImGui::PushID(static_cast<int>(i));
                float time = emitter->colorKeys[i].time;
                // Create a temporary color array for ImGui
                float col[4] = {
                    emitter->colorKeys[i].color.r,
                    emitter->colorKeys[i].color.g,
                    emitter->colorKeys[i].color.b,
                    emitter->colorKeys[i].color.a
                };
                if (ImGui::SliderFloat("Time", &time, 0.0f, 1.0f))
                {
                    emitter->colorKeys[i].time = time;
                }
                if (ImGui::ColorEdit4("Color", col))
                {
                    emitter->colorKeys[i].color = glm::vec4(col[0], col[1], col[2], col[3]);
                }
                if (ImGui::Button("Remove"))
                {
                    emitter->colorKeys.erase(emitter->colorKeys.begin() + i);
                    ImGui::PopID();
                    break; // break out of the loop so we don’t use an invalid iterator
                }
                ImGui::Separator();
                ImGui::PopID();
            }
            // Button to add a new key (defaults to time 0 and white)
            if (ImGui::Button("Add Key"))
            {
                ColorKey newKey;
                newKey.time = 0.0f;
                newKey.color = glm::vec4(1.0f);
                emitter->colorKeys.push_back(newKey);
            }
            ImGui::TreePop();
        }

        // Burst
        if(ImGui::TreeNode("Bursts"))
        {
            for(size_t i=0; i<emitter->burstTimes.size(); i++)
            {
                ImGui::PushID((int)i);
                ImGui::DragFloat("Time", &emitter->burstTimes[i], 0.01f, 0.f, 999999.f);
                ImGui::DragInt("Count", &emitter->burstCounts[i], 1, 0, 100000);
                if(ImGui::Button("Remove")) {
                    emitter->burstTimes.erase(emitter->burstTimes.begin() + i);
                    emitter->burstCounts.erase(emitter->burstCounts.begin()+ i);
                    ImGui::PopID();
                    break;
                }
                ImGui::PopID();
                ImGui::Separator();
            }
            if(ImGui::Button("Add Burst")) {
                emitter->burstTimes.push_back(1.f);
                emitter->burstCounts.push_back(10);
            }
            ImGui::TreePop();
        }
        if (ImGui::CollapsingHeader("Debug: Active Particles", ImGuiTreeNodeFlags_DefaultOpen))
        {
            // Number of active
            auto& ps = emitter->getParticles();
            int activeCount = (int) ps.size();
            ImGui::Text("Active Count: %d", activeCount);

            // Only showing 10 particles for debugging, any more will be wasteful
            int showCount = std::min(activeCount, 10);
            for (int i = 0; i < showCount; i++)
            {
                const auto& p = ps[i];
                if(ImGui::TreeNode((void*)(intptr_t)i, "Particle %d", i))
                {
                    ImGui::Text("Age: %.2f / %.2f", p.age, p.lifetime);
                    ImGui::Text("Pos: (%.2f, %.2f, %.2f)", p.position.x, p.position.y, p.position.z);
                    ImGui::Text("Vel: (%.2f, %.2f, %.2f)", p.velocity.x, p.velocity.y, p.velocity.z);
                    ImGui::Text("Size: %.2f, Rotation: %.2f", p.size, p.rotation);
                    ImGui::Text("Alpha: %.2f", p.color.a);
                    ImGui::TreePop();
                }
            }
        }
    }
    // */
    ImGui::End(); // end Inspector
}

void Program::DrawTopBar()
{
    //SCENE PICKER
    //For the future:
        //Add file
            //save and load scenes
            //Load full on scene XMLs
        //Add ADD
            //Add nodes + when i rewrite codebase to ecs u add components
    ImGui::SetNextWindowPos(ImVec2(0,0));
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, 20));
    ImGui::Begin("TopBar", nullptr,
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar |
                 ImGuiWindowFlags_NoScrollWithMouse);

    if (ImGui::Button("Solar System"))
    {
        SwitchScene(SceneType::SolarSystem);
    }
    ImGui::SameLine();
    if (ImGui::Button("Random Scene"))
    {
        SwitchScene(SceneType::Random);
    }
    ImGui::SameLine();
    if (ImGui::Button("Particle Scene"))
    {
        SwitchScene(SceneType::Particle);
    }

    ImGui::End();
}

#pragma endregion

#pragma region Helper Creations
void RecreateSceneHelper(int bounds)
{
    bounds = 10;
    Scene::Instance().SetBounds(bounds, true);
    

    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<float> distPos(-bounds, bounds);
    uniform_real_distribution<float> rgb(0.0f, 255.0f);

    ParticleNode* Emitter = new ParticleNode("ParticleSystemNode");

    Emitter->setTransform(glm::mat4(1.0f));
    Emitter->Play();
    Emitter->spawnParticles(20);

    Scene::Instance().AddNode(Emitter);

    for (int i = 1; i <= amount/2; ++i)
    {
        DebugCube* cube = new DebugCube("cube_" + to_string(i));

        glm::vec3 randomPos(distPos(gen), distPos(gen) / 4, distPos(gen));
        cube->setTransform(glm::translate(glm::mat4(1.0f), randomPos));
        
        // GLubyte color = rgb(gen);//i * 20;

        // GLubyte r = color
        //        ,g = color
        //        ,b = color;

        
        GLubyte r = rgb(gen)
               ,g = rgb(gen)
               ,b = rgb(gen);
        
        cube->setColor(r,g,b);
        Scene::Instance().AddNode(cube);
    }

    for (int i = 1; i <= amount/2; ++i)
    {
        DebugSphere* sphere = new DebugSphere("sphere_" + to_string(i), 0.5);

        glm::vec3 randomPos(distPos(gen), distPos(gen) / 4, distPos(gen));
        sphere->setTransform(glm::translate(glm::mat4(1.0f), randomPos));
        
        // GLubyte color = rgb(gen);//i * 20;

        // GLubyte r = color
        //        ,g = color
        //        ,b = color;
        
        GLubyte r = rgb(gen)
               ,g = rgb(gen)
               ,b = rgb(gen);

        sphere->setColor(r,g,b);
        Scene::Instance().AddNode(sphere);
    }

    for (int i = 0; i < 2; i++)
    {
        glm::vec3 pos = glm::vec3(distPos(gen), distPos(gen), distPos(gen));
        Scene::Instance().AddLight(Light(pos,  glm::vec3(1,1,1), 10.0f, 10.0f));
        DebugRender::Instance().DrawCircle(pos, 10.0f, vec3(1,1,0));
    }
    
}

void BuildAsteroidField(int count, float innerRadius, float outerRadius)
{

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> distAngle(0.0f, 360.0f);
    std::uniform_real_distribution<float> distRadius(innerRadius, outerRadius);
    std::uniform_real_distribution<float> distShape(0.0f, 1.0f);  // 50% chance sphere/cube
    std::uniform_real_distribution<float> distHeight(-5.0f, 5.0f); // random offset
    std::uniform_real_distribution<float> distScale(0.2f, 1.0f);   // random size
    std::uniform_real_distribution<float> distColor(0.0f, 255.0f);

    for(int i = 0; i < count; i++)
    {
        float angleDeg = distAngle(gen);
        float angleRad = glm::radians(angleDeg);
        float radius   = distRadius(gen);

        // Random position in ring
        float x = radius * cos(angleRad);
        float z = radius * sin(angleRad);
        float y = distHeight(gen);

        GLubyte r = (GLubyte)distColor(gen);
        GLubyte g = (GLubyte)distColor(gen);
        GLubyte b = (GLubyte)distColor(gen);

        // Randomly pick sphere or cube
        Node* asteroid;
        if(distShape(gen) < 0.5f)
        {
            asteroid = new DebugSphere("AsteroidSphere_" + std::to_string(i),
                                       distScale(gen));  // sphere radius
            // asteroid->setColor(r, g, b);
        }
        else
        {asteroid = new DebugCube("AsteroidCube_" + std::to_string(i)); 
            // asteroid->setColor(r, g, b);
        }

        glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z));
        asteroid->setTransform(T);

        //astroid doesnt render if not added to scene
        Scene::Instance().AddNode(asteroid);
    }
}

void BuildSolarSystem(int bounds)
{
    bounds = 40;
    Scene::Instance().SetBounds(bounds);

    //SUN                                    ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///
    DebugSphere* sun = new DebugSphere("Sun", 3.0f);
    sun->setTransform(glm::mat4(1.0f)); // at origin
    sun->setColor(255, 255, 0);          // yellow

    //Orbit node for Earth (child of sunn)   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///
    gEarthOrbit = new Node("EarthOrbit");
    gEarthOrbit->setTransform(glm::mat4(1.0f));
    sun->addChild(gEarthOrbit);

    DebugSphere* earth = new DebugSphere("Earth", 1.0f);
    earth->setTransform(glm::translate(glm::mat4(1.0f), vec3(10.0f, 0.0f, 0.0f)));
    earth->setColor(0, 0, 255);
    gEarthOrbit->addChild(earth);

    //Orbit node for Moon (child of earth)   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///
    gMoonOrbit = new Node("MoonOrbit");
    gMoonOrbit->setTransform(glm::mat4(1.0f));
    earth->addChild(gMoonOrbit);

    DebugSphere* moon = new DebugSphere("Moon", 0.3f);
    moon->setTransform(glm::translate(glm::mat4(1.0f), vec3(2.0f, 0.0f, 0.0f)));
    moon->setColor(200, 200, 200);
    gMoonOrbit->addChild(moon);

    //Orbit node for Jupiter (child of sun)   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///
    gJupiterOrbit = new Node("JupiterOrbit");
    gJupiterOrbit->setTransform(glm::mat4(1.0f));
    sun->addChild(gJupiterOrbit);

    DebugSphere* Jupiter = new DebugSphere("Jupiter", 0.8f); //DebugCube
    Jupiter->setTransform(glm::translate(glm::mat4(1.0f), vec3(15.0f, 0.0f, 0.0f)));
    Jupiter->setColor(200, 160, 120);
    gJupiterOrbit->addChild(Jupiter);

    //Orbit node for Venus (child of sun)   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///   ///
    gVenusOrbit = new Node("VenusOrbit");
    gVenusOrbit->setTransform(glm::mat4(1.0f));
    sun->addChild(gVenusOrbit);

    DebugSphere* venus = new DebugSphere("Venus", 0.8f);
    venus->setTransform(glm::translate(glm::mat4(1.0f), vec3(-8.0f, 0.0f, 0.0f)));
    venus->setColor(255, 150, 100);
    gVenusOrbit->addChild(venus);

    sun->SetReactToLight(false);
    Scene::Instance().AddLight(Light(vec3(0.0f), vec3(1,1,1), 1.0f, 50.0f));
    Scene::Instance().AddNode(gMoon);
    Scene::Instance().AddNode(gEarthOrbit);
    Scene::Instance().AddNode(gJupiterOrbit);
    Scene::Instance().AddNode(gVenusOrbit);

    Scene::Instance().AddNode(moon);
    Scene::Instance().AddNode(earth);
    Scene::Instance().AddNode(Jupiter);
    Scene::Instance().AddNode(venus);

    Scene::Instance().AddNode(sun);

    BuildAsteroidField(800, 25.0f, 40.0f);
}

void UpdateSolarSystem(float dt)
{
    static float earthOrbitSpeed = 0.5f;
    static float moonOrbitSpeed  = 0.9f;
    static float venusOrbitSpeed = 0.35f;
    static float jupiterOrbitSpeed = 0.35f;

    static float earthAngle = 0.0f;
    static float moonAngle  = 0.0f;
    static float venusAngle = 0.0f;
    static float jupiterAngle = 0.0f;

    //See faster orbits
    dt *= speedMultipler;

    earthAngle   += dt * earthOrbitSpeed;
    moonAngle    += dt * moonOrbitSpeed;
    venusAngle   += dt * venusOrbitSpeed;
    jupiterAngle += dt * jupiterOrbitSpeed;

    //Adding funny rotations :) ()
    if(OrbitAxis == vec3(0))
        OrbitAxis = vec3(0,1,0);


    if(gEarthOrbit)
        gEarthOrbit->setTransform(glm::rotate(glm::mat4(1.0f),  earthAngle, OrbitAxis));
    if(gMoonOrbit)
        gMoonOrbit->setTransform(glm::rotate(glm::mat4(1.0f),   moonAngle,  OrbitAxis));
    if(gVenusOrbit)
        gVenusOrbit->setTransform(glm::rotate(glm::mat4(1.0f),  venusAngle, OrbitAxis));
    if(gJupiterOrbit)
        gJupiterOrbit->setTransform(glm::rotate(glm::mat4(1.0f),  jupiterAngle, OrbitAxis));
}

void buildParticleScene()
{
    Scene::Instance().Clear();
    
    Scene::Instance().SetBounds(bounds, true);

    ParticleNode* Emitter = new ParticleNode("ParticleSystemNode0");
    ParticleNode* Emitter1 = new ParticleNode("ParticleSystemNode1");
    ParticleNode* Emitter2 = new ParticleNode("ParticleSystemNode2");

    volpe::Texture* texture0 = volpe::TextureManager().CreateTexture("data/smoke.png");
    volpe::Texture* texture1 = volpe::TextureManager().CreateTexture("data/baby1.png");
    volpe::Texture* texture2 = volpe::TextureManager().CreateTexture("data/baby.png");

    Emitter->GetMaterial()->SetTexture("u_texture", texture0);
    std::cout<<"E1: "<<Emitter->GetMaterial()<<"\n";
    Emitter1->GetMaterial()->SetTexture("u_texture", texture1);
    std::cout<<"E2: "<<Emitter1->GetMaterial()<<"\n";
    Emitter2->GetMaterial()->SetTexture("u_texture", texture2);
    std::cout<<"E3: "<<Emitter2->GetMaterial()<<"\n";

    // Emitter->setTransform(glm::mat4(1.0f));
    // Emitter->Play();

    

    Scene::Instance().AddNode(Emitter);
    Scene::Instance().AddNode(Emitter1);
    Scene::Instance().AddNode(Emitter2);
}

#pragma endregion

#pragma region Scenes

static SceneType g_sceneType = SceneType::Particle;

void Program::SwitchScene(SceneType newScene)
{
    Scene::Instance().Clear();
    g_sceneType = newScene;

    switch(newScene)
    {
        case SceneType::SolarSystem:
            BuildSolarSystem(bounds);
            Scene::Instance().BuildOctTree();
            solarSystem = true;
            break;
        case SceneType::Random:
            RecreateSceneHelper(bounds);
            Scene::Instance().BuildQuadTree();
            solarSystem = false;
            break;
        case SceneType::Particle:
        default:
            buildParticleScene();
            Scene::Instance().BuildOctTree();
            solarSystem = false;
            break;
    }
}


#pragma endregion

void Program::init()
{
    #pragma region Initalizing Shit
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);

    // Create ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();  
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(m_pApp->getWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 330");


    #pragma endregion
    
    fpsCamera = new FirstPersonCamera(m_pApp);

    orbitCamera = new OrbitCamera(m_pApp);
    orbitCamera->focusOn(glm::vec3(-10.0f,-10.0f,-10.0f),glm::vec3(10.0f,10.0f,10.0f));

    Scene::Instance().SetActiveCamera(fpsCamera);

    Scene::Instance().ShowDebugText();
    
    Scene::Instance().InitLights();

    //starter scene
    buildParticleScene();
}

void Program::update(float dt)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    DrawTopBar();

    DrawSceneHierarchy();

    DrawInspector();
    DrawDebugWindow();

    // if(m_pApp->isKeyJustDown('P') || m_pApp->isKeyJustDown('p'))
    // {
    //     buildParticleScene();
    // }

    if(solarSystem)
    {
        //Toggle rotation axis X Y Z 
        if(m_pApp->isKeyJustDown('I')) {
            if(OrbitAxis.x == 1)
                OrbitAxis.x = 0;
            else
                OrbitAxis.x = 1;
        }
        if(m_pApp->isKeyJustDown('O')) {
            if(OrbitAxis.y == 1)
                OrbitAxis.y = 0;
            else
                OrbitAxis.y = 1;
        }
        if(m_pApp->isKeyJustDown('P')) {
            if(OrbitAxis.z == 1)
                OrbitAxis.z = 0;
            else
                OrbitAxis.z = 1;
        }
        if(m_pApp->isKeyJustDown('J')) 
            speedMultipler *= 0.8;
    
        if(m_pApp->isKeyJustDown('K')) 
            speedMultipler *= 1.2;
        
        UpdateSolarSystem(dt);
        Scene::Instance().BuildOctTree();  //REAL TIME CULLING OF MOVING OBJECTS!!! :D
    }
    else
    {
        if(m_pApp->isKeyJustDown('E') || m_pApp->isKeyJustDown('e'))
        {
            //Toggle between quadtree and octtree
            // Scene::Instance().Clear();
            if(Scene::Instance().getWhichTree()) //true is quad, false is oct. here theyre swapped to switch
                Scene::Instance().BuildOctTree();
            else
                Scene::Instance().BuildQuadTree();
        }
    }

    /* MOVED ALL THIS TO IMGUI SCENE UIII
    if (m_pApp->isKeyJustDown('F') || m_pApp->isKeyJustDown('f')) {
        Scene::Instance().ToggleUseDebugFrustum(fpsCamera);}

    if (m_pApp->isKeyJustDown('B') || m_pApp->isKeyJustDown('b')) {
        solarSystem = true;

        Scene::Instance().Clear();
        BuildSolarSystem(bounds);

        Scene::Instance().BuildOctTree();}

    if(m_pApp->isKeyJustDown('R') || m_pApp->isKeyJustDown('r'))
    {
        solarSystem = false;

        Scene::Instance().Clear();
        RecreateSceneHelper(bounds);
        // Scene::Instance().RandomInitScene(amount);

        if(Scene::Instance().getWhichTree())
            Scene::Instance().BuildQuadTree();
        else
            Scene::Instance().BuildOctTree();
    }

    if(m_pApp->isKeyJustDown('Q') || m_pApp->isKeyJustDown('q'))
    {
        Scene::Instance().ToggleQuadTreeRender();
    }

    if(m_pApp->isKeyJustDown('L')) { //HOLD L TO MOVE LIGHTS
        // random move lights
        Scene::Instance().MoveLights();}
    if(m_pApp->isKeyJustDown('G')) { //TOGGLE BOUNDING VOLUME 
        Scene::Instance().ToggleBoundingVolumeDebug();}

    */
    if(m_pApp->isKeyJustDown('C')) { //switch cameras
        whichCamera = !whichCamera;
        if(whichCamera)
            Scene::Instance().SetActiveCamera(orbitCamera);
        else
            Scene::Instance().SetActiveCamera(fpsCamera);
    }
    
    //DEBUGGING THE TEXT POSITIONS CUZ ITS SHIT
    //looks funny keeping for now
    // if(m_pApp->isKeyDown('W') || m_pApp->isKeyJustDown('s')) //Move FPS counter
    // {
    //     std::random_device rd;
    //     std::mt19937 gen(rd());
    //     // -640.0f, 360.0f
    //     std::uniform_real_distribution<float> distPos(0.0f, 20.0f);
    //     Scene::Instance().setTextBoxPos(-640.0f + distPos(gen), 360.0f - distPos(gen));
    // }

    Scene::Instance().Update(dt, m_pApp->getScreenSize().x, m_pApp->getScreenSize().y);
}

void Program::draw(int width, int height)
{
    // if(solarSystem)
        // glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    // else
        glClearColor(0.0f, 0.5f, 0.5f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Scene::Instance().Render(width, height);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


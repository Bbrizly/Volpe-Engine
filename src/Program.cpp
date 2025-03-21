#include "Program.h"
#include "SceneSerializer.h"
#include "BoidNode.h"
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

#pragma region Helper Creations
void RecreateSceneHelper(int bounds)
{
    bounds = 10;
    Scene::Instance().SetBounds(bounds, true);
    
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<float> distPos(-bounds, bounds);
    uniform_real_distribution<float> rgb(0.0f, 255.0f);

    for (int i = 1; i <= amount/2; ++i)
    {
        DebugCube* cube = new DebugCube("cube_" + to_string(i));

        glm::vec3 randomPos(distPos(gen), distPos(gen) / 4, distPos(gen));
        cube->setTransform(glm::translate(glm::mat4(1.0f), randomPos));
        
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
        
        GLubyte r = rgb(gen)
               ,g = rgb(gen)
               ,b = rgb(gen);

        sphere->setColor(r,g,b);
        Scene::Instance().AddNode(sphere);
    }

    for (int i = 0; i < 2; i++)
    {
        glm::vec3 pos = glm::vec3(distPos(gen), distPos(gen), distPos(gen));
        Scene::Instance().AddLight(pos, glm::vec3(1,1,1), 10.0f, 10.0f);
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
    std::uniform_real_distribution<float> distColor(100.0f, 255.0f);

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
                                       distScale(gen));

            DebugSphere* sphere = dynamic_cast<DebugSphere*>(asteroid);
            sphere->setColor(r, g, b);
        }
        else
        {   asteroid = new DebugCube("AsteroidCube_" + std::to_string(i)); 
            DebugCube* cube = dynamic_cast<DebugCube*>(asteroid);
            cube->setColor(r, g, b);
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
    Scene::Instance().AddLight(vec3(0.0f), glm::vec3(1,1,1), 1.0f, 50.0f);
    // Scene::Instance().AddNode(gMoon);
    // Scene::Instance().AddNode(gEarthOrbit);
    // Scene::Instance().AddNode(gJupiterOrbit);
    // Scene::Instance().AddNode(gVenusOrbit);

    // Scene::Instance().AddNode(moon);
    // Scene::Instance().AddNode(earth);
    // Scene::Instance().AddNode(Jupiter);
    // Scene::Instance().AddNode(venus);

    Scene::Instance().AddNode(sun);

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

    volpe::Texture* texture0 = volpe::TextureManager().CreateTexture("data/Textures/smoke.png");
    volpe::Texture* texture1 = volpe::TextureManager().CreateTexture("data/Textures/minecraft.png");
    volpe::Texture* texture2 = volpe::TextureManager().CreateTexture("data/Textures/baby.png");

    Emitter->GetMaterial()->SetTexture("u_texture", texture0);
    std::cout<<"E1: "<<Emitter->GetMaterial()<<"\n";
    Emitter1->GetMaterial()->SetTexture("u_texture", texture1);
    std::cout<<"E2: "<<Emitter1->GetMaterial()<<"\n";
    Emitter2->GetMaterial()->SetTexture("u_texture", texture2);
    std::cout<<"E3: "<<Emitter2->GetMaterial()<<"\n";

    Scene::Instance().AddNode(Emitter);
    Scene::Instance().AddNode(Emitter1);
    Scene::Instance().AddNode(Emitter2);
}

#pragma endregion

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

bool IsChild(Node* candidate, Node* node)
{
    if (!node)
        return false;
        
    for (Node* child : node->getChildren())
    {
        if (child == candidate)
            return true;
        if (IsChild(candidate, child))
            return true;
    }
    return false;
}

void DrawNodeRecursive(Node* node, int indentLevel) {
    if (!node) return;
    
    if (indentLevel > 0) 
        ImGui::Indent(indentLevel * 15.0f);

    bool isSelected = (node == g_selectedNode);
    
    if (ImGui::Selectable(node->getName().c_str(), isSelected))
        g_selectedNode = node;
        
    if (indentLevel > 0)
        ImGui::Unindent(indentLevel * 15.0f);
    
    for (Node* child : node->getChildren())
        DrawNodeRecursive(child, indentLevel + 1);
}

float debugWindowHeight = 500.0f;
bool culled = false;
void Program::DrawSceneHierarchy()
{
    float sceneWidth = 230.0f;
    float topOffset = 18.0f;
    float sceneHeight = ImGui::GetIO().DisplaySize.y - (topOffset + debugWindowHeight);
    ImGui::SetNextWindowPos(ImVec2(0, topOffset));
    ImGui::SetNextWindowSize(ImVec2(sceneWidth, sceneHeight));
    ImGui::Begin("Scene Hierarchy", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    ImGui::Checkbox("Show Grid", &Scene::Instance().showGrid);
    ImGui::SameLine();
    ImGui::Checkbox("Rebuild Tree", &rebuildTreeEveryFrame);
    
    ImGui::Checkbox("Culled", &culled);
    ImGui::SameLine();
    ImGui::Text("| Top Nodes: %i", Scene::Instance().GetNodes().size());
    vector<Node*> nodes;
    if(culled) nodes = Scene::Instance().GetNodesToRender();
    else       nodes = Scene::Instance().GetNodes();
        
    for (auto* node : nodes)
    {
        if(node->getParent() == nullptr)
            DrawNodeRecursive(node, 0);
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
            ImGui::Text("MS Budget:");
            ImGui::Text("Camera Update: %.2f ms", Scene::Instance().getAvgCameraUpdateMs());
            ImGui::Text("Nodes Update: %.2f ms", Scene::Instance().getAvgNodeUpdateMs());
            ImGui::Text("Scene Creation Time: %.2f ms", Scene::Instance().getSceneCreationTime());
            ImGui::Text("Tree Build Time: %.2f ms", Scene::Instance().getTreeBuildTime());
            ImGui::Text("Bounding Vol Update: %.2f ms", Scene::Instance().getAvgBoundingVolumeMs());
            ImGui::Text("Extract Frustum: %.2f ms", Scene::Instance().getAvgFrustumExtractMs());
            ImGui::Text("Tree Query: %.2f ms", Scene::Instance().getAvgQuadTreeQueryMs());
            ImGui::Text("Light Query: %.2f ms", Scene::Instance().getAvgLightQuery());
            ImGui::Separator();
            ImGui::Text("Scene Debug:");
            ImGui::Text("Existing Nodes: %d", (int)Scene::Instance().GetNodes().size());
            ImGui::Text("Nodes Visible: %d", (int)Scene::Instance().GetNodesToRender().size());
            // ImGui::Text("Lights In Scene: %d", (int)Scene::Instance().GetLights().size());
            ImGui::Text("Nodes Affected by Light: %d", (int)Scene::Instance().getNodesAffectedByLight());
            ImGui::Separator();
            ImGui::Text("Toggles:");
            ImGui::Text("QuadTree Render: %s", Scene::Instance().getShowDebug() ? "ON" : "OFF");
            ImGui::Text("Visual Bounding Vols: %s", Scene::Instance().getShowBoundingVolumes() ? "ON" : "OFF");
            ImGui::Text("Debug Frustum: %s", Scene::Instance().getUseDebugFrustum() ? "ON" : "OFF");
            ImGui::Text("CURRENT TREE: %s", Scene::Instance().getActiveTreeName().c_str());
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
            if (ImGui::Button("Toggle Tree Render"))
            {
                Scene::Instance().ToggleQuadTreeRender();
            }
            if (ImGui::Button("    Add Bounding Vol debug"))
            {
                Scene::Instance().ToggleBoundingVolumeDebug();
            }
            if (ImGui::Button("Switch Spatial Tree"))
            {
                if(Scene::Instance().getWhichTree()) //true is quad, false is oct. here theyre swapped to switch
                    Scene::Instance().BuildOctTree();
                else
                    Scene::Instance().BuildQuadTree();
            }
            if (ImGui::Button("Toggle Debug Frustum"))
            {
                Scene::Instance().ToggleUseDebugFrustum(fpsCamera);
            }
            
            
            
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

void deleteNode(Node* n)
{
    // if(n->getParent())
    //         n->getParent()->removeChild(n);
    Scene::Instance().RemoveNode(n);
    n = nullptr;
}

void Program::DrawInspector()
{
    float inspectorWidth = 300.0f;
    float topOffset = 18.0f;
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
    
    //GENERIC Node info
    
    if (ImGui::InputText("Name", nameBuf, IM_ARRAYSIZE(nameBuf)))
    {
        string nameSet = nameBuf;
        if(nameBuf[0] == '\0') {nameSet = "NULL";} //needed cuz deleting whats in textbox crashes program
        g_selectedNode->setName(std::string(nameSet));
    }
    ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 40);
    if (ImGui::SmallButton("Delete"))
    {
        deleteNode(g_selectedNode);
        g_selectedNode = nullptr;
        ImGui::End();
        return;
    }

    glm::mat4 localT = g_selectedNode->getTransform();
    glm::vec3 pos = ExtractTranslation(localT);
    glm::vec3 scl = ExtractScale(localT);

    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
    {
        glm::vec3 pos, scl;
        glm::quat rot;
        g_selectedNode->getTransformDecomposed(pos, rot, scl);

        // Convert quaternion to degrees
        glm::vec3 eulerDegrees = glm::degrees(glm::eulerAngles(rot));

        // Position
        if(ImGui::DragFloat3("Position", (float*)&pos, 0.1f)){
            // rebuild
            glm::quat newQ = glm::quat(glm::radians(eulerDegrees));
            g_selectedNode->setTransformDecomposed(pos, newQ, scl);
        }
        // Scale
        if(ImGui::DragFloat3("Scale", (float*)&scl, 0.01f, 0.001f, 1000.f)){
            glm::quat newQ = glm::quat(glm::radians(eulerDegrees));
            g_selectedNode->setTransformDecomposed(pos, newQ, scl);
        }
        // Rotation
        if(ImGui::DragFloat3("Rotation", (float*)&eulerDegrees, 0.1f, -360.f, 360.f)){
            // Rebuild from euler
            glm::quat newQ = glm::quat(glm::radians(eulerDegrees));
            g_selectedNode->setTransformDecomposed(pos, newQ, scl);
        }

        bool react = g_selectedNode->GetReactToLight(); //REACT TO LIGHT SHIT
        if (ImGui::Checkbox("React to Light", &react))
        {
            g_selectedNode->SetReactToLight(react);
        }
        ImGui::SameLine();
        if(g_selectedNode->GetMaterial())
            ImGui::Text("Shdr: %s", g_selectedNode->GetMaterial()->GetName().c_str());
    }

    // Node specific BUT once I convert codebase to entity system, this will muchhh simpler
    if (auto* light = dynamic_cast<LightNode*>(g_selectedNode))
    {
        ImGui::Text("Light Node Inspector");
        ImGui::Separator();
        
        glm::vec3 lightColor = light->color;
        if (ImGui::ColorEdit3("Light Color", (float*)&lightColor))
            light->color = lightColor;

        float intensity = light->intensity;
        if (ImGui::DragFloat("Intensity", &intensity, 0.1f, 0.0f, 100.0f))
            light->intensity = intensity;

        float radius = light->GetRadius();
        if (ImGui::DragFloat("Radius", &radius, 0.1f, 0.0f, 100.0f))
        {
            light->SetRadius(radius);
        }
    }
    else if (auto* cube = dynamic_cast<DebugCube*>(g_selectedNode))
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
    else if (auto* emitter = dynamic_cast<ParticleNode*>(g_selectedNode))
    {
        #pragma region SAVING AND LOADING && TEXTURE SWITCHING
        if (ImGui::Button("Save Emitter"))
        {
            static char emitterFileName[256] = "MyEmitter.emitter.yaml"; // default
            ImGui::OpenPopup("SaveEmitterPopup");
        }

        // The SaveEmitterPopup
        if (ImGui::BeginPopupModal("SaveEmitterPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            // text input for the file name
            static char emitterFileName[256] = "MyEmitter.emitter.yaml";
            ImGui::InputText("Emitter File", emitterFileName, IM_ARRAYSIZE(emitterFileName));

            if (ImGui::Button("Save##Emitter"))
            {
                std::string filename = std::string("data/Saved/") + emitterFileName;
                SceneSerializer::SaveEmitter(emitter, filename);
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel##Emitter"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        // For Load Emitter:
        ImGui::SameLine();
        if (ImGui::Button("Load Emitter"))
        {
            ImGui::OpenPopup("LoadEmitterPopup");
        }

        if (ImGui::BeginPopupModal("LoadEmitterPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            static char loadFileName[256] = "";
            ImGui::InputText("Emitter File", loadFileName, IM_ARRAYSIZE(loadFileName));
            if (ImGui::Button("Load##EmitterLoad"))
            {
                std::string filename = std::string("data/Saved/") + loadFileName;
                ParticleNode* loaded = SceneSerializer::LoadEmitter(filename);
                if (loaded)
                {
                    *emitter = *loaded;
                }
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel##EmitterLoad"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::SameLine();
        if (ImGui::Button("Load Texture"))
        {
            ImGui::OpenPopup("LoadTexturePopup");
        }

        if (ImGui::BeginPopupModal("LoadTexturePopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            static char loadFileName[256] = "";
            ImGui::InputText("Texture File", loadFileName, IM_ARRAYSIZE(loadFileName));
            if (ImGui::Button("Load##TextureLoad"))
            {
                
                std::string filename = std::string("data/Textures/") + loadFileName;
                volpe::Texture* texture0 = volpe::TextureManager().CreateTexture(filename);
                if(texture0)
                    emitter->GetMaterial()->SetTexture("u_texture", texture0);
                
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel##EmitterLoad"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        #pragma endregion
        
        ImGui::Separator();
        // ImGui::Text("Particle System Controls");
        if (ImGui::CollapsingHeader("Emitter Controls", ImGuiTreeNodeFlags_DefaultOpen))
        {
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
            if(ImGui::Button("End")) { emitter->End(); }
            ImGui::SameLine();
            if(ImGui::Button("Restart")) {
                emitter->Restart();
            }
        
            ImGui::Text("Current State: %s",
            (emitter->systemState == ParticleSystemState::Playing) ? "Playing" :
            (emitter->systemState == ParticleSystemState::Paused)  ? "Paused"  : 
                                                                    "Stopped");
        }
        //Emitter mode
        if (ImGui::CollapsingHeader("Emitter Settings", ImGuiTreeNodeFlags_DefaultOpen))
        {
            static const char* emitterModeNames[] = {"Continuous","Burst"};
            int currentModeIndex = (emitter->emitterMode == EmitterMode::Continuous) ? 0 : 1;
            if(ImGui::Combo("Emitter Mode", &currentModeIndex, emitterModeNames, IM_ARRAYSIZE(emitterModeNames)))
            {
                emitter->emitterMode = (currentModeIndex == 0) ? EmitterMode::Continuous : EmitterMode::Burst;
            }                                                

            ImGui::Text("Active Particles: %d", emitter->getParticles().size());

            ImGui::PushItemWidth(60); 
            ImGui::DragInt("Max Particles", &emitter->maxParticles, 1, 1, 100000);
            ImGui::SameLine();
            ImGui::DragFloat("Duration", &emitter->duration, 0.25, -1, 100000);
            ImGui::PopItemWidth();

            ImGui::DragFloat("Emission Rate", &emitter->emissionRate, 0.1f, 0.f, 9999.f);

            // ImGui::Checkbox("Local Space", &emitter->localSpace);
            ImGui::Checkbox("Glow", &emitter->glow);
            if(emitter->glow)
            {
                ImGui::SameLine();
                ImGui::PushItemWidth(60); 
                ImGui::DragFloat("Intensity", &emitter->glowIntensity, 0.05f, 0.0f);
                ImGui::PopItemWidth();
            }
            
            ImGui::Checkbox("Face Camera", &emitter->faceCamera);
            if(!emitter->faceCamera)
            {
                ImGui::DragFloat3("Custom Look Dir", (float*)&emitter->customLookDir, 0.05f);
                ImGui::DragFloat3("Custom Up Dir",   (float*)&emitter->customUpDir,   0.05f);
            }
            else
            {
                ImGui::Checkbox(",##XAxis", &emitter->lockXAxis);
                ImGui::SameLine();
                ImGui::Checkbox(",##YAxis", &emitter->lockYAxis);
                ImGui::SameLine();
                ImGui::Checkbox("Lock (x,y,z)##ZAxis", &emitter->lockZAxis);
            }
            ImGui::DragFloat2("Stretch (X,Y)", (float*)&emitter->defaultStretch, 0.01f, 0.0f, 10.0f);
        }
        if (ImGui::CollapsingHeader("Shape & Spawn Parameters"))
        {
            // Emitter shape
            static const char* shapeNames[] = {"Point","Sphere","Donut","Cone","Box","Mesh"};
            int shapeIdx = (int)emitter->shape;
            if(ImGui::Combo("Shape", &shapeIdx, shapeNames, IM_ARRAYSIZE(shapeNames))) {
                emitter->shape = (EmitterShape)shapeIdx;
            }
            ImGui::PushItemWidth(50);
            ImGui::DragFloat(" - ##size", (float*)&emitter->startSizeMin, 0.05f); ImGui::SameLine();
            ImGui::DragFloat("Spawn Size range", (float*)&emitter->startSizeMax, 0.05f);
            
            ImGui::DragFloat(" - ##lifetime", (float*)&emitter->lifetimeMin, 0.05f); ImGui::SameLine();
            ImGui::DragFloat("Lifetime range", (float*)&emitter->lifetimeMax, 0.05f);
            
            ImGui::DragFloat(" - ##rot", (float*)&emitter->rotationMin, 0.05f); ImGui::SameLine();
            ImGui::DragFloat("Rotation range", (float*)&emitter->rotationMax, 0.05f);
            
            ImGui::DragFloat(" - ##rotSpeed", (float*)&emitter->rotationSpeedMin, 0.05f); ImGui::SameLine();
            ImGui::DragFloat("Rotation Speed range", (float*)&emitter->rotationSpeedMax, 0.05f);
            
            ImGui::DragFloat(" - ##vel", (float*)&emitter->velocityScaleMin, 0.05f); ImGui::SameLine();
            ImGui::DragFloat("Random Velocity range", (float*)&emitter->velocityScaleMax, 0.05f);
            
            ImGui::DragFloat(" - ##alpha", (float*)&emitter->startAlphaMin, 0.05f); ImGui::SameLine();
            ImGui::DragFloat("Alpha range", (float*)&emitter->startAlphaMax, 0.05f);

            ImGui::PopItemWidth();

            ImGui::DragFloat3("Spawn Pos", (float*)&emitter->spawnPosition, 0.05f);
            ImGui::DragFloat3("Spawn Vel", (float*)&emitter->spawnVelocity, 0.05f);
        }

        if (ImGui::CollapsingHeader("Affectors"))
        {
            // Over-lifetime
            #pragma region AFFECTORS
            ImGui::Separator();
            ImGui::Text("Affectors:");
            static const char* affectorTypes[] = { "Acceleration", "FadeOverLife", "ScaleOverLife", "TowardsPoint", "AwayFromPoint", "DiePastAxis" };
            static int selectedAffType = 0;

            if(ImGui::BeginCombo("##affTypes", affectorTypes[selectedAffType]))
            {
                for (int i = 0; i < IM_ARRAYSIZE(affectorTypes); i++)
                {
                    bool isSelected = (selectedAffType == i);
                    if (ImGui::Selectable(affectorTypes[i], isSelected))
                    {
                        selectedAffType = i;
                    }
                    if (isSelected) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            ImGui::SameLine();
            if(ImGui::Button("Add Affector"))
            {
                Affector* newA = nullptr;
                if(selectedAffType == 0) // Acceleration
                {
                    newA = new AccelerationAffector(glm::vec3(0,0,0));
                }
                else if(selectedAffType == 1) // FadeOverLife
                {
                    newA = new FadeOverLifeAffector(1.0f, 0.0f);
                }
                else if(selectedAffType == 2) // ScaleOverLife
                {
                    newA = new ScaleOverLifeAffector(1.f, 2.f);
                }
                else if(selectedAffType == 3) // TowardsPoint
                {
                    newA = new TowardsPointAffector(glm::vec3(0,0,0), 1.0f);
                }
                else if(selectedAffType == 4) // AwayFromPoint
                {
                    newA = new AwayFromPointAffector(glm::vec3(0,0,0), 1.0f);
                }
                if(selectedAffType == 5) // DiePastAxis
                {
                    newA = new DiePastAxisAffector(1, -10.0f, true);
                }
                if(newA)
                {
                    emitter->AddAffector(newA);
                }
            }

            auto& affList = emitter->getAffectors();
            for (int i = 0; i < (int)affList.size(); i++)
            {
                ImGui::PushID(i);
                Affector* A = affList[i];
                std::string uniqueID = "##" + std::to_string(i);

                // DYNAMIC CAST HELL 
                if(auto* av = dynamic_cast<AccelerationAffector*>(A))
                {
                    ImGui::Text("AccelerationAffector");
                    float vel[3] = { av->velocityToAdd.x, av->velocityToAdd.y, av->velocityToAdd.z };
                    if(ImGui::DragFloat3("Acc##aff" , vel, 0.1f))
                    {
                        av->velocityToAdd = glm::vec3(vel[0], vel[1], vel[2]);
                    }
                    ImGui::Checkbox("Local to Node", &av->localToNode);
                }
                else if(auto* fo = dynamic_cast<FadeOverLifeAffector*>(A))
                {
                    ImGui::Text("FadeOverLifeAffector");
                    ImGui::DragFloat("Start Alpha", &fo->startAlpha, 0.01f, 0.f, 1.f);
                    ImGui::DragFloat("End Alpha", &fo->endAlpha, 0.01f, 0.f, 1.f);
                }
                else if(auto* so = dynamic_cast<ScaleOverLifeAffector*>(A))
                {
                    ImGui::Text("ScaleOverLifeAffector");
                    ImGui::DragFloat("Start Scale", &so->startScale, 0.01f, 0.f, 10.f);
                    ImGui::DragFloat("End Scale", &so->endScale, 0.01f, 0.f, 10.f);
                }
                else if(auto* tp = dynamic_cast<TowardsPointAffector*>(A))
                {
                    ImGui::Text("TowardsPointAffector");
                    float p[3] = { tp->target.x, tp->target.y, tp->target.z };
                    if(ImGui::DragFloat3("Target##tpa", p, 0.1f))
                    {
                        tp->target = glm::vec3(p[0], p[1], p[2]);
                    }
                    ImGui::DragFloat("Strength", &tp->strength, 0.1f, -999.f, 999.f);
                    ImGui::Checkbox("Local to Node", &tp->localToNode);
                }                                                           //ADD ABIlity to maybe set a node as pointAffector??? WOULD BE SICK WITH THE SUN
                else if(auto* aw = dynamic_cast<AwayFromPointAffector*>(A)) //OMG I JUST REALIZED AWAY FROM POINT IS JUST TOWARDS POINT BUT NEGATIVEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE FUCKING DUMBASS
                {
                    ImGui::Text("AwayFromPointAffector");
                    float c[3] = { aw->center.x, aw->center.y, aw->center.z };
                    if(ImGui::DragFloat3("Center##awa", c, 0.1f))
                    {
                        aw->center = glm::vec3(c[0], c[1], c[2]);
                    }
                    ImGui::DragFloat("Strength", &aw->strength, 0.1f, -999.f, 999.f);
                    ImGui::Checkbox("Local to Node", &aw->localToNode);
                }
                else if(auto* dpa = dynamic_cast<DiePastAxisAffector*>(A))
                {
                    ImGui::Text("DiePastAxisAffector");
                    // Combo box for axis selection:
                    const char* axisNames[] = { "X", "Y", "Z" };
                    int axisInt = dpa->axis;
                    if(ImGui::Combo("Axis", &axisInt, axisNames, IM_ARRAYSIZE(axisNames)))
                    {
                        dpa->axis = axisInt;
                    }
                    ImGui::DragFloat("Threshold", &dpa->threshold, 0.1f, -1000.f, 1000.f);
                    ImGui::Checkbox("Die if > Threshold", &dpa->greaterThan);
                    ImGui::Checkbox("Local to Node", &dpa->localToNode);
                }
                else
                {
                    ImGui::Text("Unknown Affector");
                }
                // ImGui::SameLine();
                if(ImGui::SmallButton("Remove##aff"))
                {
                    emitter->RemoveAffector(i);
                }
                ImGui::PopID();
            }
    #pragma endregion
        
            //Cplor gradient
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
        }
        if (ImGui::CollapsingHeader("Debug: Active Particles"))
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
    else if (auto* effect = dynamic_cast<EffectNode*>(g_selectedNode))
    {
        if(!g_selectedNode || !effect) return;

        #pragma region Saving & loadingg

        // Save
        if (ImGui::Button("Save Effect"))
        {
            ImGui::OpenPopup("SaveEffectPopup");
        }
        if (ImGui::BeginPopupModal("SaveEffectPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            static char effectFile[256] = "MyEffect.effect.yaml";
            ImGui::InputText("Effect File", effectFile, IM_ARRAYSIZE(effectFile));
            if (ImGui::Button("Save##Effect"))
            {
                std::string filename = std::string("data/Saved/") + effectFile;
                SceneSerializer::SaveEffectNode(effect, filename);
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel##Effect"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        // Load
        ImGui::SameLine();
        if (ImGui::Button("Load Effect"))
        {
            ImGui::OpenPopup("LoadEffectPopup");
        }

        if (ImGui::BeginPopupModal("LoadEffectPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            static char loadEffectFile[256] = "";
            ImGui::InputText("Effect File", loadEffectFile, IM_ARRAYSIZE(loadEffectFile));
            if (ImGui::Button("Load##Effect"))
            {
                
                std::string filename = std::string("data/Saved/") + loadEffectFile;
                
                // delete g_selectedNode;
                // g_selectedNode = nullptr;
                deleteNode(g_selectedNode);
                EffectNode* newFx = SceneSerializer::LoadEffectNode(Scene::Instance(), filename);
                g_selectedNode = newFx;

                // effect = newFx;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel##Effect"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        #pragma endregion
        ImGui::Separator();
        ImGui::Text("Effect Controls");
        if(ImGui::Button("Play")) { effect->Play(); }
        ImGui::SameLine();
        if(ImGui::Button("Stop")) { effect->Stop(); }
        ImGui::SameLine();
        if(ImGui::Button("Pause")) { effect->Pause(); }
        ImGui::SameLine();
        if(ImGui::Button("End")) { effect->End(); }
        ImGui::SameLine();
        if(ImGui::Button("Restart")) { effect->Restart(); }

        //Better performance?
        ImGui::Checkbox("Combine Draws", &effect->combineDraws);

        ImGui::Separator();
        ImGui::Text("Emitters inside this Effect:");

        for (auto* child : effect->getChildren())
        {
            auto* emitter = dynamic_cast<ParticleNode*>(child);
            if (!emitter) continue;

            bool isSelected = (emitter == g_selectedNode);
            
            if (ImGui::Selectable(emitter->getName().c_str(), false, 0, ImVec2(ImGui::GetContentRegionAvail().x - 60,0)))
            {
                g_selectedNode = emitter;
            }

            ImGui::SameLine();
            if(ImGui::SmallButton((std::string("Remove##") + emitter->getName()).c_str()))
            {
                effect->removeChild(emitter);
                emitter = nullptr; //Could be issue
                // Scene::Instance().RemoveNode(emitter); 
                break;
            }
        }

        if (ImGui::Button("Add Emitter"))
        {
            ParticleNode* newEmitter = new ParticleNode("Emitter_" + std::to_string(addedNode++));
            newEmitter->GetMaterial()->SetTexture("u_texture", volpe::TextureManager().CreateTexture("data/Textures/smoke.png"));

            effect->addEmitter(newEmitter);
        }
    }
    else if (auto* boidNode = dynamic_cast<BoidNode*>(g_selectedNode))
    {
        ImGui::Text("Boid Node Inspector");
        ImGui::Separator();

        // Edit overall BoidNode properties
        ImGui::DragInt("Max Boids", &boidNode->maxBoids, 1, 1, 10000);
        ImGui::DragFloat("World Bounds", &boidNode->worldBounds, 0.1f, 1.0f, 100.0f);
        // ImGui::SameLine();
        if (ImGui::Button("Load Texture"))
        {
            ImGui::OpenPopup("LoadTexturePopup");
        }

        if (ImGui::BeginPopupModal("LoadTexturePopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            static char loadFileName[256] = "";
            ImGui::InputText("Texture File", loadFileName, IM_ARRAYSIZE(loadFileName));
            if (ImGui::Button("Load##BoidTextureLoad"))
            {
                
                std::string filename = std::string("data/Textures/") + loadFileName;
                volpe::Texture* texture0 = volpe::TextureManager().CreateTexture(filename);
                if(texture0)
                boidNode->GetMaterial()->SetTexture("u_texture", texture0);
                
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel##BoidLoad"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::Checkbox("Wrap Around", &boidNode->wrapAround);
        ImGui::Checkbox("Alignment", &boidNode->doAlignment);
        ImGui::Checkbox("Cohesion", &boidNode->doCohesion);
        ImGui::Checkbox("Separation", &boidNode->doSeparation);
        
        // Display the current number of boids
        int currentCount = (int)boidNode->getBoids().size();
        ImGui::Text("Current Boids: %d", currentCount);
        
        // Allow spawning a new set of boids with a desired count.
        static int spawnCount = 10;
        ImGui::DragInt("Spawn Count", &spawnCount, 1, 1, 1000);
        if (ImGui::Button("Respawn Boids"))
        {
            boidNode->SpawnBoids(spawnCount);
        }
        
        ImGui::Separator();

        if (ImGui::CollapsingHeader("Boid Default Parameters", ImGuiTreeNodeFlags_DefaultOpen))
        {
            static float defaultMaxSpeed      = 5.0f;
            static float defaultMaxForce      = 0.25f;
            static float defaultNeighborRadius = 4.0f;
            static float defaultSeparationDist = 1.0f;
            static float defaultSize          = 0.5f;
            static float defaultRotation      = 0.0f;
            
            ImGui::PushItemWidth(100.0f);
            if (ImGui::DragFloat("Default Max Speed",       &defaultMaxSpeed, 0.1f, 0.1f, 100.0f)){}
            if (ImGui::DragFloat("Default Max Force",       &defaultMaxForce, 0.01f, 0.01f, 10.0f)){}
            if (ImGui::DragFloat("Default Neighbor Radius", &defaultNeighborRadius, 0.1f, 0.1f, 100.0f)){}
            if (ImGui::DragFloat("Default Separation Dist", &defaultSeparationDist, 0.1f, 0.1f, 100.0f)){}
            if (ImGui::DragFloat("Default Boid Size",       &defaultSize, 0.01f, 0.1f, 10.0f)){}
            if (ImGui::DragFloat("Default Boid Rotation",   &defaultRotation, 0.1f, -360.0f, 360.0f)){}
            if (ImGui::Button("Apply Defaults to Existing Boids"))
            {
                //Had to make list of boids public but will refactor later
                for (auto& b : boidNode->m_boids) {
                    b.maxSpeed       = defaultMaxSpeed;
                    b.maxForce       = defaultMaxForce;
                    b.neighborRadius = defaultNeighborRadius;
                    b.separationDist = defaultSeparationDist;
                    b.size           = defaultSize;
                    b.rotation       = defaultRotation;
                }
            }
            ImGui::PopItemWidth();
        }
    }

    //=-=-=-=- REPARENTING AT RUNTIMEE -=-=-=-=-=
    ImGui::Separator();
    if (ImGui::CollapsingHeader("Reparent"))
    {
        if (ImGui::Button("Set as Root"))
        {
            if (g_selectedNode->getParent())
                g_selectedNode->getParent()->removeChild(g_selectedNode);
        }
        
        //list all possible parent nodes
        std::vector<Node*> allNodes = Scene::Instance().GetNodes();
        std::vector<Node*> possibleParents;
        std::vector<std::string> parentNames;
        for (Node* n : allNodes)
        {
            //remove any current nodes children and self from lsit
            if (n == g_selectedNode || IsChild(n, g_selectedNode))
                continue;
            possibleParents.push_back(n);
            parentNames.push_back(n->getName());
        }
        
        static int selectedParentIndex = -1;
        if (!parentNames.empty())
        {
            const char* comboLabel = (selectedParentIndex >= 0 && selectedParentIndex < parentNames.size()) ? parentNames[selectedParentIndex].c_str() : "None";

            if (ImGui::BeginCombo("New Parent", comboLabel))
            {
                for (int i = 0; i < parentNames.size(); i++)
                {
                    bool isSelected = (selectedParentIndex == i);
                    if (ImGui::Selectable(parentNames[i].c_str(), isSelected))
                        selectedParentIndex = i;
                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            
            if (selectedParentIndex >= 0 && selectedParentIndex < possibleParents.size())
            {
                if (ImGui::Button("Make Child Of Selected Parent"))
                {
                    // Remove child's parents 
                    if (g_selectedNode->getParent())
                        g_selectedNode->getParent()->removeChild(g_selectedNode);
                    // Add node as child of our node
                    possibleParents[selectedParentIndex]->addChild(g_selectedNode);
                }
            }
        }
        else
        {
            ImGui::Text("No valid parent nodes available");
        }
    }

    ImGui::End();
}

void Program::DrawTopBar()
{
    static bool open_save_popup = false;
    static bool open_load_popup = false;

    if (ImGui::BeginMainMenuBar())
    {             
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Save Scene"))
                open_save_popup = true;
            if (ImGui::MenuItem("Load Scene"))
                open_load_popup = true;
            if (ImGui::MenuItem("Quick save and Exit"))
                SceneSerializer::SaveScene(Scene::Instance(), "data/Saved/autosave");
                
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Add"))
        {
            if (ImGui::MenuItem("Cube"))
            {
                DebugCube* cube = new DebugCube("cube_" + to_string(addedNode++));
                Scene::Instance().AddNode(cube);
                g_selectedNode = cube;
            }
            if (ImGui::MenuItem("Sphere"))
            {
                DebugSphere* sphere = new DebugSphere("sphere_" + to_string(addedNode++), 0.5);
                Scene::Instance().AddNode(sphere);
                g_selectedNode = sphere; //for sexy convenience
            }
            if (ImGui::MenuItem("Light"))
            {
                LightNode* light = new LightNode("light_" + to_string(addedNode++), vec3(1),5,5);
                Scene::Instance().AddNode(light);
                g_selectedNode = light;
            }
            if (ImGui::MenuItem("Particle Node"))
            {
                ParticleNode* Emitter = new ParticleNode("ParticleSystemNode_" + to_string(addedNode++));
                volpe::Texture* texture0 = volpe::TextureManager().CreateTexture("data/Textures/smoke.png");
                Emitter->GetMaterial()->SetTexture("u_texture", texture0);
                Scene::Instance().AddNode(Emitter);
                g_selectedNode = Emitter;
            }
            if (ImGui::MenuItem("Effect"))
            {
                // Creates empty effect, Add button for loading effect from file l8r
                EffectNode* effect = new EffectNode("Effect_" + to_string(addedNode++));
                Scene::Instance().AddNode(effect);
                g_selectedNode = effect;
            }
            if (ImGui::MenuItem("BOIDSS"))
            {
                BoidNode* boid = new BoidNode("BoidNode_" + to_string(addedNode++));
                Scene::Instance().AddNode(boid);
                g_selectedNode = boid;
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::MenuItem("New Scene"))
        {
            SwitchScene(SceneType::Random);
            Scene::Instance().Clear();
            Scene::Instance().BuildOctTree();
        }
        if (ImGui::BeginMenu("Solar System"))
        {
            if (ImGui::MenuItem("Default solar system"))
            {
                SwitchScene(SceneType::SolarSystem);
            }
            if (ImGui::MenuItem("COOL SHIT solar system"))
            {
                SwitchScene(SceneType::SolarSystem);
                EffectNode* effect = new EffectNode("Effect_" + to_string(addedNode++));
                Scene::Instance().AddNode(effect);
                effect = SceneSerializer::LoadEffectNode(Scene::Instance(), "data/Saved/dunno.effect.yaml");
                Scene::Instance().ReBuildTree();
            }
            if (ImGui::MenuItem("Astroids included?"))
            {
                SwitchScene(SceneType::SolarSystem);
                BuildAsteroidField(200, 25.0f, 40.0f);
            }
            ImGui::EndMenu();
        }
        if (ImGui::MenuItem("Random Scene"))
            SwitchScene(SceneType::Random);
        if (ImGui::MenuItem("Particle Scene"))
            SwitchScene(SceneType::Particle);
        if (ImGui::MenuItem("Fire Scene"))
        {
            SwitchScene(SceneType::Particle);
            SceneSerializer::LoadScene(Scene::Instance(), "data/Saved/firescene");
            Scene::Instance().ReBuildTree();
        }
        if (ImGui::MenuItem("Rain Scene"))
        {
            SwitchScene(SceneType::Particle);
            SceneSerializer::LoadScene(Scene::Instance(), "data/Saved/rain1");
            Scene::Instance().ReBuildTree();
        }
        if (ImGui::MenuItem("Magic spell"))
        {
            SwitchScene(SceneType::Particle);
            SceneSerializer::LoadScene(Scene::Instance(), "data/Saved/magicspell");
            Scene::Instance().ReBuildTree();
        }
        if (ImGui::MenuItem("Jet"))
        {
            SwitchScene(SceneType::Particle);
            SceneSerializer::LoadScene(Scene::Instance(), "data/Saved/jet");
            Scene::Instance().ReBuildTree();
        }



        ImGui::EndMainMenuBar();
    }

    if (open_save_popup)
    {
        ImGui::OpenPopup("Save Scene");
        open_save_popup = false;
    }
    if (open_load_popup)
    {
        ImGui::OpenPopup("Load Scene");
        open_load_popup = false;
    }

    // --- Save Scene Popup ---
    if (ImGui::BeginPopupModal("Save Scene", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        static char saveFileName[256] = "";
        ImGui::Text("Enter file name to save:");
        ImGui::InputText("File Name", saveFileName, IM_ARRAYSIZE(saveFileName));
        if (ImGui::Button("Save", ImVec2(120, 0)))
        {
            std::string filename(saveFileName);
            filename = "data/Saved/" + filename;
            SceneSerializer::SaveScene(Scene::Instance(), filename);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    // --- Load Scene Popup ---
    if (ImGui::BeginPopupModal("Load Scene", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        static char loadFileName[256] = "";
        ImGui::Text("Enter file name to load:");
        ImGui::InputText("File Name", loadFileName, IM_ARRAYSIZE(loadFileName));
        if (ImGui::Button("Load", ImVec2(120, 0)))
        {
            std::string filename(loadFileName);
            filename = "data/Saved/" + filename;
            SceneSerializer::LoadScene(Scene::Instance(), filename);
            Scene::Instance().ReBuildTree();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
}

void Program::DrawPerformanceGraphs() {
    {
        m_cameraUpdateTimes[m_perfBufferIndex]       = Scene::Instance().getAvgCameraUpdateMs();
        m_nodeUpdateTimes[m_perfBufferIndex]         = Scene::Instance().getAvgNodeUpdateMs();
        m_boundingVolumeTimes[m_perfBufferIndex]     = Scene::Instance().getAvgBoundingVolumeMs();
        m_TreeBuildTime[m_perfBufferIndex]           = Scene::Instance().getTreeBuildTime();
        m_treeQueryTimes[m_perfBufferIndex]          = Scene::Instance().getAvgQuadTreeQueryMs();
        m_lightQueryTimes[m_perfBufferIndex]         = Scene::Instance().getAvgLightQuery();
        m_perfBufferIndex = (m_perfBufferIndex + 1) % kPerfBufferSize;
    }
    ImGui::Begin("Performance Graphs");

    // Adjusted graph height and range for clearer visualization
    const ImVec2 graphSize(0, 50);  // Increase height to 200 pixels
    float graphMin = 0.0f;
    float graphMax = 5.0f;

    ImGui::Text("Camera Update (ms)");
    ImGui::PlotLines("##Camera", m_cameraUpdateTimes, kPerfBufferSize, m_perfBufferIndex, nullptr, graphMin, graphMax, graphSize);

    ImGui::Text("Node Update (ms)");
    ImGui::PlotLines("##Nodes", m_nodeUpdateTimes, kPerfBufferSize, m_perfBufferIndex, nullptr, graphMin, graphMax, graphSize);

    ImGui::Text("Bounding Volumes (ms)");
    ImGui::PlotLines("##BV", m_boundingVolumeTimes, kPerfBufferSize, m_perfBufferIndex, nullptr, graphMin, graphMax, graphSize);

    ImGui::Text("Tree Building (ms)");
    ImGui::PlotLines("##TreeBuild", m_TreeBuildTime, kPerfBufferSize, m_perfBufferIndex, nullptr, graphMin, graphMax, graphSize);

    ImGui::Text("Tree Query (ms)");
    ImGui::PlotLines("##TreeQuery", m_treeQueryTimes, kPerfBufferSize, m_perfBufferIndex, nullptr, graphMin, graphMax, graphSize);

    ImGui::Text("Light Query (ms)");
    ImGui::PlotLines("##LightQuery", m_lightQueryTimes, kPerfBufferSize, m_perfBufferIndex, nullptr, graphMin, graphMax, graphSize);

    ImGui::End();
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
    glfwSwapInterval(0);
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

    Scene::Instance().SetActiveCamera(orbitCamera);
    
    Scene::Instance().InitLights();
    Scene::Instance().showGrid = false;
    rebuildTreeEveryFrame = true;

    // buildParticleScene();
    // BuildSolarSystem(bounds);
    RecreateSceneHelper(bounds);

    Scene& scene = Scene::Instance();
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

    DrawPerformanceGraphs();

    if(rebuildTreeEveryFrame)
        Scene::Instance().ReBuildTree();

    // if(m_pApp->isKeyJustDown('J') || m_pApp->isKeyJustDown('j'))
    // {
    //     // buildParticleScene();
    //     Scene::Instance().Clear();
    //     SceneSerializer::LoadScene(Scene::Instance() ,"autosave");
    //     Scene::Instance().ReBuildTree();
    // }
    // if(m_pApp->isKeyJustDown('H') || m_pApp->isKeyJustDown('h'))
    // {
    //     Scene& scene = Scene::Instance();
    //     SceneSerializer::SaveScene(scene, "autosave");
    // }

    if(solarSystem)
    {
        //Toggle rotation axis X Y Z 
        // if(m_pApp->isKeyJustDown('I')) {
        //     if(OrbitAxis.x == 1)
        //         OrbitAxis.x = 0;
        //     else
        //         OrbitAxis.x = 1;
        // }
        // if(m_pApp->isKeyJustDown('O')) {
        //     if(OrbitAxis.y == 1)
        //         OrbitAxis.y = 0;
        //     else
        //         OrbitAxis.y = 1;
        // }
        // if(m_pApp->isKeyJustDown('P')) {
        //     if(OrbitAxis.z == 1)
        //         OrbitAxis.z = 0;
        //     else
        //         OrbitAxis.z = 1;
        // }
        // if(m_pApp->isKeyJustDown('J')) 
        //     speedMultipler *= 0.8;
    
        // if(m_pApp->isKeyJustDown('K')) 
        //     speedMultipler *= 1.2;
        
        UpdateSolarSystem(dt);
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

    if(m_pApp->isKeyJustDown(GLFW_KEY_RIGHT_ALT)) { //switch cameras
        whichCamera = !whichCamera;
        if(whichCamera)
            Scene::Instance().SetActiveCamera(orbitCamera);
        else
            Scene::Instance().SetActiveCamera(fpsCamera);
    }
    
    Scene::Instance().Update(dt, m_pApp->getScreenSize().x, m_pApp->getScreenSize().y);
}

void Program::draw(int width, int height)
{
    // if(solarSystem)
    //     glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    // else
    //     glClearColor(0.0f, 0.5f, 0.5f, 0.0f);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Scene::Instance().Render(width, height);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

#include "SceneManager.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

SceneManager::SceneManager(volpe::App* pApp)
: m_root(nullptr), m_pApp(pApp)
{
    // Create the scene root node
    m_root = new Node("root");
}

SceneManager::~SceneManager()
{
    if (m_root) {
        delete m_root;
        m_root = nullptr;
    }
}

// Create a full scene with hierarchical DebugCubes and lights
void SceneManager::initScene()
{
    m_pGrid = new Grid3D(30);

    m_pOrbitCam = new OrbitCamera(m_pApp);
    m_pOrbitCam->focusOn(glm::vec3(-10.0f,-10.0f,-10.0f),glm::vec3(10.0f,10.0f,10.0f));

    DebugCube* parentCube = new DebugCube("parentCube");
    parentCube->setTransform(glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0)));
    m_root->addChild(parentCube);
    m_allNodes.push_back(parentCube);

    DebugCube* childCube1 = new DebugCube("childCube1");
    DebugCube* childCube2 = new DebugCube("childCube2");
    DebugCube* childCube3 = new DebugCube("childCube2");

    childCube1->setTransform(glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, 0.0f))); // Right
    childCube2->setTransform(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 2.0f, 0.0f))); // Up
    childCube3->setTransform(glm::translate(glm::mat4(1.0f), glm::vec3(10.0f, 0.0f, 0.0f))); // Further rightt

    parentCube->addChild(childCube1);
    parentCube->addChild(childCube2);
    parentCube->addChild(childCube3);

    m_allNodes.push_back(childCube1);
    m_allNodes.push_back(childCube2);
    m_allNodes.push_back(childCube3);

    for (int i = 2; i < 5; ++i)
    {
        DebugCube* floatingCube = new DebugCube("floatingCube_" + std::to_string(i));
        float x = (float)((i % 2 == 0) ? i * 3 : -i * 3);
        float y = (float)((i % 3 == 0) ? i : -i);
        float z = (float)((i % 2 == 0) ? -i * 2 : i * 2);

        floatingCube->setTransform(glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z)));
        m_root->addChild(floatingCube);
        m_allNodes.push_back(floatingCube);
    }

    m_lights.push_back( Light(glm::vec3(0, 5, 5),  glm::vec3(1,1,1), 1.0f) );
    m_lights.push_back( Light(glm::vec3(5, 5, 0),  glm::vec3(1,0,0), 1.0f) );
    m_lights.push_back( Light(glm::vec3(-5, 5, 0), glm::vec3(0,1,0), 1.0f) );
    m_lights.push_back( Light(glm::vec3(0, 5, -5), glm::vec3(0,0,1), 1.0f) );

    m_root->updateBoundingVolume();
}

void SceneManager::update(float dt)
{
    m_pOrbitCam->update(dt);
    m_pGrid->update(dt);

    Node* parentCube = nullptr;
    for (auto* n : m_allNodes) {
        if (n->getName() == "parentCube") {
            parentCube = n;
            break;
        }
    }
    if (parentCube) {
        glm::mat4 curTransform = parentCube->getTransform();
        glm::mat4 rotation = glm::rotate(curTransform, dt * 0.5f, glm::vec3(0, 1, 0));
        parentCube->setTransform(rotation);
    }

    m_root->update(dt);
    m_root->updateBoundingVolume();
}

void SceneManager::draw(int width, int height)
{
	glm::mat4 mProj = m_pOrbitCam->getProjMatrix(width, height);
	glm::mat4 mView = m_pOrbitCam->getViewMatrix();

    m_root->draw(mProj, mView);
    m_pGrid->render(mView,mProj);
}








/*#include "SceneManager.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

SceneManager::SceneManager()
: m_root(nullptr)
{
    // Create an empty root node
    m_root = new Node("root");
}

SceneManager::~SceneManager()
{
    if (m_root) {
        delete m_root;
        m_root = nullptr;
    }
}

// Create some cubes (including child relationships) + some lights
void SceneManager::initScene()
{
    // 1) Create a few DebugCubes
    DebugCube* parentCube = new DebugCube("parentCube");
    m_root->addChild(parentCube);
    m_allNodes.push_back(parentCube);

    // Make 2 child cubes
    DebugCube* childCube1 = new DebugCube("childCube1");
    DebugCube* childCube2 = new DebugCube("childCube2");
    parentCube->addChild(childCube1);
    parentCube->addChild(childCube2);
    m_allNodes.push_back(childCube1);
    m_allNodes.push_back(childCube2);

    // Set transforms
    // parentCube is at origin, childCube1 is a bit to the right, childCube2 is up
    childCube1->setTransform(glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.f, 0.f)));
    childCube2->setTransform(glm::translate(glm::mat4(1.0f), glm::vec3(0.f, 2.0f, 0.f)));

    // 2) Optionally create many cubes (like 32) to show multiple lights
    for (int i = 0; i < 5; ++i) // just 5 extra for demonstration
    {
        DebugCube* c = new DebugCube("extraCube_" + std::to_string(i));
        m_root->addChild(c);
        m_allNodes.push_back(c);

        float x = (float)(i * 3);
        float z = (float)((i%2)*5); 
        glm::mat4 t = glm::translate(glm::mat4(1.0f), glm::vec3(x, 0.f, z));
        c->setTransform(t);
    }

    // 3) Add lights. We can do up to 16 or more. 
    // Let’s make a few lights scattered around
    m_lights.push_back( Light(glm::vec3(0,5,5),  glm::vec3(1,1,1), 1.0f) );
    m_lights.push_back( Light(glm::vec3(5,5,0),  glm::vec3(1,0,0), 1.0f) );
    m_lights.push_back( Light(glm::vec3(-5,5,0), glm::vec3(0,1,0), 1.0f) );
    m_lights.push_back( Light(glm::vec3(0,5,-5), glm::vec3(0,0,1), 1.0f) );

    // You could add up to 16 lights, or 32. For demonstration, let's keep it small.
}

// Example update: rotate the “parentCube” a bit, update bounding volumes
void SceneManager::update(float dt)
{
    // Spin the parentCube
    Node* parentCube = nullptr;
    for (auto* n : m_allNodes) {
        if (n->getName() == "parentCube") {
            parentCube = n; 
            break;
        }
    }
    if (parentCube) {
        glm::mat4 cur = parentCube->getTransform();
        // rotate around Y axis
        glm::mat4 spin = glm::rotate(cur, dt, glm::vec3(0,1,0));
        parentCube->setTransform(spin);
    }

    m_root->update(dt);
    m_root->updateBoundingVolume();
}

// A simple “draw everything,” ignoring how many lights are used
void SceneManager::draw(const glm::mat4& viewProj)
{
    m_root->draw(viewProj);
}

// This variant shows how you might do a single-light approach
// (You would, in practice, bind a “light” shader that has uniform uLightPos, etc.)
void SceneManager::drawSingleLight(const glm::mat4& viewProj)
{
    if (m_lights.empty()) {
        // fallback if no lights
        draw(viewProj);
        return;
    }

    // Let’s pick the first light in the array as our single light
    Light mainLight = m_lights[0];

    // Bind your single-light shader, set uniform(s)...
    // pseudo-code:
    //   singleLightShader->use();
    //   singleLightShader->setVec3("uLightPos", mainLight.position);
    //   singleLightShader->setVec3("uLightColor", mainLight.color);
    //   singleLightShader->setFloat("uLightIntensity", mainLight.intensity);

    // Then draw all nodes
    draw(viewProj);
}

// This variant picks the 2 nearest lights to each Node, sets them as uniforms, then draws
void SceneManager::drawTwoLightsPerObject(const glm::mat4& viewProj)
{
    // pseudo‐code, you’d have a 2‐light shader that expects:
    //   uniform LightInfo uLight1;  uniform LightInfo uLight2;
    //   struct LightInfo { vec3 position; vec3 color; float intensity; };

    // twoLightShader->use();

    // For each node in the scene
    for (auto* node : m_allNodes)
    {
        std::vector<Light> chosen = pickLightsForNode(node);

        // Set uniforms for the node’s best 2 lights
        // e.g. 
        // twoLightShader->setVec3("uLight1.position",  chosen[0].position);
        // twoLightShader->setVec3("uLight1.color",     chosen[0].color);
        // twoLightShader->setFloat("uLight1.intensity",chosen[0].intensity);
        //
        // twoLightShader->setVec3("uLight2.position",  chosen[1].position);
        // twoLightShader->setVec3("uLight2.color",     chosen[1].color);
        // twoLightShader->setFloat("uLight2.intensity",chosen[1].intensity);

        // Then call node->draw(...) individually
        node->draw(viewProj);
    }
}

// Return up to 2 closest lights for the node based on bounding‐sphere center
std::vector<Light> SceneManager::pickLightsForNode(const Node* node)
{
    std::vector<Light> results;
    if (m_lights.empty()) {
        return results;
    }

    // World-space center of node’s bounding sphere
    glm::vec3 centerLocal = node->getBoundingSphere().center;
    glm::vec4 centerWorld4 = node->getWorldTransform() * glm::vec4(centerLocal, 1.0f);
    glm::vec3 centerWorld = glm::vec3(centerWorld4);

    // measure distance to each light
    struct LightDist {
        float dist;
        Light light;
    };
    std::vector<LightDist> lightDists;
    for (auto& L : m_lights)
    {
        float d = glm::length(L.position - centerWorld);
        lightDists.push_back({ d, L });
    }
    // sort ascending
    std::sort(lightDists.begin(), lightDists.end(), 
        [](const LightDist& a, const LightDist& b) {
            return a.dist < b.dist;
        }
    );

    // pick up to 2
    int maxLights = 2;
    for (int i = 0; i < maxLights && i < (int)lightDists.size(); ++i) {
        results.push_back(lightDists[i].light);
    }
    return results;
}
*/
#include "Program.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <iostream>

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
}

void Program::init()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    
    fpsCamera = new FirstPersonCamera(m_pApp);

    orbitCamera = new OrbitCamera(m_pApp);
    orbitCamera->focusOn(glm::vec3(-10.0f,-10.0f,-10.0f),glm::vec3(10.0f,10.0f,10.0f));

    Scene::Instance().SetActiveCamera(orbitCamera);

    Scene::Instance().RandomInitScene();

}

void Program::update(float dt)
{
    vector<Node*> nodes = Scene::Instance().GetNodes();

    for (auto* n : nodes) {
        if (n->getName() == "parentCube") {
            
            glm::mat4 rotation = glm::rotate(n->getTransform(), dt * 0.6f, glm::vec3(1, 1, 1));
            n->setTransform(rotation);
        }
        else if (n->getName() == "childCube1") {
            
            glm::mat4 rotation = glm::rotate(n->getTransform(), dt * 0.5f, glm::vec3(0, 1, 0));
            n->setTransform(rotation);
        }
        else if (n->getName() == "childCube2") {
            
            glm::mat4 rotation = glm::rotate(n->getTransform(), dt * 0.4f, glm::vec3(1, 1, 1));
            n->setTransform(rotation);
        }
    }

    Scene::Instance().Update(dt);
}

void Program::draw(int width, int height)
{
    Scene::Instance().Render(width, height);
}

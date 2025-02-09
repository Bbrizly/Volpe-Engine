#include "Scene.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>


Scene& Scene::Instance() {
    static Scene instance;
    return instance;
}

Scene::Scene()
{
    // m_root = new Node("root");
}

Scene::~Scene()
{
    if (!m_nodes.empty())
        m_nodes.clear();
    
    if(!m_objectsToRender.empty())
        m_objectsToRender.clear();
}

void Scene::AddNode(Node* node) {
    if(node)
        m_nodes.push_back(node);
}

void Scene::SetActiveCamera(Camera* cam) {
    m_activeCamera = cam;
}

void Scene::RandomInitScene()
{
    m_pGrid = new Grid3D(30);

    // m_pOrbitCam->focusOn(glm::vec3(-10.0f,-10.0f,-10.0f),glm::vec3(10.0f,10.0f,10.0f));

    DebugCube* parentCube = new DebugCube("parentCube");
    parentCube->setTransform(glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0)));

    DebugCube* childCube1 = new DebugCube("childCube1");
    DebugCube* childCube2 = new DebugCube("childCube2");
    DebugCube* childCube3 = new DebugCube("childCube2");

    childCube1->setTransform(glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, 0.0f))); // Right
    childCube2->setTransform(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 2.0f, 0.0f))); // Up
    childCube3->setTransform(glm::translate(glm::mat4(1.0f), glm::vec3(10.0f, 0.0f, 0.0f))); // Further rightt

    parentCube->addChild(childCube1);
    childCube1->addChild(childCube2);
    childCube2->addChild(childCube3);

    AddNode(parentCube);
    AddNode(childCube1);
    AddNode(childCube2);
    AddNode(childCube3);

    for (int i = 2; i < 5; ++i)
    {
        DebugCube* floatingCube = new DebugCube("floatingCube_" + std::to_string(i));
        float x = (float)((i % 2 == 0) ? i * 3 : -i * 3);
        float y = (float)((i % 3 == 0) ? i : -i);
        float z = (float)((i % 2 == 0) ? -i * 2 : i * 2);

        floatingCube->setTransform(glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z)));
        AddNode(floatingCube);
    }

    m_lights.push_back( Light(glm::vec3(0, 5, 5),  glm::vec3(1,1,1), 1.0f) );
    m_lights.push_back( Light(glm::vec3(5, 5, 0),  glm::vec3(1,0,0), 1.0f) );
    m_lights.push_back( Light(glm::vec3(-5, 5, 0), glm::vec3(0,1,0), 1.0f) );
    m_lights.push_back( Light(glm::vec3(0, 5, -5), glm::vec3(0,0,1), 1.0f) );

}

void Scene::Update(float dt)
{
    m_activeCamera->update(dt);


    for(auto node : m_nodes)
        node->update(dt);

    for(auto node : m_nodes)
        node->updateBoundingVolume();
}

void Scene::Render(int width, int height)
{
	glm::mat4 mProj = m_activeCamera->getProjMatrix(width, height);
	glm::mat4 mView = m_activeCamera->getViewMatrix();
	
    for(auto node : m_nodes)
        node->draw(mProj, mView);

    m_pGrid->render(mView,mProj);
}

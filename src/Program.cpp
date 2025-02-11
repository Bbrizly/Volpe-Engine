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
}

int amount = 5; //AMOUTN TEMPORORARY DELETE LATEERRRRR 

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

    Scene::Instance().ShowDebugText();
    
    Scene::Instance().InitLights();

    Scene::Instance().RandomInitScene(amount);

    // Scene::Instance().BuildQuadTree();
    Scene::Instance().BuildOctTree();

}

void Program::update(float dt)
{
    //DEBUGGING THE TEXT POSITIONS CUZ ITS SHIT
    //looks funny keeping for now
    if(m_pApp->isKeyDown('W') || m_pApp->isKeyJustDown('s')) //Move FPS counter
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        // -640.0f, 360.0f
        std::uniform_real_distribution<float> distPos(0.0f, 40.0f);

        Scene::Instance().setTextBoxPos(-640.0f + distPos(gen), 360.0f - distPos(gen));
    }

    if (m_pApp->isKeyJustDown('F') || m_pApp->isKeyJustDown('f')) {
        Scene::Instance().ToggleUseDebugFrustum();
    }


    if(m_pApp->isKeyJustDown('R') || m_pApp->isKeyJustDown('r'))
    {
        Scene::Instance().Clear();
        Scene::Instance().RandomInitScene(amount);
        if(Scene::Instance().getWhichTree())
            Scene::Instance().BuildQuadTree();
        else
            Scene::Instance().BuildOctTree();
    }

    if(m_pApp->isKeyJustDown('Q') || m_pApp->isKeyJustDown('q'))
    {
        Scene::Instance().ToggleQuadTreeRender();
    }

    if(m_pApp->isKeyJustDown('A') || m_pApp->isKeyJustDown('a'))
    {
        //Toggle between quadtree and octtree
        if(Scene::Instance().getWhichTree()) //true is quad, false is oct. here theyre swapped to switch
            Scene::Instance().BuildOctTree();
        else
            Scene::Instance().BuildQuadTree();
    }
    
    if(m_pApp->isKeyJustDown('L')) {
        // random move lights
        Scene::Instance().RandomMoveLights(); //move each light randomly
    }

    // vector<Node*> nodes = Scene::Instance().GetNodes();
    // for (auto* n : nodes) {
    //     if (n->getName() == "parentCube") {
    //         glm::mat4 rotation = glm::rotate(n->getTransform(), dt * 0.6f, glm::vec3(1, 1, 1));
    //         n->setTransform(rotation);
    //     }
    //     else if (n->getName() == "childCube1") {
    //         glm::mat4 rotation = glm::rotate(n->getTransform(), dt * 0.5f, glm::vec3(0, 1, 0));
    //         n->setTransform(rotation);
    //     }
    //     else if (n->getName() == "childCube2") {
    //         glm::mat4 rotation = glm::rotate(n->getTransform(), dt * 0.4f, glm::vec3(1, 1, 1));
    //         n->setTransform(rotation);
    //     }
    // }

    Scene::Instance().Update(dt, m_pApp->getScreenSize().x, m_pApp->getScreenSize().y);
}

void Program::draw(int width, int height)
{
    Scene::Instance().Render(width, height);
}

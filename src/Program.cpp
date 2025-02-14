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

int amount = 100; //AMOUTN TEMPORORARY DELETE LATEERRRRR 

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

    Scene::Instance().SetActiveCamera(fpsCamera);

    Scene::Instance().ShowDebugText();
    
    Scene::Instance().InitLights();

    Scene::Instance().RandomInitScene(amount);

    Scene::Instance().BuildQuadTree();
    // Scene::Instance().BuildOctTree();

}

void Program::update(float dt)
{
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

    if (m_pApp->isKeyJustDown('F') || m_pApp->isKeyJustDown('f')) {
        Scene::Instance().ToggleUseDebugFrustum(fpsCamera);
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

    if(m_pApp->isKeyJustDown('E') || m_pApp->isKeyJustDown('e'))
    {
        //Toggle between quadtree and octtree
        if(Scene::Instance().getWhichTree()) //true is quad, false is oct. here theyre swapped to switch
            Scene::Instance().BuildOctTree();
        else
            Scene::Instance().BuildQuadTree();
    }
    
    if(m_pApp->isKeyJustDown('L')) { //HOLD L TO MOVE LIGHTS
        // random move lights
        Scene::Instance().MoveLights();
    }

    if(m_pApp->isKeyJustDown('C')) { //switch cameras
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
    Scene::Instance().Render(width, height);
}

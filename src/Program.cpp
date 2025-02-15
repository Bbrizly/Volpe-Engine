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

int amount = 200; //AMOUTN TEMPORORARY DELETE LATEERRRRR 

void RecreateSceneHelper(int bounds)
{
    Scene::Instance().createGrid(bounds);
    // /* GRID 
    int gridSize = std::ceil(std::cbrt(amount)); // Determine the grid dimensions (N x N x N)
    float spacing = (2.0f * bounds) / gridSize; // Adjust spacing to fit within bounds

    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<float> rgb(0.0f, 255.0f);
    /*
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
                
                GLubyte r = rgb(gen)
                       ,g = rgb(gen)
                       ,b = rgb(gen);
                cube->setColor(r,g,b);

                Scene::Instance().AddNode(cube); // Add cube to scene
                cubeCount++;
            }
        }
    }
    */
    int cubeCount = 0;
    for (int     x = 0; x < gridSize && cubeCount < amount; x++) {
        for (int y = 0; y < gridSize && cubeCount < amount; y++) {
            for (int z = 0; z < gridSize && cubeCount < amount; z++) {
                
                float posX = -bounds + x * spacing;
                float posY = -bounds + y * spacing;
                float posZ = -bounds + z * spacing;

                glm::vec3 position = glm::vec3(posX, posY, posZ);

                DebugCube* cube = new DebugCube("Cube_" + to_string(cubeCount));
                cube->setTransform(glm::translate(glm::mat4(1.0f), position));

                // std::cout << "Cube_" << cubeCount << " location: ("
                //           << trans.x << ", " << trans.y << ", " << trans.z << ")\n";
                
                // std::cout << " worldTransform location: ("
                //     << transa <<")\n";

                GLubyte r = rgb(gen)
                ,g = rgb(gen)
                ,b = rgb(gen);
                cube->setColor(r,g,b);

                Scene::Instance().AddNode(cube); // Add cube to scene
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

    Scene::Instance().SetActiveCamera(fpsCamera);

    Scene::Instance().ShowDebugText();
    
    Scene::Instance().InitLights();

    bounds = 30;
    Scene::Instance().RandomInitScene(amount);
    /////////////////////////////////////////////////////////

    // RecreateSceneHelper(bounds);

    /////////////////////////////////////////////////////////

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
        // RecreateSceneHelper(bounds);
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

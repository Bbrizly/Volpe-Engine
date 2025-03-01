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
    /* GRID like pattern
    int gridSize = std::ceil(std::cbrt(amount)); // Determine the grid dimensions (N x N x N)
    float spacing = (2.0f * bounds) / gridSize; // Adjust spacing to fit within bounds

    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<float> rgb(0.0f, 255.0f);
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


    */

    // /*
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<float> distPos(-bounds, bounds);
    uniform_real_distribution<float> rgb(0.0f, 255.0f);

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
    // We'll place asteroids in random orbits between [innerRadius, outerRadius].
    // You can tweak these as you like.

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


        // Position the asteroid
        glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z));
        asteroid->setTransform(T);

        // Very important: add to Scene so culling will see each asteroid.
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

    BuildAsteroidField(200, 25.0f, 40.0f);
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
#pragma endregion
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

    // RecreateSceneHelper(bounds);
    BuildSolarSystem(bounds);
    // Scene::Instance().BuildQuadTree();
    Scene::Instance().BuildOctTree();

}

void Program::update(float dt)
{
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
        // Scene::Instance().BuildOctTree();  //REAL TIME CULLING OF MOVING OBJECTS!!! :D
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

    if (m_pApp->isKeyJustDown('F') || m_pApp->isKeyJustDown('f')) {
        Scene::Instance().ToggleUseDebugFrustum(fpsCamera);
    }

    if (m_pApp->isKeyJustDown('B') || m_pApp->isKeyJustDown('b')) {
        solarSystem = true;

        Scene::Instance().Clear();
        BuildSolarSystem(bounds);

        Scene::Instance().BuildOctTree();
    }

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
        Scene::Instance().MoveLights();
    }
    if(m_pApp->isKeyJustDown('G')) { //TOGGLE BOUNDING VOLUME 
        Scene::Instance().ToggleBoundingVolumeDebug();
    }

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
    if(solarSystem)
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    else
        glClearColor(0.0f, 0.5f, 0.5f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Scene::Instance().Render(width, height);
}

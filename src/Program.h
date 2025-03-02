#pragma once
#include <vector>
#include "Node.h"
#include "DebugCube.h"
#include "DebugSphere.h"
#include "Scene.h"
#include "../samplefw/FirstPersonCamera.h"
#include "../samplefw/OrbitCamera.h"
#include "../samplefw/Grid3D.h"

#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <iostream>
#include <random>

#include "../volpe/TextRendering/TextRenderer.h"
#include "../volpe/TextRendering/TextBox.h"

#include "../thirdparty/imgui/imgui.h"
#include "../thirdparty/imgui/imgui_impl_glfw.h"
#include "../thirdparty/imgui/imgui_impl_opengl3.h"

#include "ParticleNode.h"

class Program
{
private:
    volpe::App* m_pApp = nullptr;
    FirstPersonCamera* fpsCamera = nullptr;
    OrbitCamera* orbitCamera = nullptr;
    bool whichCamera = false; //true = orbit, false = fps
    bool solarSystem = true;
    void DrawSceneManagerUI();

public:
    Program(volpe::App* pApp);
    ~Program();

    void init();
    void update(float dt);
    void draw(int width, int height);

    void drawSingleLight(const glm::mat4& viewProj);
    void drawTwoLightsPerObject(const glm::mat4& viewProj);
};

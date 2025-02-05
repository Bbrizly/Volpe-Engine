#include <stdio.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../volpe/Volpe.h"
#include "SceneManager.h"
using namespace std;

class Main : public volpe::App {
private:
    SceneManager* scene;

public:
    Main() 
        : App("Volpe Engine")
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        // glDisable(GL_DEPTH_TEST);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glDisable(GL_CULL_FACE);

        scene = new SceneManager(this);
        scene->initScene();

    }

    ~Main() override 
    {
        delete scene;
    }

    void update(float dt) override 
    {
        scene->update(dt);
    }

    void render() override 
    {
        glClearColor(0.0f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        scene->draw(m_width,m_height);

    }
};

int main(int, char**) {
    Main main;
    main.run();
    return 0;
}

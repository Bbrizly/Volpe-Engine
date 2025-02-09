#include <stdio.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../volpe/Volpe.h"
#include "Program.h"
using namespace std;

class Main : public volpe::App {
private:
    Program* program;

public:
    Main() 
        : App("Volpe Engine")
    {
        program = new Program(this);
        program->init();

    }

    ~Main() override 
    {
        delete program;
    }

    void update(float dt) override 
    {
        program->update(dt);
    }

    void render() override 
    {
        glClearColor(0.0f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        program->draw(m_width,m_height);

    }
};

int main(int, char**) {
    Main main;
    main.run();
    return 0;
}

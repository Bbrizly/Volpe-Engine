#pragma once
#define GLEW_NO_GLU
#include "W_Types.h"
#include <string>

namespace volpe
{
    class App
    {
    public:
        App(const std::string &name);
        virtual ~App();

        virtual void run();
        virtual void update(float dt) = 0;
        virtual void render() = 0;

        bool isKeyDown(int key) const;
        bool isKeyJustDown(int key) const;
        bool isLMBDown() const;
        bool isRMBDown() const;
        bool isMMBDown() const;
        
        void App::UnlockCursor();
        void App::LockCursor();
        glm::vec2 getMousePos() const;
        glm::vec2 getScreenSize() const;
        glm::vec2 getMouseScroll() const { return m_mouseScroll; }
        GLFWwindow* getWindow() const { return m_pWindow; }

        // Used internally but needs to be public
        void _setMouseScroll(const glm::vec2 &scroll);

    protected:
        int m_width = 0;
        int m_height = 0;

    private:
        void _init();
        void _internalUpdate(float dt);

        GLFWwindow *m_pWindow = nullptr;
        std::string m_name;
        glm::vec2 m_mouseScroll;

        static const int NUM_KEYS = GLFW_KEY_LAST - GLFW_KEY_SPACE;
        bool m_lastKeysDown[NUM_KEYS];
    };
}
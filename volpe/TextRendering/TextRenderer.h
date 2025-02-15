#pragma once
#include "../volpe/Volpe.h"
#include "TextBox.h"
#include "Font.h"
#include <glm/glm.hpp>
#include "../samplefw/Sample.h"
#include "TextVertex.h"
#include <string>
#include <vector>
#include "TextTable.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
using namespace std;

class Grid2D;

class TextRenderer {
private:
    volpe::Program* TextShader;

    vector<TextBox*> m_textBoxes;

    TextTable* m_pTextTable = nullptr;
    
    int m_screenWidth = 1280;
    int m_screenHeight = 720;

    int m_baseWidth = 1280;
    int m_baseHeight = 720;


public:
    TextRenderer();
    ~TextRenderer();
    
    void setScreenSize(int width, int height) {m_screenWidth = width; m_screenHeight = height; UpdateLocations();}

    void UpdateLocations()
    {
        // makes sure textbox locations are translated from current screen size to the new screen size
        float scaleX = static_cast<float>(m_screenWidth) / m_baseWidth;
        float scaleY = static_cast<float>(m_screenHeight) / m_baseHeight;
        
        for (auto textBox : m_textBoxes) {
            glm::vec2 basePos = textBox->GetBasePosition();
            glm::vec2 newPos = glm::vec2(basePos.x * scaleX, basePos.y * scaleY);
            textBox->SetPosition(newPos.x, newPos.y);
        }
    }

    void init(TextTable* pTable);
    void init();
    void update(float dt);
    void render(const glm::mat4& proj, const glm::mat4& view);

    Font* createFont(const std::string& fontDataPath);
    TextBox* createTextBox(Font* font, const std::string& text, float x, float y, float width, float height);

    void setTextBox(TextBox* textBox);
};
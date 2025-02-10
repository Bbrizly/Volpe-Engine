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
    
public:
    TextRenderer();
    ~TextRenderer();

    void init(TextTable* pTable);
    void update(float dt);
    void render(const glm::mat4& proj, const glm::mat4& view);

    Font* createFont(const std::string& fontDataPath);
    TextBox* createTextBox(Font* font, const std::string& text, float x, float y, float width, float height);

    void setTextBox(TextBox* textBox);
};
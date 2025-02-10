#include "TextRenderer.h"
using namespace std;

TextRenderer::TextRenderer()
    : TextShader(nullptr) {}

TextRenderer::~TextRenderer() {
    if (TextShader) {
        volpe::ProgramManager::DestroyProgram(TextShader);
    }
}

void TextRenderer::init(TextTable* pTable) {
    TextShader = volpe::ProgramManager::CreateProgram("data/2d.vsh", "data/2d.fsh");
    
    m_pTextTable = pTable;
}

void TextRenderer::update(float dt) {}

void TextRenderer::setTextBox(TextBox* textBox) {
    m_textBoxes.push_back(textBox);}

void TextRenderer::render(const glm::mat4& proj, const glm::mat4& view) {
    if(!TextShader || m_textBoxes.empty()) return;

    for( auto textBox : m_textBoxes ){
        textBox->Render(proj,view);
    }
}

Font* TextRenderer::createFont(const string& fontDataPath) {
    return new Font(fontDataPath);
}

TextBox* TextRenderer::createTextBox(Font* font, const string& text, float x, float y, float width, float height) {
    auto textBox = new TextBox(font, text, width, height, TextShader, m_pTextTable);

    
    textBox->SetPosition(x, y);
    return textBox;
}

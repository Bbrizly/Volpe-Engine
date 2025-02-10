#include "TextBox.h"

TextBox::TextBox(Font* font,
                 const string& text,
                 float width,
                 float height,
                 volpe::Program* shader,
                 TextTable* pTable)
    : m_font(font),
      m_pTextTable(pTable),
      m_text(text),
      m_observeID(""),
      m_positionX(0.f),
      m_positionY(0.f),
      m_width(width),
      m_height(height),
      m_pProgram(shader),
      m_color(255,255,255,255)
{
    if(m_pTextTable) { //Trying to get if info changes in texttable for it to change the rendered text.
        m_pTextTable->RegisterOnChange([this](){
            this->OnTextTableChanged();
        });
    }
    GenerateVertices();
}

TextBox::~TextBox()
{
    if(m_vertexBuffer) {
        volpe::BufferManager::DestroyBuffer(m_vertexBuffer);
        m_vertexBuffer = nullptr;
    }
    if(m_vertexDecl) {
        delete m_vertexDecl;
        m_vertexDecl = nullptr;
    }
}

void TextBox::SetText(const string& text)
{
    m_text = text;
    GenerateVertices();
}

void TextBox::SetText(const char* fmt, ...)
{
    if(!fmt) return;
    char buf[2048];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    SetText(string(buf));
}

void TextBox::SetTextTable(TextTable* pTable)
{
    m_pTextTable = pTable;
    if(m_pTextTable) {
        m_pTextTable->RegisterOnChange([this](){
            this->OnTextTableChanged();
        });
    }
    GenerateVertices();
}

void TextBox::OnTextTableChanged()
{
    if(!m_observeID.empty() && m_pTextTable) {
        try {
            m_text = m_pTextTable->GetString(m_observeID);
        } catch(...) {}
    }
    GenerateVertices();
}

void TextBox::SetPosition(float x, float y)
{
    m_positionX = x;
    m_positionY = y;
    GenerateVertices();
}

void TextBox::SetColor(float r, float g, float b, float a)
{
    m_color = glm::vec4(r,g,b,a);
    GenerateVertices();
}

void TextBox::SetAlignment(int alignment)
{
    m_hAlign = alignment;
    GenerateVertices();
}

void TextBox::SetVerticalAlignment(int vAlignment)
{
    m_vAlign = vAlignment;
    GenerateVertices();
}

void TextBox::SetVisualization(bool visualization)
{
    m_visualization = visualization;
    GenerateVertices();
}

void TextBox::GenerateVertices()
{
    // std::cout << "Generating vertices for text: " << m_text << std::endl;
    m_vertices.clear();

    if(m_visualization) {
        GenerateBoundingBoxVertices();
    }

    if(m_text.empty()) {
        pushVertexData(m_vertexBuffer, m_vertexDecl, m_vertices);
        return;
    }

    string substituted = SubstitutePlaceholders(m_text);

    float scale = 1.0f;
    vector<string> finalLines;
    FindBestMiniFontAndWrap(substituted, finalLines);

    bool truncated = false;
    float totalH = finalLines.size() * (m_font->GetLineHeight(m_fontIndex) * scale);
    if(totalH > m_height) {
        truncated = true;
        TruncateLines(finalLines, scale);
    }

    BuildVerticesFromLines(finalLines, scale, truncated);

    pushVertexData(m_vertexBuffer, m_vertexDecl, m_vertices);

    // std::cout << "Total vertices generated: " << m_vertices.size() << std::endl;
}

string TextBox::SubstitutePlaceholders(const string& raw) const
{
    if(!m_pTextTable) return raw;
    return m_pTextTable->Substitute(raw);
}

vector<string> TextBox::TokenizeText(const string& text) const
{
    vector<string> tokens;
    string cur;
    for(size_t i=0; i<text.size(); i++) {
        char c = text[i];
        if(c=='\n') {
            if(!cur.empty()) {
                tokens.push_back(cur);
                cur.clear();
            }
            tokens.push_back("\n"); 
        }
        else if(isspace((unsigned char)c)) {
            if(!cur.empty()) {
                tokens.push_back(cur);
                cur.clear();
            }
        }
        else {
            cur.push_back(c);
        }
    }
    if(!cur.empty()) {
        tokens.push_back(cur);
        cur.clear();
    }
    return tokens;
}

vector<string> TextBox::WrapAndHyphenate(const string& text,
                                                   float scale) const
{
    vector<string> lines;
    auto tokens = TokenizeText(text);

    float maxWidth = m_width;
    float spaceW  = CalculateWordWidth(" ") * scale;

    string currentLine;

    for(size_t i=0; i<tokens.size(); i++){
        const string& token = tokens[i];

        // forcedd line break, could be good if u just write hella and throw it into texttable
        if(token=="\n") {
            lines.push_back(currentLine);
            currentLine.clear();
            continue;
        }

        float tokenW = CalculateWordWidth(token) * scale;
        float currentW = CalculateWordWidth(currentLine) * scale;
        float widthIfAdded = currentLine.empty()
                             ? tokenW
                             : (currentW + spaceW + tokenW);

        if(tokenW > maxWidth) {
            if(!currentLine.empty()) {
                lines.push_back(currentLine);
                currentLine.clear();
            }
            string sub;
            for(size_t c=0; c<token.size(); c++){
                sub.push_back(token[c]);
                float subW = CalculateWordWidth(sub)*scale;
                if(subW > maxWidth) {
                    if(!sub.empty()) sub.pop_back();
                    if(!sub.empty()) sub.pop_back();
                    sub += "-";
                    lines.push_back(sub);
                    sub.clear();
                    c--;
                }
            }
            if(!sub.empty()) {
                lines.push_back(sub);
                sub.clear();
            }
            continue;
        }

        if(widthIfAdded <= maxWidth) {
            if(currentLine.empty()) {
                currentLine = token;
            } else {
                currentLine += " " + token;
            }
        } else {
            lines.push_back(currentLine);
            currentLine = token;
        }
    }

    if(!currentLine.empty()){
        lines.push_back(currentLine);
    }

    return lines;
}

bool TextBox::FitsInBox(const vector<string>& lines, float scale) const
{
    float lineH = m_font->GetLineHeight(m_fontIndex)*scale;
    float totalH = lines.size()*lineH;
    if(totalH > m_height) return false;

    for(auto& ln : lines){
        float w = CalculateWordWidth(ln)*scale;
        if(w > m_width) return false;
    }
    return true;
}

void TextBox::FindBestMiniFontAndWrap(const string& text,
                                      vector<string>& outLines)
{
    float scale = 1.0f;

    int attempt = m_fontIndex;
    int numFonts = (int)m_font->GetFonts().size();

    while(attempt < numFonts)
    {
        // switch to a mini-er font
        int backupIndex = m_fontIndex;
        m_fontIndex = attempt;

        auto lines = WrapAndHyphenate(text, scale);
        if(FitsInBox(lines, scale)) {
            outLines = lines;
            return;
        }
        m_fontIndex = backupIndex;
        attempt++;
    }
    m_fontIndex = numFonts - 1;
    outLines = WrapAndHyphenate(text, scale);
}

// Remove lines from bottom 1 by 1 until total height fits. Then add "..." at the end (font forget to rmeove 3 chars before)
void TextBox::TruncateLines(vector<string>& lines, float scale)
{
    float lineH = m_font->GetLineHeight(m_fontIndex)*scale;
    if(lines.empty()) return;

    while(!lines.empty()) {
        float curH = lines.size()*lineH;
        if(curH <= m_height) {
            break;
        }
        lines.pop_back();
    }

    if(!lines.empty()) {
        string& lastLine = lines.back();
        if(lastLine.size()>3){
            lastLine.resize(lastLine.size()-3);
        }
        lastLine += "...";
    }
}

void TextBox::BuildVerticesFromLines(const vector<string>& lines,
                                     float scale,
                                     bool truncated)
{
    BuildVerticesActual(lines, scale);
}

void TextBox::BuildVerticesActual(const vector<string>& lines,
                                  float scale)
{
    if(!m_font || !m_font->GetTexture()) {return;}
    
    // cout<<m_font->GetTexture()->m_width;

    float texW  = m_font->GetTexture()->GetWidth();
    float texH  = m_font->GetTexture()->GetHeight();
    float lineH = m_font->GetLineHeight(m_fontIndex)* scale;

    float totalH = lines.size()* lineH;
    float startY = m_positionY;

    if(m_vAlign==1) {
        startY = m_positionY - ((m_height - totalH)*0.5f);
    } else if(m_vAlign==2) {
        startY = m_positionY - (m_height - totalH);
    }

    float cursorY = startY;
    for(size_t li=0; li<lines.size(); li++){
        const string& line = lines[li];
        float lineWidth = CalculateWordWidth(line)*scale;
        glm::vec2 cursor = CalculateAlignmentCursor(line, lineWidth, cursorY, scale);

        for(size_t i=0; i<line.size(); i++){
            char c = line[i];
            if(i>0) {
                char prevC = line[i-1];
                ApplyKerning(prevC, c, scale, cursor.x);
            }
            const CharInfo& ch = m_font->GetCharacter(c, m_fontIndex);

            GenerateCharacterVertices(ch, cursor.x, cursor.y, texW, texH, scale);
            cursor.x += (ch.xAdvance * scale);
        }
        cursorY -= lineH;
    }
}

void TextBox::GenerateCharacterVertices(const CharInfo& ch,
                                        float x, float y,
                                        float texW, float texH,
                                        float scale)
{
    float xpos = x + ch.xOffset*scale;
    float ypos = y - ch.yOffset*scale;
    float w = ch.uEnd*scale;
    float h = ch.vEnd*scale;

    float u0 = ch.uStart / texW;
    float v0 = ch.vStart / texH;
    float u1 = (ch.uStart + ch.uEnd)/texW;
    float v1 = (ch.vStart + ch.vEnd)/texH;

    GLubyte R = (GLubyte)m_color.r;
    GLubyte G = (GLubyte)m_color.g;
    GLubyte B = (GLubyte)m_color.b;
    GLubyte A = (GLubyte)m_color.a;

    float page = (float)ch.page;

    TextVertex v1a = {xpos,   ypos - h, 0.f, R,G,B,A, u0, v1, page};
    TextVertex v1b = {xpos+w, ypos - h, 0.f, R,G,B,A, u1, v1, page};
    TextVertex v1c = {xpos,   ypos,     0.f, R,G,B,A, u0, v0, page};

    TextVertex v2a = {xpos+w, ypos - h, 0.f, R,G,B,A, u1, v1, page};
    TextVertex v2b = {xpos+w, ypos,     0.f, R,G,B,A, u1, v0, page};
    TextVertex v2c = {xpos,   ypos,     0.f, R,G,B,A, u0, v0, page};

    m_vertices.push_back(v1a);
    m_vertices.push_back(v1b);
    m_vertices.push_back(v1c);

    m_vertices.push_back(v2a);
    m_vertices.push_back(v2b);
    m_vertices.push_back(v2c);
}

glm::vec2 TextBox::CalculateAlignmentCursor(const string& line,
                                            float lineWidth,
                                            float cursorY,
                                            float scale) const
{
    switch(m_hAlign){
    case 1: // center
        return glm::vec2(m_positionX + (m_width - lineWidth)*0.5f, cursorY);
    case 2: // right
        return glm::vec2(m_positionX + (m_width - lineWidth), cursorY);
    default: // left
        return glm::vec2(m_positionX, cursorY);
    }
}

void TextBox::ApplyKerning(char prevChar, char c,
                           float scale, float& xCursor)
{
    auto& km = m_font->GetKerning(m_fontIndex);
    auto it = km.find(make_pair(prevChar,c));
    if(it!=km.end()) {
        xCursor += it->second * scale;
    }
}

float TextBox::CalculateWordWidth(const string& word) const
{
    float w=0.f;
    for(size_t i=0; i<word.size(); i++){
        char c = word[i];
        const CharInfo& ch = m_font->GetCharacter(c, m_fontIndex);

        if(i>0) {
            char p = word[i-1];
            auto& kern = m_font->GetKerning(m_fontIndex);
            auto it = kern.find(make_pair(p,c));
            if(it!=kern.end()){
                w += it->second;
            }
        }
        w += ch.xAdvance;
    }
    return w;
}

void TextBox::GenerateBoundingBoxVertices()
{
    float layer=-1.f;
    GLubyte R=255, G=255, B=255, A=80;
    float left   = m_positionX;
    float top    = m_positionY;
    float right  = m_positionX + m_width;
    float bottom = m_positionY - m_height;

    TextVertex vA= {left,  bottom, 0.f, R,G,B,A, 0.f,0.f, layer};
    TextVertex vB= {right, bottom, 0.f, R,G,B,A, 0.f,0.f, layer};
    TextVertex vC= {left,  top,    0.f, R,G,B,A, 0.f,0.f, layer};
    TextVertex vD= {right, top,    0.f, R,G,B,A, 0.f,0.f, layer};

    m_vertices.push_back(vA); m_vertices.push_back(vB); m_vertices.push_back(vC);
    m_vertices.push_back(vB); m_vertices.push_back(vD); m_vertices.push_back(vC);
}

void TextBox::pushVertexData(volpe::VertexBuffer*& vBuffer,
                             volpe::VertexDeclaration*& vDecl,
                             const vector<TextVertex>& inVerts)
{
    if(vBuffer) {
        volpe::BufferManager::DestroyBuffer(vBuffer);
        vBuffer = nullptr;
    }
    if(vDecl) {
        delete vDecl;
        vDecl = nullptr;
    }
    if(inVerts.empty()) {
        m_numVertices=0;
        return;
    }

    vBuffer = volpe::BufferManager::CreateVertexBuffer(
                  inVerts.data(),
                  inVerts.size()*sizeof(TextVertex));
    vDecl = new volpe::VertexDeclaration();
    vDecl->Begin();
    vDecl->AppendAttribute(volpe::AT_Position, 3, volpe::CT_Float);
    vDecl->AppendAttribute(volpe::AT_Color,    4, volpe::CT_UByte);
    vDecl->AppendAttribute(volpe::AT_TexCoord1,2, volpe::CT_Float);
    vDecl->AppendAttribute(volpe::AT_TexCoord2,1, volpe::CT_Float);
    vDecl->SetVertexBuffer(vBuffer);
    vDecl->End();

    m_numVertices = inVerts.size();
}

void TextBox::Render(const glm::mat4& proj, const glm::mat4& view)
{
    if(!m_pProgram || !m_vertexBuffer || !m_vertexDecl || m_numVertices==0) {
        std::cout << "Skipping render: No shader, buffer, or vertices." << std::endl;
        return;
    }

    // std::cout << "Rendering text with " << m_numVertices << " vertices." << std::endl;

    m_pProgram->Bind();
    m_pProgram->SetUniform("projection", proj);
    m_pProgram->SetUniform("view", view);

    if(m_font && m_font->GetTexture()) {
        // std::cout << "Binding font texture." << std::endl;
        m_font->GetTexture()->Bind(0);
        m_pProgram->SetUniform("u_texture", 0);
    }

    m_vertexDecl->Bind();
    glDrawArrays(GL_TRIANGLES, 0, m_numVertices);
}

Font* TextBox::GetFont() const
{
    return m_font;
}

const vector<TextVertex>& TextBox::GetVertices() const
{
    return m_vertices;
}

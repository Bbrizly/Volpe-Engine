#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <unordered_map>
#include "Font.h"
#include "TextVertex.h"
#include "TextTable.h"
#include <sstream>
#include <cstdarg>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

class TextTable;

class TextBox
{
public:
    TextBox(Font* font,
            const string& text,
            float width,
            float height,
            float x,
            float y,
            volpe::Program* shader,
            TextTable* pTable);
    ~TextBox();

    // If you want to re-use the same text box but with new text
    // it re-generates the geometry
    void SetText(const string& text);

    // C-style varargs
    void SetText(const char* fmt, ...);

    //makes textbox watch a specific text in TextTable, if it changes regen text
    void ObserveTextID(const string& textID);

    void SetTextTable(TextTable* pTable);
    void SetPosition(float x, float y);
    glm::vec2 GetPosition() {return glm::vec2(m_positionX,m_positionY);}
    glm::vec2 GetBasePosition() {return glm::vec2(m_basePositionX, m_basePositionY);}
    void SetColor(float r, float g, float b, float a);
    void SetAlignment(int alignment);          // 0=left,1=center,2=right
    void SetVerticalAlignment(int vAlignment); // 0=top,1=middle,2=bottom
    void SetVisualization(bool visualization); // debug bounding box
    void SetShrinkToFit(bool enable);

    void SetFont(Font* f) {m_font = f; GenerateVertices();}

    // If we have multiple font sizes in a single Font, pick the index
    // 0 => largest, 1 => smaller, etc.
    void SetFontIndex(int index) { m_fontIndex = index; GenerateVertices(); }

    // main call
    void Render(const glm::mat4& proj, const glm::mat4& view);

    // convenience
    Font* GetFont() const;
    const vector<TextVertex>& GetVertices() const;

    // Re-generate geometry if text table changed
    // (This is called automatically if you used ObserveTextID)
    void OnTextTableChanged();

private:
    void GenerateVertices();
    string SubstitutePlaceholders(const string& raw) const;

    vector<string> TokenizeText(const string& text) const;

    vector<string> WrapAndHyphenate(const string& text,
                                              float scale) const;

    bool FitsInBox(const vector<string>& lines, float scale) const;

    void FindBestMiniFontAndWrap(const string& text, vector<string>& finalLines);

    void TruncateLines(vector<string>& lines, float scale);

    void BuildVerticesFromLines(const vector<string>& lines,
                                float scale,
                                bool truncated);
    void BuildVerticesActual(const vector<string>& lines,
                             float scale);
    void GenerateCharacterVertices(const CharInfo& ch, float x, float y,
                                   float texW, float texH, float scale);

    glm::vec2 CalculateAlignmentCursor(const string& line,
                                       float lineWidth,
                                       float cursorY,
                                       float scale) const;
    void ApplyKerning(char prevChar, char c, float scale, float& xCursor);

    float CalculateWordWidth(const string& word) const;

    void GenerateBoundingBoxVertices();

    void pushVertexData(volpe::VertexBuffer*& vBuffer,
                        volpe::VertexDeclaration*& vDecl,
                        const vector<TextVertex>& inVerts);

private:
    Font* m_font;
    TextTable* m_pTextTable;  // for placeholders
    string m_text;       // the raw text (could be a direct string or a textID)
    string m_observeID;  // if non-empty, we auto-get the text from the table

    float m_positionX, m_positionY;
    float m_basePositionX, m_basePositionY;
    float m_width, m_height;

    // alignment
    int m_hAlign = 0; // 0=left,1=center,2=right
    int m_vAlign = 0; // 0=top,1=middle,2=bottom

    // debugging
    bool m_visualization = true;
    // bool m_shrinkToFit = false;

    int m_fontIndex = 0;

    glm::vec4 m_color;

    volpe::Program* m_pProgram;
    volpe::VertexBuffer* m_vertexBuffer = nullptr;
    volpe::VertexDeclaration* m_vertexDecl = nullptr;
    int m_numVertices = 0;

    vector<TextVertex> m_vertices;
};
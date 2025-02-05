#include "Grid2D.h"

struct Vertex
{
	GLfloat x,y;
    unsigned char r,g,b,a;
};

const float AXIS_EXTENT = 8.0f;
const unsigned char xAxisColor[] = { 255, 73, 27, 255 };
const unsigned char yAxisColor[] = { 87, 151, 81, 255 };

static const Vertex axesVertices[] = {
	// x-axis
	{ -AXIS_EXTENT, 0, xAxisColor[0], xAxisColor[1], xAxisColor[2], xAxisColor[3] },
	{ AXIS_EXTENT, 0, xAxisColor[0], xAxisColor[1], xAxisColor[2], xAxisColor[3] },
	// y-axis
	{ 0, -AXIS_EXTENT, yAxisColor[0], yAxisColor[1], yAxisColor[2], yAxisColor[3] },
	{ 0, AXIS_EXTENT, yAxisColor[0], yAxisColor[1], yAxisColor[2], yAxisColor[3] },
};


Grid2D::Grid2D(int linesPerHalfSpace)
    : m_color(0.4f,0.4f,0.4f,1.0)
{
    m_pProgram = volpe::ProgramManager::CreateProgram("samplefw/data/grid.vsh", "samplefw/data/grid.fsh");

    _createGrid(linesPerHalfSpace);
    _createAxes();
}

Grid2D::~Grid2D()
{
    delete m_pDecl;
    volpe::BufferManager::DestroyBuffer(m_pVB);
    volpe::ProgramManager::DestroyProgram(m_pProgram);
}

void Grid2D::update(float dt)
{
}

void Grid2D::render(const glm::mat4& mView, const glm::mat4& mProj)
{
    m_pProgram->SetUniform("projection", mProj);
    m_pProgram->SetUniform("view", mView);
    m_pProgram->SetUniform("world", glm::mat4(1.0f));    
    m_pProgram->SetUniform("color", m_color);
	m_pProgram->Bind();
	m_pDecl->Bind();
    glDrawArrays(GL_LINES, 0, m_numVerts);

    if(m_showAxes) 
    {
        m_pAxesDecl->Bind();
        m_pProgram->SetUniform("color", volpe::Color4(1.0f, 1.0f, 1.0f, 1.0f));
        m_pProgram->Bind();
        glDrawArrays(GL_LINES, 0, 4);
    }
}

void Grid2D::_createAxes()
{
    m_pAxesVB = volpe::BufferManager::CreateVertexBuffer(axesVertices, sizeof(Vertex) * 2 /* verts per line */ * 2 /* axes */);
    m_pAxesDecl = new volpe::VertexDeclaration();
    m_pAxesDecl->Begin();
    m_pAxesDecl->AppendAttribute(volpe::AT_Position, 2, volpe::CT_Float);
    m_pAxesDecl->AppendAttribute(volpe::AT_Color, 4, volpe::CT_UByte);
    m_pAxesDecl->SetVertexBuffer(m_pAxesVB);
    m_pAxesDecl->End();
}

void Grid2D::_createGrid(int linesPerHalfSpace)
{
    // * 2 * 2 = 2 half spaces, 2 axes
    // + 2 = 1 for x/y-axis itself
    const int TOTAL_GRID_LINES = linesPerHalfSpace * 2 * 2 + 2; 
    const float MAX_GRID_EXTENT = (float) linesPerHalfSpace;

    m_numVerts = TOTAL_GRID_LINES * 2;

    Vertex* pGridVerts = new Vertex[m_numVerts];
    int idx = 0;


    // Xs first
    {
        // x-axis (grid line)
        {
            Vertex *pA = &pGridVerts[idx++];
            Vertex *pB = &pGridVerts[idx++];

            pA->x = 0.0f;
            pA->y = -MAX_GRID_EXTENT;

            pB->x = 0.0f;
            pB->y = MAX_GRID_EXTENT;
        }

        // positive x grid lines
        for (int x = 1; x <= linesPerHalfSpace; x++)
        {
            float xPos = (float) x;
            Vertex *pA = &pGridVerts[idx++];
            Vertex *pB = &pGridVerts[idx++];

            pA->x = xPos;
            pA->y = -MAX_GRID_EXTENT;

            pB->x = xPos;
            pB->y = MAX_GRID_EXTENT;
        }

        // negative x grid lines
        for (int x = 1; x <= linesPerHalfSpace; x++)
        {
            float xPos = (float) -x;
            Vertex *pA = &pGridVerts[idx++];
            Vertex *pB = &pGridVerts[idx++];

            pA->x = xPos;
            pA->y = -MAX_GRID_EXTENT;

            pB->x = xPos;
            pB->y = MAX_GRID_EXTENT;
        }
    }

    // Then Ys
    {
        // y-axis (grid line)
        {
            Vertex *pA = &pGridVerts[idx++];
            Vertex *pB = &pGridVerts[idx++];

            pA->x = -MAX_GRID_EXTENT;
            pA->y = 0.0f;

            pB->x = MAX_GRID_EXTENT;
            pB->y = 0.0f;
        }

        // positive y grid lines
        for (int y = 1; y <= linesPerHalfSpace; y++)
        {
            float yPos = (float) y;
            Vertex *pA = &pGridVerts[idx++];
            Vertex *pB = &pGridVerts[idx++];

            pA->x = -MAX_GRID_EXTENT;
            pA->y = yPos;

            pB->x = MAX_GRID_EXTENT;
            pB->y = yPos;
        }

        // negative y grid lines
        for (int y = 1; y <= linesPerHalfSpace; y++)
        {
            float yPos = (float) -y;
            Vertex *pA = &pGridVerts[idx++];
            Vertex *pB = &pGridVerts[idx++];

            pA->x = -MAX_GRID_EXTENT;
            pA->y = yPos;

            pB->x = MAX_GRID_EXTENT;
            pB->y = yPos;
        }

        for(int i = 0; i < m_numVerts; ++i)
        {
            Vertex* pVert = &pGridVerts[i];
            pVert->r = pVert->g = pVert->b = pVert->a = 255;
        }

        m_pVB = volpe::BufferManager::CreateVertexBuffer(pGridVerts, sizeof(Vertex) * m_numVerts);
        m_pDecl = new volpe::VertexDeclaration();
        m_pDecl->Begin();
        m_pDecl->AppendAttribute(volpe::AT_Position, 2, volpe::CT_Float);
        m_pDecl->AppendAttribute(volpe::AT_Color, 4, volpe::CT_UByte);
        m_pDecl->SetVertexBuffer(m_pVB);
        m_pDecl->End();

        delete[] pGridVerts;
    }
}
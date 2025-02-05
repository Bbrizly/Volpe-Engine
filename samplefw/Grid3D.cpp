#include "Grid3D.h"

struct Vertex
{
	GLfloat x,y,z;
    unsigned char r,g,b,a;
};

const float AXIS_EXTENT = 8.0f;
const unsigned char xAxisColor[] = { 255, 73, 27, 255 };
const unsigned char yAxisColor[] = { 87, 151, 81, 255 };
const unsigned char zAxisColor[] = { 0, 0, 255, 255 };

static const Vertex axesVertices[] = {
	// x-axis
	{ -AXIS_EXTENT, 0, 0, xAxisColor[0], xAxisColor[1], xAxisColor[2], xAxisColor[3] },
	{ AXIS_EXTENT, 0, 0, xAxisColor[0], xAxisColor[1], xAxisColor[2], xAxisColor[3] },
	// y-axis
	{ 0, -AXIS_EXTENT, 0, yAxisColor[0], yAxisColor[1], yAxisColor[2], yAxisColor[3] },
	{ 0, AXIS_EXTENT, 0, yAxisColor[0], yAxisColor[1], yAxisColor[2], yAxisColor[3] },
	// z-axis
	{ 0, 0, -AXIS_EXTENT, zAxisColor[0], zAxisColor[1], zAxisColor[2], zAxisColor[3] },
	{ 0, 0, AXIS_EXTENT, zAxisColor[0], zAxisColor[1], zAxisColor[2], zAxisColor[3] },
};


Grid3D::Grid3D(int linesPerHalfSpace, float gridSpacing)
    : m_color(0.4f,0.4f,0.4f,1.0)
{
    m_pMatGrid = volpe::MaterialManager::CreateMaterial("__gridMat");
    m_pMatGrid->SetProgram("samplefw/data/grid.vsh", "samplefw/data/grid.fsh");
    m_pMatGrid->SetDepthTest(true);

    m_pMatAxes = volpe::MaterialManager::CreateMaterial("__gridAxes");
    m_pMatAxes->SetProgram("samplefw/data/grid.vsh", "samplefw/data/grid.fsh");
    m_pMatAxes->SetDepthTest(false);

    // axis color comes from vertex attributes which we multiply by white
    m_pMatAxes->SetUniform("color", volpe::Color4(1.0f, 1.0f, 1.0f, 1.0f)); 

    _createGrid(linesPerHalfSpace, gridSpacing);
    _createAxes();
}

Grid3D::~Grid3D()
{
    delete m_pDecl;
    volpe::BufferManager::DestroyBuffer(m_pVB);
    volpe::MaterialManager::DestroyMaterial(m_pMatGrid);
    volpe::MaterialManager::DestroyMaterial(m_pMatAxes);
}

void Grid3D::update(float dt)
{
}

void Grid3D::render(const glm::mat4& mView, const glm::mat4& mProj)
{
    m_pMatGrid->SetUniform("projection", mProj);
    m_pMatGrid->SetUniform("view", mView);
    m_pMatGrid->SetUniform("world", glm::mat4(1.0f));    
    m_pMatGrid->SetUniform("color", m_color);
    m_pMatGrid->SetDepthTest(true);
	m_pMatGrid->Apply();
	m_pDecl->Bind();
    glDrawArrays(GL_LINES, 0, m_numVerts);

    if(m_showAxes) 
    {
        m_pAxesDecl->Bind();
        m_pMatAxes->SetUniform("projection", mProj);
        m_pMatAxes->SetUniform("view", mView);
        m_pMatAxes->SetUniform("world", glm::mat4(1.0f));    
        m_pMatAxes->Apply();
        glDrawArrays(GL_LINES, 0, 6);
        glEnable(GL_DEPTH_TEST);
    }
}

void Grid3D::_createAxes()
{
    m_pAxesVB = volpe::BufferManager::CreateVertexBuffer(axesVertices, sizeof(Vertex) * 2 /* verts per line */ * 3 /* axes */);
    m_pAxesDecl = new volpe::VertexDeclaration();
    m_pAxesDecl->Begin();
    m_pAxesDecl->AppendAttribute(volpe::AT_Position, 3, volpe::CT_Float);
    m_pAxesDecl->AppendAttribute(volpe::AT_Color, 4, volpe::CT_UByte);
    m_pAxesDecl->SetVertexBuffer(m_pAxesVB);
    m_pAxesDecl->End();
}

void Grid3D::_createGrid(int linesPerHalfSpace, float gridSpacing)
{
    // * 2 * 2 = 2 half spaces, 2 axes
    // + 2 = 1 for x/z-axis itself
    const int TOTAL_GRID_LINES = linesPerHalfSpace * 2 * 2 + 2; 
    const float MAX_GRID_EXTENT = linesPerHalfSpace * gridSpacing;

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
            pA->y = 0.0f;
            pA->z = -MAX_GRID_EXTENT;

            pB->x = 0.0f;
            pB->y = 0.0f;
            pB->z = MAX_GRID_EXTENT;
        }

        // positive x grid lines
        for (int x = 1; x <= linesPerHalfSpace; x++)
        {
            float xPos = x * gridSpacing;
            Vertex *pA = &pGridVerts[idx++];
            Vertex *pB = &pGridVerts[idx++];

            pA->x = xPos;
            pA->y = 0;
            pA->z = -MAX_GRID_EXTENT;

            pB->x = xPos;
            pB->y = 0;
            pB->z = MAX_GRID_EXTENT;
        }

        // negative x grid lines
        for (int x = 1; x <= linesPerHalfSpace; x++)
        {
            float xPos = -x * gridSpacing;
            Vertex *pA = &pGridVerts[idx++];
            Vertex *pB = &pGridVerts[idx++];

            pA->x = xPos;
            pA->y = 0;
            pA->z = -MAX_GRID_EXTENT;

            pB->x = xPos;
            pB->y = 0;
            pB->z = MAX_GRID_EXTENT;
        }
    }

    // Then Zs
    {
        // z-axis (grid line)
        {
            Vertex *pA = &pGridVerts[idx++];
            Vertex *pB = &pGridVerts[idx++];

            pA->x = -MAX_GRID_EXTENT;
            pA->y = 0.0f;
            pA->z = 0.0f;

            pB->x = MAX_GRID_EXTENT;
            pB->y = 0.0f;
            pB->z = 0.0f;
        }

        // positive z grid lines
        for (int z = 1; z <= linesPerHalfSpace; z++)
        {
            float zPos = z * gridSpacing;
            Vertex *pA = &pGridVerts[idx++];
            Vertex *pB = &pGridVerts[idx++];

            pA->x = -MAX_GRID_EXTENT;
            pA->y = 0;
            pA->z = zPos;

            pB->x = MAX_GRID_EXTENT;
            pB->y = 0;
            pB->z = zPos;
        }

        // negative y grid lines
        for (int z = 1; z <= linesPerHalfSpace; z++)
        {
            float zPos = -z * gridSpacing;
            Vertex *pA = &pGridVerts[idx++];
            Vertex *pB = &pGridVerts[idx++];

            pA->x = -MAX_GRID_EXTENT;
            pA->y = 0;
            pA->z = zPos;

            pB->x = MAX_GRID_EXTENT;
            pB->y = 0;
            pB->z = zPos;
        }

        for(int i = 0; i < m_numVerts; ++i)
        {
            Vertex* pVert = &pGridVerts[i];
            pVert->r = pVert->g = pVert->b = pVert->a = 255;
        }

        m_pVB = volpe::BufferManager::CreateVertexBuffer(pGridVerts, sizeof(Vertex) * m_numVerts);
        m_pDecl = new volpe::VertexDeclaration();
        m_pDecl->Begin();
        m_pDecl->AppendAttribute(volpe::AT_Position, 3, volpe::CT_Float);
        m_pDecl->AppendAttribute(volpe::AT_Color, 4, volpe::CT_UByte);
        m_pDecl->SetVertexBuffer(m_pVB);
        m_pDecl->End();

        delete[] pGridVerts;
    }
}

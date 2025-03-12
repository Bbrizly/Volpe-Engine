#pragma once
#include "Node.h"
#include "../volpe/Volpe.h"
#include <glm/glm.hpp>
#include <vector>
#include <random>
#include "Boid.h"

class BoidNode : public Node
{
public:
    BoidNode(const std::string& name);
    virtual ~BoidNode();

    int   maxBoids;
    float worldBounds;      
    bool  wrapAround;   
    bool  doAlignment;
    bool  doCohesion;
    bool  doSeparation;

    void SpawnBoids(int count);
    void ClearBoids();

    virtual void update(float dt) override;
    virtual void draw(const glm::mat4& proj, const glm::mat4& view) override;
    virtual void UpdateBoundingVolume() override;

    std::vector<Boid> getBoids() const;
    std::vector<Boid> m_boids;

private:

    std::mt19937 m_randGen;
    std::uniform_real_distribution<float> m_dist01;

    volpe::VertexBuffer*      m_vb   = nullptr;
    volpe::VertexDeclaration* m_decl = nullptr;
    GLsizeiptr prevBufferSize = 0;

    struct QuadVertex
    {
        GLfloat x, y, z;
        GLfloat u, v, w;
        GLfloat cr, cg, cb, ca;
    };

private:

    void   applyBoidRules(float dt);
    glm::vec3 computeSeparation(int i);
    glm::vec3 computeAlignment(int i);
    glm::vec3 computeCohesion(int i);

    void buildVertexData(const glm::mat4& view);

    void wrapBoid(Boid& b); //SO SEXYYYY AHHHHH

    void getCameraRightUp(const glm::mat4& view, glm::vec3& outRight, glm::vec3& outUp);
};

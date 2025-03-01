#pragma once
#include "Node.h"
#include "../volpe/Volpe.h"
#include <glm/gtc/matrix_transform.hpp>
#include <random>

class ParticleNode : public Node {
public:
    struct Particle {
        glm::vec3 position;
        glm::vec3 velocity;
        float life;
        float maxLife;
    };

    ParticleNode(const std::string& name);
    virtual ~ParticleNode();

    void spawnParticles(int count);

    virtual void update(float dt) override;

    virtual void draw(const glm::mat4& proj, const glm::mat4& view) override;

private:

    float minDist = -5;
    float maxDist = 5;

    float minSpeed = -1;
    float maxSpeed = 1;

    float minLife = 1;
    float maxLife = 4;

    int goalAmount = 1000;

    std::vector<Particle> m_particles;

    std::mt19937 m_randGen;

    volpe::VertexBuffer*       m_vb   = nullptr;
    volpe::VertexDeclaration*  m_decl = nullptr;

    std::uniform_real_distribution<float> m_distPos;
    std::uniform_real_distribution<float> m_distVel;
    std::uniform_real_distribution<float> m_distLife;  

    struct QuadVertex {
        float x, y, z;   
        float u, v;      
    };

    volpe::Material* m_material;

    void buildVertexData(const glm::mat4& view);

    void getCameraRightUp(const glm::mat4& view, glm::vec3& outRight, glm::vec3& outUp);
};

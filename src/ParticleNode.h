#pragma once
#include "Node.h"
#include "../volpe/Volpe.h"
#include <glm/gtc/matrix_transform.hpp>
#include <random>
#include <vector>
#include <string>

#include "Particle.h"
#include "Affector.h"

enum class EmitterMode
{
    Continuous,
    Burst
};

enum class EmitterShape
{
    Point,
    Sphere,
    Donut,
    Cone,
    Box,
    Mesh
};

struct ColorKey
{
    float time;        // 0.0 -> 1.0
    glm::vec4 color;
};

enum class ParticleSystemState
{
    Stopped,
    Playing,
    Paused
};

static const int MAX_PARTICLES_INTERNAL = 50000; //increase for debugging

class ParticleNode : public Node 
{
public:
    bool     useTextureArray = false;
    GLuint   textureArrayID  = 0;
    int      numTextures     = 0;   // how many layers in the array
    
    std::vector<ColorKey> colorKeys;

    // =-=-=-=-=-=-=-=-= Emitter Controls =-=-=-=-=-=-=-=-=-=

    glm::vec2 defaultStretch = glm::vec2(1.0f, 1.0f);
    
    bool lockXAxis = false;
    bool lockZAxis = false;
    bool lockYAxis = false;
    
    float startSizeMin = 0.5f;   // min size at birth
    float startSizeMax = 1.5f;   // max size at birth
    float startAlphaMin= 1.0f;   // base alpha
    float startAlphaMax= 1.0f;   // base alpha
    float lifetimeMin  = 1.0f;   // min lifetime
    float lifetimeMax  = 4.0f;   // max lifetime
    float rotationMin  = 0.0f;   // min rotation at birth
    float rotationMax  = 360.0f; // max rotation at birth
    float rotationSpeedMin = -7.5f;  // min rotation speed
    float rotationSpeedMax = 7.5f;   // max rotation speed
    float velocityScaleMin = 0.0f;   // min velocity scale
    float velocityScaleMax = 2.0f;   // max velocity scale
    
    float emissionRate;           // spawn N particles per second
    float duration;           // spawn N particles per second

    bool  localSpace;             // local or world coordinate system
    bool glow;
    float glowIntensity = 0.0f;
    int   maxParticles;           // max particles
    EmitterShape shape;           // spawn shape
    glm::vec3 spawnPosition;      // offset
    glm::vec3 spawnVelocity;      // base velocity
    
    EmitterMode emitterMode = EmitterMode::Continuous;
    
    bool faceCamera = true;
    glm::vec3 customLookDir = glm::vec3(0,0,1); 
    glm::vec3 customUpDir   = glm::vec3(0,1,0);

    // BURST
    std::vector<float> burstTimes;    // times at which to emit many at once
    std::vector<int>   burstCounts;   // how many particsl for each burst

    // =-=-=-=-=-=-=-=-= State =-=-=-=-=-=-=-=-=-=
    ParticleSystemState systemState;

    ParticleNode(const std::string& name);
    virtual ~ParticleNode();    
    
    // std::vector<Particle> getParticles() {return m_particles;}
    std::vector<Particle> getParticles() { return std::vector<Particle>(m_particles, m_particles + m_numParticles); }
    std::vector<Affector*> getAffectors() const {return m_affectors;}

    void AddAffector(Affector* x) { if(x) m_affectors.push_back(x);}
    void RemoveAffector(int index) 
    {
        Affector* A = m_affectors[index];
        m_affectors.erase(m_affectors.begin() + index);
        delete A;     
    }

    // =-=-=-=-=-=-=-=-= Controlling partcisystm =-=-=-=-=-=-=-=-=-=
    void Play();
    void Stop();
    void End();
    bool ended = false;
    void Pause();
    void Restart(); // does Stop(), then Play()
    void spawnParticles(int count);  // spawns immediately

    virtual void update(float dt) override;
    virtual void draw(const glm::mat4& proj, const glm::mat4& view) override;

private:

    // std::vector<Particle> m_particles; 
    Particle m_particles[MAX_PARTICLES_INTERNAL];
    int m_numParticles = 0;
    std::vector<Affector*> m_affectors;
    float  m_emissionAdder;             // for continuous spawning
    float  m_totalTime;                 // track total for bursts (ADD MORE FUNCTIONALITY LATER (LOOK INTO UNITY))

    std::mt19937 m_randGen;                  
    std::uniform_real_distribution<float> m_dist01;

    volpe::VertexBuffer*      m_vb   = nullptr;
    volpe::VertexDeclaration* m_decl = nullptr;
    GLsizeiptr prevSZ;

    struct QuadVertex {
        GLfloat x, y, z;        // Position
        GLfloat u, v, w;        // uv + texture layer
        GLfloat cr, cg, cb, ca; // RGBA

        QuadVertex()
            : x(0), y(0), z(0),
            u(0.0f), v(0.0f), w(0.0),
            cr(0.0f), cg(0.0f), cb(0.0), ca(1.0f)
        {}

        QuadVertex(GLfloat px, GLfloat py, GLfloat pz,
            GLfloat pu, GLfloat pv, GLfloat pw,
            GLfloat pr, GLfloat pg, GLfloat pb, GLfloat pa)
            : x(px), y(py), z(pz),
            u(pu), v(pv), w(pw),
            cr(pr), cg(pg), cb(pb), ca(pa)
        {}
    };
    
    virtual void UpdateBoundingVolume() override;

    void handleBursts(float dt);
    void buildVertexData(const glm::mat4& view);
    void getCameraRightUp(const glm::mat4& view, glm::vec3& outRight, glm::vec3& outUp);
    Particle createNewParticle();
    glm::vec4 EvaluateColorGradient(float lifeRatio);
};

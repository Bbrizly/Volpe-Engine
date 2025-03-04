#pragma once
#include "Node.h"
#include "../volpe/Volpe.h"
#include <glm/gtc/matrix_transform.hpp>
#include <random>
#include <vector>
#include <string>

enum class EmitterShape
{
    Point,
    Sphere,
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

class ParticleNode : public Node 
{
public:
    struct Particle {
        glm::vec3 position;
        glm::vec3 velocity;       
        glm::vec3 acceleration;   
        float     lifetime;       // total time to live (inseconds)
        float     age;            // how long it has been alive
        float     size;           // current size
        float     initialSize;    // size at birth ;; for over-time effects
        float     rotation;       // billboard rotation in degrees
        float     rotationSpeed;  
        glm::vec4 color;          // current RGBA (ADD GRADIENT FUNCTIONALITY TO EMITTER)
        glm::vec4 initialColor;   // color at birth ;; for over-time effects
        float textureIndex;  // which layer in the array

        Particle()
            : position(0), velocity(0), acceleration(0),
              lifetime(4.0f), age(0.0f),
              size(1.0f), initialSize(1.0f),
              rotation(0.0f), rotationSpeed(0.0f),
              color(1.0f), initialColor(1.0f) {}
    };

    bool     useTextureArray = false;
    GLuint   textureArrayID  = 0;
    int      numTextures     = 0;   // how many layers in the array
    
    std::vector<ColorKey> colorKeys;

    // =-=-=-=-=-=-=-=-= Emitter Controls =-=-=-=-=-=-=-=-=-=
    
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
    bool  localSpace;             // local or world coordinate system
    int   maxParticles;           // max particles
    EmitterShape shape;           // spawn shape
    glm::vec3 spawnPosition;      // offset
    glm::vec3 spawnVelocity;      // base velocity
    glm::vec3 globalAcceleration; // affectors like gravity

    // BURST
    std::vector<float> burstTimes;    // times at which to emit many at once
    std::vector<int>   burstCounts;   // how many particsl for each burst

    // Over-lifetime multipliers (CUrrently linear, use imgui curves maybe)
    float sizeOverLife;   
    float alphaOverLife;  

    // =-=-=-=-=-=-=-=-= State =-=-=-=-=-=-=-=-=-=
    ParticleSystemState systemState;

    ParticleNode(const std::string& name);
    virtual ~ParticleNode();    
    std::vector<Particle> getParticles() {return m_particles;}
    void LoadFromXML(const std::string& xmlPath);

    // =-=-=-=-=-=-=-=-= Controlling partcisystm =-=-=-=-=-=-=-=-=-=
    void Play();
    void Stop();
    void Pause();
    void Restart(); // does Stop(), then Play()
    void spawnParticles(int count);  // spawns immediately



    virtual void update(float dt) override;
    virtual void draw(const glm::mat4& proj, const glm::mat4& view) override;

private:

    std::vector<Particle> m_particles; 
    float  m_emissionAdder;             // for continuous spawning
    float  m_totalTime;                 // track total for bursts (ADD MORE FUNCTIONALITY LATER (LOOK INTO UNITY))

    std::mt19937 m_randGen;                  
    std::uniform_real_distribution<float> m_dist01;

    volpe::VertexBuffer*      m_vb   = nullptr;
    volpe::VertexDeclaration* m_decl = nullptr;
    GLsizeiptr prevSZ; // previous size of vertex buffer FOR UPDATE
    // volpe::Material*          m_material = nullptr;

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

    void handleBursts(float dt);
    void buildVertexData(const glm::mat4& view);
    void getCameraRightUp(const glm::mat4& view, glm::vec3& outRight, glm::vec3& outUp);
    Particle createNewParticle();
    glm::vec4 EvaluateColorGradient(float lifeRatio);
};

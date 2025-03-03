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

        Particle()
            : position(0), velocity(0), acceleration(0),
              lifetime(4.0f), age(0.0f),
              size(1.0f), initialSize(1.0f),
              rotation(0.0f), rotationSpeed(0.0f),
              color(1.0f), initialColor(1.0f) {}
    };

    // =-=-=-=-=-=-=-=-= Emitter Controls =-=-=-=-=-=-=-=-=-=
    float emissionRate;           // spawn N particles per second
    bool  localSpace;             // local or world coordinate s
    int   maxParticles;           // max particles
    EmitterShape shape;           // Where spawn position will be at (Currently shapes are hardoceded)
    glm::vec3 spawnPosition;      // offset
    glm::vec3 spawnVelocity;      // base velocity
    glm::vec3 globalAcceleration; // Affectors like gravity

    // BURST
    std::vector<float> burstTimes;    // times at which to emit many at once
    std::vector<int>   burstCounts;   // how many particsl for each burst

    // Over-lifetime multipliers (CURRrently linear, use imgui curves maybe)
    float sizeOverLife;   
    float alphaOverLife;  

    // =-=-=-=-=-=-=-=-= State =-=-=-=-=-=-=-=-=-=
    ParticleSystemState systemState;

    ParticleNode(const std::string& name);
    virtual ~ParticleNode();    
    std::vector<Particle> getParticles() {return m_particles;}

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
    volpe::Material*          m_material = nullptr;

    struct QuadVertex {
        GLfloat x, y, z;       // Position
        GLfloat u, v;          // UV 

        QuadVertex()
            : x(0), y(0), z(0),
            u(0.0f), v(0.0f)
        {}

        QuadVertex(GLfloat px, GLfloat py, GLfloat pz,
            GLfloat pu, GLfloat pv)
            : x(px), y(py), z(pz),
            u(pu), v(pv)
        {}
    };

    void handleBursts(float dt);
    void buildVertexData(const glm::mat4& view);
    void getCameraRightUp(const glm::mat4& view, glm::vec3& outRight, glm::vec3& outUp);
    Particle createNewParticle();
};

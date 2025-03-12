#include "ParticleNode.h"
#include "../samplefw/BoundingVolumes/SphereVolume.h"
#include <glm/gtc/matrix_inverse.hpp>
#include <algorithm>
#include <cmath>
#include "Scene.h"
#include "iostream"


ParticleNode::ParticleNode(const std::string& name)
: Node(name)
, emissionRate(10.0f)
, localSpace(false)
, glow(false)
, maxParticles(1000)
, shape(EmitterShape::Point)
, spawnPosition(0.0f)
, spawnVelocity(0.0f, 1.0f, 0.0f)
, systemState(ParticleSystemState::Stopped)
, m_emissionAdder(0.0f)
, m_totalTime(0.0f)
, duration(-1)
, m_randGen(std::random_device{}())
, m_dist01(0.0f, 1.0f)
{
    SphereVolume* vol = new SphereVolume(glm::vec3(0), 20.0f);
    SetBoundingVolume(vol);

    m_material = volpe::MaterialManager::CreateMaterial(name + "Mat"); //to work for multiple particle systems.
    m_material->SetProgram("data/particle.vsh", "data/particle.fsh");
    m_ownsMaterial = true;
    SetReactToLight(false);
    m_numParticles = 0;
    Play();
}

ParticleNode::~ParticleNode()
{
    m_numParticles = 0;
    // m_particles.clear();
    if(m_vb) {
        volpe::BufferManager::DestroyBuffer(m_vb);
        m_vb = nullptr;
    }
    if(m_decl) {
        delete m_decl;
        m_decl = nullptr;
    }
    if(m_material) {
        volpe::MaterialManager::DestroyMaterial(m_material);
        m_material = nullptr;
    }
}

// =-=-=-=-=-=-=-=-=CONTROL SHITS=-=-=-=-=-=-=-=-=-=
void ParticleNode::Play()
{
    ended = false;
    if(systemState == ParticleSystemState::Playing) return;
    systemState = ParticleSystemState::Playing;
}

void ParticleNode::Stop()
{
    systemState = ParticleSystemState::Stopped;

    m_numParticles = 0;

    m_emissionAdder = 0.0f;
    m_totalTime = 0.0f;
}
void ParticleNode::End()
{
    systemState = ParticleSystemState::Stopped;
    ended = true;

    m_emissionAdder = 0.0f;
    m_totalTime = 0.0f;
}

void ParticleNode::Pause()
{
    if(systemState == ParticleSystemState::Playing) {
        systemState = ParticleSystemState::Paused;
    }
}

void ParticleNode::Restart()
{
    Stop();
    Play();
}

// =-=-=-=-=-=-=-=-=UPDATE PARTICLES=-=-=-=-=-=-=-=-=-=
void ParticleNode::update(float dt)
{
    Node::update(dt);
    if(dt <= 0.0f) return;

    #pragma region System State
    if (m_totalTime >= duration && duration > 0)
    {
        End();
    }
    else if(systemState == ParticleSystemState::Stopped && !ended) {
        m_numParticles = 0;
        return;
    }

    if(systemState == ParticleSystemState::Paused)
        dt = 0;
    #pragma endregion
    
    #pragma region Emitter Mode
    if(emitterMode == EmitterMode::Continuous)
    {
        if(emissionRate > 0.0f)
        {
            m_emissionAdder += (emissionRate * dt);
            int spawnCount = (int)std::floor(m_emissionAdder);
            if(spawnCount>0)
            {
                m_emissionAdder -= spawnCount;
                spawnParticles(spawnCount);
            }
        }
    }
    else if (emitterMode == EmitterMode::Burst)
    {
        if (!burstTimes.empty()) {
            
            float maxBurst = *std::max_element(burstTimes.begin(), burstTimes.end());
            
            if (m_totalTime > maxBurst && m_numParticles == 0) {
                m_totalTime = 0.0f;
            }
        }
    }

    m_totalTime += dt;
    handleBursts(dt);

    #pragma endregion

    #pragma region Update each particle
    int i = 0;
    while(i < m_numParticles)
    {
        Particle& p = m_particles[i];
        p.age += dt;

        // swap of old age :)
        if(p.age >= p.lifetime) {
            m_particles[i] = m_particles[m_numParticles - 1];
            m_numParticles--;
            continue;
        }

        // Rotation
        p.rotation += p.rotationSpeed * dt;
        // Position
        p.position += p.velocity * dt;

        // Over-lifetime color
        float t = (p.age / p.lifetime);
        if(t>1.f) t=1.f;
        p.color = EvaluateColorGradient(t);

        // Apply affectors
        for(auto x : m_affectors) {
            x->Apply(p,dt,getWorldTransform());
        }

        i++;
    }

    if(m_numParticles > maxParticles) {
        m_numParticles = maxParticles;
    }
    #pragma endregion

}

void ParticleNode::UpdateBoundingVolume()
{
    if(m_numParticles > 0) {
        glm::vec3 pMin = m_particles[0].position;
        glm::vec3 pMax = m_particles[0].position;
        for (int i=0; i<m_numParticles; i++) {
            const Particle& p = m_particles[i];
            pMin = glm::min(pMin, p.position);
            pMax = glm::max(pMax, p.position);
        }
        glm::vec3 newCenter = (pMin + pMax) * 0.5f;
        float newRadius = glm::length(pMax - newCenter);
        SphereVolume* sphere = dynamic_cast<SphereVolume*>(m_boundingVolume);
        if(sphere) {
            sphere->center        = newCenter;
            sphere->radius        = newRadius;
            sphere->m_localCenter = newCenter;
            sphere->m_initialRadius = newRadius;
        }
    }
}

void ParticleNode::draw(const glm::mat4& proj, const glm::mat4& view)
{
    
    // If not playing or no material skip
    if((systemState == ParticleSystemState::Stopped && !ended) || m_numParticles == 0 || !m_material) {
        Node::draw(proj, view);
        return;
    }

    // Get camera position 
    glm::vec3 camPos = (glm::inverse(view))[3];

    std::sort(m_particles, m_particles + m_numParticles,
    [&camPos](const Particle &a, const Particle &b) {
        float da = glm::dot(a.position - camPos, a.position - camPos);
        float db = glm::dot(b.position - camPos, b.position - camPos);
        return da > db;
    });

    buildVertexData(view);

    if(!m_vb || !m_decl) {
        Node::draw(proj, view);
        return;
    }

    if(glow)
        m_material->SetUniform("u_glowIntensity", glowIntensity);
    else
        m_material->SetUniform("u_glowIntensity", 0.0f);

    m_material->SetUniform("projection", proj);
    m_material->SetUniform("view",       view);
    

    glm::mat4 worldMat(1.0f);
    if(localSpace) {
        worldMat = getWorldTransform();
    }
    glm::mat4 worldIT = glm::transpose(glm::inverse(worldMat));
    m_material->SetUniform("world",   worldMat);
    m_material->SetUniform("worldIT", worldIT);

    m_material->SetUniform("useTexture", true);

    m_material->SetUniform("u_color", glm::vec3(1,1,1));

    m_material->Apply();
    m_decl->Bind();

    int totalVerts = m_numParticles * 6;

    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    if(glow)
        glBlendFunc(GL_ONE,GL_ONE);
    else
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    

    glDrawArrays(GL_TRIANGLES, 0, totalVerts);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    Node::draw(proj, view);
}

void ParticleNode::spawnParticles(int count)
{
    if(systemState == ParticleSystemState::Stopped) return;
    if(count <= 0) return;

    for(int i=0; i<count; i++)
    {
        if(m_numParticles >= maxParticles || m_numParticles >= MAX_PARTICLES_INTERNAL) {
            break;
        }
        Particle p = createNewParticle();
        m_particles[m_numParticles++] = p;
    }
}

Particle ParticleNode::createNewParticle()
{
    Particle p;

    p.lifetime = lifetimeMin + (lifetimeMax - lifetimeMin) * m_dist01(m_randGen);
    p.age      = 0.0f;

    p.rotation = rotationMin + (rotationMax - rotationMin) * m_dist01(m_randGen);
    p.rotationSpeed = rotationSpeedMin + (rotationSpeedMax - rotationSpeedMin) * m_dist01(m_randGen);

    // color
    float alpha = startAlphaMin + (startAlphaMax - startAlphaMin) * m_dist01(m_randGen);
    p.color        = glm::vec4(1,1,1,alpha); // ADD COLOUR GRADIENTTTT AAAAAA
    p.initialColor = p.color;
    
    if(useTextureArray && numTextures > 0) {
        p.textureIndex = rand() % numTextures;
        // p.textureIndex = 1; //UNHARCDODE LATERRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR
    } else {
        p.textureIndex = 0;
    }

    // size
    p.initialSize = startSizeMin + (startSizeMax - startSizeMin) * m_dist01(m_randGen);
    p.size        = p.initialSize;
    
    p.stretch = defaultStretch;

    // acceleration
    p.acceleration = glm::vec3(0);

    // shape-based spawn
    glm::vec3 localSpawn(0);
    switch(shape)  // ------------ REMEMBER TO UNHARDCODE IMPORTANTTTT
    {
        case EmitterShape::Point:
            localSpawn = glm::vec3(0);
            break;
        case EmitterShape::Sphere:
        {
            float theta = 2.f*PI*m_dist01(m_randGen);
            float phi   = std::acos(1.f -2.f*m_dist01(m_randGen));
            float rr    = 1.5f*m_dist01(m_randGen);
            localSpawn.x = rr*std::sin(phi)*std::cos(theta);
            localSpawn.y = rr*std::sin(phi)*std::sin(theta);
            localSpawn.z = rr*std::cos(phi);
        }
        break;
        case EmitterShape::Cone:
        {
            float angle =  PI/6.f; 
            float r = 1.f*m_dist01(m_randGen);
            float aa=2.f*3.14159f*m_dist01(m_randGen);
            localSpawn.x = r*std::cos(aa)*angle;
            localSpawn.y = 0.0f;
            localSpawn.z = r*std::sin(aa)*angle;
        }
        break;
        case EmitterShape::Box:
        {
            localSpawn.x = 2.f*m_dist01(m_randGen)-1.f;
            localSpawn.y = 2.f*m_dist01(m_randGen)-1.f;
            localSpawn.z = 2.f*m_dist01(m_randGen)-1.f;
        }
        break;
        case EmitterShape::Donut:
        {
            float majorRadius = 1.5f;
            float minorRadius = 0.2f;

            float u = 2.0f * PI * m_dist01(m_randGen);
            float v = 2.0f * PI * m_dist01(m_randGen);

            float cosU = cosf(u);
            float sinU = sinf(u);
            float cosV = cosf(v);
            float sinV = sinf(v);

            float R = majorRadius;
            float r = minorRadius;

            float x = (R + r*cosV) * cosU;
            float z = (R + r*cosV) * sinU;
            float y = r * sinV;

            localSpawn = glm::vec3(x, y, z);
        }
        break;
        case EmitterShape::Mesh:
            // ACCORDING TO WHATEVER MESH IN NODE
        default:
            localSpawn = glm::vec3(0);
        break;
    }

    //  offset by spawnPosition
    localSpawn += spawnPosition;

   if(!localSpace) {
        glm::vec4 worldSpawn = getWorldTransform() * glm::vec4(localSpawn, 1.0f);
        p.position = glm::vec3(worldSpawn);
    } else {
        p.position = localSpawn;
    }

    // velocity
    glm::vec3 randomDir(m_dist01(m_randGen) - 0.5f,
                        m_dist01(m_randGen) - 0.5f,
                        m_dist01(m_randGen) - 0.5f);
    randomDir = glm::normalize(randomDir);
    float sp = velocityScaleMin + (velocityScaleMax - velocityScaleMin) * m_dist01(m_randGen);
    //Give ability to lock directions
    p.velocity = spawnVelocity + randomDir * sp;

    // transform velocity if not local
    if(!localSpace) {
        glm::mat3 rotOnly = glm::mat3(glm::transpose(glm::inverse(getWorldTransform())));
        p.velocity = rotOnly * p.velocity;
    }

    return p;
}

void ParticleNode::handleBursts(float dt)
{
    for(size_t i=0; i<burstTimes.size(); i++)
    {
        float t = burstTimes[i];
        //IF past burst time then spawn particles
        if((m_totalTime - dt) < t && m_totalTime >= t) {
            spawnParticles(burstCounts[i]);
        }
    }
}

void ParticleNode::buildVertexData(const glm::mat4& view)
{
    if(m_numParticles <= 0) return;

    std::vector<QuadVertex> data;
    data.reserve(m_numParticles*6);

    glm::vec3 camRight, camUp;
    if(faceCamera)
    {
        getCameraRightUp(view, camRight, camUp);
    }
    else
    {
        camUp           = glm::normalize(customUpDir);
        glm::vec3 fwd   = glm::normalize(customLookDir);
        camRight        = glm::normalize(glm::cross(camUp,fwd));
    }

    for(int i=0; i<m_numParticles; i++)
    {
        const Particle& p = m_particles[i];
        if(p.age>=p.lifetime) continue;

        float t = p.age / p.lifetime;
        glm::vec4 color = EvaluateColorGradient(t);

        // final alpha
        float finalAlpha = color.a * p.color.a;
        float c = std::cos(glm::radians(p.rotation));
        float s = std::sin(glm::radians(p.rotation));

        float halfStretchX = p.size * p.stretch.x * 0.5f;
        float halfStretchY = p.size * p.stretch.y * 0.5f;

        glm::vec3 rRot = ( c*camRight + s*camUp ) * halfStretchX;
        glm::vec3 uRot = (-s*camRight + c*camUp ) * halfStretchY;

        glm::vec3 bl = p.position - rRot - uRot;
        glm::vec3 br = p.position + rRot - uRot;
        glm::vec3 tl = p.position - rRot + uRot;
        glm::vec3 tr = p.position + rRot + uRot;

        float texLayer = p.textureIndex;
        
        QuadVertex v0 = {bl.x, bl.y, bl.z,  0.f, 1.f, texLayer, color.r, color.g, color.b, finalAlpha};
        QuadVertex v1 = {br.x, br.y, br.z,  1.f, 1.f, texLayer, color.r, color.g, color.b, finalAlpha};
        QuadVertex v2 = {tl.x, tl.y, tl.z,  0.f, 0.f, texLayer, color.r, color.g, color.b, finalAlpha};
        QuadVertex v3 = {tr.x, tr.y, tr.z,  1.f, 0.f, texLayer, color.r, color.g, color.b, finalAlpha};

        data.push_back(v0);
        data.push_back(v1);
        data.push_back(v2);
        data.push_back(v2);
        data.push_back(v1);
        data.push_back(v3);
    }
    
    GLsizeiptr sz = (GLsizeiptr)(data.size()*sizeof(QuadVertex));
    //If try to update buffer, particle system breaks... Look into pooling
    if(!m_vb || sz != prevSZ) {
        if(m_vb) {
            volpe::BufferManager::DestroyBuffer(m_vb);
            m_vb = nullptr;
        }
        m_vb = volpe::BufferManager::CreateVertexBuffer(data.data(), (unsigned int)sz);
        if(m_decl) {
            delete m_decl;
            m_decl = nullptr;
        }
        m_decl = new volpe::VertexDeclaration();
        m_decl->Begin();
          m_decl->SetVertexBuffer(m_vb);
          m_decl->AppendAttribute(volpe::AT_Position,   3, volpe::CT_Float);
          m_decl->AppendAttribute(volpe::AT_TexCoord1,  3, volpe::CT_Float);
          m_decl->AppendAttribute(volpe::AT_Color,      4, volpe::CT_Float);
        m_decl->End();
        prevSZ = sz;//FOR FIXING UPDATE BUFFER 
    } else {
        // std::cout<<"SAVED COMPUTING POWER\n";
        volpe::BufferManager::UpdateVertexBuffer(m_vb, data.data(), (unsigned int)sz);
    } 
}

// =-=-=-=-=-=-=-=-=HELPERS=-=-=-=-=-=-=-=-=-=

void ParticleNode::getCameraRightUp(const glm::mat4& view, glm::vec3& outRight, glm::vec3& outUp)
{
    glm::mat3 camRot = glm::inverse(glm::mat3(view));
    glm::vec3 camRight = glm::normalize(glm::vec3(camRot[0]));
    glm::vec3 camUp    = glm::normalize(glm::vec3(camRot[1]));

    const glm::vec3 globalRight   = glm::vec3(1, 0, 1);
    const glm::vec3 globalUp      = glm::vec3(0, 1, 0);

    // and if not locked then use 1 (takes cam values)
    glm::vec3 mask( lockXAxis ? 0.0f : 1.0f,
                    lockYAxis ? 0.0f : 1.0f,
                    lockZAxis ? 0.0f : 1.0f );
                
    glm::vec3 invMask = glm::vec3(1.0f) - mask;

    //mask is just xyz as 1 and 0. so if x is 0 then we multiply it 
    //by camright and we lose the camright and are left with the added globalRight that is defined bfeore
    glm::vec3 blendedRight = (camRight * mask) + (globalRight * invMask);
    blendedRight = glm::normalize(blendedRight);
    glm::vec3 blendedUp = (camUp * mask) + (globalUp * invMask);
    blendedUp = glm::normalize(blendedUp - glm::dot(blendedUp, blendedRight) * blendedRight);


    outRight = blendedRight;
    outUp = blendedUp;
}

glm::vec4 ParticleNode::EvaluateColorGradient(float t)
{
    if(colorKeys.empty()) {
        return glm::vec4(1.0f);
    }
    if(t <= colorKeys.front().time) {
        return colorKeys.front().color; //KEEPS FUCKING BREAKING //fix limit to 8 keys // fixed & removed limit
    }
    if(t >= colorKeys.back().time) {
        return colorKeys.back().color;
    }

    // find the two keys around t
    for(size_t i=0; i<colorKeys.size()-1; i++)
    {
        const auto& k1 = colorKeys[i];
        const auto& k2 = colorKeys[i+1];
        if(t >= k1.time && t <= k2.time)
        {
            float range = k2.time - k1.time;
            float alpha = (t - k1.time)/range;
            // Lerp between
            return glm::mix(k1.color, k2.color, alpha);
        }
    }
    return colorKeys.back().color;
}


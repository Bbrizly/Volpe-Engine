#include "ParticleNode.h"
#include "SphereVolume.h"
#include <glm/gtc/matrix_inverse.hpp>
#include <algorithm>
#include <cmath>
#include "Scene.h"
// #include <tinyxml2.h>
// #include <xmllite.h>
// using namespace tinyxml2;

ParticleNode::ParticleNode(const std::string& name)
: Node(name)
, emissionRate(10.0f)
, localSpace(false)
, maxParticles(1000)
, shape(EmitterShape::Point)
, spawnPosition(0.0f)
, spawnVelocity(0.0f, 1.0f, 0.0f)
, globalAcceleration(0.0f, -9.81f, 0.0f)
, sizeOverLife(1.0f)
, alphaOverLife(1.0f)
, systemState(ParticleSystemState::Stopped)
, m_emissionAdder(0.0f)
, m_totalTime(0.0f)
, m_randGen(std::random_device{}())
, m_dist01(0.0f, 1.0f)
{
    // Bounding volume just so it appears, in the future make it so it encapsulates all particles
    SphereVolume* vol = new SphereVolume(glm::vec3(0), 20.0f);
    SetBoundingVolume(vol);

    // m_material = volpe::MaterialManager::CreateMaterial("ParticleSystemMaterial");
    // std:string s_name = name;
    // if(name.empty()) s_name = "unnamed";
    m_material = volpe::MaterialManager::CreateMaterial(name + "Mat"); //to work for multiple particle systems.
    m_material->SetProgram("data/particle.vsh", "data/particle.fsh");
    SetReactToLight(false);
    // std::cout<<"Constuctor Material: "<<m_material<<"\n";
    // std::cout<<"GetMaterial: "<<GetMaterial()<<"\n";
}

ParticleNode::~ParticleNode()
{
    m_particles.clear();
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
    if(systemState == ParticleSystemState::Playing) return;
    systemState = ParticleSystemState::Playing;
}

void ParticleNode::Stop()
{
    systemState = ParticleSystemState::Stopped;
    // Clear all active
    m_particles.clear();
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

    if(systemState == ParticleSystemState::Stopped) {
        // no update, no spawn, no existing
        m_particles.clear();
        return;
    }

    if(systemState == ParticleSystemState::Paused) {
        // not sure how to make them render and not update their position
        // skip the rest
        return;
    }
    // prevDt = dt;

    //FOR BURSTS
    m_totalTime += dt;
    handleBursts(dt);

    //
    if(emissionRate > 0.0f) {
        m_emissionAdder += (emissionRate * dt);
        int spawnCount = (int)std::floor(m_emissionAdder);
        if(spawnCount > 0) {
            m_emissionAdder -= spawnCount;
            spawnParticles(spawnCount);
        }
    }

    for(auto& p : m_particles) 
    {
        p.age += dt;
        if(p.age >= p.lifetime) continue; // remove soon

        // Acceleration
        glm::vec3 totalAccel = p.acceleration + globalAcceleration;
        p.velocity += totalAccel * dt;

        // Rotation
        p.rotation += p.rotationSpeed * dt;

        // Position
        p.position += p.velocity * dt;

        
        // =-=-=-=-=-=-=-=-=TWEENING=-=-=-=-=-=-=-=-=-=

        // Over-lifetime
        float t = (p.age / p.lifetime);
        if(t>1.f) t=1.f;

        // Size
        float finalSize = p.initialSize * sizeOverLife;
        p.size = p.initialSize + (finalSize - p.initialSize)*t;

        
        p.color = EvaluateColorGradient(t);

        // Alpha
        float a0 = p.initialColor.a;
        float a1 = p.initialColor.a * alphaOverLife;
        float newAlpha = a0 + (a1 - a0)*t;
        p.color.a = newAlpha;
    }

    //delete of old age
    m_particles.erase(
        std::remove_if(m_particles.begin(), m_particles.end(),
            [&](const Particle& part){
                return (part.age >= part.lifetime);
            }
        ),
        m_particles.end()
    );

    if((int)m_particles.size() > maxParticles) {
        m_particles.resize(maxParticles);
    }
}

void ParticleNode::draw(const glm::mat4& proj, const glm::mat4& view)
{
    
    // If not playing or no material skip
    if(systemState != ParticleSystemState::Playing || m_particles.empty() || !m_material) {
        Node::draw(proj, view);
        return;
    }

    // std::cout<<"Draw Material: "<<m_material<<"\n";
    // std::cout<<"GetMaterial: "<<GetMaterial()<<"\n";

    // // Get camera position 
    // glm::vec3 camPos = Scene::Instance().GetActiveCamera()->getViewMatrix()[3];
    // // Sort particles in descending order (farthest first)
    // std::sort(m_particles.begin(), m_particles.end(),
    //     [&camPos](const ParticleNode::Particle &a, const ParticleNode::Particle &b) {
    //         float da = glm::distance(a.position, camPos);
    //         float db = glm::distance(b.position, camPos);
    //         return da > db;
    // });


    buildVertexData(view);

    if(!m_vb || !m_decl) {
        Node::draw(proj, view);
        return;
    }

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

    int totalVerts = (int)m_particles.size()*6;
    glDepthMask(GL_FALSE);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    
    // glEnable(GL_BLEND);
    // glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    glDrawArrays(GL_TRIANGLES, 0, totalVerts);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_TRUE);

    Node::draw(proj, view);
}

void ParticleNode::spawnParticles(int count)
{
    if(systemState == ParticleSystemState::Stopped) return;
    if(count <= 0) return;

    for(int i=0; i<count; i++)
    {
        if((int)m_particles.size() >= maxParticles) break;
        Particle p = createNewParticle();
        m_particles.push_back(p);
    }
}

ParticleNode::Particle ParticleNode::createNewParticle()
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
            localSpawn.y = r*std::sin(aa)*angle;
            localSpawn.z = 0.0f;
        }
        break;
        case EmitterShape::Box:
        {
            localSpawn.x = 2.f*m_dist01(m_randGen)-1.f;
            localSpawn.y = 2.f*m_dist01(m_randGen)-1.f;
            localSpawn.z = 2.f*m_dist01(m_randGen)-1.f;
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
    if(m_particles.empty()) return;

    std::vector<QuadVertex> data;
    data.reserve(m_particles.size()*6);

    glm::vec3 camRight, camUp;
    getCameraRightUp(view, camRight, camUp);

    for(const auto& p : m_particles)
    {
        if(p.age>=p.lifetime) continue;

        float t = p.age / p.lifetime;
        glm::vec4 color = EvaluateColorGradient(t);

        // final alpha
        float finalAlpha = color.a * p.color.a;
        float c = std::cos(glm::radians(p.rotation));
        float s = std::sin(glm::radians(p.rotation));

        glm::vec3 rRot = ( c*camRight + s*camUp )*(p.size*0.5f);
        glm::vec3 uRot = (-s*camRight + c*camUp )*(p.size*0.5f);

        glm::vec3 bl = p.position - rRot - uRot;
        glm::vec3 br = p.position + rRot - uRot;
        glm::vec3 tl = p.position - rRot + uRot;
        glm::vec3 tr = p.position + rRot + uRot;

        float texLayer = p.textureIndex;

        // QuadVertex v0 = { bl.x, bl.y, bl.z, 0.f, 0.f };
        // QuadVertex v1 = { br.x, br.y, br.z, 1.f, 0.f };
        // QuadVertex v2 = { tl.x, tl.y, tl.z, 0.f, 1.f };
        // QuadVertex v3 = { tr.x, tr.y, tr.z, 1.f, 1.f };
        
        QuadVertex v0 = {bl.x, bl.y, bl.z,  0.f, 0.f, texLayer, color.r, color.g, color.b, finalAlpha};
        QuadVertex v1 = {br.x, br.y, br.z,  1.f, 0.f, texLayer, color.r, color.g, color.b, finalAlpha};
        QuadVertex v2 = {tl.x, tl.y, tl.z,  0.f, 1.f, texLayer, color.r, color.g, color.b, finalAlpha};
        QuadVertex v3 = {tr.x, tr.y, tr.z,  1.f, 1.f, texLayer, color.r, color.g, color.b, finalAlpha};

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
        m_vb = volpe::BufferManager::CreateVertexBuffer(data.data(), (unsigned int)sz);

        m_decl = new volpe::VertexDeclaration();
        m_decl->Begin();
          m_decl->SetVertexBuffer(m_vb);
          m_decl->AppendAttribute(volpe::AT_Position,   3, volpe::CT_Float);
          m_decl->AppendAttribute(volpe::AT_TexCoord1,  3, volpe::CT_Float);
          m_decl->AppendAttribute(volpe::AT_Color,      4, volpe::CT_Float);
        m_decl->End();
    } else {
        // std::cout<<"SAVED COMPUTING POWER\n";
        volpe::BufferManager::UpdateVertexBuffer(m_vb, data.data(), (unsigned int)sz);
    }
    prevSZ = sz; //FOR FIXING UPDATE BUFFER 
}

// =-=-=-=-=-=-=-=-=HELPERS=-=-=-=-=-=-=-=-=-=
void ParticleNode::getCameraRightUp(const glm::mat4& view, glm::vec3& outRight, glm::vec3& outUp)
{
    //billboard codee
    glm::mat3 rot = glm::inverse(glm::mat3(view));
    outRight = glm::normalize(glm::vec3(rot[0][0], rot[0][1], rot[0][2]));
    outUp    = glm::normalize(glm::vec3(rot[1][0], rot[1][1], rot[1][2]));
}

glm::vec4 ParticleNode::EvaluateColorGradient(float t)
{
    if(colorKeys.empty()) {
        // fallback to some default
        return glm::vec4(1.0f);
    }
    if(t <= colorKeys.front().time) {
        return colorKeys.front().color;
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
            // Lerp
            return glm::mix(k1.color, k2.color, alpha);
        }
    }
    return colorKeys.back().color;
}

/*
void ParticleNode::LoadFromXML(const std::string& xmlPath)
{
    XMLDocument doc;
    if(doc.LoadFile(xmlPath.c_str()) == XML_SUCCESS)
    {
        XMLElement* root = doc.RootElement();
        if(!root) return;

        // read attributes
        root->QueryFloatAttribute("emissionRate", &emissionRate);
        root->QueryIntAttribute("maxParticles", &maxParticles);

        // read shape
        const char* shapeStr = root->Attribute("shape");
        if(shapeStr)
        {
            if(strcmp(shapeStr, "Point")==0) shape=EmitterShape::Point;
            else if(strcmp(shapeStr, "Sphere")==0) shape=EmitterShape::Sphere;
        }

        // read startSize
        root->QueryFloatAttribute("startSize", &startSize);
        root->QueryFloatAttribute("startAlpha", &startAlpha);

        // read color keys
        XMLElement* colorKeysElem = root->FirstChildElement("ColorKeys");
        if(colorKeysElem)
        {
            colorKeys.clear();
            for(XMLElement* ck = colorKeysElem->FirstChildElement("Key");
                ck; ck=ck->NextSiblingElement("Key"))
            {
                float time=0;  ck->QueryFloatAttribute("time", &time);
                float r=1,g=1,b=1,a=1;
                ck->QueryFloatAttribute("r", &r);
                ck->QueryFloatAttribute("g", &g);
                ck->QueryFloatAttribute("b", &b);
                ck->QueryFloatAttribute("a", &a);
                ColorKey ckey;
                ckey.time = time;
                ckey.color= glm::vec4(r,g,b,a);
                colorKeys.push_back(ckey);
            }
            // Sort by time
            std::sort(colorKeys.begin(), colorKeys.end(), 
                [](const ColorKey&c1, const ColorKey&c2){
                    return c1.time < c2.time;
                });
        }
    }
}

*/
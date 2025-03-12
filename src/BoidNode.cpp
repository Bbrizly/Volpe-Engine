#include "BoidNode.h"
#include "../samplefw/BoundingVolumes/SphereVolume.h"
#include <glm/gtc/matrix_inverse.hpp>
#include <algorithm>
#include <cmath>

BoidNode::BoidNode(const std::string& name)
    : Node(name)
    , maxBoids(100)
    , worldBounds(20.0f)
    , wrapAround(true)
    , doAlignment(true)
    , doCohesion(true)
    , doSeparation(true)
    , m_randGen(std::random_device{}())
    , m_dist01(0.0f, 1.0f)
{
    SphereVolume* vol = new SphereVolume(glm::vec3(0), worldBounds);
    SetBoundingVolume(vol);

    m_material = volpe::MaterialManager::CreateMaterial(name + "_BoidMat");
    m_material->SetProgram("data/particle.vsh", "data/particle.fsh");
    m_ownsMaterial = true;

    SetReactToLight(false);
}

BoidNode::~BoidNode()
{
    ClearBoids();
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

void BoidNode::SpawnBoids(int count)
{
    ClearBoids();

    for(int i=0; i<count; ++i)
    {
        Boid b;
        // random position
        float x = (m_dist01(m_randGen)*2.0f - 1.0f)*worldBounds;
        float y = (m_dist01(m_randGen)*2.0f - 1.0f)*worldBounds;
        float z = (m_dist01(m_randGen)*2.0f - 1.0f)*worldBounds;
        b.position = glm::vec3(x,y,z);

        // random velocity
        float vx = (m_dist01(m_randGen)*2.0f - 1.0f)*2.0f;
        float vy = (m_dist01(m_randGen)*2.0f - 1.0f)*2.0f;
        float vz = (m_dist01(m_randGen)*2.0f - 1.0f)*2.0f;
        b.velocity = glm::vec3(vx,vy,vz);

        // random color for fun
        float r = m_dist01(m_randGen);
        float g = m_dist01(m_randGen);
        float bl= m_dist01(m_randGen);
        b.color = glm::vec4(r,g,bl,1.0f);

        // default boid parameters
        b.maxSpeed       = 5.0f;
        b.maxForce       = 0.25f;
        b.neighborRadius = 4.0f;
        b.separationDist = 1.0f;
        b.size           = 0.5f;
        b.rotation       = 0.0f;

        m_boids.push_back(b);
    }
}

void BoidNode::ClearBoids()
{
    m_boids.clear();
}

std::vector<Boid> BoidNode::getBoids() const
{
    return m_boids;
}

void BoidNode::update(float dt)
{
    Node::update(dt);

    applyBoidRules(dt);

    for(auto& b : m_boids)
    {
        b.velocity += b.acceleration * dt;
        if(glm::length(b.velocity) > b.maxSpeed)
        {
            b.velocity = glm::normalize(b.velocity)*b.maxSpeed;
        }
        b.position += b.velocity*dt;
        
        if(wrapAround)
            wrapBoid(b);

        b.acceleration = glm::vec3(0.0f);

        b.age += dt;
    }
}
/*
Look at every other BOID (couldnt get oct-tree to wokr right now)
Do the big 3:
    Separation (dont hit other shits)
    Alignment (alight in direction of others)
    Cohesion (move towards center of group)
*/
void BoidNode::applyBoidRules(float dt)
{
    if(m_boids.empty()) return;

    for(size_t i=0; i<m_boids.size(); i++)
    {
        glm::vec3 sep(0.0f), ali(0.0f), coh(0.0f);
        if(doSeparation) sep = computeSeparation((int)i);
        if(doAlignment)  ali = computeAlignment((int)i);
        if(doCohesion)   coh = computeCohesion((int)i);

        // add all effects together and apply
        glm::vec3 accel = sep*1.5f + ali*1.0f + coh*1.0f;
        m_boids[i].acceleration += accel;
    }
}

// Separation (avoid hitting other boids)
glm::vec3 BoidNode::computeSeparation(int idx)
{
    Boid& me = m_boids[idx];
    glm::vec3 steer(0.0f);
    int count=0;
    for(size_t j=0; j<m_boids.size(); j++)
    {
        if(j== (size_t)idx) continue;
        Boid& other = m_boids[j];
        float dist = glm::distance(me.position, other.position);
        if(dist < me.neighborRadius && dist > 0.0001f)
        {
            // if too close push away
            if(dist < me.separationDist)
            {
                glm::vec3 diff = me.position - other.position;
                diff = glm::normalize(diff)/dist; 
                steer += diff;
                count++;
            }
        }
    }
    if(count > 0)
    {
        steer /= (float)count;
    }
    if(glm::length(steer) > 0.0001f)
    {
        steer = glm::normalize(steer)*me.maxSpeed - me.velocity;
        if(glm::length(steer) > me.maxForce)
            steer = glm::normalize(steer)*me.maxForce;
    }
    return steer;
}

// Alignment (head towards dir of other boids)
glm::vec3 BoidNode::computeAlignment(int idx)
{
    Boid& me = m_boids[idx];
    glm::vec3 avgVel(0.0f);
    int count=0;
    for(size_t j=0; j<m_boids.size(); j++)
    {
        if(j== (size_t)idx) continue;
        Boid& other = m_boids[j];
        float dist = glm::distance(me.position, other.position);
        if(dist < me.neighborRadius)
        {
            avgVel += other.velocity;
            count++;
        }
    }
    if(count > 0)
    {
        avgVel /= (float)count;
        avgVel = glm::normalize(avgVel)*me.maxSpeed;
        glm::vec3 steer = avgVel - me.velocity;
        if(glm::length(steer) > me.maxForce)
            steer = glm::normalize(steer)*me.maxForce;
        return steer;
    }
    return glm::vec3(0.0f);
}

// Cohesion (turn to move toward the average position other boids)
glm::vec3 BoidNode::computeCohesion(int idx)
{
    Boid& me = m_boids[idx];
    glm::vec3 center(0.0f);
    int count=0;
    for(size_t j=0; j<m_boids.size(); j++)
    {
        if(j== (size_t)idx) continue;
        Boid& other = m_boids[j];
        float dist = glm::distance(me.position, other.position);
        if(dist < me.neighborRadius)
        {
            center += other.position;
            count++;
        }
    }
    if(count > 0)
    {
        center /= (float)count;
        glm::vec3 desired = center - me.position;
        desired = glm::normalize(desired)*me.maxSpeed;
        glm::vec3 steer = desired - me.velocity;
        if(glm::length(steer) > me.maxForce)
            steer = glm::normalize(steer)*me.maxForce;
        return steer;
    }
    return glm::vec3(0.0f);
}

void BoidNode::wrapBoid(Boid& b)
{
    if(b.position.x > worldBounds)  b.position.x = -worldBounds;
    else if(b.position.x < -worldBounds) b.position.x = worldBounds;
    
    if(b.position.y > worldBounds)  b.position.y = -worldBounds;
    else if(b.position.y < -worldBounds) b.position.y = worldBounds;
    
    if(b.position.z > worldBounds)  b.position.z = -worldBounds;
    else if(b.position.z < -worldBounds) b.position.z = worldBounds;
}

void BoidNode::UpdateBoundingVolume()
{
    if(!m_boids.empty())
    {
        glm::vec3 minP(999999.0f), maxP(-999999.0f);
        
        for(auto &b : m_boids)
        {
            minP = glm::min(minP, b.position);
            maxP = glm::max(maxP, b.position);
        }
        glm::vec3 center = 0.5f*(minP+maxP);
        float radius = glm::length(maxP - center);

        SphereVolume* sphere = dynamic_cast<SphereVolume*>(m_boundingVolume);
        if(sphere)
        {
            sphere->center = center;
            sphere->radius = radius+1.0f;
        }
    }
}

void BoidNode::draw(const glm::mat4& proj, const glm::mat4& view)
{
    if(m_boids.empty() || !m_material)
    {
        Node::draw(proj, view);
        return;
    }

    buildVertexData(view);

    if(!m_vb || !m_decl)
    {
        Node::draw(proj, view);
        return;
    }

    m_material->SetUniform("projection", proj);
    m_material->SetUniform("view",       view);

    glm::mat4 worldMat = getWorldTransform();
    glm::mat4 worldIT  = glm::transpose(glm::inverse(worldMat));
    m_material->SetUniform("world",   worldMat);
    m_material->SetUniform("worldIT", worldIT);

    m_material->SetUniform("useTexture", true);
    m_material->SetUniform("u_color", glm::vec3(1,1,1));

    m_material->Apply();
    m_decl->Bind();

    int totalVerts = (int)(m_boids.size()*6);

    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    glDrawArrays(GL_TRIANGLES, 0, totalVerts);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    Node::draw(proj, view);
}

void BoidNode::buildVertexData(const glm::mat4& view)
{
    std::vector<QuadVertex> data;
    data.reserve(m_boids.size()*6);

    glm::vec3 camRight, camUp;
    getCameraRightUp(view, camRight, camUp);

    for(auto& b : m_boids)
    {
        float c = std::cos(glm::radians(b.rotation));
        float s = std::sin(glm::radians(b.rotation));

        // rotate the camera rightup by boid rotation (if i make boids spin)
        glm::vec3 rRot = ( c*camRight + s*camUp )*(b.size*0.5f);
        glm::vec3 uRot = (-s*camRight + c*camUp )*(b.size*0.5f);

        glm::vec3 bl = b.position - rRot - uRot;
        glm::vec3 br = b.position + rRot - uRot;
        glm::vec3 tl = b.position - rRot + uRot;
        glm::vec3 tr = b.position + rRot + uRot;

        float rr = b.color.r;
        float gg = b.color.g;
        float bb = b.color.b;
        float aa = b.color.a;

        QuadVertex v0 = { bl.x,bl.y,bl.z,  0.f,0.f,0.f,  rr,gg,bb,aa };
        QuadVertex v1 = { br.x,br.y,br.z,  1.f,0.f,0.f,  rr,gg,bb,aa };
        QuadVertex v2 = { tl.x,tl.y,tl.z,  0.f,1.f,0.f,  rr,gg,bb,aa };
        QuadVertex v3 = { tr.x,tr.y,tr.z,  1.f,1.f,0.f,  rr,gg,bb,aa };

        data.push_back(v0);
        data.push_back(v1);
        data.push_back(v2);

        data.push_back(v2);
        data.push_back(v1);
        data.push_back(v3);
    }

    GLsizeiptr neededSize = (GLsizeiptr)(data.size()*sizeof(QuadVertex));
    if(!m_vb || neededSize != prevBufferSize)
    {
        if(m_vb)
        {
            volpe::BufferManager::DestroyBuffer(m_vb);
            m_vb = nullptr;
        }
        m_vb = volpe::BufferManager::CreateVertexBuffer(data.data(), (unsigned int)neededSize);
        if(m_decl)
        {
            delete m_decl;
            m_decl = nullptr;
        }
        m_decl = new volpe::VertexDeclaration();
        m_decl->Begin();
          m_decl->SetVertexBuffer(m_vb);
          m_decl->AppendAttribute(volpe::AT_Position, 3, volpe::CT_Float);
          m_decl->AppendAttribute(volpe::AT_TexCoord1,3, volpe::CT_Float);
          m_decl->AppendAttribute(volpe::AT_Color,   4, volpe::CT_Float);
        m_decl->End();
        prevBufferSize = neededSize;
    }
    else
    {
        volpe::BufferManager::UpdateVertexBuffer(m_vb, data.data(), (unsigned int)neededSize);
    }
}

void BoidNode::getCameraRightUp(const glm::mat4& view, glm::vec3& outRight, glm::vec3& outUp)
{
    glm::mat3 rot = glm::inverse(glm::mat3(view));
    outRight = glm::normalize(glm::vec3(rot[0][0], rot[0][1], rot[0][2]));
    outUp    = glm::normalize(glm::vec3(rot[1][0], rot[1][1], rot[1][2]));
}

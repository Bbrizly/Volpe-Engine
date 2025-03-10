#pragma once
#include "ParticleNode.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Affector
{
public:
    bool localToNode = false;

    virtual ~Affector() {}
    
    virtual void Apply(Particle& p, float dt, const glm::mat4& emitterWorldMatrix)=0;
};

class AccelerationAffector : public Affector
{
public:
    glm::vec3 velocityToAdd;

    AccelerationAffector(const glm::vec3& vel= glm::vec3(0,-9.81f,0))
    : velocityToAdd(vel)
    {}

    virtual void Apply(Particle& p, float dt, const glm::mat4& emitterWorldMatrix) override
    {
        glm::vec3 accel = velocityToAdd;
        if(localToNode)
        {
            glm::mat3 rot = glm::mat3(emitterWorldMatrix);
            accel = rot * velocityToAdd;
        }
        p.velocity += accel * dt;
    }
};

class ScaleOverLifeAffector : public Affector
{
public:
    float startScale;
    float endScale;

    ScaleOverLifeAffector(float s=1.f, float e=2.f)
    : startScale(s), endScale(e)
    {}

    virtual void Apply(Particle& p, float, const glm::mat4&) override
    {
        if(p.lifetime<=0.f) return;
        float t= p.age/p.lifetime;
        if(t>1.f) t=1.f;
        float cur= startScale + (endScale - startScale)*t;
        p.size= p.initialSize*cur;
    }
};

class FadeOverLifeAffector : public Affector
{
public:
    float startAlpha;
    float endAlpha;

    FadeOverLifeAffector(float s=1.f, float e=0.f)
    : startAlpha(s), endAlpha(e)
    {}

    virtual void Apply(Particle& p, float, const glm::mat4&) override
    {
        if(p.lifetime<=0.f) return;
        float t= p.age/p.lifetime;
        if(t>1.f) t=1.f;
        float A= startAlpha + (endAlpha - startAlpha)*t;
        p.color.a= A;
    }
};

class TowardsPointAffector : public Affector
{
public:
    glm::vec3 target;
    float     strength;

    TowardsPointAffector(const glm::vec3& tgt, float s=1.0f)
    : target(tgt), strength(s)
    {}

    virtual void Apply(Particle& p, float dt, const glm::mat4& emitterWorldMatrix) override
    {
        glm::vec3 finalTarget = target;
        if(localToNode)
        {
            glm::vec4 tmp = emitterWorldMatrix * glm::vec4(target,1.f);
            finalTarget = glm::vec3(tmp);
        }

        glm::vec3 dir = finalTarget - p.position;
        float dist = glm::length(dir);
        if(dist < 0.0001f) return;
        dir = glm::normalize(dir);
        
        p.velocity += dir * strength * dt;
    }
};

class AwayFromPointAffector : public Affector
{
public:
    glm::vec3 center;
    float     strength;

    AwayFromPointAffector(const glm::vec3& c, float s=1.0f)
    : center(c), strength(s)
    {}

    virtual void Apply(Particle& p, float dt, const glm::mat4& emitterWorldMatrix) override
    {
        glm::vec3 finalCenter = center;
        if(localToNode)
        {
            glm::vec4 tmp = emitterWorldMatrix * glm::vec4(center,1.f);
            finalCenter = glm::vec3(tmp);
        }

        glm::vec3 dir = p.position - finalCenter;
        float dist = glm::length(dir);
        if(dist < 0.0001f) return;
        dir = glm::normalize(dir);
        p.velocity += dir * strength * dt;
    }
};

class DiePastAxisAffector : public Affector
{
public:
    int axis;           
    float threshold;    
    bool greaterThan; //choose greather than or less than
    DiePastAxisAffector(int axis, float threshold, bool greaterThan = true)
        : axis(axis), threshold(threshold), greaterThan(greaterThan) {}

    virtual void Apply(Particle& p, float dt, const glm::mat4& emitterWorldMatrix) override
    {
        float posComponent = 0.f;
        glm::vec3 emitterPos = emitterWorldMatrix[3];
        float testThreshold = threshold;

        if (axis == 0) //x
        {
            if(localToNode)
            {
                testThreshold += emitterPos.x;
            }
            posComponent = p.position.x;
        }
        else if (axis == 1)//y
        {
            if(localToNode)
            {
                testThreshold += emitterPos.y;
            }
            posComponent = p.position.y;
        }
        else if (axis == 2) //Z
        {
            if(localToNode)
            {
                testThreshold += emitterPos.z;
            }
            posComponent = p.position.z;

        }

        if (greaterThan)
        {
            if (posComponent > testThreshold)
                p.age = p.lifetime - 0.00001f; // particle dies
                // p.age = p.lifetime - 0.01f; // particle dies
        }
        else
        {
            if (posComponent < testThreshold)
                p.age = p.lifetime - 0.000001f;
        }
    }
};


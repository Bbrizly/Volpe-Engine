#pragma once
// #include "Particle.h"
#include "ParticleNode.h"
#include <glm/glm.hpp>

class Affector
{
public:
    virtual ~Affector() {}
    virtual void Apply(Particle& p, float dt)=0;
};

class AddVelocityAffector : public Affector
{
public:
    glm::vec3 velocityToAdd;

    AddVelocityAffector(const glm::vec3& vel= glm::vec3(0,-9.81f,0))
    : velocityToAdd(vel)
    {}

    virtual void Apply(Particle& p, float dt) override
    {
        p.velocity += velocityToAdd*dt;
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

    virtual void Apply(Particle& p, float /*dt*/) override
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

    virtual void Apply(Particle& p, float /*dt*/) override
    {
        if(p.lifetime<=0.f) return;
        float t= p.age/p.lifetime;
        if(t>1.f) t=1.f;
        float A= startAlpha + (endAlpha - startAlpha)*t;
        p.color.a= A;
    }
};

/*
  TowardsPointAffector:
  Accelerates the particle toward a specified point in space.
*/
class TowardsPointAffector : public Affector
{
public:
    glm::vec3 target;
    float     strength;

    TowardsPointAffector(const glm::vec3& tgt, float s=1.0f)
    : target(tgt), strength(s)
    {}

    virtual void Apply(Particle& p, float dt) override
    {
        glm::vec3 dir = target - p.position;
        float dist = glm::length(dir);
        if(dist < 0.0001f) return;
        dir = glm::normalize(dir);
        // accelerate toward target
        p.velocity += dir * strength * dt;
    }
};

/*
  AwayFromPointAffector:
  Applies force pushing the particle away from a point.
*/
class AwayFromPointAffector : public Affector
{
public:
    glm::vec3 center;
    float     strength;

    AwayFromPointAffector(const glm::vec3& c, float s=1.0f)
    : center(c), strength(s)
    {}

    virtual void Apply(Particle& p, float dt) override
    {
        glm::vec3 dir = p.position - center;
        float dist = glm::length(dir);
        if(dist < 0.0001f) return;
        dir = glm::normalize(dir);
        p.velocity += dir * strength * dt;
    }
};


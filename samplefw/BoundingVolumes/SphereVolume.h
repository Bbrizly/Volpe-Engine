#pragma once
#include "BoundingVolume.h"
#include "AABBVolume.h"
#include <glm/glm.hpp>
#include <cmath>
#include "../src/DebugRender.h"

class AABBVolume;

class SphereVolume : public BoundingVolume
{
public:
    glm::vec3 center;
    float     radius;

    glm::vec3 m_localCenter;
    float m_initialRadius;

    SphereVolume()
        : center(0,0,0), radius(1.0f),
        m_localCenter(0, 0, 0), m_initialRadius(1.0f) {}

    SphereVolume(const glm::vec3& c, float r)
        : center(c), radius(r),
        m_localCenter(c), m_initialRadius(r) {}

    virtual std::string getType() const override { return "Sphere"; }

    // For culling
    virtual bool IntersectsFrustum(const Frustum& frustum) const override;

    // Overlaps
    virtual bool Overlaps(const BoundingVolume& other) const override;

    // Overlaps with AABB
    
    virtual bool OverlapsAABB(const AABBVolume& aabb) const override;

    // Overlaps with Sphere
    virtual bool OverlapsSphere(const SphereVolume& sphere) const override;
    
    virtual void ExpandToFit(const BoundingVolume& childVolume,
        const glm::mat4& childWorldTransform) override;

        
    virtual void DrawMe() override;

    virtual void UpdateVolume(const glm::mat4& worldTransform) override
    {
        glm::vec4 transformedCenter = worldTransform * glm::vec4(m_localCenter, 1.0f);
        center = glm::vec3(transformedCenter);

        // std::cout << "\nLocal center:"<< m_localCenter.x << ", " << m_localCenter.y << ", " << m_localCenter.z <<
        //              "\nCenter: " << center.x << ", " << center.y << ", " << center.z;

        float scaleX = glm::length(glm::vec3(worldTransform[0]));
        float scaleY = glm::length(glm::vec3(worldTransform[1]));
        float scaleZ = glm::length(glm::vec3(worldTransform[2]));
        float maxScale = std::max(scaleX, std::max(scaleY, scaleZ));

        radius = m_initialRadius * maxScale;
    }
};

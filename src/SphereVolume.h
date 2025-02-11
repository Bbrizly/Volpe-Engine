#pragma once
#include "BoundingVolume.h"
#include "AABBVolume.h"
#include <glm/glm.hpp>
#include <cmath>

class AABBVolume;

class SphereVolume : public BoundingVolume
{
public:
    glm::vec3 center;
    float     radius;

    SphereVolume()
        : center(0,0,0), radius(1.0f) {}

    SphereVolume(const glm::vec3& c, float r)
        : center(c), radius(r) {}

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
};

#pragma once
#include "BoundingVolume.h"
#include "SphereVolume.h"
#include <glm/glm.hpp>
#include <algorithm>
#include <cmath>

class AABBVolume : public BoundingVolume
{
public:
    glm::vec3 min;
    glm::vec3 max;

    AABBVolume() : min(0.0f), max(0.0f) {}
    
    AABBVolume(glm::vec3& mn, glm::vec3& mx)
        : min(mn), max(mx) 
    {}

    // For culling
    bool IntersectsFrustum(const Frustum& frustum) const;

    // Overlaps - the main entry point
    virtual bool Overlaps(const BoundingVolume& other) const override;

    // Overlaps AABB
    virtual bool OverlapsAABB(const AABBVolume& aabb) const override;

    // Overlaps Sphere
    virtual bool OverlapsSphere(const SphereVolume& sphere) const override;

    virtual void ExpandToFit(const BoundingVolume& childVolume, 
        const glm::mat4& childWorldTransform) override;
};

#pragma once
#include "Frustum.h"

class AABBVolume;  
class SphereVolume;

class BoundingVolume
{
public:
    virtual ~BoundingVolume() {}

    // for culling
    virtual bool IntersectsFrustum(const Frustum& frustum) const = 0;

    // Overlap tests
    virtual bool Overlaps(const BoundingVolume& other) const = 0;

    // for Overlaps(AABB) 
    virtual bool OverlapsAABB(const AABBVolume& aabb) const = 0;
    // for Overlaps(Sphere)
    virtual bool OverlapsSphere(const SphereVolume& sphere) const = 0;
    
    // Based on childern the bounding box will expand to fit them all
    virtual void ExpandToFit(const BoundingVolume& childVolume, 
        const glm::mat4& childWorldTransform) = 0;
};

#pragma once
#include "../src/Frustum.h"
#include "iostream"

class AABBVolume;  
class SphereVolume;

class BoundingVolume
{
public:
    virtual ~BoundingVolume() {}

    virtual std::string getType() const = 0;

    virtual void UpdateVolume(const glm::mat4& worldTransform) = 0;

    virtual void DrawMe() = 0;

    virtual bool IntersectsFrustum(const Frustum& frustum) const = 0;

    // voeralaps whatev
    virtual bool Overlaps(const BoundingVolume& other) const = 0;

    // for Overlaps AABB
    virtual bool OverlapsAABB(const AABBVolume& aabb) const = 0;
    // for Overlaps Sphere
    virtual bool OverlapsSphere(const SphereVolume& sphere) const = 0;
    
    virtual void ExpandToFit(const BoundingVolume& childVolume, 
        const glm::mat4& childWorldTransform) = 0;

};

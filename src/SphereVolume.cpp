#include "SphereVolume.h"
#include "iostream"
// For culling
bool SphereVolume::IntersectsFrustum(const Frustum& frustum) const 
{
    std::cout<<"SPHERE\n";
    for (int i = 0; i < 6; ++i)
    {
        const glm::vec4& plane = frustum.planes[i];
        glm::vec3 normal(plane.x, plane.y, plane.z);
        float dist = glm::dot(normal, center) + plane.w;
        if(dist < -radius)
            return false;
    }
    return true;
}

// Overlaps
bool SphereVolume::Overlaps(const BoundingVolume& other) const 
{
    return other.OverlapsSphere(*this);
}

// Overlaps with AABB
bool SphereVolume::OverlapsAABB(const AABBVolume& aabb) const 
{
    return aabb.OverlapsSphere(*this);
}

// Overlaps with Sphere
bool SphereVolume::OverlapsSphere(const SphereVolume& sphere) const 
{
    float dist = glm::distance(center, sphere.center);
    return (dist <= (radius + sphere.radius));
}

void SphereVolume::ExpandToFit(const BoundingVolume& childVolume,
                               const glm::mat4& childWorldTransform)
{
    const auto* childAABB = dynamic_cast<const AABBVolume*>(&childVolume);
    if(childAABB)
    {
        // expand Radius so that it is >= the distance from the middle to a cornor of the box
        
        glm::vec4 center4 = glm::vec4(center,1.0f);
        glm::vec3 localParentCenter(center);

        // find distance from parent's center in world
        glm::vec3 corners[8] = {
            { childAABB->min.x, childAABB->min.y, childAABB->min.z },
            { childAABB->max.x, childAABB->min.y, childAABB->min.z },
            { childAABB->min.x, childAABB->max.y, childAABB->min.z },
            { childAABB->max.x, childAABB->max.y, childAABB->min.z },
            { childAABB->min.x, childAABB->min.y, childAABB->max.z },
            { childAABB->max.x, childAABB->min.y, childAABB->max.z },
            { childAABB->min.x, childAABB->max.y, childAABB->max.z },
            { childAABB->max.x, childAABB->max.y, childAABB->max.z }
        };
        for(int i=0; i<8; i++)
        {
            glm::vec4 worldPt = childWorldTransform * glm::vec4(corners[i],1.0f);
            
            glm::vec3 diff = glm::vec3(worldPt) - localParentCenter; 
            float dist = glm::length(diff);
            if(dist > radius)
                radius = dist;
        }
        return;
    }

    const auto* childSphere = dynamic_cast<const SphereVolume*>(&childVolume);
    if(childSphere)
    {
        glm::vec4 childCenter4 = childWorldTransform * glm::vec4(childSphere->center,1.0f);
        glm::vec3 childCenterW(childCenter4.x, childCenter4.y, childCenter4.z);

        glm::vec3 parentCenter = glm::vec3(center);
        glm::vec3 diff = childCenterW - parentCenter;
        float dist = glm::length(diff) + childSphere->radius;
        if(dist > radius)
            radius = dist;
    }
}

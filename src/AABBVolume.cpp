#include "AABBVolume.h"

//culling
bool AABBVolume::IntersectsFrustum(const Frustum& frustum) const 
{
    // glm::vec3 center = 0.5f*(min + max);
    // glm::vec3 extents= 0.5f*(max - min);
    // for (int i = 0; i < 6; ++i)
    // {
    //     const glm::vec4& plane = frustum.planes[i];
    //     glm::vec3 normal(plane.x, plane.y, plane.z);
    //     float d = plane.w;
    //     float r = extents.x * std::fabs(normal.x)
    //             + extents.y * std::fabs(normal.y)
    //             + extents.z * std::fabs(normal.z);
    //     float dist = glm::dot(normal, center) + d;
    //     if (dist < -r)
    //         return false;
    // }
    // return true;
    glm::vec3 center = (min + max) * 0.5f;
    glm::vec3 extents = max - center;
    
    for(int i = 0; i < 6; ++i) {
        const glm::vec4& plane = frustum.planes[i];
        float distance = glm::dot(glm::vec3(plane), center) + plane.w;
        float radius = glm::dot(extents, glm::abs(glm::vec3(plane)));
        
        if(distance < -radius) return false;
    }
    return true;
}

bool AABBVolume::Overlaps(const BoundingVolume& other) const 
{
    //Makes sure it uses correct function in other bounding volume
    return other.OverlapsAABB(*this);
}

// Overlaps AABB
bool AABBVolume::OverlapsAABB(const AABBVolume& aabb) const 
{
    if (max.x < aabb.min.x || min.x > aabb.max.x) return false;
    if (max.y < aabb.min.y || min.y > aabb.max.y) return false;
    if (max.z < aabb.min.z || min.z > aabb.max.z) return false;
    return true;
}

bool AABBVolume::OverlapsSphere(const SphereVolume& sphere) const
{
    const glm::vec3& c = sphere.center;
    float r = sphere.radius;

    float distSq = 0.0f;
    // X
    if (c.x < min.x) distSq += (min.x - c.x)*(min.x - c.x);
    else if (c.x > max.x) distSq += (c.x - max.x)*(c.x - max.x);

    // Y
    if (c.y < min.y) distSq += (min.y - c.y)*(min.y - c.y);
    else if (c.y > max.y) distSq += (c.y - max.y)*(c.y - max.y);

    // Z
    if (c.z < min.z) distSq += (min.z - c.z)*(min.z - c.z);
    else if (c.z > max.z) distSq += (c.z - max.z)*(c.z - max.z);

    return distSq <= (r*r);
}

void AABBVolume::ExpandToFit(const BoundingVolume& childVolume, 
                             const glm::mat4& childWorldTransform)
{
    // if child is AABB
    const auto* childAABB = dynamic_cast<const AABBVolume*>(&childVolume);
    if(childAABB)
    {
        
        glm::vec3 localCorners[8] = {
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
            glm::vec4 worldPt = childWorldTransform * glm::vec4(localCorners[i], 1.0f);
            min.x = std::min(min.x, worldPt.x);
            min.y = std::min(min.y, worldPt.y);
            max.x = std::max(max.x, worldPt.x);
            max.y = std::max(max.y, worldPt.y);
            min.z = std::min(min.z, worldPt.z);
            max.z = std::max(max.z, worldPt.z);
        }
        return;
    }

    // IF CHILD IS A SPHERE
    const auto* childSphere = dynamic_cast<const SphereVolume*>(&childVolume);
    if(childSphere)
    {
        // Just expand to fit in the radius 
        glm::vec4 center4 = childWorldTransform * glm::vec4(childSphere->center, 1.0f);
        glm::vec3 center3(center4.x, center4.y, center4.z);
        float r = childSphere->radius;

        glm::vec3 childMin = center3 - glm::vec3(r);
        glm::vec3 childMax = center3 + glm::vec3(r);

        min.x = std::min(min.x, childMin.x);
        min.y = std::min(min.y, childMin.y);
        min.z = std::min(min.z, childMin.z);
        max.x = std::max(max.x, childMax.x);
        max.y = std::max(max.y, childMax.y);
        max.z = std::max(max.z, childMax.z);
    }
}
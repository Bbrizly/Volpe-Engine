#include "AABBVolume.h"

bool AABBVolume::IntersectsFrustum(const Frustum& frustum) const 
{
    glm::vec3 center = (min + max) * 0.5f;
    glm::vec3 halfExtents = (max - min) * 0.5f;

    for (int i = 0; i < 6; ++i) {
        const glm::vec4& plane = frustum.planes[i];
        glm::vec3 normal(plane);
        float distance = glm::dot(normal, center) + plane.w;
        float radius = glm::dot(halfExtents, glm::abs(normal));
        if (distance < -radius)
            return false;
    }
    return true;
}

bool AABBVolume::Overlaps(const BoundingVolume& other) const 
{
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

void AABBVolume::DrawMe() //DEBUGGGGGGGGGG
{
    glm::vec3 corners[8] = {
        glm::vec3(min.x, min.y, min.z), // 0
        glm::vec3(max.x, min.y, min.z), // 1
        glm::vec3(max.x, max.y, min.z), // 2
        glm::vec3(min.x, max.y, min.z), // 3
        glm::vec3(min.x, min.y, max.z), // 4
        glm::vec3(max.x, min.y, max.z), // 5
        glm::vec3(max.x, max.y, max.z), // 6
        glm::vec3(min.x, max.y, max.z)  // 7
    };
    glm::vec3 color(0.0f, 1.0f, 0.0f);

    // Draw bottom face
    DebugRender::Instance().DrawLine(corners[0], corners[1], color, "BoundingVolumes");
    DebugRender::Instance().DrawLine(corners[1], corners[2], color, "BoundingVolumes");
    DebugRender::Instance().DrawLine(corners[2], corners[3], color, "BoundingVolumes");
    DebugRender::Instance().DrawLine(corners[3], corners[0], color, "BoundingVolumes");

    // Draw top face
    DebugRender::Instance().DrawLine(corners[4], corners[5], color, "BoundingVolumes");
    DebugRender::Instance().DrawLine(corners[5], corners[6], color, "BoundingVolumes");
    DebugRender::Instance().DrawLine(corners[6], corners[7], color, "BoundingVolumes");
    DebugRender::Instance().DrawLine(corners[7], corners[4], color, "BoundingVolumes");

    // Draw vertical edges
    DebugRender::Instance().DrawLine(corners[0], corners[4], color, "BoundingVolumes");
    DebugRender::Instance().DrawLine(corners[1], corners[5], color, "BoundingVolumes");
    DebugRender::Instance().DrawLine(corners[2], corners[6], color, "BoundingVolumes");
    DebugRender::Instance().DrawLine(corners[3], corners[7], color, "BoundingVolumes");
    // std::cout<<"DRAWING\n\n";


}
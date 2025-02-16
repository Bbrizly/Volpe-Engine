#pragma once
#include "BoundingVolume.h"
#include "SphereVolume.h"
#include <glm/glm.hpp>
#include <algorithm>
#include <cmath>
#include "iostream"
#include "DebugRender.h"

class AABBVolume : public BoundingVolume
{
public:
    glm::vec3 localMin;
    glm::vec3 localMax;
    glm::vec3 min;
    glm::vec3 max;

    AABBVolume() : localMin(0.0f), localMax(0.0f), min(0.0f), max(0.0f) {}
    
    AABBVolume(glm::vec3& mn, glm::vec3& mx)
        : localMin(mn), localMax(mx), min(mn), max(mx) 
    {}

    virtual std::string getType() const override { return "AABB"; }

    bool IntersectsFrustum(const Frustum& frustum) const;

    // Overlaps whatever
    virtual bool Overlaps(const BoundingVolume& other) const override;

    // Overlaps AABB
    virtual bool OverlapsAABB(const AABBVolume& aabb) const override;

    // Overlaps Sphere
    virtual bool OverlapsSphere(const SphereVolume& sphere) const override;

    virtual void ExpandToFit(const BoundingVolume& childVolume, 
        const glm::mat4& childWorldTransform) override;

    virtual void UpdateVolume(const glm::mat4& worldTransform) override
    {
        // get 8 corners from LOCALLL bounds
        glm::vec3 localCorners[8] = {
            glm::vec3(localMin.x, localMin.y, localMin.z),
            glm::vec3(localMax.x, localMin.y, localMin.z),
            glm::vec3(localMax.x, localMax.y, localMin.z),
            glm::vec3(localMin.x, localMax.y, localMin.z),
            glm::vec3(localMin.x, localMin.y, localMax.z),
            glm::vec3(localMax.x, localMin.y, localMax.z),
            glm::vec3(localMax.x, localMax.y, localMax.z),
            glm::vec3(localMin.x, localMax.y, localMax.z)
        };
    
        glm::vec3 newMin(1e9f), newMax(-1e9f);
        for (int i = 0; i < 8; i++) {
            glm::vec4 worldCorner = worldTransform * glm::vec4(localCorners[i], 1.0f);
            newMin = glm::min(newMin, glm::vec3(worldCorner));
            newMax = glm::max(newMax, glm::vec3(worldCorner));
        }
        min = newMin;
        max = newMax;
        // std::cout<<"min: "<<min.x<<","<<min.y<<","<<min.z<<". max: "<< max.x<<","<<max.y<<","<<max.z<<std::endl;
    }

    
    virtual void DrawMe() override;
};

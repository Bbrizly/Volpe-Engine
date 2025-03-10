#pragma once
#include "Node.h"
#include "../samplefw/BoundingVolumes/SphereVolume.h"
#include <glm/glm.hpp>

class LightNode : public Node
{
public:
    glm::vec3 color;
    float     intensity;
    float     radius;

    LightNode(const std::string& name,
              const glm::vec3& colorIn,
              float intensityIn,
              float radiusIn)
    : Node(name)
    , color(colorIn)
    , intensity(intensityIn)
    , radius(radiusIn)
    {
        SphereVolume* sphere = new SphereVolume(glm::vec3(0,0,0), radius);
        SetBoundingVolume(sphere);
        reactToLight = false;
    }

    virtual ~LightNode() = default;

    // glm::vec3 getWorldPosition() const
    // {
    //     return getWorldTransform()[3];
    //     // glm::mat4 w = getWorldTransform();
    //     // glm::vec3 what = w[3];
    //     // return glm::vec3(w[3][0], w[3][1], w[3][2]);
    // }

    // virtual void UpdateBoundingVolume() override
    // {
    //     Node::UpdateBoundingVolume(); 
    //     // Node::UpdateBoundingVolume will do "m_boundingVolume->UpdateVolume(getWorldTransform())".
    //     // That ensures the bounding sphere is placed at the correct world center. 
    //     // If you want to reassign radius from 'this->radius' each frame, do:
    //     if (m_boundingVolume) {
    //         if(auto* sphere = dynamic_cast<SphereVolume*>(m_boundingVolume))
    //             sphere->radius = radius; 
    //     }
    // }
};

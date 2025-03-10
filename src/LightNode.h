#pragma once
#include "Node.h"
#include "../samplefw/BoundingVolumes/SphereVolume.h"
#include <glm/glm.hpp>

class LightNode : public Node
{
public:
    glm::vec3 color;
    float     intensity;
    float     m_radius;

    LightNode(const std::string& name,
              const glm::vec3& colorIn,
              float intensityIn,
              float radiusIn)
    : Node(name)
    , color(colorIn)
    , intensity(intensityIn)
    , m_radius(radiusIn)
    {
        SphereVolume* sphere = new SphereVolume(glm::vec3(0,0,0), m_radius);
        SetBoundingVolume(sphere);
        reactToLight = false;
    }

    virtual ~LightNode() = default;
    float GetRadius() const { return m_radius; }
    void SetRadius(float r)
    {
        m_radius = r;
        if(m_boundingVolume) {
            if(auto* x = dynamic_cast<SphereVolume*>(m_boundingVolume))
            {
                x->radius = r;
            }
        }
    }
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

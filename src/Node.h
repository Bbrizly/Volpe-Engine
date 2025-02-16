#pragma once

#include "../volpe/Volpe.h"
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include "BoundingVolume.h"

class Node
{
protected:
    std::string       m_name;
    glm::mat4         m_localTransform;
    Node*             m_parent;
    std::vector<Node*> m_children;
    
    volpe::Material*  m_pMaterial = nullptr;

    BoundingVolume* m_boundingVolume;

public:
    Node(const std::string& name = "unnamed");
    virtual ~Node();

    void addChild(Node* child);
    void removeChild(Node* child);

    virtual void update(float dt);
    virtual void draw(const glm::mat4& proj, const glm::mat4& view, bool skipbind = false);

    std::vector<int> m_affectingLights;
    
    // Basic getters/setters
    void setName(const std::string& name);
    std::string getName() const;

    void setTransform(const glm::mat4& transform);
    glm::mat4 getTransform() const;

    glm::mat4 getWorldTransform() const;
    
    volpe::Material* GetMaterial() const { return m_pMaterial; }
    void SetMaterial(volpe::Material* mat) { m_pMaterial = mat; }

    // Bounding volume
    void SetBoundingVolume(BoundingVolume* volume);
    BoundingVolume* GetBoundingVolume() const;
    virtual void UpdateBoundingVolume();
};

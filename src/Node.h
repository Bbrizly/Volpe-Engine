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
    
    volpe::Program* m_pProgram = 0;

    BoundingVolume* m_boundingVolume;

public:
    Node(const std::string& name);
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
    
    volpe::Program* GetProgram() const { return m_pProgram; }
    void SetProgram(volpe::Program* prog) { m_pProgram = prog; }

    // Bounding volume
    void SetBoundingVolume(BoundingVolume* volume);
    BoundingVolume* GetBoundingVolume() const;
    virtual void UpdateBoundingVolume();
};

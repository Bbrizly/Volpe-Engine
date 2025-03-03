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

    volpe::Material* m_material = nullptr;

    BoundingVolume* m_boundingVolume;
    
    bool reactToLight = true;

public:
    Node(const std::string& name = "unnamed");
    virtual ~Node();

    void addChild(Node* child);
    void removeChild(Node* child);

    virtual void update(float dt);
    virtual void draw(const glm::mat4& proj, const glm::mat4& view);

    std::vector<int> m_affectingLights;
    bool GetReactToLight() {return reactToLight;}
    void SetReactToLight(bool x) {reactToLight = x;}
    
    // Basic getters/setters
    void setName(const std::string& name);
    std::string getName() const;

    void setTransform(const glm::mat4& transform);
    glm::mat4 getTransform() const;

    glm::mat4 getWorldTransform() const;
    
    volpe::Material* GetMaterial() const { return m_material; }
    void SetMaterial(volpe::Material* mat) { m_material = mat; }

    // volpe::Program* GetProgram() const { return m_pProgram; }
    // void SetProgram(volpe::Program* prog) { m_pProgram = prog; }

    // Bounding volume
    void SetBoundingVolume(BoundingVolume* volume);
    BoundingVolume* GetBoundingVolume() const;
    virtual void UpdateBoundingVolume();
};

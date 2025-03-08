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
    std::vector<Node*> getChildren() {return m_children;}
    Node*              getParent() {return m_parent;}


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
    
    void getTransformDecomposed(glm::vec3& outPos, glm::quat& outRot, glm::vec3& outScale) const;
    void setTransformDecomposed(const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale);

    void SetBoundingVolume(BoundingVolume* volume);
    BoundingVolume* GetBoundingVolume() const;
    virtual void UpdateBoundingVolume();
};

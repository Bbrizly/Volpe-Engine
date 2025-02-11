#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

// Simple bounding sphere for example
struct BoundingSphere {
    glm::vec3 center;
    float     radius;
};

class Node
{
protected:
    std::string       m_name;
    glm::mat4         m_localTransform;
    BoundingSphere    m_boundingSphere;
    Node*             m_parent;
    std::vector<Node*> m_children;

public:
    Node(const std::string& name);
    virtual ~Node();

    void addChild(Node* child);
    void removeChild(Node* child);

    virtual void update(float dt);
    virtual void draw(const glm::mat4& proj, const glm::mat4& view);//const glm::mat4& viewProj);

    // Basic getters/setters
    void        setName(const std::string& name);
    std::string getName() const;

    void        setTransform(const glm::mat4& transform);
    glm::mat4   getTransform() const;

    glm::mat4   getWorldTransform() const;

    // Bounding volume
    void                 setBoundingSphere(const BoundingSphere& bs);
    const BoundingSphere getBoundingSphere() const;
    void                 updateBoundingVolume();
};

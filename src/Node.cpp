#include "Node.h"

Node::Node(const std::string& name)
: m_name(name),
  m_localTransform(1.0f),
  m_parent(nullptr)
{
    // Bounding sphere w radius 1
    m_boundingSphere.center = glm::vec3(0,0,0);
    m_boundingSphere.radius = 1.0f;
}

Node::~Node()
{
    for (auto* c : m_children) {
        delete c;
    }
    m_children.clear();
}

void Node::addChild(Node* child)
{
    if (!child) return;
    // If child had a previous parent, replace it with this node
    if (child->m_parent) {
        child->m_parent->removeChild(child);
    }
    m_children.push_back(child);
    child->m_parent = this;
}

void Node::removeChild(Node* child)
{
    auto it = std::find(m_children.begin(), m_children.end(), child);
    if (it != m_children.end()) {
        child->m_parent = nullptr;
        m_children.erase(it);
    }
}

void Node::update(float dt)
{
    for (auto* c : m_children) {
        c->update(dt);
    }
}

void Node::draw(const glm::mat4& proj, const glm::mat4& view)
{
    for (auto* c : m_children) {
        c->draw(proj, view);
    }
}

void Node::setName(const std::string& name) { m_name = name; }
std::string Node::getName() const { return m_name; }

void Node::setTransform(const glm::mat4& transform) { m_localTransform = transform; }
glm::mat4 Node::getTransform() const { return m_localTransform; }

glm::mat4 Node::getWorldTransform() const
{
    if (m_parent) { //rrecursive call
        return m_parent->getWorldTransform() * m_localTransform;
    }
    return m_localTransform;
}

void Node::setBoundingSphere(const BoundingSphere& bs) { m_boundingSphere = bs; }
const BoundingSphere Node::getBoundingSphere() const { return m_boundingSphere; }

// Example of “expanding” the bounding sphere to encompass children
void Node::updateBoundingVolume()
{
    // Start with local bounding sphere
    float maxRadius = m_boundingSphere.radius;
    glm::vec3 center = m_boundingSphere.center; // local origin

    for (auto* c : m_children) {
        BoundingSphere childBS = c->getBoundingSphere();

        // Child center in world coords
        glm::vec4 childCenterWorld = c->getWorldTransform() * glm::vec4(childBS.center, 1.0f);
        glm::vec3 childCenter = glm::vec3(childCenterWorld);

        float dist = glm::length(childCenter - center) + childBS.radius;
        if (dist > maxRadius) {
            maxRadius = dist;
        }
    }
    m_boundingSphere.radius = maxRadius;

    // Recursively update children’s bounding volumes after that
    for (auto* c : m_children) {
        c->updateBoundingVolume();
    }
}



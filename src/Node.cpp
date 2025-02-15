#include "Node.h"

Node::Node(const std::string& name)
: m_name(name),
  m_localTransform(1.0f),
  m_parent(nullptr),
  m_boundingVolume(nullptr)
{
    
}

Node::~Node()
{
    for (auto* c : m_children) {
        delete c;
    }
    m_children.clear();

    if(m_boundingVolume)
        delete m_boundingVolume;
    m_boundingVolume =nullptr;
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



BoundingVolume* Node::GetBoundingVolume() const { return m_boundingVolume; }

void Node::SetBoundingVolume(BoundingVolume* volume)
{
    if(m_boundingVolume && m_boundingVolume != volume)
    {
        delete m_boundingVolume;
    }
    m_boundingVolume = volume;
}

// EXpanding the bounding volume to encompass children
void Node::UpdateBoundingVolume()
{
    for (auto* c : m_children)
    {
        c->UpdateBoundingVolume();
    }

    if(!m_boundingVolume) 
        return;

    for (auto* c : m_children)
    {

        BoundingVolume* childVol = c->GetBoundingVolume();
        if(!childVol) continue;

        glm::mat4 childWorld = c->getWorldTransform();

        m_boundingVolume->ExpandToFit(*childVol, childWorld);
        
    }
    m_boundingVolume->UpdateVolume(getWorldTransform());

}



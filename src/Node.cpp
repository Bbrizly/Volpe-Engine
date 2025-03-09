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

    if(m_material && m_ownsMaterial) {
        volpe::MaterialManager::DestroyMaterial(m_material);
        m_material = nullptr;
    }
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

void Node::draw(const glm::mat4& proj, const glm::mat4& view) //remove skip bind??? 
{
    for (auto* c : m_children) {
        c->draw(proj, view);
    }
}

void Node::setName(const std::string& name) { m_name = name; }
std::string Node::getName() const { return m_name; }

void Node::setTransform(const glm::mat4& transform) { m_localTransform = transform; if(m_boundingVolume){ m_boundingVolume->UpdateVolume(getWorldTransform());}} //UPDATING VOLUME TWICE
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
    if(!volume) return;

    if (m_boundingVolume) {
        if (typeid(*m_boundingVolume) != typeid(*volume)) {
            delete m_boundingVolume;
        }
    }
    m_boundingVolume = volume;
}

// EXpanding the bounding volume to cover dem children
void Node::UpdateBoundingVolume()
{
    if(!m_boundingVolume) 
        return;
    // std::cout<<"NAME: "<<m_name<<std::endl;
    m_boundingVolume->UpdateVolume(getWorldTransform());
    
    for (auto* c : m_children)
    {
        c->UpdateBoundingVolume();
    }

    // for (auto* c : m_children) // IT WORKS WITH THE SOLAR SYSTEM BUT FUCKS UP DEBUG LINES
    // {
    //     BoundingVolume* childVol = c->GetBoundingVolume();
    //     if(!childVol) continue;
    //     glm::mat4 childWorld = c->getWorldTransform();
    //     m_boundingVolume->ExpandToFit(*childVol, childWorld);
    // }
    
}


void Node::getTransformDecomposed(glm::vec3& outPos, glm::quat& outRot, glm::vec3& outScale) const
{
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(m_localTransform, outScale, outRot, outPos, skew, perspective);
}

void Node::setTransformDecomposed(const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale)
{
    glm::mat4 T = glm::translate(glm::mat4(1.0f), pos);
    glm::mat4 R = glm::toMat4(rot);
    glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);
    
    m_localTransform = T * R * S;
    if(m_boundingVolume){
        m_boundingVolume->UpdateVolume(getWorldTransform());
    }
}
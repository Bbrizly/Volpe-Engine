#include "QuadTree.h"
#include "AABBVolume.h"
#include "SphereVolume.h"

// This method projects a 3D bounding volume (either an AABB or Sphere) into a 2D AABB on the XZ plane.
AABB2D QuadTree::FlattenBoundingVolume(const BoundingVolume* bv) const {
    AABB2D result;
    // Try AABBVolume first.
    if (const AABBVolume* aabb = dynamic_cast<const AABBVolume*>(bv)) {
        result.min = glm::vec2(aabb->min.x, aabb->min.z);
        result.max = glm::vec2(aabb->max.x, aabb->max.z);
    }
    else if (const SphereVolume* sphere = dynamic_cast<const SphereVolume*>(bv)) {
        result.min = glm::vec2(sphere->center.x - sphere->radius, sphere->center.z - sphere->radius);
        result.max = glm::vec2(sphere->center.x + sphere->radius, sphere->center.z + sphere->radius);
    }
    else {
        // Fallback: if no specialized type, assume the bounding volume provides an AABBVolume.
        // (You might add a virtual method to BoundingVolume that returns its AABB.)
        result.min = glm::vec2(0.0f);
        result.max = glm::vec2(0.0f);
    }
    return result;
}

// AABB2D method
bool AABB2D::IntersectsFrustum(const Frustum& frustum) const {
    // For simplicity, we treat the 2D box as an AABB in the XZ plane with y=0.
    glm::vec3 min3D(min.x, 0.0f, min.y);
    
    glm::vec3 max3D(max.x, 0.0f, max.y);
    
    for (int i = 0; i < 6; ++i) {
        const glm::vec4& plane = frustum.planes[i];
        glm::vec3 normal(plane.x, plane.y, plane.z);
        float distance = plane.w;
        
        float r = 0.5f * (max3D.x - min3D.x) * fabs(normal.x) +
                  0.5f * (max3D.z - min3D.z) * fabs(normal.z);
        glm::vec3 center = (min3D + max3D) * 0.5f;
        float d = glm::dot(normal, center) + distance;
        if (d < -r)
            return false;
    }
    return true;
}

QuadTree::QuadTree(const AABB2D& bounds, int level)
    : m_bounds(bounds), m_level(level)
{}
QuadTree::~QuadTree() {
    for (auto child : m_children)
        delete child;
    m_children.clear();
}

bool QuadTree::Contains(const glm::vec2& point) const {
    return (point.x >= m_bounds.min.x && point.x < m_bounds.max.x &&
            point.y >= m_bounds.min.y && point.y < m_bounds.max.y);
}

// Insert a node into the quadtree.
void QuadTree::Insert(Node* node)
{
    if (!node) return;
    
    // Get the node’s bounding volume and flatten it.
    BoundingVolume* bv = node->GetBoundingVolume();
    if (!bv) return;
    AABB2D nodeAABB = FlattenBoundingVolume(bv);
    
    // Test for overlap with this quadtree region.
    if (nodeAABB.max.x < m_bounds.min.x || nodeAABB.min.x > m_bounds.max.x ||
        nodeAABB.max.y < m_bounds.min.y || nodeAABB.min.y > m_bounds.max.y)
    {
        return; // no overlap
    }
    
    // If we haven’t subdivided yet and are under capacity, store this node.
    if (m_children.empty() && (m_nodes.size() < MAX_OBJECTS || m_level == MAX_LEVELS)) {
        m_nodes.push_back(node);
        return;
    }
    
    // If no children yet, subdivide and push down current nodes.
    if (m_children.empty()) {
        Subdivide();
        for (auto* existing : m_nodes) {
            BoundingVolume* existingBV = existing->GetBoundingVolume();
            if (!existingBV) continue;
            AABB2D existingAABB = FlattenBoundingVolume(existingBV);
            for (auto* child : m_children) {
                if (!(existingAABB.max.x < child->m_bounds.min.x ||
                      existingAABB.min.x > child->m_bounds.max.x ||
                      existingAABB.max.y < child->m_bounds.min.y ||
                      existingAABB.min.y > child->m_bounds.max.y))
                {
                    child->Insert(existing);
                    break;
                }
            }
        }
        m_nodes.clear();
    }
    
    // Insert the new node into the appropriate child.
    for (auto* child : m_children) {
        if (!(nodeAABB.max.x < child->m_bounds.min.x ||
              nodeAABB.min.x > child->m_bounds.max.x ||
              nodeAABB.max.y < child->m_bounds.min.y ||
              nodeAABB.min.y > child->m_bounds.max.y))
        {
            child->Insert(node);
            return;
        }
    }
}

// Query the quadtree for nodes whose bounding volumes intersect the given frustum.
void QuadTree::Query(const Frustum& frustum, std::vector<Node*>& results)
{
    if (!m_bounds.IntersectsFrustum(frustum))
        return;
    
    for (auto* node : m_nodes) {
        if (node->GetBoundingVolume()->IntersectsFrustum(frustum))
            results.push_back(node);
    }
    
    for (auto* child : m_children) {
        child->Query(frustum, results);
    }
}

// Query nodes that intersect a light sphere.
void QuadTree::QueryLight(const glm::vec3& lightPos, float lightRadius, std::vector<Node*>& results)
{
    // Create a temporary 2D AABB for the light (projected on XZ)
    AABB2D lightAABB;
    lightAABB.min = glm::vec2(lightPos.x - lightRadius, lightPos.z - lightRadius);
    lightAABB.max = glm::vec2(lightPos.x + lightRadius, lightPos.z + lightRadius);
    
    // If this quadtree region does not overlap the light’s projection, skip.
    if (m_bounds.max.x < lightAABB.min.x || m_bounds.min.x > lightAABB.max.x ||
        m_bounds.max.y < lightAABB.min.y || m_bounds.min.y > lightAABB.max.y)
    {
        return;
    }
    
    // Check nodes at this level.
    for (auto* node : m_nodes) {
        // Here we simply test the node’s 2D projection for overlap with the light’s AABB.
        AABB2D nodeAABB = FlattenBoundingVolume(node->GetBoundingVolume());
        if (!(nodeAABB.max.x < lightAABB.min.x || nodeAABB.min.x > lightAABB.max.x ||
              nodeAABB.max.y < lightAABB.min.y || nodeAABB.min.y > lightAABB.max.y))
        {
            results.push_back(node);
        }
    }
    
    // Recurse into children.
    for (auto* child : m_children) {
        child->QueryLight(lightPos, lightRadius, results);
    }
}

void QuadTree::Subdivide()
{
    float midX = (m_bounds.min.x + m_bounds.max.x) * 0.5f;
    float midY = (m_bounds.min.y + m_bounds.max.y) * 0.5f;
    
    AABB2D topLeft(glm::vec2(m_bounds.min.x, midY), glm::vec2(midX, m_bounds.max.y));
    AABB2D topRight(glm::vec2(midX, midY), glm::vec2(m_bounds.max.x, m_bounds.max.y));
    AABB2D bottomLeft(glm::vec2(m_bounds.min.x, m_bounds.min.y), glm::vec2(midX, midY));
    AABB2D bottomRight(glm::vec2(midX, m_bounds.min.y), glm::vec2(m_bounds.max.x, midY));
    
    // Create child nodes.
    m_children.push_back(new QuadTree(topLeft, m_level + 1));
    m_children.push_back(new QuadTree(topRight, m_level + 1));
    m_children.push_back(new QuadTree(bottomLeft, m_level + 1));
    m_children.push_back(new QuadTree(bottomRight, m_level + 1));
    
    // Note: You may choose to also move current m_nodes into children here (done in Insert).
}

void QuadTree::BuildDebugLines() {
    float fixedY = 0.0f;
    glm::vec3 p1(m_bounds.min.x, fixedY, m_bounds.min.y);
    glm::vec3 p2(m_bounds.max.x, fixedY, m_bounds.min.y);
    glm::vec3 p3(m_bounds.max.x, fixedY, m_bounds.max.y);
    glm::vec3 p4(m_bounds.min.x, fixedY, m_bounds.max.y);
    
    DebugRender::Instance().DrawSquare(p1, p2, p3, p4, glm::vec3(1.0f), "Tree");
    
    for (auto* child : m_children)
        child->BuildDebugLines();
}

void QuadTree::Render(const glm::mat4& proj, const glm::mat4& view) {
    // Optionally, call BuildDebugLines() and then draw them via DebugRender.
    DebugRender::Instance().Render(proj, view);
}

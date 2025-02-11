#include "QuadTree.h"

bool AABB2D::IntersectsFrustum(const Frustum& frustum) const {
    glm::vec3 min3D(min.x, 0.0f, min.y);
    glm::vec3 max3D(max.x, 0.0f, max.y);

    // Check each frustum plane against the AABB
    for (int i = 0; i < 6; ++i) {
        const glm::vec4& plane = frustum.planes[i];
        glm::vec3 normal(plane.x, plane.y, plane.z);
        float distance = plane.w;

        // Calculate the AABB's projected interval onto the plane normal
        float r = 0.5f * (max3D.x - min3D.x) * abs(normal.x) +
                  0.5f * (max3D.z - min3D.z) * abs(normal.z);

        // Calculate the AABB's center point (y=0)
        glm::vec3 center = (min3D + max3D) * 0.5f;

        // Signed distance from center to plane
        float d = glm::dot(normal, center) + distance;

        // If the distance is outside the projected interval, the AABB is outside the frustum
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

    // DebugRender::Instance().Clear();
}

static bool Overlaps2D(const AABB2D& quadRegion, const AABBVolume& box3D)
{
    // flatten box3D onto XZ plane
    float minX = box3D.min.x;
    float maxX = box3D.max.x;
    float minZ = box3D.min.z;
    float maxZ = box3D.max.z;

    // Compare with quadRegion in XZ
    if (maxX < quadRegion.min.x || minX > quadRegion.max.x) return false;
    if (maxZ < quadRegion.min.y || minZ > quadRegion.max.y) return false;
    return true;
}

void QuadTree::Insert(Node* node)
{
    if(!node) return;

    DebugCube* cube = dynamic_cast<DebugCube*>(node);
    if(!cube) {
        // If you only have cubes, that’s fine; or handle other node types
        return;
    }

    // 1) get the node's 3D bounding box
    AABBVolume nodeBox = cube->getWorldAABB3D();

    // 2) flatten to see if it overlaps this 2D region
    if(!Overlaps2D(m_bounds, nodeBox)) {
        // no overlap => skip
        return;
    }

    // 3) if no children and not at capacity, store it
    if(m_children.empty() && (m_nodes.size() < MAX_OBJECTS || m_level == MAX_LEVELS))
    {
        m_nodes.push_back(node);
        return;
    }

    // if no children yet, subdiv
    if(m_children.empty()) {
        Subdivide();

        // push existing nodes down
        for(auto* existing : m_nodes)
        {
            DebugCube* ec = dynamic_cast<DebugCube*>(existing);
            if(!ec) continue;

            AABBVolume eBox = ec->getWorldAABB3D();
            for(auto* child : m_children)
            {
                if(Overlaps2D(child->m_bounds, eBox)) {
                    child->Insert(existing);
                    break;
                }
            }
        }
        m_nodes.clear();
    }

    // now insert the new node
    for(auto* child : m_children)
    {
        if(Overlaps2D(child->m_bounds, nodeBox)) {
            child->Insert(node);
            return; // or store multiple if partial overlap
        }
    }
}

/*void QuadTree::Insert(Node* node)
{
    if (!node) return;

    // Get node 2D position (x, z)
    glm::mat4 world = node->getWorldTransform();
    glm::vec3 pos3D = glm::vec3(world[3]);
    glm::vec2 pos2D(pos3D.x, pos3D.z);
    
    // std::cout << "Inserting Node: " << node->getName() << " at (" << pos2D.x << ", " << pos2D.y << ")\n";

    // If this node not in quad, move on
    if (!Contains(pos2D)) {
        return;
    }
    
    if (m_children.empty() &&
        (m_nodes.size() < MAX_OBJECTS || m_level == MAX_LEVELS))
    {
        m_nodes.push_back(node);
        return;
    }

    if (m_children.empty()) {
        Subdivide();

        // push all existing nodes down to the children
        for (auto* existing : m_nodes) {
            glm::vec3 exPos3D = glm::vec3(existing->getWorldTransform()[3]);
            glm::vec2 exPos2D(exPos3D.x, exPos3D.z);

            // Cross ref parent quad nodes with children and assign them
            for (auto* child : m_children) {
                if (child->Contains(exPos2D)) {
                    child->Insert(existing);
                    break;
                }
            }
        }
        // Parent has nothing
        m_nodes.clear();
    }

    // insert the new node into whichever child contains it
    for (auto* child : m_children) {
        if (child->Contains(pos2D)) {
            child->Insert(node);
            break; 
        }
    }
}
*/

void QuadTree::Query(const Frustum& frustum, std::vector<Node*>& results)
{
    // AABB vs. frustum check
    if (!m_bounds.IntersectsFrustum(frustum)) {
        return;
    }

    // Check each node stored here
    for (auto node : m_nodes) {
        if(node->GetBoundingVolume()->IntersectsFrustum(frustum)){
        // if (SphereIntersectsFrustum(node->GetBoundingVolume(), frustum)) {
            results.push_back(node);
        }
    }

    // Recurse children
    for (auto child : m_children) {
        child->Query(frustum, results);
    }
}

// A “light sphere” query that uses bounding-sphere vs. bounding-box
static bool AABBvsSphere(const AABBVolume& box, const glm::vec3& center, float radius)
{
    float distSq = 0.f;
    if(center.x < box.min.x) distSq += (box.min.x - center.x)*(box.min.x - center.x);
    else if(center.x > box.max.x) distSq += (center.x - box.max.x)*(center.x - box.max.x);

    if(center.y < box.min.y) distSq += (box.min.y - center.y)*(box.min.y - center.y);
    else if(center.y > box.max.y) distSq += (center.y - box.max.y)*(center.y - box.max.y);

    if(center.z < box.min.z) distSq += (box.min.z - center.z)*(box.min.z - center.z);
    else if(center.z > box.max.z) distSq += (center.z - box.max.z)*(center.z - box.max.z);

    return (distSq <= radius*radius);
}

void QuadTree::QueryLight(const glm::vec3& lightPos, float lightRadius, std::vector<Node*>& results)
{
    // If 2D region doesn't overlap the sphere (flatten?), do a quick test
    // For a more advanced approach, we do bounding-sphere vs region. 
    // We'll skip for brevity or do Overlaps2D(...)? 
    // If you want to be thorough, flatten the sphere to a bounding circle in XZ, compare with m_bounds.
    // Let’s just assume we proceed if partial overlap is possible.

    // check each node
    for(auto* node : m_nodes)
    {
        DebugCube* cube = dynamic_cast<DebugCube*>(node);
        if(!cube) continue;
        AABBVolume box = cube->getWorldAABB3D();
        if(AABBvsSphere(box, lightPos, lightRadius)) {
            results.push_back(node);
        }
    }

    // recurse children
    for(auto* child : m_children)
        child->QueryLight(lightPos, lightRadius, results);
}


void QuadTree::BuildDebugLines() {
    float fixedY = 0.0f;
    glm::vec3 p1(m_bounds.min.x, fixedY, m_bounds.min.y);
    glm::vec3 p2(m_bounds.max.x, fixedY, m_bounds.min.y);
    glm::vec3 p3(m_bounds.max.x, fixedY, m_bounds.max.y);
    glm::vec3 p4(m_bounds.min.x, fixedY, m_bounds.max.y);
    
    // std::cout << "m_bounds.min: (" << m_bounds.min.x << ", " << m_bounds.min.y << ")" << std::endl;
    // std::cout << "m_bounds.max: (" << m_bounds.max.x << ", " << m_bounds.max.y << ")" << std::endl;

    DebugRender::Instance().DrawSquare(p1,p2,p3,p4,glm::vec3(1.0f));
    
    for (auto child : m_children)
    {
        child->BuildDebugLines();
    }
}

bool QuadTree::Contains(const glm::vec2& point) const {
    return (point.x >= m_bounds.min.x && point.x < m_bounds.max.x &&
            point.y >= m_bounds.min.y && point.y < m_bounds.max.y);
}
void QuadTree::Subdivide()
{
    float midX = (m_bounds.min.x + m_bounds.max.x) * 0.5f;
    float midY = (m_bounds.min.y + m_bounds.max.y) * 0.5f;

    // Create four candidate AABBs for child regions
    AABB2D topLeft    = { glm::vec2(m_bounds.min.x, midY),  glm::vec2(midX, m_bounds.max.y) };
    AABB2D topRight   = { glm::vec2(midX, midY),           glm::vec2(m_bounds.max.x, m_bounds.max.y) };
    AABB2D bottomLeft = { glm::vec2(m_bounds.min.x, m_bounds.min.y), glm::vec2(midX, midY) };
    AABB2D bottomRight= { glm::vec2(midX, m_bounds.min.y), glm::vec2(m_bounds.max.x, midY) };

    // We’ll gather the objects that belong in each quadrant
    std::vector<Node*> tlNodes;
    std::vector<Node*> trNodes;
    std::vector<Node*> blNodes;
    std::vector<Node*> brNodes;

    // Sort existing m_nodes into whichever quadrant they belong
    for (auto* node : m_nodes) {
        glm::vec3 pos3D = glm::vec3(node->getWorldTransform()[3]);
        glm::vec2 p(pos3D.x, pos3D.z);

        if (p.x >= m_bounds.min.x && p.x < midX &&
            p.y >= midY          && p.y < m_bounds.max.y)
        {
            tlNodes.push_back(node);
        }
        else if (p.x >= midX          && p.x < m_bounds.max.x &&
                 p.y >= midY          && p.y < m_bounds.max.y)
        {
            trNodes.push_back(node);
        }
        else if (p.x >= m_bounds.min.x && p.x < midX &&
                 p.y >= m_bounds.min.y && p.y < midY)
        {
            blNodes.push_back(node);
        }
        else if (p.x >= midX          && p.x < m_bounds.max.x &&
                 p.y >= m_bounds.min.y && p.y < midY)
        {
            brNodes.push_back(node);
        }
    }

    // Clear the parent’s node list so we don’t store them twice
    m_nodes.clear();

    // Now create children ONLY for those quadrants that actually hold objects
    // or if you want to allow further splits later. Typically, you can do:
    if (!tlNodes.empty()|| true) {
        auto* child = new QuadTree(topLeft, m_level + 1);
        child->m_nodes = std::move(tlNodes);  // hand over its objects
        m_children.push_back(child);
    }
    if (!trNodes.empty()|| true) {
        auto* child = new QuadTree(topRight, m_level + 1);
        child->m_nodes = std::move(trNodes);
        m_children.push_back(child);
    }
    if (!blNodes.empty()|| true) {
        auto* child = new QuadTree(bottomLeft, m_level + 1);
        child->m_nodes = std::move(blNodes);
        m_children.push_back(child);
    }
    if (!brNodes.empty()|| true) {
        auto* child = new QuadTree(bottomRight, m_level + 1);
        child->m_nodes = std::move(brNodes);
        m_children.push_back(child);
    }
}

/*
void QuadTree::Subdivide() {
    float midX = (m_bounds.min.x + m_bounds.max.x) * 0.5f;
    float midY = (m_bounds.min.y + m_bounds.max.y) * 0.5f;
    
    AABB2D childBounds;
    // Top-left
    childBounds.min = glm::vec2(m_bounds.min.x, midY);
    childBounds.max = glm::vec2(midX, m_bounds.max.y);
    m_children.push_back(new QuadTree(childBounds, m_level + 1));
    
    // Top-right
    childBounds.min = glm::vec2(midX, midY);
    childBounds.max = glm::vec2(m_bounds.max.x, m_bounds.max.y);
    m_children.push_back(new QuadTree(childBounds, m_level + 1));
    
    // Bottom-left
    childBounds.min = glm::vec2(m_bounds.min.x, m_bounds.min.y);
    childBounds.max = glm::vec2(midX, midY);
    m_children.push_back(new QuadTree(childBounds, m_level + 1));
    
    // Bottom-right
    childBounds.min = glm::vec2(midX, m_bounds.min.y);
    childBounds.max = glm::vec2(m_bounds.max.x, midY);
    m_children.push_back(new QuadTree(childBounds, m_level + 1));
}
*/



#include "OctTree.h"


OctTree::OctTree(const AABBVolume& bounds, int level)//= 0)
    : m_bounds(bounds)
    , m_level(level)
{
    for (int i=0; i<NUM_CHILDREN; i++){
        m_children[i] = nullptr;
    }
}

OctTree::~OctTree()
{
    for (int i=0; i<NUM_CHILDREN; i++){
        if (m_children[i]) {
            delete m_children[i];
            m_children[i] = nullptr;
        }
    }
}

bool OctTree::Contains(const glm::vec3& point) const
{
    return (point.x >= m_bounds.min.x && point.x < m_bounds.max.x &&
            point.y >= m_bounds.min.y && point.y < m_bounds.max.y &&
            point.z >= m_bounds.min.z && point.z < m_bounds.max.z);
}

void OctTree::Insert(Node* node)
{
    if(!node) return;

    BoundingVolume* boundingVol = node->GetBoundingVolume();
    if(!boundingVol) return;

    if(!boundingVol->OverlapsAABB(m_bounds))
        return;
    

    if(!m_children[0] && (m_nodes.size()<MAX_OBJECTS || m_level==MAX_LEVELS))
    {
        m_nodes.push_back(node);
        return;
    }

    if(!m_children[0]) {
        Subdivide();

        for(auto* existing : m_nodes) {
            BoundingVolume* childBV = existing->GetBoundingVolume();
            if(!childBV) continue;

            for(int i=0; i<NUM_CHILDREN; i++){
                if(m_children[i]->m_bounds.Overlaps(*childBV)) {
                    m_children[i]->Insert(existing);
                    break;
                }
            }
        }
        m_nodes.clear();
    }

    // insert new
    for(int i=0; i<NUM_CHILDREN; i++){
        if(m_children[i]->m_bounds.Overlaps(*boundingVol)) {
            m_children[i]->Insert(node);
            return;
        }
    }
}

void OctTree::Query(const Frustum& frustum, std::vector<Node*>& results)
{
    //Bounding box fully outside
    if (!m_bounds.IntersectsFrustum(frustum)) {
        return;
    }    
    

    for (auto* node : m_nodes) {
        // If bounding vol of node is inside frustum it gets added
        if (node->GetBoundingVolume()->IntersectsFrustum(frustum)){
            results.push_back(node);
        }
    }

    for (int i=0; i<NUM_CHILDREN; i++){
        if (m_children[i]) {
            m_children[i]->Query(frustum, results);
        }
    }
}

static bool AABBvsSphere(const AABBVolume& box, const glm::vec3& c, float r)
{
    SphereVolume* sphere = new SphereVolume(c, r);
    return sphere->OverlapsAABB(box);
    // float distSq = 0.f;
    // if(c.x < box.min.x) distSq += (box.min.x - c.x)*(box.min.x - c.x);
    // else if(c.x > box.max.x) distSq += (c.x - box.max.x)*(c.x - box.max.x);

    // if(c.y < box.min.y) distSq += (box.min.y - c.y)*(box.min.y - c.y);
    // else if(c.y > box.max.y) distSq += (c.y - box.max.y)*(c.y - box.max.y);

    // if(c.z < box.min.z) distSq += (box.min.z - c.z)*(box.min.z - c.z);
    // else if(c.z > box.max.z) distSq += (c.z - box.max.z)*(c.z - box.max.z);

    // return (distSq <= r*r);
}

void OctTree::QueryLight(const glm::vec3& lightPos, float lightRadius, std::vector<Node*>& results)
{
    // skip if region doesn't overlap sphere
    if(!AABBvsSphere(m_bounds, lightPos, lightRadius)) 
        return;

    // check each node
    for(auto* node : m_nodes)
    {
        SphereVolume lightSphere(lightPos, lightRadius);        
        BoundingVolume* boundingVol = node->GetBoundingVolume();
        if(!boundingVol) return;

        if(lightSphere.Overlaps(*boundingVol))
            results.push_back(node);
    }

    // recurse
    for(int i=0; i<8; i++){
        if(m_children[i])
            m_children[i]->QueryLight(lightPos, lightRadius, results);
    }
}

// region bounding box vs sphere
static bool RegionOverlapsSphere(const AABBVolume& region, const glm::vec3& center, float r)
{
    return AABBvsSphere(region, center, r);
}

void OctTree::Subdivide()
{
    // We create 8 sub-boxes
    glm::vec3 size = m_bounds.max - m_bounds.min;
    glm::vec3 half = size * 0.5f;
    glm::vec3 mid  = m_bounds.min + half;

    auto makeAABB = [&](float minX, float minY, float minZ, float maxX, float maxY, float maxZ){
        AABBVolume box;
        box.min = glm::vec3(minX, minY, minZ);
        box.max = glm::vec3(maxX, maxY, maxZ);
        return box;
    };

    glm::vec3 minP = m_bounds.min;
    glm::vec3 maxP = m_bounds.max;

    // Child 0 (left/bottom/back)
    AABBVolume c0 = makeAABB(minP.x, minP.y, minP.z,
                         mid.x,   mid.y,   mid.z);
    // Child 1 (right/bottom/back)
    AABBVolume c1 = makeAABB(mid.x,   minP.y, minP.z,
                         maxP.x,  mid.y,  mid.z);
    // Child 2 (left/top/back)
    AABBVolume c2 = makeAABB(minP.x,  mid.y,  minP.z,
                         mid.x,    maxP.y, mid.z);
    // Child 3 (right/top/back)
    AABBVolume c3 = makeAABB(mid.x,   mid.y,  minP.z,
                         maxP.x,  maxP.y, mid.z);

    // Child 4 (left/bottom/front)
    AABBVolume c4 = makeAABB(minP.x,  minP.y,  mid.z,
                         mid.x,    mid.y,  maxP.z);
    // Child 5 (right/bottom/front)
    AABBVolume c5 = makeAABB(mid.x,   minP.y,  mid.z,
                         maxP.x,  mid.y,   maxP.z);
    // Child 6 (left/top/front)
    AABBVolume c6 = makeAABB(minP.x,   mid.y,   mid.z,
                         mid.x,     maxP.y, maxP.z);
    // Child 7 (right/top/front)
    AABBVolume c7 = makeAABB(mid.x,    mid.y,   mid.z,
                         maxP.x,   maxP.y,  maxP.z);

    // Create the 8 children
    m_children[0] = new OctTree(c0, m_level + 1);
    m_children[1] = new OctTree(c1, m_level + 1);
    m_children[2] = new OctTree(c2, m_level + 1);
    m_children[3] = new OctTree(c3, m_level + 1);
    m_children[4] = new OctTree(c4, m_level + 1);
    m_children[5] = new OctTree(c5, m_level + 1);
    m_children[6] = new OctTree(c6, m_level + 1);
    m_children[7] = new OctTree(c7, m_level + 1);
}


void OctTree::BuildDebugLines()
{
    // Check if this node is a leaf (no children) and contains nodes
    bool isLeaf = true;
    for (int i = 0; i < NUM_CHILDREN; ++i) {
        if (m_children[i] != nullptr) {
            isLeaf = false;
            break;
        }
    }

    if (isLeaf && !m_nodes.empty()) {
        glm::vec3 pMin = m_bounds.min;
        glm::vec3 pMax = m_bounds.max;

        glm::vec3 c[8] = {
            glm::vec3(pMin.x, pMin.y, pMin.z),  // 0
            glm::vec3(pMax.x, pMin.y, pMin.z),  // 1
            glm::vec3(pMin.x, pMax.y, pMin.z),  // 2
            glm::vec3(pMax.x, pMax.y, pMin.z),  // 3
            glm::vec3(pMin.x, pMin.y, pMax.z),  // 4
            glm::vec3(pMax.x, pMin.y, pMax.z),  // 5
            glm::vec3(pMin.x, pMax.y, pMax.z),  // 6
            glm::vec3(pMax.x, pMax.y, pMax.z)   // 7
        };

        auto drawEdge = [&](int a, int b) {
            DebugRender::Instance().DrawLine(c[a], c[b], glm::vec3(0.0f, 1.0f, 0.0f), "Tree");
        };

        drawEdge(0, 1);
        drawEdge(1, 3);
        drawEdge(3, 2);
        drawEdge(2, 0);
        
        drawEdge(4, 5);
        drawEdge(5, 7);
        drawEdge(7, 6);
        drawEdge(6, 4);
        
        drawEdge(0, 4);
        drawEdge(1, 5);
        drawEdge(2, 6);
        drawEdge(3, 7);
    }

    for (int i = 0; i < NUM_CHILDREN; i++) {
        if (m_children[i]) {
            m_children[i]->BuildDebugLines();
        }
    }
}
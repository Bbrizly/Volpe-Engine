#include "OctTree.h"
#include "DebugRender.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <algorithm>

// ------------------- AABB3D Utility -------------------
bool AABB3D::IntersectsFrustum(const Frustum& frustum) const
{
    // We do a rough "AABB vs. frustum" check:
    // For each plane, project the AABB extents onto the plane normal
    // and see if it's behind the plane.

    // This is a standard technique: find the AABB center and half-extents,
    // then use "p-vertex / n-vertex" or direct radius approach.

    glm::vec3 center = 0.5f * (min + max);   // center of AABB
    glm::vec3 extents = 0.5f * (max - min); // half-size in each dimension

    for (int i = 0; i < 6; ++i) {
        glm::vec4 plane = frustum.planes[i];
        glm::vec3 normal(plane.x, plane.y, plane.z);
        float d = plane.w;

        // "Radius" of this AABB, in direction of plane normal
        float r = extents.x * std::fabs(normal.x)
                + extents.y * std::fabs(normal.y)
                + extents.z * std::fabs(normal.z);

        // Distance from center to plane
        float dist = glm::dot(normal, center) + d;

        // If the AABB is completely behind this plane
        if (dist < -r) {
            return false; // outside
        }
    }
    return true; // overlap
}

// ------------------- OctTree Implementation -------------------
OctTree::OctTree(const AABB3D& bounds, int level /*=0*/)
    : m_bounds(bounds)
    , m_level(level)
{
    // no children yet
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
    if (!node) return;

    // Extract world position from node
    glm::mat4 world = node->getWorldTransform();
    glm::vec3 pos3D = glm::vec3(world[3]); // x,y,z from the translation part

    // If not in this box, skip
    if (!Contains(pos3D)) {
        return;
    }

    // DebugRender::Instance().DrawCircle(pos3D, 0.2f, glm::vec3(1.0f));

    // If we have no children and we haven't exceeded capacity or levels, store the node
    if (!m_children[0] && ((int)m_nodes.size() < MAX_OBJECTS || m_level == MAX_LEVELS)) {
        m_nodes.push_back(node);
        return;
    }

    // If no children yet, subdivide
    if (!m_children[0]) {
        Subdivide();

        // push existing nodes down into children
        for (auto* existing : m_nodes) {
            glm::vec3 ePos = glm::vec3(existing->getWorldTransform()[3]);
            for (int i=0; i<NUM_CHILDREN; i++){
                if (m_children[i]->Contains(ePos)){
                    m_children[i]->Insert(existing);
                    break;
                }
            }
        }
        m_nodes.clear();
    }

    // Insert new node into whichever child contains it
    for (int i=0; i<NUM_CHILDREN; i++){
        if (m_children[i]->Contains(pos3D)){
            m_children[i]->Insert(node);
            break;
        }
    }
}

void OctTree::Query(const Frustum& frustum, std::vector<Node*>& results)
{
    // 1) AABB vs. frustum check: if bounding box is fully outside, skip
    if (!m_bounds.IntersectsFrustum(frustum)) {
        return;
    }

    // 2) Check each node stored at this level
    for (auto* node : m_nodes) {
        // If bounding sphere of node is inside frustum, add it
        if (SphereIntersectsFrustum(node->getBoundingSphere(), frustum)) {
            results.push_back(node);
        }
    }

    // 3) Recurse children
    for (int i=0; i<NUM_CHILDREN; i++){
        if (m_children[i]) {
            m_children[i]->Query(frustum, results);
        }
    }
}

bool OctTree::SphereIntersectsFrustum(const BoundingSphere& sphere, const Frustum& frustum) const
{
    // same logic you used in QuadTree
    for (int i = 0; i < 6; i++) {
        glm::vec3 planeNormal = glm::vec3(frustum.planes[i]);
        float distance = glm::dot(planeNormal, sphere.center) + frustum.planes[i].w;
        if (distance < -sphere.radius)
            return false;
    }
    return true;
}

void OctTree::Subdivide()
{
    // We create 8 sub-boxes
    glm::vec3 size = m_bounds.max - m_bounds.min;
    glm::vec3 half = size * 0.5f; // half the extent
    glm::vec3 mid  = m_bounds.min + half; // midpoint

    // Now each child is one "octant" of the parent bounding box
    // child 0: min        -> mid
    // child 1: (x-mid)    -> (x-max, y-mid, z-mid), etc.
    // We'll systematically define them:

    // We'll create a small helper lambda to quickly build AABB3D
    auto makeAABB = [&](float minX, float minY, float minZ, float maxX, float maxY, float maxZ){
        AABB3D box;
        box.min = glm::vec3(minX, minY, minZ);
        box.max = glm::vec3(maxX, maxY, maxZ);
        return box;
    };

    glm::vec3 minP = m_bounds.min; // for convenience
    glm::vec3 maxP = m_bounds.max;

    // Child 0 (left/bottom/back)
    AABB3D c0 = makeAABB(minP.x, minP.y, minP.z,
                         mid.x,   mid.y,   mid.z);
    // Child 1 (right/bottom/back)
    AABB3D c1 = makeAABB(mid.x,   minP.y, minP.z,
                         maxP.x,  mid.y,  mid.z);
    // Child 2 (left/top/back)
    AABB3D c2 = makeAABB(minP.x,  mid.y,  minP.z,
                         mid.x,    maxP.y, mid.z);
    // Child 3 (right/top/back)
    AABB3D c3 = makeAABB(mid.x,   mid.y,  minP.z,
                         maxP.x,  maxP.y, mid.z);

    // Child 4 (left/bottom/front)
    AABB3D c4 = makeAABB(minP.x,  minP.y,  mid.z,
                         mid.x,    mid.y,  maxP.z);
    // Child 5 (right/bottom/front)
    AABB3D c5 = makeAABB(mid.x,   minP.y,  mid.z,
                         maxP.x,  mid.y,   maxP.z);
    // Child 6 (left/top/front)
    AABB3D c6 = makeAABB(minP.x,   mid.y,   mid.z,
                         mid.x,     maxP.y, maxP.z);
    // Child 7 (right/top/front)
    AABB3D c7 = makeAABB(mid.x,    mid.y,   mid.z,
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

// void OctTree::Render(const glm::mat4& proj, const glm::mat4& view)
// {
//     if (keepOld) {
//         keepOld = false;
//         DebugRender::Instance().Clear();
//         BuildDebugLines();
//     }
 
//     glDisable(GL_DEPTH_TEST);
//     DebugRender::Instance().Render(proj, view);
//     glEnable(GL_DEPTH_TEST);
// }

void OctTree::BuildDebugLines()
{
    // DebugRender::Instance().Clear();
    // Draw the bounding box edges as lines
    // We'll do 12 lines of the cuboid
    glm::vec3 pMin = m_bounds.min;
    glm::vec3 pMax = m_bounds.max;

    // We'll define 8 corners of the cube:
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

    // We connect edges: (0->1,1->3,3->2,2->0) = bottom face
    // (4->5,5->7,7->6,6->4) = top face
    // vertical edges (0->4,1->5,2->6,3->7)

    auto drawEdge = [&](int a, int b) {
        DebugRender::Instance().DrawLine(c[a], c[b], glm::vec3(0.0f,1.0f,0.0f));
    };

    // bottom
    drawEdge(0,1);
    drawEdge(1,3);
    drawEdge(3,2);
    drawEdge(2,0);
    // top
    drawEdge(4,5);
    drawEdge(5,7);
    drawEdge(7,6);
    drawEdge(6,4);
    // vertical edges
    drawEdge(0,4);
    drawEdge(1,5);
    drawEdge(2,6);
    drawEdge(3,7);

    // Recurse
    for (int i=0; i<NUM_CHILDREN; i++){
        if (m_children[i]) {
            m_children[i]->BuildDebugLines();
        }
    }
}


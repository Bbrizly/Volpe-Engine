#pragma once
#include <vector>
#include "Node.h"
#include "Frustum.h"
#include "DebugRender.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <algorithm>
#include "DebugCube.h"
#include "AABB3D.h"

// 3D Oct-Tree
class OctTree
{
public:
    OctTree(const AABB3D& bounds, int level = 0);
    ~OctTree();

    void Insert(Node* node);
    void Query(const Frustum& frustum, std::vector<Node*>& results);
    void QueryLight(const glm::vec3& lightPos, float lightRadius, std::vector<Node*>& results);


    void BuildDebugLines();
    void Render(const glm::mat4& proj, const glm::mat4& view);

private:
    AABB3D m_bounds;
    int    m_level;

    // Store child pointers. If not subdivided, children are nullptr
    static const int NUM_CHILDREN = 8;
    OctTree* m_children[NUM_CHILDREN] = { nullptr };

    std::vector<Node*> m_nodes;

    // Tuning constants
    static const int MAX_OBJECTS = 4;
    static const int MAX_LEVELS  = 5;

    void Subdivide();
    bool Contains(const glm::vec3& point) const;
    bool SphereIntersectsFrustum(const BoundingSphere& sphere, const Frustum& frustum) const;

    // If true, we have not yet built debug lines
    bool keepOld = true;
};


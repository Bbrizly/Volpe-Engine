#ifndef QUADTREE_H
#define QUADTREE_H

#include <vector>
#include <glm/glm.hpp>
#include "Node.h"
#include "Frustum.h"

#include "DebugRender.h"
#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h>
#include <iostream>

struct AABB2D {
    glm::vec2 min;
    glm::vec2 max;
    bool IntersectsFrustum(const Frustum& frustum) const;
};

class QuadTree {
public:
    QuadTree(const AABB2D& bounds, int level = 0);
    ~QuadTree();
    
    void Insert(Node* node);
    void Query(const Frustum& frustum, std::vector<Node*>& results);
    
    void BuildDebugLines();
    void Render(const glm::mat4& proj, const glm::mat4& view);

private:
    bool Contains(const glm::vec2& point) const;
    void Subdivide();
    bool SphereIntersectsFrustum(const BoundingSphere& sphere, const Frustum& frustum);

    void DrawLine(const glm::vec3& a, const glm::vec3& b, const glm::mat4& proj, const glm::mat4& view);

    AABB2D m_bounds;
    int m_level;
    std::vector<Node*> m_nodes;
    std::vector<QuadTree*> m_children;

    bool keepOld = true;
    
    static const int MAX_OBJECTS = 4;
    static const int MAX_LEVELS  = 5;
};

#endif // QUADTREE_H

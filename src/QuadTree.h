#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "Node.h"
#include "Frustum.h"
#include "DebugRender.h"
#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h>
#include <iostream>
#include "../samplefw/BoundingVolumes/AABB2D.h"
#include "../samplefw/BoundingVolumes/AABBVolume.h"
#include "../samplefw/BoundingVolumes/SphereVolume.h"

class QuadTree {
public:
    QuadTree(const AABB2D& bounds, int level = 0);
    ~QuadTree();
    
    void Insert(Node* node);
    void Query(const Frustum& frustum, std::vector<Node*>& results);
    void QueryLight(const glm::vec3& lightPos, float lightRadius, std::vector<Node*>& results);
    
    void BuildDebugLines();
    void Render(const glm::mat4& proj, const glm::mat4& view);

private:
    bool Contains(const glm::vec2& point) const;
    void Subdivide();
    
    AABB2D FlattenBoundingVolume(const BoundingVolume* bv) const;
    
    AABB2D m_bounds;
    int m_level;
    std::vector<Node*> m_nodes;
    std::vector<QuadTree*> m_children;
    
    bool keepOld = true;
    
    static const int MAX_OBJECTS = 4;
    static const int MAX_LEVELS  = 5;
};


#ifndef DEBUGLAYER_H
#define DEBUGLAYER_H

#include <string>
#include <vector>
#include <glm/glm.hpp>

struct DebugLine {
    glm::vec3 start;
    glm::vec3 end;
    glm::vec3 color;
};

class DebugLayer {
public:
    DebugLayer(const std::string& name) : m_name(name), m_enabled(true), m_transform(1.0f) {}
    ~DebugLayer() {};

    void AddLine(const glm::vec3& start, const glm::vec3& end, const glm::vec3& color) {m_lines.push_back({start,end,color});}
    
    void Clear() {m_lines.clear();}
    
    const std::vector<DebugLine>& GetLines() const {return m_lines;}
    
    void SetTransform(const glm::mat4& transform) {m_transform = transform;}

    const glm::mat4& GetTransform() const {return m_transform;}
    
    void SetEnabled(bool enabled) {m_enabled = enabled;}
    bool IsEnabled() const {return m_enabled;}
    
    const std::string& GetName() const {return m_name;}
private:
    std::string m_name;
    bool m_enabled;
    glm::mat4 m_transform;
    std::vector<DebugLine> m_lines;
};

#endif // DEBUGLAYER_H

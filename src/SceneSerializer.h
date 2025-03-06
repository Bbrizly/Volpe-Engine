#pragma once

#include <string>
#include "Scene.h"
#include "ParticleNode.h"
#include <yaml-cpp/yaml.h>

YAML::Emitter& operator<<(YAML::Emitter& out, const glm::mat4& mat);

struct SerializedTransform {
    glm::vec3 pos;
    glm::quat rot;
    glm::vec3 scl;
};

class SceneSerializer
{
public:
    static void SaveScene(Scene& scene, const std::string& givenFilePath);
    static void LoadScene(Scene& scene, const std::string& givenFilePath);

private:
    static YAML::Node SerializeNode(Node* node);
    static Node*     DeserializeNode(const YAML::Node& nodeData, Scene& scene, Node* parent);

    // for gradient keysss
    static YAML::Node SerializeColorKeys(const std::vector<ColorKey>& keys);
    static void DeserializeColorKeys(const YAML::Node& node, std::vector<ColorKey>& outKeys);

    // Transform to YAML
    static YAML::Node SerializeTransform(const Node* node);
    static void       DeserializeTransform(Node* node, const YAML::Node& transformNode);

    static YAML::Node SerializeParticleNode(const ParticleNode* emitter);
    static void       DeserializeParticleNode(const YAML::Node& n, ParticleNode* emitter);
};

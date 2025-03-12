#pragma once

#include <string>
#include "Scene.h"
#include "ParticleNode.h"
#include "Affector.h"
#include "EffectNode.h"
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
    
    static void DeserializeParticleNodeFromFile(const std::string& filePath, ParticleNode* outEmitter);

    
    static void SaveEffectNode(EffectNode* effect, const std::string& filePath);
    static EffectNode* LoadEffectNode(Scene& scene, const std::string& filePath);

    static void SaveEmitter(ParticleNode* emitter, const std::string& filePath);
    static ParticleNode* LoadEmitter(const std::string& filePath);

private:
    int variance = 0;
    static YAML::Node SerializeNode(Node* node);
    static Node*     DeserializeNode(const YAML::Node& nodeData, Scene& scene, Node* parent);
    
    static YAML::Node SerializeEffectNode(EffectNode* fx);
    static Node* DeserializeEffectNode(const YAML::Node& nodeData, Scene& scene);

    // for gradient keysss
    static YAML::Node SerializeColorKeys(const std::vector<ColorKey>& keys);
    static void DeserializeColorKeys(const YAML::Node& node, std::vector<ColorKey>& outKeys);

    // Transform to YAML
    static YAML::Node SerializeTransform(const Node* node);
    static void       DeserializeTransform(Node* node, const YAML::Node& transformNode);

    static YAML::Node SerializeParticleNode(const ParticleNode* emitter);
    static void       DeserializeParticleNode(const YAML::Node& n, ParticleNode* emitter);

    static YAML::Node SerializeAffectors(const std::vector<Affector*>& affs);
    static void       DeserializeAffectors(const YAML::Node& n, ParticleNode* emitter);
};

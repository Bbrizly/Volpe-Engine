#pragma once

#include <string>
#include "Scene.h"
#include <yaml-cpp/yaml.h>

#include "ParticleNode.h" // So that ParticleNode and EmitterShape are defined

// Free function operator<< for glm::mat4.
YAML::Emitter& operator<<(YAML::Emitter& out, const glm::mat4& mat);

class SceneSerializer
{
public:
    // Save the entire scene to a YAML file.
    static void SaveScene(Scene& scene, std::string& filePath);

    // Load the scene from a YAML file.
    static void LoadScene(Scene& scene, std::string& filePath);

private:
    // Helper to serialize an individual node.
    static YAML::Node SerializeNode(Node* node);

    // Helper to deserialize a node and add it to 'scene'.
    static Node* DeserializeNode(YAML::Node& nodeData, Scene& scene, Node* parent);

    // Helper to read a glm::mat4 from a YAML node.
    static glm::mat4 ReadMat4(YAML::Node& node);
};

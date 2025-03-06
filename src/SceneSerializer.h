#pragma once

#include <string>
#include "Scene.h"
#include <yaml-cpp/yaml.h>

#include "ParticleNode.h"

struct SerializedTransform {
    glm::vec3 pos;
    glm::quat rot;
    glm::vec3 scl;
};

class SceneSerializer
{
public:

    static void SaveScene(Scene& scene, const std::string& filePath);
    static void LoadScene(Scene& scene, const std::string& filePath);


private:
    static YAML::Node SerializeNode(Node* node);

    static Node* DeserializeNode(YAML::Node& nodeData, Scene& scene, Node* parent);

    static glm::mat4 ReadMat4(YAML::Node& node);
};

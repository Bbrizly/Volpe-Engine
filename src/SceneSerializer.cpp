#include "SceneSerializer.h"
#include <fstream>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

YAML::Emitter& operator<<(YAML::Emitter& out, const glm::mat4& mat)
{
    out << YAML::BeginSeq;
    for (int row = 0; row < 4; ++row)
    {
        for (int col = 0; col < 4; ++col)
        {
            float value = mat[row][col];
            out << value;
        }
    }
    out << YAML::EndSeq;
    return out;
}

glm::mat4 SceneSerializer::ReadMat4(YAML::Node& node)
{
    glm::mat4 out(1.0f);
    if (!node.IsSequence() || node.size() != 16) {
        std::cerr << "[SceneSerializer] Invalid mat4 in YAML. Using identity.\n";
        return out;
    }
    int idx = 0;
    for (int row = 0; row < 4; row++){
        for (int col = 0; col < 4; col++){
            out[row][col] = node[idx++].as<float>();
        }
    }
    return out;
}

void SceneSerializer::SaveScene(Scene& scene, std::string& filePath)
{
    YAML::Node root;
    root["Scene"] = "VolpeScene";

    YAML::Node nodesNode;
    auto& allNodes = scene.GetNodes();
    for (auto* node : allNodes)
    {
        nodesNode.push_back( SerializeNode(node) );
    }
    root["Nodes"] = nodesNode;

    // Serialize lights.
    YAML::Node lightsNode;
    auto& lights = scene.GetLights();
    for(auto& L : lights)
    {
        YAML::Emitter posEmitter;
        posEmitter << YAML::Flow << YAML::BeginSeq << L.position.x << L.position.y << L.position.z << YAML::EndSeq;
        YAML::Emitter colEmitter;
        colEmitter << YAML::Flow << YAML::BeginSeq << L.color.r << L.color.g << L.color.b << YAML::EndSeq;
        
        YAML::Node ln;
        ln["Position"]  = YAML::Load(posEmitter.c_str());
        ln["Color"]     = YAML::Load(colEmitter.c_str());
        ln["Radius"]    = L.radius;
        ln["Intensity"] = L.intensity;
        lightsNode.push_back(ln);
    }

    root["Lights"] = lightsNode;

    // Write YAML out to file.
    std::ofstream fout(filePath);
    if(!fout.is_open()){
        std::cerr << "[SceneSerializer] Could not open file for writing: " << filePath << "\n";
        return;
    }
    fout << root;
    fout.close();
    std::cout << "[SceneSerializer] Saved scene to " << filePath << "\n";
}

void SceneSerializer::LoadScene(Scene& scene, std::string& filePath)
{
    YAML::Node root;
    try {
        root = YAML::LoadFile(filePath);
    }
    catch(YAML::Exception& e) {
        std::cerr << "[SceneSerializer] YAML Exception while loading: " << e.what() << std::endl;
        return;
    }
    if(!root["Nodes"]){
        std::cerr << "[SceneSerializer] No 'Nodes' found in file. Aborting load.\n";
        return;
    }

    scene.Clear();

    if(root["Scene"]){
        std::string sceneName = root["Scene"].as<std::string>();
        std::cout << "[SceneSerializer] Loading scene: " << sceneName << "\n";
    }

    // Deserialize nodes.
    YAML::Node& nodesNode = root["Nodes"];
    for(auto nodeData : nodesNode){
        DeserializeNode(nodeData, scene, nullptr);
    }

    // Deserialize lights.
    if(root["Lights"]){
        auto lightsNode = root["Lights"];
        for(auto ln : lightsNode){
            Light L;
            if(ln["Position"] && ln["Position"].size() == 3){
                L.position.x = ln["Position"][0].as<float>();
                L.position.y = ln["Position"][1].as<float>();
                L.position.z = ln["Position"][2].as<float>();
            }
            if(ln["Color"] && ln["Color"].size() == 3){
                L.color.r = ln["Color"][0].as<float>();
                L.color.g = ln["Color"][1].as<float>();
                L.color.b = ln["Color"][2].as<float>();
            }
            if(ln["Radius"])    L.radius    = ln["Radius"].as<float>();
            if(ln["Intensity"]) L.intensity = ln["Intensity"].as<float>();
            scene.AddLight(L);
        }
    }

    std::cout << "[SceneSerializer] Scene loaded from " << filePath << "\n";
}

//SINGLE NODE TO YAML
YAML::Node SceneSerializer::SerializeNode(Node* node)
{
    YAML::Node n;
    if(!node) return n;

    n["Name"] = node->getName();

    
    glm::mat4 localTransform = node->getTransform();
    {
        YAML::Emitter emitter;
        emitter << localTransform;
        YAML::Node matNode = YAML::Load(emitter.c_str());
        n["LocalTransform"] = matNode;
    }

    
    if (auto* c = dynamic_cast<DebugCube*>(node)) {
        n["Type"] = "DebugCube";
        glm::vec3 col = c->getColor();
        YAML::Node colorNode;
        colorNode.push_back(col.r);
        colorNode.push_back(col.g);
        colorNode.push_back(col.b);
        n["Color"] = colorNode;
    }
    else if (auto* s = dynamic_cast<DebugSphere*>(node)) {
        n["Type"] = "DebugSphere";
        glm::vec3 col = s->getColor();
        float radius  = s->getRadius();
        YAML::Node colorNode;
        colorNode.push_back(col.r);
        colorNode.push_back(col.g);
        colorNode.push_back(col.b);
        n["Color"] = colorNode;
        n["Radius"] = radius;
    }
    else if (auto* emitter = dynamic_cast<ParticleNode*>(node)) {
        n["Type"] = "ParticleNode";
        n["EmissionRate"] = emitter->emissionRate;
        n["MaxParticles"] = emitter->maxParticles;
        n["Shape"]        = static_cast<int>(emitter->shape);
        n["LocalSpace"]   = emitter->localSpace;
        // add rest of particle shits here
    }
    else {
        n["Type"] = "Node";
    }


    return n;
}

//Single Node from yaml to scene
Node* SceneSerializer::DeserializeNode(YAML::Node& nodeData, Scene& scene, Node* parent)
{
    if(!nodeData["Name"] || !nodeData["Type"]){
        std::cerr << "[SceneSerializer] Node missing required fields.\n";
        return nullptr;
    }
    std::string name = nodeData["Name"].as<std::string>();
    std::string type = nodeData["Type"].as<std::string>();

    glm::mat4 localTransform(1.0f);
    if(nodeData["LocalTransform"]){
        localTransform = ReadMat4(nodeData["LocalTransform"]);
    }

    Node* newNode = nullptr;
    if(type == "DebugCube"){
        auto* c = new DebugCube(name);
        newNode = c;
        if(nodeData["Color"] && nodeData["Color"].IsSequence() && nodeData["Color"].size() == 3){
            float r = nodeData["Color"][0].as<float>();
            float g = nodeData["Color"][1].as<float>();
            float b = nodeData["Color"][2].as<float>();
            c->setColor((GLubyte)(r*255), (GLubyte)(g*255), (GLubyte)(b*255));
        }
    }
    else if(type == "DebugSphere"){
        float radius = 1.0f;
        if(nodeData["Radius"]) radius = nodeData["Radius"].as<float>();
        auto* s = new DebugSphere(name, radius);
        newNode = s;
        if(nodeData["Color"] && nodeData["Color"].IsSequence() && nodeData["Color"].size() == 3){
            float r = nodeData["Color"][0].as<float>();
            float g = nodeData["Color"][1].as<float>();
            float b = nodeData["Color"][2].as<float>();
            s->setColor((GLubyte)(r*255), (GLubyte)(g*255), (GLubyte)(b*255));
        }
    }
    else if(type == "ParticleNode"){
        auto* emitter = new ParticleNode(name);
        newNode = emitter;
        if(nodeData["EmissionRate"])
            emitter->emissionRate = nodeData["EmissionRate"].as<float>();
        if(nodeData["MaxParticles"])
            emitter->maxParticles = nodeData["MaxParticles"].as<int>();
        if(nodeData["Shape"])
            emitter->shape = (EmitterShape)nodeData["Shape"].as<int>();
        if(nodeData["LocalSpace"])
            emitter->localSpace = nodeData["LocalSpace"].as<bool>();

    }
    else {
        newNode = new Node(name);
    }

    newNode->setTransform(localTransform);
    scene.AddNode(newNode);

    return newNode;
}

#include "SceneSerializer.h"
#include <fstream>
#include <iostream>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
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

//SUPER BUGGY Quaternion implementaion
static YAML::Node MakeQuatNode(const glm::quat& q)
{
    YAML::Node node;
    node.SetStyle(YAML::EmitterStyle::Flow);
    node.push_back(q.x);
    node.push_back(q.y);
    node.push_back(q.z);
    node.push_back(q.w);
    return node;
}
static glm::quat ReadQuat(const YAML::Node& n)
{
    glm::quat q(1,0,0,0);
    if(n.IsSequence() && n.size() == 4){
        q.x = n[0].as<float>();
        q.y = n[1].as<float>();
        q.z = n[2].as<float>();
        q.w = n[3].as<float>();
    }
    return q;
}

//  SAVING
void SceneSerializer::SaveScene(Scene& scene, const std::string& givenFilePath)
{
    YAML::Node root;
    root["Scene"] = "VolpeScene";

    // Save nodes
    YAML::Node allNodes;
    auto& nodes = scene.GetNodes();
    for(auto* n : nodes){
        allNodes.push_back( SerializeNode(n) );
    }
    root["Nodes"] = allNodes;

    YAML::Node lightsArr;
    for(auto& L : scene.GetLights()){
        YAML::Node ln;
        // position
        {
            YAML::Node pos;
            pos.SetStyle(YAML::EmitterStyle::Flow);
            pos.push_back(L.position.x);
            pos.push_back(L.position.y);
            pos.push_back(L.position.z);
            ln["Position"] = pos;
        }
        // color
        {
            YAML::Node col;
            col.SetStyle(YAML::EmitterStyle::Flow);
            col.push_back(L.color.r);
            col.push_back(L.color.g);
            col.push_back(L.color.b);
            ln["Color"] = col;
        }
        ln["Radius"]    = L.radius;
        ln["Intensity"] = L.intensity;
        lightsArr.push_back(ln);
    }
    root["Lights"] = lightsArr;

    // filePath += ".yaml";
    std::string filePath = givenFilePath;
    filePath += ".yaml";
    std::ofstream fout(filePath);
    if(!fout.is_open()){
        std::cerr<<"[SceneSerializer] Cannot open file for write: "<<filePath<<"\n";
        return;
    }
    fout << root;
    fout.close();

    std::cout<<"[SceneSerializer] Saved scene to "<<filePath<<"\n";
}

void SceneSerializer::LoadScene(Scene& scene, const std::string& givenFilePath)
{
    YAML::Node root;
    std::string filePath = givenFilePath;
    filePath += ".yaml";
    try {
        root = YAML::LoadFile(filePath);
    }
    catch(const YAML::Exception& e){
        std::cerr<<"[SceneSerializer] YAML Error: "<<e.what()<<"\n";
        return;
    }
    if(!root["Nodes"]){
        std::cerr<<"[SceneSerializer] No 'Nodes' in file. Aborting.\n";
        return;
    }

    scene.Clear();

    if(root["Scene"]){
        std::cout<<"[SceneSerializer] Loading scene: "<<root["Scene"].as<std::string>()<<"\n";
    }

    const YAML::Node& nodes = root["Nodes"];
    for(auto n : nodes){
        DeserializeNode(n, scene, nullptr);
    }

    if(root["Lights"]){
        auto Ls = root["Lights"];
        for(auto ln : Ls){
            Light l;
            if(ln["Position"] && ln["Position"].IsSequence() && ln["Position"].size() == 3){
                l.position.x = ln["Position"][0].as<float>();
                l.position.y = ln["Position"][1].as<float>();
                l.position.z = ln["Position"][2].as<float>();
            }
            if(ln["Color"] && ln["Color"].IsSequence() && ln["Color"].size() == 3){
                l.color.r = ln["Color"][0].as<float>();
                l.color.g = ln["Color"][1].as<float>();
                l.color.b = ln["Color"][2].as<float>();
            }
            if(ln["Radius"])    l.radius    = ln["Radius"].as<float>();
            if(ln["Intensity"]) l.intensity = ln["Intensity"].as<float>();
            scene.AddLight(l);
        }
    }

    std::cout<<"[SceneSerializer] Scene loaded from "<<filePath<<"\n";
}

YAML::Node SceneSerializer::SerializeNode(Node* node)
{
    YAML::Node n;
    n["Name"] = node->getName();

    // diff values for each type of object
    if(auto* c = dynamic_cast<DebugCube*>(node)){
        n["Type"] = "DebugCube";
        glm::vec3 col = c->getColor();
        YAML::Node colNode;
        colNode.SetStyle(YAML::EmitterStyle::Flow);
        colNode.push_back(col.r);
        colNode.push_back(col.g);
        colNode.push_back(col.b);
        n["Color"] = colNode;
    }
    else if(auto* s = dynamic_cast<DebugSphere*>(node)){
        n["Type"] = "DebugSphere";
        glm::vec3 col = s->getColor();
        YAML::Node colNode;
        colNode.SetStyle(YAML::EmitterStyle::Flow);
        colNode.push_back(col.r);
        colNode.push_back(col.g);
        colNode.push_back(col.b);
        n["Color"]  = colNode;
        n["Radius"] = s->getRadius();
    }
    else if(auto* emitter = dynamic_cast<ParticleNode*>(node)){
        n["Type"] = "ParticleNode";
        n["ParticleData"] = SerializeParticleNode(emitter);
    }
    else {
        n["Type"] = "Node";
    }

    n["Transform"] = SerializeTransform(node);

    return n;
}

Node* SceneSerializer::DeserializeNode(const YAML::Node& nodeData, Scene& scene, Node* parent)
{
    if(!nodeData["Name"] || !nodeData["Type"]){
        std::cerr<<"[SceneSerializer] Node missing Name or Type.\n";
        return nullptr;
    }
    std::string name = nodeData["Name"].as<std::string>();
    std::string type = nodeData["Type"].as<std::string>();

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
        if(nodeData["Radius"]){
            radius = nodeData["Radius"].as<float>();
        }
        auto* s = new DebugSphere(name, radius);
        newNode = s;
        if(nodeData["Color"] && nodeData["Color"].IsSequence() && nodeData["Color"].size()==3){
            float r = nodeData["Color"][0].as<float>();
            float g = nodeData["Color"][1].as<float>();
            float b = nodeData["Color"][2].as<float>();
            s->setColor((GLubyte)(r*255), (GLubyte)(g*255), (GLubyte)(b*255));
        }
    }
    else if(type == "ParticleNode"){
        auto* emitter = new ParticleNode(name);
        newNode = emitter;
        if(nodeData["ParticleData"]){
            DeserializeParticleNode(nodeData["ParticleData"], emitter);
        }
    }
    else{
        newNode = new Node(name);
    }

    if(newNode){
        // Apply transform
        if(nodeData["Transform"]){
            DeserializeTransform(newNode, nodeData["Transform"]);
        }

        // Deal with children
        if(parent){
            parent->addChild(newNode);
            scene.AddNode(newNode);
        } else {
            scene.AddNode(newNode);
        }
        // scene.AddNode(newNode);

        // node has any children
        if(nodeData["Children"]){
            for(auto childData : nodeData["Children"]){
                DeserializeNode(childData, scene, newNode);
            }
        }
    }
    return newNode;
}


YAML::Node SceneSerializer::SerializeTransform(const Node* node)
{
    YAML::Node transform;
    glm::vec3 pos, scl;
    glm::quat rot;
    node->getTransformDecomposed(pos, rot, scl);

    // Position
    {
        YAML::Node p;
        p.SetStyle(YAML::EmitterStyle::Flow);
        p.push_back(pos.x);
        p.push_back(pos.y);
        p.push_back(pos.z);
        transform["Position"] = p;
    }
    // Rotation
    {
        YAML::Node r = MakeQuatNode(rot);
        transform["Rotation"] = r;
    }
    // Scale
    {
        YAML::Node s;
        s.SetStyle(YAML::EmitterStyle::Flow);
        s.push_back(scl.x);
        s.push_back(scl.y);
        s.push_back(scl.z);
        transform["Scale"] = s;
    }
    return transform;
}

void SceneSerializer::DeserializeTransform(Node* node, const YAML::Node& transformNode)
{
    glm::vec3 pos(0), scl(1.0f);
    glm::quat rot(1,0,0,0);

    if(transformNode["Position"] && transformNode["Position"].IsSequence() && transformNode["Position"].size() == 3){
        pos.x = transformNode["Position"][0].as<float>();
        pos.y = transformNode["Position"][1].as<float>();
        pos.z = transformNode["Position"][2].as<float>();
    }
    if(transformNode["Rotation"] && transformNode["Rotation"].IsSequence() && transformNode["Rotation"].size() == 4){
        rot = ReadQuat(transformNode["Rotation"]);
    }
    if(transformNode["Scale"] && transformNode["Scale"].IsSequence() && transformNode["Scale"].size() == 3){
        scl.x = transformNode["Scale"][0].as<float>();
        scl.y = transformNode["Scale"][1].as<float>();
        scl.z = transformNode["Scale"][2].as<float>();
    }
    node->setTransformDecomposed(pos, rot, scl);
}

//Partcile node
YAML::Node SceneSerializer::SerializeParticleNode(const ParticleNode* emitter)
{
    YAML::Node n;
    n["emissionRate"]       = emitter->emissionRate;
    n["localSpace"]         = emitter->localSpace;
    n["maxParticles"]       = emitter->maxParticles;
    n["shape"]              = (int) emitter->shape;
    // n["spawnPosition"]      = YAML::Flow << YAML::BeginSeq << emitter->spawnPosition.x << emitter->spawnPosition.y << emitter->spawnPosition.z << YAML::EndSeq;
    // n["spawnVelocity"]      = YAML::Flow << YAML::BeginSeq << emitter->spawnVelocity.x << emitter->spawnVelocity.y << emitter->spawnVelocity.z << YAML::EndSeq;
    // n["globalAcceleration"] = YAML::Flow << YAML::BeginSeq << emitter->globalAcceleration.x << emitter->globalAcceleration.y << emitter->globalAcceleration.z << YAML::EndSeq;

    n["globalAcceleration"].SetStyle(YAML::EmitterStyle::Flow);
    n["globalAcceleration"].push_back(emitter->globalAcceleration.x);
    n["globalAcceleration"].push_back(emitter->globalAcceleration.y);
    n["globalAcceleration"].push_back(emitter->globalAcceleration.z);

    n["spawnPosition"].SetStyle(YAML::EmitterStyle::Flow);
    n["spawnPosition"].push_back(emitter->spawnPosition.x);
    n["spawnPosition"].push_back(emitter->spawnPosition.y);
    n["spawnPosition"].push_back(emitter->spawnPosition.z);

    n["spawnVelocity"].SetStyle(YAML::EmitterStyle::Flow);
    n["spawnVelocity"].push_back(emitter->spawnVelocity.x);
    n["spawnVelocity"].push_back(emitter->spawnVelocity.y);
    n["spawnVelocity"].push_back(emitter->spawnVelocity.z);

    // lifetime range
    n["lifetimeMin"] = emitter->lifetimeMin;
    n["lifetimeMax"] = emitter->lifetimeMax;

    // size, alpha range
    n["startSizeMin"]  = emitter->startSizeMin;
    n["startSizeMax"]  = emitter->startSizeMax;
    n["startAlphaMin"] = emitter->startAlphaMin;
    n["startAlphaMax"] = emitter->startAlphaMax;

    // rotation range
    n["rotationMin"]      = emitter->rotationMin;
    n["rotationMax"]      = emitter->rotationMax;
    n["rotationSpeedMin"] = emitter->rotationSpeedMin;
    n["rotationSpeedMax"] = emitter->rotationSpeedMax;

    // velocity scale
    n["velocityScaleMin"] = emitter->velocityScaleMin;
    n["velocityScaleMax"] = emitter->velocityScaleMax;

    // Over-lifetime
    n["sizeOverLife"]  = emitter->sizeOverLife;
    n["alphaOverLife"] = emitter->alphaOverLife;

    // bursts
    if(!emitter->burstTimes.empty()){
        YAML::Node bursts;
        for(size_t i=0; i<emitter->burstTimes.size(); i++){
            YAML::Node b;
            b["time"]  = emitter->burstTimes[i];
            b["count"] = emitter->burstCounts[i];
            bursts.push_back(b);
        }
        n["bursts"] = bursts;
    }

    n["colorKeys"] = SerializeColorKeys(emitter->colorKeys); // AAAAAAAAAAAAAAAAAAAAAAA

    if(emitter->GetMaterial()){
        auto* tex = emitter->GetMaterial()->GetTexture("u_texture");
        if(tex){
            std::string texPath = tex->texPath;
            n["texturePath"] = texPath;
        }
    }

    return n;
}

void SceneSerializer::DeserializeParticleNode(const YAML::Node& n, ParticleNode* emitter)
{
    // basic fields
    if(n["emissionRate"])       emitter->emissionRate       = n["emissionRate"].as<float>();
    if(n["localSpace"])         emitter->localSpace         = n["localSpace"].as<bool>();
    if(n["maxParticles"])       emitter->maxParticles       = n["maxParticles"].as<int>();
    if(n["shape"])              emitter->shape              = (EmitterShape)n["shape"].as<int>();

    if(n["spawnPosition"] && n["spawnPosition"].size()==3){
        emitter->spawnPosition.x = n["spawnPosition"][0].as<float>();
        emitter->spawnPosition.y = n["spawnPosition"][1].as<float>();
        emitter->spawnPosition.z = n["spawnPosition"][2].as<float>();
    }
    if(n["spawnVelocity"] && n["spawnVelocity"].size()==3){
        emitter->spawnVelocity.x = n["spawnVelocity"][0].as<float>();
        emitter->spawnVelocity.y = n["spawnVelocity"][1].as<float>();
        emitter->spawnVelocity.z = n["spawnVelocity"][2].as<float>();
    }
    if(n["globalAcceleration"] && n["globalAcceleration"].size()==3){
        emitter->globalAcceleration.x = n["globalAcceleration"][0].as<float>();
        emitter->globalAcceleration.y = n["globalAcceleration"][1].as<float>();
        emitter->globalAcceleration.z = n["globalAcceleration"][2].as<float>();
    }

    // lifetime
    if(n["lifetimeMin"]) emitter->lifetimeMin = n["lifetimeMin"].as<float>();
    if(n["lifetimeMax"]) emitter->lifetimeMax = n["lifetimeMax"].as<float>();

    // size/alpha
    if(n["startSizeMin"])  emitter->startSizeMin  = n["startSizeMin"].as<float>();
    if(n["startSizeMax"])  emitter->startSizeMax  = n["startSizeMax"].as<float>();
    if(n["startAlphaMin"]) emitter->startAlphaMin = n["startAlphaMin"].as<float>();
    if(n["startAlphaMax"]) emitter->startAlphaMax = n["startAlphaMax"].as<float>();

    // rotation
    if(n["rotationMin"])      emitter->rotationMin = n["rotationMin"].as<float>();
    if(n["rotationMax"])      emitter->rotationMax = n["rotationMax"].as<float>();
    if(n["rotationSpeedMin"]) emitter->rotationSpeedMin = n["rotationSpeedMin"].as<float>();
    if(n["rotationSpeedMax"]) emitter->rotationSpeedMax = n["rotationSpeedMax"].as<float>();

    // velocity scale
    if(n["velocityScaleMin"]) emitter->velocityScaleMin = n["velocityScaleMin"].as<float>();
    if(n["velocityScaleMax"]) emitter->velocityScaleMax = n["velocityScaleMax"].as<float>();

    // over-lifetime
    if(n["sizeOverLife"])  emitter->sizeOverLife  = n["sizeOverLife"].as<float>();
    if(n["alphaOverLife"]) emitter->alphaOverLife = n["alphaOverLife"].as<float>();

    // bursts
    if(n["bursts"]){
        auto arr = n["bursts"];
        for(auto b : arr){
            float t = b["time"].as<float>();
            int   c = b["count"].as<int>();
            emitter->burstTimes.push_back(t);
            emitter->burstCounts.push_back(c);
        }
    }

    // color keys
    if(n["colorKeys"]){
        DeserializeColorKeys(n["colorKeys"], emitter->colorKeys);
    }

    // texture path
    if(n["texturePath"]){
        std::string texPath = n["texturePath"].as<std::string>();
        // loads that texture n apply to material
        auto* mat = emitter->GetMaterial();
        if(mat){
            volpe::Texture* tex = volpe::TextureManager().CreateTexture(texPath.c_str());
            if(tex){
                mat->SetTexture("u_texture", tex);
            }
        }
    }
}

//Color keys (bane of my existance)
YAML::Node SceneSerializer::SerializeColorKeys(const std::vector<ColorKey>& keys)
{
    YAML::Node arr;
    for(const auto& ck : keys){
        YAML::Node entry;
        entry["time"] = ck.time;
        {
            YAML::Node c;
            c.SetStyle(YAML::EmitterStyle::Flow);
            c.push_back(ck.color.r);
            c.push_back(ck.color.g);
            c.push_back(ck.color.b);
            c.push_back(ck.color.a);
            entry["color"] = c;
        }
        arr.push_back(entry);
    }
    return arr;
}

void SceneSerializer::DeserializeColorKeys(const YAML::Node& node, std::vector<ColorKey>& outKeys)
{
    outKeys.clear();
    for(auto ckNode : node)
    {
        ColorKey ck;
        ck.time = 0.0f;
        if(ckNode["time"])  ck.time = ckNode["time"].as<float>();
        if(ckNode["color"] && ckNode["color"].IsSequence() && ckNode["color"].size()==4){
            float r = ckNode["color"][0].as<float>();
            float g = ckNode["color"][1].as<float>();
            float b = ckNode["color"][2].as<float>();
            float a = ckNode["color"][3].as<float>();
            ck.color = glm::vec4(r,g,b,a);
        }
        outKeys.push_back(ck);
    }

    //Sort by time so dont dont look weird
    std::sort(outKeys.begin(), outKeys.end(), [](const ColorKey&a, const ColorKey&b){
        return a.time < b.time;
    });
}

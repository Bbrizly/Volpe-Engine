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
    else if(auto* fx = dynamic_cast<EffectNode*>(node))
    {
        n["Type"] = "Effect";
        n["combineDraws"] = fx->combineDraws; 
    }
    else {
        n["Type"] = "Node";
    }

    n["Transform"] = SerializeTransform(node);

    const auto& childList = node->getChildren();
    if (!childList.empty())
    {
        YAML::Node childArray;
        for (auto* childNode : childList)
        {
            YAML::Node childYAML = SerializeNode(childNode);

            if(childYAML)
                childArray.push_back(childYAML);
        }
        n["Children"] = childArray;
    }

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
    else if(type == "Effect")
    {
        auto* fx = new EffectNode(name);
        newNode = fx;

        if(nodeData["combineDraws"]) {
            fx->combineDraws = nodeData["combineDraws"].as<bool>();
        }
    }
    else{
        newNode = new Node(name);
    }

    /*if(newNode)
    {
        // 1) Always put it in the Scene’s node list so we can do culling, iteration, etc.
        scene.AddNode(newNode);

        // 2) Then add it as a child of 'parent' if 'parent' is not null
        if(parent)
        {
            // If the parent is an EffectNode and the new node is a ParticleNode, 
            // we can also call `effect->addEmitter(...)` 
            // but we *still* do scene.AddNode(...) above to ensure it's the same pointer in the scene.

            if(auto* parentEffect = dynamic_cast<EffectNode*>(parent))
            {
                // If it’s a ParticleNode => effect->addEmitter(...) calls addChild internally
                if(auto* childEmitter = dynamic_cast<ParticleNode*>(newNode))
                    parentEffect->addEmitter(childEmitter);
                else
                    parentEffect->addChild(newNode);
            }
            else
            {
                // Normal parent-child
                parent->addChild(newNode);
            }
        }

        // 3) Recurse if we have child YAML
        if(nodeData["Children"])
        {
            for(auto childData : nodeData["Children"])
            {
                DeserializeNode(childData, scene, newNode);
            }
        }
    }*/

    
    if(newNode){
        // Apply transform
        if(nodeData["Transform"]){
            DeserializeTransform(newNode, nodeData["Transform"]);
        }

        // Deal with children
        if(parent){
            if(auto* parentEffect = dynamic_cast<EffectNode*>(parent))
            {
                if(auto* childEmitter = dynamic_cast<ParticleNode*>(newNode))
                    parentEffect->addEmitter(childEmitter);
                
                else
                    parentEffect->addChild(newNode);
            }
            else 
            {
                parent->addChild(newNode);
            }
            
        } else {
            scene.AddNode(newNode);
        }

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
    n["duration"]           = emitter->duration;
    n["localSpace"]         = emitter->localSpace;
    n["glow"]              = emitter->glow;
    n["maxParticles"]       = emitter->maxParticles;
    n["shape"]              = (int) emitter->shape;
    
    n["emitterMode"]   = (emitter->emitterMode == EmitterMode::Continuous) ? 0 : 1;
    n["faceCamera"]    = emitter->faceCamera;
    //for face camera vectors:
    {
        YAML::Node look;
        look.SetStyle(YAML::EmitterStyle::Flow);
        look.push_back(emitter->customLookDir.x);
        look.push_back(emitter->customLookDir.y);
        look.push_back(emitter->customLookDir.z);
        n["customLookDir"] = look;
    }
    {
        YAML::Node up;
        up.SetStyle(YAML::EmitterStyle::Flow);
        up.push_back(emitter->customUpDir.x);
        up.push_back(emitter->customUpDir.y);
        up.push_back(emitter->customUpDir.z);
        n["customUpDir"] = up;
    }

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

    if(!emitter->getAffectors().empty()){
        n["affectors"] = SerializeAffectors(emitter->getAffectors());
    }

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
    if(n["duration"])           emitter->duration           = n["duration"].as<float>();
    if(n["localSpace"])         emitter->localSpace         = n["localSpace"].as<bool>();
    if(n["glow"])               emitter->glow               = n["glow"].as<bool>();
    if(n["maxParticles"])       emitter->maxParticles       = n["maxParticles"].as<int>();

    if(n["shape"]){
        int shapeVal = n["shape"].as<int>();
        emitter->shape = (EmitterShape) shapeVal;
    }

    if(n["emitterMode"]){
        int modeVal = n["emitterMode"].as<int>();
        emitter->emitterMode = (modeVal == 0) ? EmitterMode::Continuous : EmitterMode::Burst;
    }

    if(n["faceCamera"]){
        emitter->faceCamera = n["faceCamera"].as<bool>();
    }

    if(n["customLookDir"] && n["customLookDir"].IsSequence() && n["customLookDir"].size()==3)
    {
        emitter->customLookDir.x = n["customLookDir"][0].as<float>();
        emitter->customLookDir.y = n["customLookDir"][1].as<float>();
        emitter->customLookDir.z = n["customLookDir"][2].as<float>();
    }
    if(n["customUpDir"] && n["customUpDir"].IsSequence() && n["customUpDir"].size()==3)
    {
        emitter->customUpDir.x = n["customUpDir"][0].as<float>();
        emitter->customUpDir.y = n["customUpDir"][1].as<float>();
        emitter->customUpDir.z = n["customUpDir"][2].as<float>();
    }
    

    if(n["spawnPosition"] && n["spawnPosition"].IsSequence() && n["spawnPosition"].size()==3)
    {
        emitter->spawnPosition.x = n["spawnPosition"][0].as<float>();
        emitter->spawnPosition.y = n["spawnPosition"][1].as<float>();
        emitter->spawnPosition.z = n["spawnPosition"][2].as<float>();
    }
    if(n["spawnVelocity"] && n["spawnVelocity"].IsSequence() && n["spawnVelocity"].size()==3)
    {
        emitter->spawnVelocity.x = n["spawnVelocity"][0].as<float>();
        emitter->spawnVelocity.y = n["spawnVelocity"][1].as<float>();
        emitter->spawnVelocity.z = n["spawnVelocity"][2].as<float>();
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
    //affectors
    if(n["affectors"]){
        DeserializeAffectors(n["affectors"], emitter);
    }

    // texture path
    if(n["texturePath"]){
        std::string texPath = n["texturePath"].as<std::string>();
        // loads that texturea nd apply to material
        auto* mat = emitter->GetMaterial();
        if(mat){
            volpe::Texture* tex = volpe::TextureManager().CreateTexture(texPath.c_str());
            if(tex){
                mat->SetTexture("u_texture", tex);
            }
        }
    }
    emitter->Stop();
    emitter->Play();
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

void SceneSerializer::DeserializeParticleNodeFromFile(const std::string& filePath, ParticleNode* outEmitter)
{
    // We assume 'outEmitter' is already allocated.
    if(!outEmitter) return;

    YAML::Node root;
    try {
        root= YAML::LoadFile(filePath);
    }
    catch(const YAML::Exception& e){
        std::cerr<<"[SceneSerializer] Can't load emitter file "<<filePath<<" => "<< e.what()<<"\n";
        return;
    }

    auto nEm = root["emitter"];
    if(!nEm){
        std::cerr<<"[SceneSerializer] No 'emitter' node in "<<filePath<<"\n";
        return;
    }

    // name
    if(nEm["name"]){
        std::string nm= nEm["name"].as<std::string>();
        outEmitter->setName(nm);
    }
    // mode
    // if(nEm["mode"]){
    //     std::string m= nEm["mode"].as<std::string>();
    //     if(m=="continuous") outEmitter->mode= EmitterMode::Continuous;
    //     else if(m=="burst") outEmitter->mode= EmitterMode::Burst;
    // }
    // duration
    // if(nEm["duration"]){
    //     outEmitter->duration= nEm["duration"].as<float>();
    // }
    // emissionRate
    if(nEm["emissionRate"]){
        outEmitter->emissionRate= nEm["emissionRate"].as<float>();
    }
    // maxParticles
    if(nEm["max_particles"]){
        outEmitter->maxParticles= nEm["max_particles"].as<int>();
    }
    // shape
    if(nEm["shape"]){
        std::string shp= nEm["shape"].as<std::string>();
        if(shp=="Point") outEmitter->shape= EmitterShape::Point;
        else if(shp=="Sphere") outEmitter->shape= EmitterShape::Sphere;
        else if(shp=="Cone")   outEmitter->shape= EmitterShape::Cone;
        else if(shp=="Box")    outEmitter->shape= EmitterShape::Box;
        else if(shp=="Mesh")   outEmitter->shape= EmitterShape::Mesh;
    }
    // spawnPosition
    if(nEm["spawnPosition"] && nEm["spawnPosition"].IsSequence() && nEm["spawnPosition"].size()==3){
        outEmitter->spawnPosition.x= nEm["spawnPosition"][0].as<float>();
        outEmitter->spawnPosition.y= nEm["spawnPosition"][1].as<float>();
        outEmitter->spawnPosition.z= nEm["spawnPosition"][2].as<float>();
    }
    // spawnVelocity
    if(nEm["spawnVelocity"] && nEm["spawnVelocity"].IsSequence() && nEm["spawnVelocity"].size()==3){
        outEmitter->spawnVelocity.x= nEm["spawnVelocity"][0].as<float>();
        outEmitter->spawnVelocity.y= nEm["spawnVelocity"][1].as<float>();
        outEmitter->spawnVelocity.z= nEm["spawnVelocity"][2].as<float>();
    }
    // globalAcceleration
    if(nEm["globalAcceleration"] && nEm["globalAcceleration"].IsSequence() && nEm["globalAcceleration"].size()==3){
        // outEmitter->globalAcceleration.x= nEm["globalAcceleration"][0].as<float>();
        // outEmitter->globalAcceleration.y= nEm["globalAcceleration"][1].as<float>();
        // outEmitter->globalAcceleration.z= nEm["globalAcceleration"][2].as<float>();
    }
    // lifetimeMin/Max
    if(nEm["lifetimeMin"]) outEmitter->lifetimeMin= nEm["lifetimeMin"].as<float>();
    if(nEm["lifetimeMax"]) outEmitter->lifetimeMax= nEm["lifetimeMax"].as<float>();
    // sizes
    if(nEm["startSizeMin"]) outEmitter->startSizeMin= nEm["startSizeMin"].as<float>();
    if(nEm["startSizeMax"]) outEmitter->startSizeMax= nEm["startSizeMax"].as<float>();
    // alpha
    if(nEm["startAlphaMin"]) outEmitter->startAlphaMin= nEm["startAlphaMin"].as<float>();
    if(nEm["startAlphaMax"]) outEmitter->startAlphaMax= nEm["startAlphaMax"].as<float>();
    // rotation
    if(nEm["rotationMin"])      outEmitter->rotationMin= nEm["rotationMin"].as<float>();
    if(nEm["rotationMax"])      outEmitter->rotationMax= nEm["rotationMax"].as<float>();
    if(nEm["rotationSpeedMin"]) outEmitter->rotationSpeedMin= nEm["rotationSpeedMin"].as<float>();
    if(nEm["rotationSpeedMax"]) outEmitter->rotationSpeedMax= nEm["rotationSpeedMax"].as<float>();
    // velocity scale
    if(nEm["velocityScaleMin"]) outEmitter->velocityScaleMin= nEm["velocityScaleMin"].as<float>();
    if(nEm["velocityScaleMax"]) outEmitter->velocityScaleMax= nEm["velocityScaleMax"].as<float>();

    // colorKeys
    if(nEm["colorKeys"] && nEm["colorKeys"].IsSequence()){
        outEmitter->colorKeys.clear();
        for(auto ckNode : nEm["colorKeys"]){
            ColorKey ck;
            ck.time=0.f;
            if(ckNode["time"])  ck.time= ckNode["time"].as<float>();
            if(ckNode["color"] && ckNode["color"].IsSequence() && ckNode["color"].size()==4){
                ck.color.r= ckNode["color"][0].as<float>();
                ck.color.g= ckNode["color"][1].as<float>();
                ck.color.b= ckNode["color"][2].as<float>();
                ck.color.a= ckNode["color"][3].as<float>();
            }
            outEmitter->colorKeys.push_back(ck);
        }
        // sort them
        std::sort(outEmitter->colorKeys.begin(), outEmitter->colorKeys.end(), 
                  [](auto&a, auto&b){return a.time<b.time;});
    }

    // bursts
    if(nEm["burstTimes"] && nEm["burstTimes"].IsSequence())
    {
        outEmitter->burstTimes.clear();
        outEmitter->burstCounts.clear();
        for(auto bN : nEm["burstTimes"])
        {
            // each element might be [time, count], or a map with time: X, count: Y
            // If your emitter YAML format is simpler, parse accordingly.
            if(bN.IsSequence() && bN.size()==2){
                float t= bN[0].as<float>();
                int   c= bN[1].as<int>();
                outEmitter->burstTimes.push_back(t);
                outEmitter->burstCounts.push_back(c);
            }
        }
    }

    // parse localSpace
    if(nEm["localSpace"]) outEmitter->localSpace= nEm["localSpace"].as<bool>();

    if(nEm["affectors"] && nEm["affectors"].IsSequence()){
        for(auto affN : nEm["affectors"]){
            if(!affN["type"]) continue;
            std::string atype= affN["type"].as<std::string>();

            if(atype=="AddVelocity"){
                glm::vec3 vel(0,-9.81f,0);
                if(affN["velocity"] && affN["velocity"].IsSequence() && affN["velocity"].size()==3){
                    vel.x= affN["velocity"][0].as<float>();
                    vel.y= affN["velocity"][1].as<float>();
                    vel.z= affN["velocity"][2].as<float>();
                }
                AccelerationAffector* av= new AccelerationAffector(vel);
                outEmitter->AddAffector(av);
            }
            else if(atype=="ScaleOverLife"){
                float ss=1.f, ee=2.f;
                if(affN["startScale"]) ss= affN["startScale"].as<float>();
                if(affN["endScale"])   ee= affN["endScale"].as<float>();
                ScaleOverLifeAffector* sc= new ScaleOverLifeAffector(ss,ee);
                outEmitter->AddAffector(sc);
            }
            else if(atype=="FadeOverLife"){
                float sa=1.f, ea=0.f;
                if(affN["startAlpha"]) sa= affN["startAlpha"].as<float>();
                if(affN["endAlpha"])   ea= affN["endAlpha"].as<float>();
                FadeOverLifeAffector* fa= new FadeOverLifeAffector(sa,ea);
                outEmitter->AddAffector(fa);
            }
            // etc for any custom affector
        }
    }

    if(nEm["texturePath"]){
        std::string tp= nEm["texturePath"].as<std::string>();
        if(!tp.empty()){
            auto* mat= outEmitter->GetMaterial();
            if(mat){
                volpe::Texture* t= volpe::TextureManager().CreateTexture(tp.c_str());
                if(t){
                    mat->SetTexture("u_texture", t);
                }
            }
        }
    }

    outEmitter->Stop();
    outEmitter->Play();
}

void SceneSerializer::SaveEffectNode(EffectNode* effect, const std::string& filePath)
{
    if(!effect) return;

    YAML::Node root;
    YAML::Node eff;
    eff["name"] = effect->getName();
    eff["combineDraws"] = effect->combineDraws;

    YAML::Node emArr(YAML::NodeType::Sequence);

    for (auto* c : effect->getChildren())
    {
        auto* emitter = dynamic_cast<ParticleNode*>(c);
        if(!emitter) continue;
        YAML::Node oneEm;
        oneEm["EmitterName"] = emitter->getName();

        oneEm["Transform"] = SerializeTransform(emitter);
        oneEm["ParticleData"] = SerializeParticleNode(emitter);

        emArr.push_back(oneEm);
    }
    eff["emitters"] = emArr;

    root["effect"] = eff;

    std::string finalPath = filePath;
    if(finalPath.find(".yaml")==std::string::npos){
        finalPath += ".effect"; //.EFFECT.YAML FOR EFFECTSSS
        finalPath += ".yaml";
    }
    std::ofstream out(finalPath);
    if(!out.is_open()){
        std::cerr<<"Cannot write effect to "<<finalPath<<"\n";
        return;
    }
    out << root;
    out.close();

    std::cout<<"[SceneSerializer] Saved Effect "<<effect->getName()<<" => "<<finalPath<<"\n";
}
EffectNode* SceneSerializer::LoadEffectNode(Scene& scene, const std::string& filePath)
{
    YAML::Node root;
    try {
        root = YAML::LoadFile(filePath);
    } catch(const YAML::Exception& e){
        std::cerr<<"[SceneSerializer::LoadEffectNode] Error: "<<e.what()<<"\n";
        return nullptr;
    }
    auto nEff = root["effect"];
    if(!nEff){
        std::cerr<<"No 'effect' in file: "<<filePath<<"\n";
        return nullptr;
    }

    EffectNode* fx = new EffectNode("UnnamedEffect");
    scene.AddNode(fx);

    if(nEff["name"]){
        fx->setName(nEff["name"].as<std::string>());
    }
    if(nEff["combineDraws"]){
        fx->combineDraws = nEff["combineDraws"].as<bool>();
    }

    if(nEff["emitters"] && nEff["emitters"].IsSequence())
    {
        for(auto e : nEff["emitters"])
        {
            std::string eName = "Emitter";
            if(e["EmitterName"])
                eName = e["EmitterName"].as<std::string>();
            ParticleNode* emitter = new ParticleNode(eName);

            if(e["Transform"])
                DeserializeTransform(emitter, e["Transform"]);
            if(e["ParticleData"])
                DeserializeParticleNode(e["ParticleData"], emitter);

            fx->addEmitter(emitter);
            scene.AddNode(emitter);
        }
    }

    std::cout<<"[SceneSerializer] Loaded Effect: "<<fx->getName()<<" from "<<filePath<<"\n";
    return fx;
}

void SceneSerializer::SaveEmitter(ParticleNode* emitter, const std::string& filePath)
{
    if(!emitter) return;

    YAML::Node root;
    YAML::Node em;

    em["name"] = emitter->getName();
    em["transform"] = SerializeTransform(emitter);

    em["particle"]  = SerializeParticleNode(emitter);

    root["Emitter"] = em;

    std::string final = filePath;
    if(final.find(".yaml")==std::string::npos){

        final += ".emitter"; //EMITTER WILL BE .EMITTER.YAML
        final += ".yaml";
    }
    std::ofstream ofs(final);
    if(!ofs.is_open()){
        std::cerr<<"[SceneSerializer] Cannot write Emitter to "<<final<<"\n";
        return;
    }
    ofs << root;
    ofs.close();

    std::cout<<"[SceneSerializer] Saved Emitter => "<<final<<"\n";
}
ParticleNode* SceneSerializer::LoadEmitter(const std::string& filePath) //DECIDE IF SHOULD HARDCODEPATHS OR NOT
{
    YAML::Node root;
    try {
        root = YAML::LoadFile(filePath);
    }
    catch(const YAML::Exception& e){
        std::cerr<<"[SceneSerializer] Load Emitter error: "<<e.what()<<"\n";
        return nullptr;
    }
    auto nEm = root["Emitter"];
    if(!nEm){
        std::cerr<<"[SceneSerializer] No 'Emitter' in "<<filePath<<"\n";
        return nullptr;
    }
    ParticleNode* emitter = new ParticleNode("Emitter");
    if(nEm["name"]){
        emitter->setName(nEm["name"].as<std::string>());
    }
    // transform
    if(nEm["transform"]){
        DeserializeTransform(emitter, nEm["transform"]);
    }
    // particle
    if(nEm["particle"]){
        DeserializeParticleNode(nEm["particle"], emitter);
    }

    std::cout<<"[SceneSerializer] Loaded Emitter: "<<emitter->getName()<<" from "<<filePath<<"\n";
    return emitter;
}

YAML::Node SceneSerializer::SerializeAffectors(const std::vector<Affector*>& affs)
{
    YAML::Node arr(YAML::NodeType::Sequence);

    for (auto* A : affs)
    {
        if(!A) continue;
        YAML::Node one;
        
        if (auto* acc = dynamic_cast<AccelerationAffector*>(A))
        {
            one["type"] = "Acceleration";
            
            YAML::Node vel(YAML::NodeType::Sequence);
            vel.push_back(acc->velocityToAdd.x);
            vel.push_back(acc->velocityToAdd.y);
            vel.push_back(acc->velocityToAdd.z);
            one["velocity"] = vel;
        }
        else if (auto* fade = dynamic_cast<FadeOverLifeAffector*>(A))
        {
            one["type"] = "FadeOverLife";
            one["startAlpha"] = fade->startAlpha;
            one["endAlpha"]   = fade->endAlpha;
        }
        else if (auto* scale = dynamic_cast<ScaleOverLifeAffector*>(A))
        {
            one["type"] = "ScaleOverLife";
            one["startScale"] = scale->startScale;
            one["endScale"]   = scale->endScale;
        }
        else if (auto* toward = dynamic_cast<TowardsPointAffector*>(A))
        {
            one["type"] = "TowardsPoint";
            YAML::Node tgt(YAML::NodeType::Sequence);
            tgt.push_back(toward->target.x);
            tgt.push_back(toward->target.y);
            tgt.push_back(toward->target.z);
            one["target"]   = tgt;
            one["strength"] = toward->strength;
        }
        else if (auto* away = dynamic_cast<AwayFromPointAffector*>(A))
        {
            one["type"] = "AwayFromPoint";
            YAML::Node c(YAML::NodeType::Sequence);
            c.push_back(away->center.x);
            c.push_back(away->center.y);
            c.push_back(away->center.z);
            one["center"]   = c;
            one["strength"] = away->strength;
        }
        
        arr.push_back(one);
    }

    return arr;
}
void SceneSerializer::DeserializeAffectors(const YAML::Node& n, ParticleNode* emitter)
{
    if(!n.IsSequence()) return;

    for (auto affN : n)
    {
        if(!affN["type"]) continue;
        std::string t = affN["type"].as<std::string>();

        if(t=="Acceleration")
        {
            glm::vec3 vel(0.f, -9.81f, 0.f);
            if(affN["velocity"] && affN["velocity"].IsSequence() && affN["velocity"].size()==3)
            {
                vel.x = affN["velocity"][0].as<float>();
                vel.y = affN["velocity"][1].as<float>();
                vel.z = affN["velocity"][2].as<float>();
            }
            emitter->AddAffector(new AccelerationAffector(vel));
        }
        else if(t=="FadeOverLife")
        {
            float sA=1.f, eA=0.f;
            if(affN["startAlpha"]) sA= affN["startAlpha"].as<float>();
            if(affN["endAlpha"])   eA= affN["endAlpha"].as<float>();
            emitter->AddAffector(new FadeOverLifeAffector(sA,eA));
        }
        else if(t=="ScaleOverLife")
        {
            float sS=1.f, eS=2.f;
            if(affN["startScale"]) sS= affN["startScale"].as<float>();
            if(affN["endScale"])   eS= affN["endScale"].as<float>();
            emitter->AddAffector(new ScaleOverLifeAffector(sS,eS));
        }
        else if(t=="TowardsPoint")
        {
            glm::vec3 tgt(0,5,0);
            if(affN["target"] && affN["target"].IsSequence() && affN["target"].size()==3)
            {
                tgt.x= affN["target"][0].as<float>();
                tgt.y= affN["target"][1].as<float>();
                tgt.z= affN["target"][2].as<float>();
            }
            float str=1.f;
            if(affN["strength"]) str= affN["strength"].as<float>();
            emitter->AddAffector(new TowardsPointAffector(tgt, str));
        }
        else if(t=="AwayFromPoint")
        {
            glm::vec3 c(0,0,0);
            if(affN["center"] && affN["center"].IsSequence() && affN["center"].size()==3)
            {
                c.x= affN["center"][0].as<float>();
                c.y= affN["center"][1].as<float>();
                c.z= affN["center"][2].as<float>();
            }
            float str=1.f;
            if(affN["strength"]) str= affN["strength"].as<float>();
            emitter->AddAffector(new AwayFromPointAffector(c, str));
        }
    }
}
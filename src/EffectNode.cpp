#include "EffectNode.h"
#include <yaml-cpp/yaml.h>
#include <iostream>
#include "SceneSerializer.h"    // We'll rely on a new function to load single emitters

EffectNode::EffectNode(const std::string& name)
: Node(name)
, combineDraws(false)
{
}

EffectNode::~EffectNode()
{
    SetReactToLight(false);
    // base Node::~Node() frees children (your ParticleNodes).
}

void EffectNode::LoadFromYAML(const std::string& filePath)
{
    /* 
       Example of file.effect.yaml
       Give ability to use button for loadingeffects
       ONLY FOR EFFECTS SO: "name.effect.yaml" CUZ VSC CRASHED LAST TIEM I LOADED A SCENE
         effect:
           name: "explosion"
           emitters:
             - file: "sparks.emitter.yaml"
               offset: [0,0,1]
             - file: "smoke.emitter.yaml"
               offset: [0,4,0]
         ...
    */
    YAML::Node root;
    try {
        root = YAML::LoadFile(filePath);
    }
    catch(const YAML::Exception& e){
        std::cerr<<"[Effect] YAML error: "<< e.what()<<"\n";
        return;
    }

    auto nEff = root["effect"];
    if(!nEff){
        std::cerr<<"[Effect] No 'effect' node in "<<filePath<<"\n";
        return;
    }
    // name
    if(nEff["name"]){
        std::string nm = nEff["name"].as<std::string>();
        setName(nm);
    }

    // emitters
    if(nEff["emitters"] && nEff["emitters"].IsSequence())
    {
        for(auto eNode : nEff["emitters"])
        {
            if(!eNode["file"]){
                std::cerr<<"[Effect] Emitter missing 'file' attribute\n";
                continue;
            }
            std::string emitterFile = eNode["file"].as<std::string>();

            ParticleNode* emitter = new ParticleNode("Emitter");
            
            SceneSerializer::DeserializeParticleNodeFromFile(emitterFile, emitter);

            // If there's an offset
            if(eNode["offset"] && eNode["offset"].IsSequence() && eNode["offset"].size()==3)
            {
                float ox = eNode["offset"][0].as<float>();
                float oy = eNode["offset"][1].as<float>();
                float oz = eNode["offset"][2].as<float>();
                glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(ox,oy,oz));
                emitter->setTransform(T);
            }
            addEmitter(emitter);
        }
    }
}

void EffectNode::addEmitter(ParticleNode* emitter)
{
    if(!emitter) return;
    addChild(emitter);
}

void EffectNode::Play()
{
    for(auto* c : m_children){
        auto* emitter = dynamic_cast<ParticleNode*>(c);
        if(emitter){
            emitter->Play();
        }
    }
}

void EffectNode::Stop()
{
    for(auto* c : m_children){
        auto* emitter = dynamic_cast<ParticleNode*>(c);
        if(emitter){
            emitter->Stop();
        }
    }
}

void EffectNode::Pause()
{
    for(auto* c : m_children){
        auto* emitter = dynamic_cast<ParticleNode*>(c);
        if(emitter){
            emitter->Pause();
        }
    }
}

void EffectNode::End()
{
    for(auto* c : m_children){
        auto* emitter = dynamic_cast<ParticleNode*>(c);
        if(emitter){
            emitter->End();
        }
    }
}

void EffectNode::Restart()
{
    for(auto* c : m_children){
        auto* emitter = dynamic_cast<ParticleNode*>(c);
        if(emitter){
            emitter->Restart();
        }
    }
}

void EffectNode::update(float dt)
{
    // Optionally do logic for the effect
    Node::update(dt); // calls update on each child
}

void EffectNode::draw(const glm::mat4& proj, const glm::mat4& view)
{
    if(!combineDraws){
        Node::draw(proj, view);
        return;
    }

    Node::draw(proj, view);
}

#pragma once
#include "Node.h"
#include "ParticleNode.h"
#include <string>

/**
 * The Effect is a Node containing multiple ParticleNode (emitters).
 * 
 * Example YAML (effect.yaml):
 * effect:
 *   name: "explosion"
 *   emitters:
 *     - file: "sparks.emitter.yaml"
 *       offset: [0,0,1]
 *     - file: "debris.emitter.yaml"
 *       offset: [0,0,0]
 *     - file: "fire.emitter.yaml"
 *       offset: [0,0,0]
 *     - file: "smoke.emitter.yaml"
 *       offset: [0,4,0]
 */
class EffectNode : public Node
{
public:
    EffectNode(const std::string& name);
    virtual ~EffectNode();

    // You can load this entire Effect from a single YAML file
    void LoadFromYAML(const std::string& filePath);

    // Add a sub–emitter (ParticleNode)
    void addEmitter(ParticleNode* emitter);

    // High-level control
    void Play();
    void Stop();
    void Pause();
    void Restart();

    // Overridden from Node
    virtual void update(float dt) override;
    virtual void draw(const glm::mat4& proj, const glm::mat4& view) override;

    // If you want to combine all sub–emitters in one GPU draw:
    bool combineDraws = false;
};

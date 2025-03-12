#pragma once
#include "Node.h"
#include "ParticleNode.h"
#include <string>

class EffectNode : public Node
{
public:
    EffectNode(const std::string& name);
    virtual ~EffectNode();

    void LoadFromYAML(const std::string& filePath);

    void addEmitter(ParticleNode* emitter);

    void Play();
    void Stop();
    void Pause();
    void End();
    void Restart();

    virtual void update(float dt) override;
    virtual void draw(const glm::mat4& proj, const glm::mat4& view) override;

    bool combineDraws = false;
};

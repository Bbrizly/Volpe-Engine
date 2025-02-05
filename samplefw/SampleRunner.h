#pragma once
#include <string>
#include <vector>

class Sample;

class SampleRunner
{
public:
    SampleRunner();
    ~SampleRunner();

    void update(float dt);
    void render(int width, int height);

    void addSample(Sample* pSample);
    void switchToSampleNumber(int num);
    void switchToSampleWithName(const std::string& name);
    void nextSample();

private:
    void _sampleChanged();

    std::vector<Sample*> m_samples;
    Sample* m_pCurrSample;
    int m_currSampleNumber;
};
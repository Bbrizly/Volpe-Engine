#include "SampleRunner.h"
#include "Sample.h"

SampleRunner::SampleRunner()
    : m_currSampleNumber(-1), m_pCurrSample(nullptr)
{
}

SampleRunner::~SampleRunner()
{
    for(auto it = m_samples.begin(); it != m_samples.end(); ++it) 
    {
        auto sample = *it;
        delete sample;
    }
}

void SampleRunner::update(float dt)
{
    m_pCurrSample->update(dt);
}

void SampleRunner::render(int width, int height)
{
    m_pCurrSample->render(width, height);
}

void SampleRunner::addSample(Sample* pSample)
{
    for(auto it = m_samples.begin(); it != m_samples.end(); ++it) 
    {
        auto sample = *it;
        if(sample == pSample)
            return; // already got it
    }

    m_samples.push_back(pSample);

    if(!m_pCurrSample)
    {
        m_pCurrSample = pSample;
        m_currSampleNumber = 0;
        _sampleChanged();
    }
}

void SampleRunner::switchToSampleNumber(int num)
{
    if(num < m_samples.size())
    {
        m_pCurrSample = m_samples[num];
        m_currSampleNumber = num;
        _sampleChanged();
    }
}

void SampleRunner::switchToSampleWithName(const std::string& name)
{
    for(int i = 0; i < m_samples.size(); ++i)
    {
        if(m_samples[i]->getName() == name)
        {
            m_pCurrSample = m_samples[i];
            m_currSampleNumber = i;
            _sampleChanged();
            break;
        }
    }
}

void SampleRunner::nextSample()
{
    m_currSampleNumber = (m_currSampleNumber + 1) % m_samples.size();
    m_pCurrSample = m_samples[m_currSampleNumber];
    _sampleChanged();
}

void SampleRunner::_sampleChanged() 
{
    printf("Switched to sample %s\n", m_pCurrSample->getName().c_str());
    m_pCurrSample->init();
}
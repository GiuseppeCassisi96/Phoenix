#pragma once
#include "AnimationManager.h"
#include <unordered_map>
#include "EngineVars.h"
namespace Minerva
{
    
    class EngineStartup
    {
    public:
        std::unordered_map<std::string, SampleType> samplesTest
        {
            {"0", SampleType()},
            {"1", SampleType()},
            {"2", SampleType()},
            {"3", SampleType()},
        };
        std::vector<Animation> animations;
        Animator animator;
        SampleType choosenSample;
        float avgLOD = 0.0f;
        void RunEngine();
    private:
        
        void Start();
        void Loop();
    };
    
    
}
#pragma once
#include "AnimationManager.h"
#include "string.h"
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
            {"1", SampleType()}
            
        };
        Animation anim;
        Animator animator;
        void RunEngine();
    private:
        
        void Start();
        void Loop();
    };
    
    
}
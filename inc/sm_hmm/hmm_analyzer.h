//
// Created by hobbang5 on 2016-03-20.
//

#ifndef __HMM_ANALYZER_H__
#define __HMM_ANALYZER_H__

#include <vector>

#include "glm/glm.hpp"

// analyzer interface
class Hmm_Analyzer
{

public:
    Hmm_Analyzer() { };

    virtual ~Hmm_Analyzer() { };

    // pure virtual functions
    virtual bool get_Observation(glm::vec3 curr_observation, std::vector<float>& observation) = 0;
    virtual bool is_End() = 0;
    virtual void reset() = 0;

};

#endif //__HMM_ANALYZER_H__

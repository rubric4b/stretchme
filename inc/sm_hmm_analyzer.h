//
// Created by hobbang5 on 2016-03-20.
//

#ifndef STRETCHME_SM_HMM_ANALYZER_H
#define STRETCHME_SM_HMM_ANALYZER_H

#include <vector>

#include "glm/glm.hpp"

#define SM_DEFAULT_TS_DIMENTION 7
#define ARM_UP_THRESHOLD 7.8


class Sm_Hmm_Analyzer
{

public:
    Sm_Hmm_Analyzer();
    ~Sm_Hmm_Analyzer();

    bool get_Observation(glm::vec3 curr_observation, std::vector<float>& observation);
    bool get_Observation(std::vector<float>& curr_observation, std::vector<float>& observation);

    bool get_Stay();
    double get_Treshold();
    void reset();


private:
    bool is_prev;
    bool is_init_move; // for the first moving
    bool is_stay;
    double threshold;
    glm::vec3 prev_obseravtion;
    glm::vec3 diff;
    int diff_cnt;

};

#endif //STRETCHME_SM_HMM_ANALYZER_H

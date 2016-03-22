//
// Created by hobbang5 on 2016-03-20.
//

#include <logger.h>

#include "sm_hmm_manager.h"
#include "sm_hmm_analyzer.h"

using namespace glm;

Sm_Hmm_Analyzer::Sm_Hmm_Analyzer() :
    is_prev(false),
    is_init_move(false),
    is_stay(false),
    threshold(ARM_UP_THRESHOLD),
    prev_obseravtion(0),
    diff(0),
    diff_cnt(0)
{

}

Sm_Hmm_Analyzer::~Sm_Hmm_Analyzer()
{

}

void Sm_Hmm_Analyzer::reset() {
    is_prev = false;
    is_init_move = false;
    is_stay = false;
    prev_obseravtion = vec3(0);
    diff = vec3(0);
    diff_cnt = 0;

}

bool Sm_Hmm_Analyzer::get_Observation(std::vector<float> &curr_observation, std::vector<float> &observation) {
    if(curr_observation.size() != 3) {
        ERR("current observation size is not the same with three!\n");
        return false;
    }

    vec3 curr(curr_observation[0], curr_observation[1], curr_observation[2]);

    return get_Observation(curr, observation);
}

bool Sm_Hmm_Analyzer::get_Observation(glm::vec3 curr_observation, std::vector<float> &observation) {

    if(observation.size() != SM_DEFAULT_TS_DIMENTION) {
        ERR("observation size is not the same with SM_DEFAULT_TS_DIMENTION!\n");
        return false;
    }

    if(!is_prev) {
        prev_obseravtion = curr_observation;
        is_prev = true;
        ERR("get_Observation:: no prev!\n");
        return false;
    }

    diff = (curr_observation - prev_obseravtion);

    if(length(diff) < 0.5 ) {
//        DBG("get_Observation:: %f length not enough!\n", length(diff));
        diff_cnt++;
//        return false;
    } else {
        diff_cnt = 0;
        is_init_move = true;
        is_stay = false;
    }

    if(!is_init_move) {
        return false;
    }

    if(diff_cnt > 100) {
        is_stay = true;
        return false;
    }

    diff = normalize(diff);
    observation[0] = curr_observation.x;
    observation[1] = curr_observation.y;
    observation[2] = curr_observation.z;
    observation[3] = diff.x;
    observation[4] = diff.y;
    observation[5] = diff.z;
    observation[6] = abs((length(curr_observation)) - 9.8);

    prev_obseravtion = curr_observation;


    return true;

/*
    if(observation.size() != SM_DEFAULT_TS_DIMENTION) {
        ERR("observation size is not the same with SM_DEFAULT_TS_DIMENTION!\n");
        return false;
    }

    if(!is_prev) {
        prev_obseravtion = curr_observation;
        is_prev = true;
        ERR("get_Observation:: no init!\n");
        return false;
    }

    vec3 diff = curr_observation - prev_obseravtion;

    if(length(diff) < 0.3 ) {
//        DBG("get_Observation:: %f length not enough!\n", length(diff));
        return false;
    }

    diff = normalize(diff);
    observation[0] = diff.x;
    observation[1] = diff.y;
    observation[2] = diff.z;
    observation[3] = abs(length(curr_observation));

//    DBG("prev %5f %5f %5f\n", prev_obseravtion.x, prev_obseravtion.y, prev_obseravtion.z);
//    DBG("curr %5f %5f %5f\n", curr_observation.x, curr_observation.y, curr_observation.z);
    prev_obseravtion = curr_observation;


    return true;
    */

}



bool Sm_Hmm_Analyzer::get_Stay() {
    return is_stay;
}

double Sm_Hmm_Analyzer::get_Treshold() {
    return threshold;
}

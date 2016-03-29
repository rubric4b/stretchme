//
// Created by hobbang5 on 2016-03-23.
//


#include "sm_hmm/hmm_analyzer_armup.h"
#include "sm_hmm/hmm_model_armup.h"
#include "logger.h"

using namespace glm;

HA_ArmUp::HA_ArmUp() :
        is_prev(false),
        is_init_move(false),
        is_stay(false),
        prev_obseravtion(0),
        diff(0),
        diff_cnt(0)
{

}

HA_ArmUp::~HA_ArmUp()
{

}

void HA_ArmUp::reset() {

    is_prev = false;
    is_init_move = false;
    is_stay = false;
    prev_obseravtion = vec3(0);
    diff = vec3(0);
    diff_cnt = 0;

}

bool HA_ArmUp::get_Observation(const vec3 curr_observation, std::vector<float> &observation) {

    if(observation.size() != Hmm_ArmUp::ARM_UP_TS_DIMENSION) {
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

    if(length(diff) < 0.3 ) {
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

}

bool HA_ArmUp::is_End() {

    return is_stay;
}

bool HA_ArmUp::get_Observation(std::vector<float> &curr_observation, std::vector<float> &observation) {
    if(curr_observation.size() != 3) {
        ERR("current observation size is not the same with three!\n");
        return false;
    }

    vec3 curr(curr_observation[0], curr_observation[1], curr_observation[2]);

    return get_Observation(curr, observation);
}

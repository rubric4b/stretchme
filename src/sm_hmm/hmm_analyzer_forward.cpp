//
// Created by hobbang5 on 2016-03-23.
//


#include "sm_hmm/hmm_analyzer_forward.h"
#include "sm_hmm/hmm_model_forward.h"
#include "logger.h"

#define MOVING_THRESHOLD 0.1
#define MOVING_THRESHOLD_CNT 20

using namespace glm;

HA_Forward::HA_Forward() :
        is_prev(false),
        is_init_move(false),
        is_stay(false),
        m_prev_obseravtion(0),
        m_diff(0),
        m_nondiff_cnt(0),
        m_diff_cnt(0)
{

}

HA_Forward::~HA_Forward()
{

}

void HA_Forward::reset() {
    DBG("HA_Forward::reset()\n");
    is_prev = false;
    is_init_move = false;
    is_stay = false;
    m_prev_obseravtion = vec3(0);
    m_diff = vec3(0);
    m_nondiff_cnt = 0;
    m_diff_cnt = 0;

}

bool HA_Forward::get_Observation(const vec3 curr_observation, std::vector<float> &observation) {

    if(observation.size() != Hmm_Forward::FORWARD_TS_DIMENSION) {
        ERR("observation size is not the same with Hmm_Forward::FORWARD_TS_DIMENSION!\n");
        return false;
    }

    if(!analyze(curr_observation))
        return false;

    observation[0] = m_prev_obseravtion.x;
    observation[1] = m_prev_obseravtion.y;
    observation[2] = m_prev_obseravtion.z;
    observation[3] = m_diff.x;
    observation[4] = m_diff.y;
    observation[5] = m_diff.z;
    observation[6] = abs((length(m_prev_obseravtion)) - 9.8);

    return true;
}

bool HA_Forward::is_End() {
//    DBG("is_End() = stay, init_move(%d, %d)\n", is_stay, is_init_move);
    return (is_stay && is_init_move);
}

bool HA_Forward::get_Observation(std::vector<float> &curr_observation, std::vector<float> &observation) {
    if(curr_observation.size() != 3) {
        ERR("current observation size is not the same with three!\n");
        return false;
    }

    vec3 curr(curr_observation[0], curr_observation[1], curr_observation[2]);

    return get_Observation(curr, observation);
}

bool HA_Forward::analyze(const glm::vec3 curr_observation) {
    if(!is_prev) {
        m_prev_obseravtion = curr_observation;
        is_prev = true;
        ERR("get_Observation:: no prev!\n");
        return false;
    }

    m_diff = (curr_observation - m_prev_obseravtion);
    double len = length(m_diff);

    if (!is_init_move) {

        if (len > MOVING_THRESHOLD) { //determine motion start
            m_diff_cnt++;
            m_prev_obseravtion = curr_observation;

            if(m_diff_cnt > MOVING_THRESHOLD_CNT) {
                DBG("is_init_move = true\n");
                is_init_move = true;
            }
        } else {
            m_diff_cnt = 0;
        }

    } else { // motion start

        if(!is_stay) {
            if (len < MOVING_THRESHOLD) { //determine motion end
                m_nondiff_cnt++;

                if(m_nondiff_cnt > MOVING_THRESHOLD_CNT * 3) {
                    DBG("is_stay_true\n");
                    is_stay = true;
                }
            }else{
                m_nondiff_cnt = 0;
            }

            m_diff = (len == 0) ? vec3(0) : normalize(m_diff);
            m_prev_obseravtion = curr_observation;

            return true;
        }

    }

    return false;

}



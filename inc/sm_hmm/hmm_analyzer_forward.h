//
// Created by hobbang5 on 2016-03-23.
//

#ifndef __HMM_ANALYZER_FORWARD_H__
#define __HMM_ANALYZER_FORWARD_H__

#include "hmm_analyzer.h"

class HA_Forward : public Hmm_Analyzer {
public:
    HA_Forward();

    virtual ~HA_Forward();

    virtual bool analyze(const glm::vec3 &curr_observation) override;

    bool get_Observation(std::vector<float>& curr_observation, std::vector<float>& observation);
    virtual bool get_Observation(const glm::vec3 &curr_observation, std::vector<float> &observation) override;
	virtual bool set_Observation(const glm::vec3 &curr_observation) override;

    virtual bool is_End() override;
    virtual void reset() override;

private:
    bool is_prev;
    bool is_init_move; // for the first moving
    bool is_stay;
    glm::vec3 m_prev_obseravtion;
    glm::vec3 m_diff;
    int m_nondiff_cnt;
    int m_diff_cnt;

};

#endif //__HMM_ANALYZER_FORWARD_H__

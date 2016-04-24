//
// Created by hobbang5 on 2016-03-22.
//

#ifndef __HMM_MODEL_FORWARD_H__
#define __HMM_MODEL_FORWARD_H__

#include "hmm_model.h"
#include "hmm_analyzer_armup.h"

class Hmm_Forward : public Hmm_Model {
public:
    static unsigned int FORWARD_NB_STATE;
    static unsigned int FORWARD_TS_DIMENSION;
    static unsigned int FORWARD_WINDOW_SIZE;
    static double FORWARD_THRESHOLD;

    int m_observationCnt;

public:
    Hmm_Forward();

    virtual ~Hmm_Forward();

    Hmm_Forward(const Hmm_Forward &src);

    virtual bool is_PerformingDone_child() override;

    virtual double get_Probability_child() override;

    virtual double perform_Stretching_child(const glm::vec3 &curr_observation) override;

    virtual bool reset_child() override;

    virtual bool retrain_child() override;

private:
    // Hidden markov model using XMM lib
    xmm::HMM m_hmm;

    // observation(sensor data) analyzer
//    HA_ArmUp m_analyzer;

    // flag for performing
    bool m_isPerforming;
};

#endif //__HMM_MODEL_FORWARD_H__
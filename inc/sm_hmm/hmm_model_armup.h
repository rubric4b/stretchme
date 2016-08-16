//
// Created by hobbang5 on 2016-03-22.
//

#ifndef __HMM_MODEL_ARM_UP_H__
#define __HMM_MODEL_ARM_UP_H__

#include "hmm_model.h"

class Hmm_ArmUp : public Hmm_Model {
public:
    static unsigned int ARM_UP_NB_STATE;
    static unsigned int ARM_UP_TS_DIMENSION;
    static unsigned int ARM_UP_WINDOW_SIZE;
    static double ARM_UP_THRESHOLD;

    int m_observationCnt;

public:
    Hmm_ArmUp();

    virtual ~Hmm_ArmUp();

    Hmm_ArmUp(const Hmm_ArmUp &src);

    virtual bool is_PerformingDone_child() override;

    virtual double get_Probability_child() override;

    virtual double perform_Stretching_child(const glm::vec3 &curr_observation) override;

    virtual bool reset_child() override;

    virtual bool retrain_child() override;

private:
    // Hidden markov model using XMM lib
    xmm::HMM m_hmm;

    // flag for performing
    bool m_isPerforming;
};

#endif //__HMM_MODEL_ARM_UP_H__

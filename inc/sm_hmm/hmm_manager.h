//
// Created by hobbang5 on 2016-03-16.
//

#ifndef __SM_HMM_H__
#define __SM_HMM_H__


#include <vector>

#include "hmm_model.h"
#include "stretch_manager.h"
#include "xmm/xmm.h"

class Hmm_Manager
{
public:
    // consturctor
    Hmm_Manager();

    // destructor
    ~Hmm_Manager();

    // get loglikehood within window
    double get_Probability(StretchType type);
    double get_Probability();

    // model performing
    double perform_Stretching(StretchType type, glm::vec3 &observation);
    double perform_Stretching(glm::vec3 &observation);

    // get threshold of the motion probability within model
    double get_Threshold(StretchType type);
    double get_Threshold();

    // get the number of state
    unsigned int get_NbState(StretchType type);
    unsigned int get_NbState();

    // get the dimension of training set
    unsigned int get_TsDim(StretchType type);
    unsigned int get_TsDim();

    // get the end of performing
    bool is_End(StretchType type);
    bool is_End();

    // reset when stretching end
    void reset_Model_Performing(StretchType type);
    void reset_Model_Performing();
    void reset_All_Model_Performing();

private:
    void init_Manager();

    // current stretch type
    StretchType m_currType;

    // models HMM
    std::vector<Hmm_Model *> m_models;


public:
    // getter and setter
    StretchType get_CurrentType() { return m_currType; }
    void set_CurrentType(StretchType type) { m_currType = type; }

};

#endif //__SM_HMM_H__

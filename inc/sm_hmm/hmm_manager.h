//
// Created by hobbang5 on 2016-03-16.
//

#ifndef __SM_HMM_H__
#define __SM_HMM_H__


#include <vector>

#include "singleton.hpp"
#include "hmm_model.h"
#include "stretch_manager.h"
#include "xmm/xmm.h"
#include "hmm_analyzer.h"

class Hmm_Manager : public Singleton<Hmm_Manager>
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
    double perform_Stretching(StretchType type, const glm::vec3 &observation);
    double perform_Stretching(const glm::vec3 &observation);

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

    // retraining model
    bool retrain_Model(StretchType type);

    // analyze Observation
    bool analyze_Observation(StretchType type, glm::vec3 observation);
    bool analyze_Observation(glm::vec3 observation);

    // doen by analyzer
    bool get_End(StretchType type);
    bool get_End();

    // get motion data
    const float* const get_MotionData() {
        return m_models[m_currType]->get_MotionData();
    }

    int get_MotionCount() {
        return m_models[m_currType]->get_MotionCount();
    }

	void backup_MotionData() {
		m_models[m_currType]->backup_MotionData();
	}


private:
    void init_Manager();

    // current stretch type
    StretchType m_currType;

    // models HMM
    std::vector<Hmm_Model *> m_models;
    std::vector<Hmm_Analyzer *> m_analyzers;


public:
    // getter and setter
    StretchType get_CurrentType() { return m_currType; }
    void set_CurrentType(StretchType type) { m_currType = type; }

};

#endif //__SM_HMM_H__

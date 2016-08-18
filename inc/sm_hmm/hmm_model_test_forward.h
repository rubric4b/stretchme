//
// Created by hobbang5 on 2016-08-18.
//

#ifndef STRETCHME_HMM_MODEL_TEST_FORWARD_H
#define STRETCHME_HMM_MODEL_TEST_FORWARD_H

#include "hmm_model.h"
#include "hmm_analyzer_test.h"

class HA_Test;

class Hmm_Test_Forward : public Hmm_Model {
public:
	static const unsigned int TEST_NB_STATE;
	static const unsigned int TEST_WINDOW_SIZE;
	static const double TEST_THRESHOLD;


public:
	Hmm_Test_Forward();

	virtual ~Hmm_Test_Forward();

private: //can't use copy constructor
	Hmm_Test_Forward(const Hmm_Test_Forward &src);

public:
	virtual bool is_PerformingDone_child() override;

	virtual double get_Probability_child() override;

	virtual double perform_Stretching_child(const glm::vec3 &curr_observation) override;

	virtual bool reset_child() override;

	virtual bool retrain_child() override;

private:
	// Hidden markov model using XMM lib
	xmm::HMM m_hmm;

	HA_Test* m_testAnalyzer;
	// flag for performing
	bool m_isPerforming;
	int m_observationCnt;

};


#endif //STRETCHME_HMM_MODEL_TEST_FORWARD_H

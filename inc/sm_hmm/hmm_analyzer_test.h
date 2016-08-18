//
// Created by hobbang5 on 2016-08-16.
//

#ifndef STRETCHME_HMM_ANALYZER_TEST_H
#define STRETCHME_HMM_ANALYZER_TEST_H

#include <queue>

#include "hmm_analyzer.h"

class HA_Test : public Hmm_Analyzer {
public:
	typedef enum _ob_Dim {
		DIR_X,
		DIR_Y,
		DIR_Z,
		DIFF_X,
		DIFF_Y,
		DIFF_Z,
		LENGTH,
		TRAINSET_DIM
	} ob_Dim;

	typedef struct _ob_Container {
		float observ[TRAINSET_DIM];
	} ob_Container;

	typedef std::vector<ob_Container> VecData;
	typedef std::vector<ob_Container>::iterator VecDataIter;

	static const unsigned int INTERPOLATION_COUNT;

	HA_Test();

	virtual ~HA_Test();

	virtual bool analyze(const glm::vec3 &curr_observation) override;

	bool get_Observation(std::vector<float>& curr_observation, std::vector<float>& observation);
	virtual bool get_Observation(const glm::vec3 &curr_observation, std::vector<float> &observation) override;
	virtual bool set_Observation(const glm::vec3 &curr_observation) override;

	bool calculate_Observation(VecData &out_observ);

	virtual bool is_End() override;
	virtual void reset() override;

private:
	bool m_isInitMove; // for the first moving
	bool m_isStay;
	int m_moveCnt;
	int m_stayCnt;

	std::queue<glm::vec3> m_windowQueue;
	std::vector<glm::vec3> m_observations;

};

#endif //STRETCHME_HMM_ANALYZER_TEST_H

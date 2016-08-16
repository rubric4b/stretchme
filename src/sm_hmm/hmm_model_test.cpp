//
// Created by hobbang5 on 2016-08-16.
//
#include "sm_hmm/hmm_model_test.h"

#include <app.h>
#include <dirent.h>
#include <sstream>

#include "sm_hmm/hmm_analyzer_test.h"
#include "sm_hmm/file_handler.h"
#include "logger.h"

#define FILE_TEST "xmm_test.hmm"
#define TRAINING_FILE_PATH "/opt/usr/media/"

using glm::vec3;
using std::vector;
using std::string;
using std::stringstream;

const unsigned int Hmm_Test::TEST_NB_STATE = 4;
const unsigned int Hmm_Test::TEST_TS_DIMENSION = 7;
const unsigned int Hmm_Test::TEST_WINDOW_SIZE = 1;
//dou ble       Hmm_Test::ARM_UP_THRESHOLD        = 8;
const double       Hmm_Test::TEST_THRESHOLD = 3;

Hmm_Test::Hmm_Test() :
		m_hmm(),
		m_testAnalyzer(NULL),
		m_isPerforming(false),
		m_observationCnt(0)
{
	m_testAnalyzer = new HA_Test();
	m_analyzer = m_testAnalyzer;

//	m_hmm = xmm::HMM();
	if(!read_hmm_from_file(FILE_TEST, m_hmm)) {
//	if (true) {
		//get observations from files
		vector<HA_Test::VecData> all_observations;

		DIR *dir;
		struct dirent *ent;
		std::stringstream path;
		const string DATA_DIR = "data/armup/";
		path << app_get_resource_path() << DATA_DIR;
		if ((dir = opendir(path.str().c_str())) != NULL) {
			/* print all the files and directories within directory */
			int index(0);
			while ((ent = readdir(dir)) != NULL) {
				if (ent->d_type == 8) {
					HA_Test::VecData observ;
					observ.reserve(HA_Test::INTERPOLATION_COUNT);
					stringstream file;
					file << path.str() << ent->d_name;
					set_observ_from_file(m_analyzer, file.str().c_str(), USER_PATH);

					m_testAnalyzer->calculate_Observation(observ);
					all_observations.push_back(observ);

					index++;
					DBG("index %d, %s\n", index, ent->d_name);
					m_analyzer->reset();
				}
			}
			closedir(dir);
		} else {
			ERR("Coundn't open diretory!\n");
		}

		// setup training set
		xmm::TrainingSet trainSet(xmm::SHARED_MEMORY, HA_Test::TRAINSET_DIM);


		vector<HA_Test::VecData>::iterator iter = all_observations.begin();
		int idx(0);
		for (; iter != all_observations.end(); iter++) {
			trainSet.connect(idx, iter->at(0).observ, HA_Test::INTERPOLATION_COUNT);
			idx++;
		}
		DBG("%d training sets are set.", idx);

		// setup xmm
		m_hmm.set_trainingSet(&trainSet);
		m_hmm.set_nbStates(TEST_NB_STATE);
		m_hmm.set_transitionMode("left-right");
		m_hmm.set_covariance_mode(xmm::GaussianDistribution::DIAGONAL);
		m_hmm.set_likelihoodwindow(TEST_WINDOW_SIZE);

		// training
		m_hmm.train();

		DBG("hmm test() initialize\n");
//        DBG("%s", m_hmm.__str__().c_str());

		write_hmm_to_file(FILE_TEST, m_hmm);

	}

	// initialize Hmm_Model
	init_Hmm(TEST_NB_STATE, HA_Test::TRAINSET_DIM, TEST_THRESHOLD);
	m_hmm.performance_init();

}

Hmm_Test::~Hmm_Test() {
	if (m_analyzer)
		delete m_analyzer;

	m_analyzer = NULL;
	m_testAnalyzer = NULL;
}

bool Hmm_Test::is_PerformingDone_child() {
	return m_analyzer->is_End();
}

double Hmm_Test::get_Probability_child() {
	DBG("observation cnt = %d", m_observationCnt);

	/// To do
	//여기에 실제 트레이닝
	HA_Test::VecData ob_data;
	ob_data.reserve(HA_Test::INTERPOLATION_COUNT);

	m_testAnalyzer->calculate_Observation(ob_data);

	HA_Test::VecDataIter iter = ob_data.begin();
	for(; iter != ob_data.end(); iter++) {
		vector<float> ob(iter->observ, iter->observ + HA_Test::TRAINSET_DIM);
		m_hmm.performance_update(ob);
	}


	return (m_observationCnt < 50) ? 0 : m_hmm.results_log_likelihood;
}

double Hmm_Test::perform_Stretching_child(const vec3 &curr_observation) {
	/// To do
	// 여기에 기록 및 normalize 준비

	if (!m_isPerforming) {
		m_isPerforming = true;
	}

	if(m_testAnalyzer->set_Observation(curr_observation)) {
		m_observationCnt++;
	}

/*
	vector<float> observation(m_tsDim);
	if (m_analyzer->get_Observation(curr_observation, observation)) {
//		m_observeSeq.push_back(curr_observation);
		return m_hmm.performance_update(observation);
	}
*/

	return 0;
}

bool Hmm_Test::reset_child() {
	m_observationCnt = 0;
	m_isPerforming = false;
	m_hmm.performance_init();
	m_analyzer->reset();

	return true;
}

bool Hmm_Test::retrain_child() {

	// setup training set
	xmm::TrainingSet ts(xmm::NONE, TEST_TS_DIMENSION);

	int index(0);
	DIR *dir;
	struct dirent *ent;
	std::stringstream path;
	const string DATA_DIR = "data/";
	path << app_get_resource_path() << DATA_DIR;
	if ((dir = opendir(path.str().c_str())) != NULL) {
		/* print all the files and directories within directory */
		while ((ent = readdir(dir)) != NULL) {
			if (ent->d_type == 8) {
				stringstream file;;
				file << path.str() << ent->d_name;
				record_training_set_from_file(m_analyzer, file.str().c_str(), USER_PATH, index, TEST_TS_DIMENSION, ts);
				index++;
				DBG("index %d, %s\n", index, ent->d_name);
			}
		}
		closedir(dir);
	} else {
		ERR("Coundn't open diretory!\n");
	}

	// training set files
	const char *arm_up_training_set[] =
			{
					TRAINING_FILE_PATH"training_armup_1.csv", TRAINING_FILE_PATH"training_armup_2.csv",
					TRAINING_FILE_PATH"training_armup_3.csv"
			};

	//record training set
	for (int i = 0; i < 3; i++) {
		record_training_set_from_file(m_analyzer, arm_up_training_set[i], USER_PATH, index, TEST_TS_DIMENSION, ts);
		index++;
	}

	// setup xmm
	m_hmm.set_trainingSet(&ts);
	m_hmm.set_nbStates(TEST_NB_STATE);
	m_hmm.set_transitionMode("left-right");
	m_hmm.set_covariance_mode(xmm::GaussianDistribution::FULL);
	m_hmm.set_likelihoodwindow(TEST_WINDOW_SIZE);

	// training
	m_hmm.train();

	DBG("hmm armup() retrained\n");
//    DBG("%s", m_hmm.__str__().c_str());

	write_hmm_to_file(FILE_TEST, m_hmm);

	// initialize Hmm_Model
	init_Hmm(TEST_NB_STATE, TEST_TS_DIMENSION, TEST_THRESHOLD);
	m_hmm.performance_init();

	return true;
}



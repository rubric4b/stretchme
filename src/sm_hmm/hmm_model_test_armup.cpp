//
// Created by hobbang5 on 2016-08-16.
//
#include "sm_hmm/hmm_model_test_armup.h"

#include <app.h>
#include <dirent.h>
#include <sstream>

#include "sm_hmm/hmm_analyzer_test.h"
#include "sm_hmm/file_handler.h"
#include "logger.h"

#define TEST_MODEL_FILENAME "xmm_test_armup.hmm"
#define TRAINING_FILE_PATH "/opt/usr/media/"
#define ARMUP_DATA_FILE_PATH "data/armup/"

using glm::vec3;
using std::vector;
using std::string;
using std::stringstream;

const unsigned int Hmm_Test_Armup::TEST_NB_STATE = 5;
const unsigned int Hmm_Test_Armup::TEST_WINDOW_SIZE = 1;
const double       Hmm_Test_Armup::TEST_THRESHOLD = 0.0f;

Hmm_Test_Armup::Hmm_Test_Armup() :
		m_hmm(),
		m_testAnalyzer(NULL),
		m_isPerforming(false),
		m_observationCnt(0),
		m_backupLerpObservs() {
	m_testAnalyzer = new HA_Test();
	m_analyzer = m_testAnalyzer;

	if(!read_hmm_from_file(TEST_MODEL_FILENAME, m_hmm)) {
//	if (true) {
		//get observations from files
		vector<HA_Test::VecData> all_observations;

		DIR *dir;
		struct dirent *ent;
		std::stringstream path;
		path << app_get_resource_path() << ARMUP_DATA_FILE_PATH;
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
		m_hmm.set_covariance_mode(xmm::GaussianDistribution::FULL);
		m_hmm.set_likelihoodwindow(TEST_WINDOW_SIZE);

		// training
		m_hmm.train();

		DBG("hmm test() initialize\n");
//        DBG("%s", m_hmm.__str__().c_str());

		write_hmm_to_file(TEST_MODEL_FILENAME, m_hmm);

	}

	// initialize Hmm_Model
	init_Hmm(TEST_NB_STATE, HA_Test::TRAINSET_DIM, TEST_THRESHOLD);
	m_hmm.performance_init();

}

Hmm_Test_Armup::~Hmm_Test_Armup() {
	if (m_analyzer)
		delete m_analyzer;

	m_analyzer = NULL;
	m_testAnalyzer = NULL;
}

bool Hmm_Test_Armup::is_PerformingDone_child() {
	return m_analyzer->is_End();
}

double Hmm_Test_Armup::get_Probability_child() {
	DBG("observation cnt = %d", m_observationCnt);

	/// To do
	//여기에 실제 트레이닝
	HA_Test::VecData ob_data;
	ob_data.reserve(HA_Test::INTERPOLATION_COUNT);

	m_testAnalyzer->calculate_Observation(ob_data);
	backup_MotionData();

	HA_Test::VecDataIter iter = ob_data.begin();
	for (; iter != ob_data.end(); iter++) {
		vector<float> ob(iter->observ, iter->observ + HA_Test::TRAINSET_DIM);
		m_hmm.performance_update(ob);
	}


	return (m_observationCnt < 30) ? 0 : m_hmm.results_log_likelihood;
}

double Hmm_Test_Armup::perform_Stretching_child(const vec3 &curr_observation) {
	/// To do
	// 여기에 기록 및 normalize 준비

	if (!m_isPerforming) {
		m_isPerforming = true;
	}

	if (m_testAnalyzer->set_Observation(curr_observation)) {
		m_observationCnt++;
	}

	return 0;
}

bool Hmm_Test_Armup::reset_child() {
	m_observationCnt = 0;
	m_isPerforming = false;
	m_hmm.performance_init();
	m_analyzer->reset();

	return true;
}

bool Hmm_Test_Armup::retrain_child() {

	//get observations from files
	vector<HA_Test::VecData> all_observations;

	DIR *dir;
	struct dirent *ent;
	std::stringstream path;
	path << app_get_resource_path() << ARMUP_DATA_FILE_PATH;
	if ((dir = opendir(path.str().c_str())) != NULL) {
		/* print all the files and directories within directory */
		int index(0);
		while ((ent = readdir(dir)) != NULL) {
			if (ent->d_type == 8) {
				stringstream file;
				file << path.str() << ent->d_name;
				set_observ_from_file(m_analyzer, file.str().c_str(), USER_PATH);

				HA_Test::VecData observ;
				observ.reserve(HA_Test::INTERPOLATION_COUNT);
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

	// additional training set files
	const char *arm_up_training_set[] =
			{
					TRAINING_FILE_PATH"training_armup_1.csv",
					TRAINING_FILE_PATH"training_armup_2.csv",
					TRAINING_FILE_PATH"training_armup_3.csv"
			};

	for (int i = 0; i < 3; i++) {
		set_observ_from_file(m_analyzer, arm_up_training_set[i], USER_PATH);
		HA_Test::VecData observ;
		observ.reserve(HA_Test::INTERPOLATION_COUNT);
		m_testAnalyzer->calculate_Observation(observ);
		all_observations.push_back(observ);
		m_analyzer->reset();
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
	m_hmm.set_covariance_mode(xmm::GaussianDistribution::FULL);
	m_hmm.set_likelihoodwindow(TEST_WINDOW_SIZE);

	// training
	m_hmm.train();

	DBG("hmm test() initialize\n");
//        DBG("%s", m_hmm.__str__().c_str());

	write_hmm_to_file(TEST_MODEL_FILENAME, m_hmm);


	// initialize Hmm_Model
	init_Hmm(TEST_NB_STATE, HA_Test::TRAINSET_DIM, TEST_THRESHOLD);
	m_hmm.performance_init();

	return true;
}

void Hmm_Test_Armup::backup_MotionData() {
	m_backupLerpObservs.clear();
	m_backupLerpObservs.reserve(m_testAnalyzer->get_LerpObservation().size() * 3);
	std::vector<glm::vec3>::const_iterator cIter = m_testAnalyzer->get_LerpObservation().begin();
	for( ; cIter != m_testAnalyzer->get_LerpObservation().end(); cIter++) {
		m_backupLerpObservs.push_back(cIter->x);
		m_backupLerpObservs.push_back(cIter->y);
		m_backupLerpObservs.push_back(cIter->z);
	}
	DBG("motion data backup. size[%d]", m_backupLerpObservs.size());
}



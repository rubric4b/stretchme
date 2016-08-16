//
// Created by hobbang5 on 2016-03-22.
//
#include <app.h>
#include <dirent.h>
#include <sstream>

#include "sm_hmm/hmm_analyzer_forward.h"
#include "sm_hmm/hmm_model_forward.h"
#include "sm_hmm/file_handler.h"
#include "logger.h"

#define FILE_FORWARD "xmm_forward.hmm"
#define TRAINING_FILE_PATH "/opt/usr/media/"


using namespace std;

unsigned int Hmm_Forward::FORWARD_NB_STATE         = 4;
unsigned int Hmm_Forward::FORWARD_TS_DIMENSION     = 7;
unsigned int Hmm_Forward::FORWARD_WINDOW_SIZE      = 1;
//double       Hmm_Forward::FORWARD_THRESHOLD        = 14;
double       Hmm_Forward::FORWARD_THRESHOLD        = 13;

Hmm_Forward::Hmm_Forward() :
    m_observationCnt(0),
    m_isPerforming(false)
{
    m_analyzer = new HA_Forward();

    m_hmm = xmm::HMM();
    if(!read_hmm_from_file(FILE_FORWARD, m_hmm)) {
//    if(true) {

        // setup training set
        xmm::TrainingSet ts(xmm::NONE, FORWARD_TS_DIMENSION);

        DIR *dir;
        struct dirent *ent;
        std::stringstream path;
        const string DATA_DIR = "data/forward/";
        path << app_get_resource_path() << DATA_DIR;
        if ((dir = opendir (path.str().c_str())) != NULL) {
            /* print all the files and directories within directory */
            int index(0);
            while ((ent = readdir (dir)) != NULL) {
                if(ent->d_type == 8) {
                    stringstream file;;
                    file << path.str() << ent->d_name;
					record_training_set_from_file(m_analyzer, file.str().c_str(), USER_PATH, index, FORWARD_TS_DIMENSION, ts);
                    index++;
                    DBG("index %d, %s\n", index, ent->d_name);
                }
            }
            closedir (dir);
        } else {
            ERR("Coundn't open diretory!\n");
        }

        // setup xmm
        m_hmm.set_trainingSet(&ts);
        m_hmm.set_nbStates(FORWARD_NB_STATE);
        m_hmm.set_transitionMode("left-right");
        m_hmm.set_covariance_mode(xmm::GaussianDistribution::DIAGONAL);
        m_hmm.set_likelihoodwindow(FORWARD_WINDOW_SIZE);

        // training
        m_hmm.train();

        DBG("hmm Forward() initialize\n");
//        DBG("%s", m_hmm.__str__().c_str());

        write_hmm_to_file(FILE_FORWARD, m_hmm);

    }

    // initialize Hmm_Model
    init_Hmm(FORWARD_NB_STATE, FORWARD_TS_DIMENSION, FORWARD_THRESHOLD);
    m_hmm.performance_init();

}

Hmm_Forward::~Hmm_Forward() {
    if(m_analyzer)
        delete m_analyzer;
}

Hmm_Forward::Hmm_Forward(const Hmm_Forward &src) {

}

bool Hmm_Forward::is_PerformingDone_child() {
    return m_analyzer->is_End();
}

double Hmm_Forward::get_Probability_child() {
    DBG("observation cnt = %d", m_observationCnt);
    return (m_observationCnt < 30) ? 0 : m_hmm.results_log_likelihood;
}

double Hmm_Forward::perform_Stretching_child(const glm::vec3 &curr_observation) {
    if(!m_isPerforming) {
        m_isPerforming = true;
    }

    vector<float> observation(m_tsDim);
    if(m_analyzer->get_Observation(curr_observation, observation)) {
        m_observationCnt++;
        return m_hmm.performance_update(observation);
    }

    return 0;
}

bool Hmm_Forward::reset_child() {
    m_observationCnt = 0;
    m_isPerforming = false;
    m_hmm.performance_init();
    m_analyzer->reset();

    return true;
}

bool Hmm_Forward::retrain_child() {

    // setup training set
    xmm::TrainingSet ts(xmm::NONE, FORWARD_TS_DIMENSION);

    int index(0);
    DIR *dir;
    struct dirent *ent;
    std::stringstream path;
    const string DATA_DIR = "data/";
    path << app_get_resource_path() << DATA_DIR;
    if ((dir = opendir (path.str().c_str())) != NULL) {
        /* print all the files and directories within directory */
        while ((ent = readdir (dir)) != NULL) {
            if(ent->d_type == 8) {
                stringstream file;;
                file << path.str() << ent->d_name;
				record_training_set_from_file(m_analyzer, file.str().c_str(), USER_PATH, index, FORWARD_TS_DIMENSION, ts);
                index++;
                DBG("index %d, %s\n", index, ent->d_name);
            }
        }
        closedir (dir);
    } else {
        ERR("Coundn't open diretory!\n");
    }

    // training set files
    const char* forward_training_set[] =
        {
              TRAINING_FILE_PATH"training_forward_1.csv"
             ,TRAINING_FILE_PATH"training_forward_2.csv"
             ,TRAINING_FILE_PATH"training_forward_3.csv"
        };

    //record training set
    for(int i=0; i<3; i++) {
		record_training_set_from_file(m_analyzer, forward_training_set[i], USER_PATH, index, FORWARD_TS_DIMENSION, ts);
        index++;
    }

    // setup xmm
    m_hmm.set_trainingSet(&ts);
    m_hmm.set_nbStates(FORWARD_NB_STATE);
    m_hmm.set_transitionMode("left-right");
    m_hmm.set_covariance_mode(xmm::GaussianDistribution::FULL);
    m_hmm.set_likelihoodwindow(FORWARD_WINDOW_SIZE);

    // training
    m_hmm.train();

    DBG("hmm Forward() retrained\n");
//    DBG("%s", m_hmm.__str__().c_str());

    write_hmm_to_file(FILE_FORWARD, m_hmm);

    // initialize Hmm_Model
    init_Hmm(FORWARD_NB_STATE, FORWARD_TS_DIMENSION, FORWARD_THRESHOLD);
    m_hmm.performance_init();

    return true;
}



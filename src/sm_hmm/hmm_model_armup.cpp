//
// Created by hobbang5 on 2016-03-22.
//
#include <app.h>

#include "sm_hmm/hmm_analyzer_armup.h"
#include "sm_hmm/hmm_model_armup.h"
#include "sm_hmm/file_handler.h"
#include "logger.h"

#define FILE_ARMUP "xmm_armup.hmm"

using namespace std;

unsigned int Hmm_ArmUp::ARM_UP_NB_STATE         = 4;
unsigned int Hmm_ArmUp::ARM_UP_TS_DIMENSION     = 7;
unsigned int Hmm_ArmUp::ARM_UP_WINDOW_SIZE      = 1;
double       Hmm_ArmUp::ARM_UP_THRESHOLD        = 9;

Hmm_ArmUp::Hmm_ArmUp() :
    m_observationCnt(0),
    m_isPerforming(false)
{
    m_analyzer = HA_ArmUp();

    m_hmm = xmm::HMM();
//    if(!read_hmm_from_file(FILE_ARMUP, m_hmm)) {
    if(true) {

        // training set files
        const char* arm_up_learning_set[] =
                {
                        "data/learning_set_kaccel_4.csv",
                        "data/learning_set_kaccel_5.csv",
                        "data/learning_set_kaccel_6.csv",
                        "data/learning_set_kaccel_7.csv",
                        "data/learning_set_kaccel_8.csv",
                        "data/learning_set_kaccel_9.csv",
                        "data/learning_set_kaccel_10.csv",
                        "data/learning_set_kaccel_11.csv",
                        "data/learning_set_kaccel_12.csv",
                        "data/learning_set_kaccel_13.csv",
                        "data/learning_set_kaccel_14.csv"
                };

        // setup training set
        xmm::TrainingSet ts(xmm::NONE, ARM_UP_TS_DIMENSION);

        //record training set
        for(int i=0; i<11; i++) {
            record_training_set_from_file(arm_up_learning_set[i], i, ts);
        }

        // setup xmm
        m_hmm.set_trainingSet(&ts);
        m_hmm.set_nbStates(ARM_UP_NB_STATE);
        m_hmm.set_transitionMode("left-right");
        m_hmm.set_covariance_mode(xmm::GaussianDistribution::DIAGONAL);
        m_hmm.set_likelihoodwindow(ARM_UP_WINDOW_SIZE);

        // training
        m_hmm.train();

        DBG("hmm armup() initialize\n");
        DBG("%s", m_hmm.__str__().c_str());

        write_hmm_to_file(FILE_ARMUP, m_hmm);

    }

    // initialize Hmm_Model
    init_Hmm(ARM_UP_NB_STATE, ARM_UP_TS_DIMENSION, ARM_UP_THRESHOLD);
    m_hmm.performance_init();

}

Hmm_ArmUp::~Hmm_ArmUp() {

}

Hmm_ArmUp::Hmm_ArmUp(const Hmm_ArmUp &src) {

}

bool Hmm_ArmUp::is_PerformingDone_child() {
    return m_analyzer.is_End();
}

double Hmm_ArmUp::get_Probability_child() {
    DBG("observation cnt = %d", m_observationCnt);
    return (m_observationCnt <50) ? 0 : m_hmm.results_log_likelihood;
}

double Hmm_ArmUp::perform_Stretching_child(const glm::vec3 &curr_observation) {
    if(!m_isPerforming) {
        m_isPerforming = true;
    }

    vector<float> observation(m_tsDim);
    if(m_analyzer.get_Observation(curr_observation, observation)) {
        m_observationCnt++;
        return m_hmm.performance_update(observation);
    }

    return 0;
}

bool Hmm_ArmUp::reset_child() {
    m_observationCnt = 0;
    m_isPerforming = false;
    m_hmm.performance_init();
    m_analyzer.reset();

    return true;
}

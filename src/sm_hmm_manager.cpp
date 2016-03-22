//
// Created by hobbang5 on 2016-03-16.
//

#include <app.h>

#include "sm_hmm_manager.h"
#include "logger.h"
#include "sm_hmm_analyzer.h"

using namespace std;


static char* util_strtok(char* str, const char* delim, char** nextp)
{
    char* ret;

    if (str == NULL) {
        str = *nextp;
    }

    str += strspn(str, delim);

    if (*str == '\0') {
        return NULL;
    }

    ret = str;

    str += strcspn(str, delim);

    if (*str) {
        *str++ = '\0';
    }

    *nextp = str;

    return ret;
}


static void record_training_set_from_file(xmm::TrainingSet& ts, int index, const std::string& file_name)
{
    //buffer
    std::ifstream in_file;

    // file path
    std::string file_path = app_get_resource_path() + file_name;

    // file open
    in_file.open(file_path.c_str(), std::ios::in | std::ios::binary);
    if(!in_file.is_open() || !in_file.good()) {
        std::string msg = "Failed to open file " + file_path;
        throw std::runtime_error(msg.c_str());
    }

    // record file to training set
    int line_cnt(0), ts_cnt(0);
    char line_buffer[512];
    std::vector<float> curr_observation(3);
    std::vector<float> observation(SM_DEFAULT_TS_DIMENTION);
    Sm_Hmm_Analyzer esti = Sm_Hmm_Analyzer();
    while (!in_file.getline(line_buffer, sizeof(line_buffer)).eof()) {
        char *word, *wordPtr;
        word = util_strtok(line_buffer, ",", &wordPtr); // time
        word = util_strtok(NULL, ",", &wordPtr); // accel - x
        curr_observation[0] = atof(word);
        word = util_strtok(NULL, ",", &wordPtr); // accel - y
        curr_observation[1] = atof(word);
        word = util_strtok(NULL, ",", &wordPtr); // accel - z
        curr_observation[2] = atof(word);

        if( esti.get_Observation(curr_observation, observation) ) {

            ts.recordPhrase(index, observation);
            ts_cnt++;
        }

        line_cnt++;
    }
    in_file.close();

    DBG("%s : %d lines are read", file_name.c_str(), line_cnt);
    DBG("%s : %d observation are recorded to training set", file_name.c_str(), ts_cnt);

}

xmm::HMM* init_model(unsigned int set_cnts, const char** learning_sets,
                    unsigned int ts_dim = SM_DEFAULT_TS_DIMENTION,
                    unsigned int nb_state = SM_DEFAULT_NB_STATE,
                    unsigned int window_size = SM_DEFAULT_SMOOTHING_WINDOW_SIZE)
{

    //training set and hmm model
    xmm::TrainingSet ts(xmm::NONE, ts_dim);
    xmm::HMM* ret_model = new xmm::HMM(xmm::NONE, &ts);

    ret_model->set_nbStates(nb_state);
    ret_model->set_transitionMode("left-right");
    ret_model->set_covariance_mode(xmm::GaussianDistribution::DIAGONAL);
    ret_model->set_likelihoodwindow(window_size);

    //record training set
    for(int i=0; i<set_cnts; i++) {
        record_training_set_from_file(ts, i, learning_sets[i]);
    }

    ret_model->train();

    DBG("hmm manager::init\n");
    DBG("%s", ret_model->__str__().c_str());

    return ret_model;
}


Sm_Hmm_Manager::Sm_Hmm_Manager() :
    m_models(STRETCH_TYPE_NUM)
{
    //STRETCH_ARM_UP learning sets
    const char* arm_up_learning_set[] =
            {
//                "data/learning_set_kaccel_1.csv",
//                "data/learning_set_kaccel_2.csv",
//                "data/learning_set_kaccel_3.csv",
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

    hmm_item arm_up = { init_model(11, arm_up_learning_set),  false};

    m_models[STRETCH_ARM_UP] = arm_up;
    m_models[STRETCH_ARM_UP].model->performance_init();
    DBG("%s\n", m_models[STRETCH_ARM_UP].model->__str__().c_str());

}

Sm_Hmm_Manager::~Sm_Hmm_Manager() {
    for(int i=0; i<m_models.size(); i++)
    {
        delete m_models[i].model;
    }

}

double Sm_Hmm_Manager::get_Loglikehood(StretchType type) {
    return m_models[type].model->results_log_likelihood;
}

double Sm_Hmm_Manager::perform_Stretching(StretchType type, std::vector<float> &observation) {
    // initialize likelihood buffer of model
    if(!m_models[type].is_performing) {
        m_models[type].model->performance_init();
        m_models[type].is_performing = true;
    }

    return m_models[type].model->performance_update(observation);

}

void Sm_Hmm_Manager::reset_Model_Performing(StretchType type) {
    m_models[type].is_performing = false;
}

void Sm_Hmm_Manager::reset_All_Model_Performing() {
    for(int i=0; i<m_models.size(); i++)
    {
        m_models[i].is_performing = false;
    }
}

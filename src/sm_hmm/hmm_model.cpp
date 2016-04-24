//
// Created by hobbang5 on 2016-03-22.
//

#include "logger.h"
#include "sm_hmm/hmm_model.h"

using namespace std;

Hmm_Model::Hmm_Model() :
    m_nbState(0),
    m_tsDim(0),
    m_threshold(0),
    m_analyzer(NULL),
    m_isInit(false)
{ }

Hmm_Model::~Hmm_Model() {

}


Hmm_Model::Hmm_Model(const Hmm_Model &src) {
    // TODO : implement!

}

double Hmm_Model::get_Probability() {
   if(!get_Init()) {
       ERR("This model is not initialized\n");
       return 0;
   }

    return get_Probability_child();
}

double Hmm_Model::perform_Stretching(const glm::vec3 &curr_observation) {
    if(!get_Init()) {
        ERR("This model is not initialized\n");
        return 0;
    }

    return perform_Stretching_child(curr_observation);
}

bool Hmm_Model::is_PerformingDone() {
    if(!get_Init()) {
        ERR("This model is not initialized\n");
        return false;
    }

    return is_PerformingDone_child();
}

bool Hmm_Model::reset() {
    if(!get_Init()) {
        ERR("This model is not initialized\n");
        return false;
    }

    return reset_child();
}

bool Hmm_Model::init_Hmm(unsigned int nbState,
                         unsigned int tsDim,
                         double threshold)
{
    m_nbState = nbState;
    m_tsDim = tsDim;
    m_threshold = threshold;

    m_isInit = true;

    return true;
}

bool Hmm_Model::retrain() {
    if(!get_Init()) {
        ERR("This model is not initialized\n");
        return false;
    }
    return retrain_child();
}



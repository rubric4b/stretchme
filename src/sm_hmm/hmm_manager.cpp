//
// Created by hobbang5 on 2016-03-16.
//

#include <app.h>

#include "sm_hmm/hmm_manager.h"
#include "sm_hmm/hmm_model_armup.h"
#include "sm_hmm/hmm_model_forward.h"
#include "sm_hmm/hmm_model_test_armup.h"
#include "sm_hmm/hmm_model_test_forward.h"
#include "logger.h"

using glm::vec3;

Hmm_Manager::Hmm_Manager() :
        m_currType(STRETCH_TYPE_ARM_UP),
        m_models(STRETCH_TYPE_NUM),
        m_analyzers(STRETCH_TYPE_NUM)
{
    init_Manager();
}

Hmm_Manager::~Hmm_Manager() {

//    delete m_models[STRETCH_TYPE_ARM_UP];
//    delete m_models[STRETCH_TYPE_ARM_FORWARD];
    for(int i=0; i < m_models.size(); i++)
    {
        if(m_models[i])
            delete m_models[i];
    }

}

double Hmm_Manager::get_Probability(StretchType type) {
    return m_models[type]->get_Probability();
}

double Hmm_Manager::get_Probability() {
    return get_Probability(m_currType);
}

double Hmm_Manager::perform_Stretching(StretchType type, const vec3 &observation) {
    return m_models[type]->perform_Stretching(observation);
}

double Hmm_Manager::perform_Stretching(const vec3 &observation) {
    return perform_Stretching(m_currType, observation);
}

void Hmm_Manager::reset_Model_Performing(StretchType type) {
    m_models[type]->reset();
}

void Hmm_Manager::reset_Model_Performing() {
    reset_Model_Performing(m_currType);
}

void Hmm_Manager::reset_All_Model_Performing() {
    for (int i = 0; i < m_models.size(); i++) {
        m_models[i]->reset();
    }
}

void Hmm_Manager::init_Manager() {
//    m_models[STRETCH_TYPE_ARM_UP] = new Hmm_ArmUp();
//    m_analyzers[STRETCH_TYPE_ARM_UP] = m_models[STRETCH_TYPE_ARM_UP]->get_Analyzer();
	m_models[STRETCH_TYPE_ARM_UP] = new Hmm_Test_Armup();
    m_analyzers[STRETCH_TYPE_ARM_UP] = m_models[STRETCH_TYPE_ARM_UP]->get_Analyzer();

    m_models[STRETCH_TYPE_ARM_FORWARD] = new Hmm_Test_Forward();
    m_analyzers[STRETCH_TYPE_ARM_FORWARD] = m_models[STRETCH_TYPE_ARM_FORWARD]->get_Analyzer();

}

double Hmm_Manager::get_Threshold(StretchType type) {
    return m_models[type]->get_Threshold();
}

double Hmm_Manager::get_Threshold() {
    return get_Threshold(m_currType);
}

bool Hmm_Manager::is_End(StretchType type) {
    return m_models[type]->is_PerformingDone();
}

bool Hmm_Manager::is_End() {
    return is_End(m_currType);
}

unsigned int Hmm_Manager::get_NbState(StretchType type) {
    return m_models[type]->get_NbState();
}

unsigned int Hmm_Manager::get_NbState() {
    return get_NbState(m_currType);
}

unsigned int Hmm_Manager::get_TsDim(StretchType type) {
    return m_models[type]->get_TsDimension();
}

unsigned int Hmm_Manager::get_TsDim() {
    return get_TsDim(m_currType);
}

bool Hmm_Manager::retrain_Model(StretchType type) {
    return m_models[type]->retrain();
}

bool Hmm_Manager::analyze_Observation(StretchType type, glm::vec3 observation) {
    return m_analyzers[type]->analyze(observation);
}

bool Hmm_Manager::analyze_Observation(glm::vec3 observation) {
    return analyze_Observation(m_currType, observation);
}

bool Hmm_Manager::get_End(StretchType type) {
    return m_analyzers[type]->is_End();
}

bool Hmm_Manager::get_End() {
    return get_End(m_currType);
}












//
// Created by hobbang5 on 2016-03-16.
//

#ifndef __SM_HMM_H__
#define __SM_HMM_H__

#define SM_DEFAULT_NB_STATE 4
#define SM_DEFAULT_SMOOTHING_WINDOW_SIZE 20

#include <vector>

#include "stretch_manager.h"
#include "xmm/xmm.h"

typedef struct {
    xmm::HMM* model;
    bool is_performing;
} hmm_item;

class Sm_Hmm_Manager
{
public:
    // consturctor
    Sm_Hmm_Manager();

    // destructor
    ~Sm_Hmm_Manager();



    // get loglikehood within window
    double get_Loglikehood(StretchType type);

    // model performing
    double perform_Stretching(StretchType type, std::vector<float> &observation);



    // reset when stretching end
    void reset_Model_Performing(StretchType type);
    void reset_All_Model_Performing();

private:
    // models HMM
    std::vector<hmm_item> m_models;

};

#endif //__SM_HMM_H__

//
// Created by hobbang5 on 2015-12-15.
//

#ifndef __HMM_MODEL_H__
#define __HMM_MODEL_H__

#include <ghmm/model.h>

typedef struct _Hmm_Model {
    ghmm_dmodel model;
    ghmm_dstate* states;
    int* silent_array;
    int* id;
    double** transition_mat;
    double** transition_mat_rev;
    double** emission_mat;

} Hmm_Model;


void init_hmm();
Hmm_Model* create_hmm_model(const int state_cnt, const int symbol_cnt);
void free_hmm_model(Hmm_Model* hmm);
void save_model(Hmm_Model* hmm, const char* file_name);
Hmm_Model* read_model_from_file(const char* file_name);

int test_model();


#endif //__HMM_MODEL_H__
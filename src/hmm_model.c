//
// Created by hobbang5 on 2015-12-15.
//
#include <stdlib.h>
#include <app.h>

#include <ghmm/matrix.h>
#include <ghmm/rng.h>
#include <ghmm/sequence.h>
#include <ghmm/model.h>
#include <ghmm/viterbi.h>
#include <ghmm/foba.h>

#include <ghmm/obsolete.h>
#include <ghmm/reestimate.h>
#include <logger.h>

#include "hmm_model.h"

int HMM_MODEL_MAX_LENGTH = 0;

int seq2[] = {26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,6,9,9,9,10,13,13,10,16,16,16,16,19,19,1,4,24,16,13,7,4,4,26,26,15,15,11,26,4,24,10,10,7,7,4,14,14,3,3,6,7,7,4,4,1,0,0,0,4,4,4,4,4,1,24,24,16,16,16,16,24,24,1,1,24,24,24,24,26,26,18,18,18,18,19,19,15,15,15,15,15,15,15,15,5,5,5,12,15,16,16,16,15,5,5,25,25,25,25,25,25,25,25,25,25,25,25,25,23,20,26,20,25,25,8,8,25,20,6,20,17,17,17,14,17,26,17,6,20,17,17,17,17,17,17,17,17,17,17,17,17,17,14,17,17,17,17,17,17,17,17,17,17,3,3,17,17,17,17,17,3,17,14,17,3,3,17,3,26,26,26,26,26,26,26,26};
int seq1[] = {26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,10,7,7,7,6,26,26,26,11,8,8,8,8,15,16,16,16,16,16,19,19,19,19,19,19,16,16,26,26,7,26,26,26,26,13,10,7,26,15,8,8,25,20,6,6,7,7,26,11,3,4,4,4,4,4,1,1,4,4,4,1,1,1,1,1,1,1,1,1,1,1,22,22,19,19,19,19,19,18,18,18,18,15,15,16,16,16,16,16,15,15,15,15,15,15,15,12,15,15,15,5,8,5,5,5,5,5,5,25,25,25,25,25,25,25,25,25,25,25,17,17,17,17,17,17,14,14,14,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,3,3,3,3,14,14,0,0,0,0,0,0,0,0,0,14,14,0,0,0,0,0,0,26,26,0,0,0,0,0,0,14,0,3,26,0,0,0,26,26,26,26,0,26,0,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26};
int seq3[] = {26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,6,7,7,6,6,6,9,10,10,13,16,16,19,19,18,18,19,19,16,16,16,16,16,24,24,24,24,26,26,26,26,14,17,17,3,6,6,9,12,26,26,26,26,26,26,26,26,0,1,26,26,22,19,16,16,26,26,0,0,0,0,3,0,1,1,1,1,1,1,4,4,24,24,24,24,24,24,24,19,19,19,19,19,19,19,19,16,16,19,19,19,19,19,19,19,19,24,19,19,16,15,15,5,5,5,5,5,5,5,2,5,5,5,5,5,5,5,5,25,17,14,11,25,25,25,25,25,25,17,17,17,14,14,14,25,25,25,14,17,17,17,17,17,14,14,14,14,25,17,17,17,17,17,17,17,6,17,17,17,17,14,17,3,3,17,17,17,14,14,14,14,17,17,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,26,26,26,26,26,26};
int seq4[] = {26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,7,7,7,7,7,7,7,7,24,16,15,16,16,16,19,19,19,19,19,18,18,15,16,16,15,15,26,26,26,26,25,25,20,20,17,17,17,8,11,14,17,8,5,5,10,6,6,6,26,26,26,26,26,26,7,7,4,4,4,1,22,22,22,1,1,1,4,4,4,4,4,4,1,1,1,24,24,24,24,24,24,19,19,19,19,22,22,19,19,19,18,18,18,18,15,15,15,15,18,18,18,8,5,5,5,5,5,5,2,26,26,25,8,8,5,5,5,2,10,7,7,7,4,8,8,5,5,5,8,17,20,20,25,25,25,5,25,20,20,23,25,25,25,25,17,17,17,17,17,14,14,14,17,17,17,14,14,14,17,17,17,17,17,17,3,3,3,3,3,3,3,3,3,3,3,0,17,3,3,3,3,0,3,3,3,0,0,0,0,0,0,0,0,0,26,0,0,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,0,26,26,26,26,26};
int seq5[] = {26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,7,7,6,6,6,9,26,13,13,16,16,19,19,19,19,19,19,19,19,24,24,13,13,26,26,26,26,26,26,25,25,25,8,8,25,25,26,6,6,6,9,9,6,26,26,26,26,26,7,7,7,7,7,4,4,4,4,1,4,4,4,4,4,4,1,1,1,1,1,1,1,24,24,24,24,24,22,22,22,19,19,24,24,16,16,16,13,12,12,15,8,8,8,8,8,8,8,8,8,8,8,8,18,5,5,5,5,2,25,2,2,2,2,5,5,25,2,2,2,5,8,25,25,25,25,25,25,25,17,17,17,17,20,6,6,20,17,17,17,17,17,17,17,17,14,17,17,17,17,17,3,3,14,14,0,14,14,17,3,17,0,0,0,0,3,17,3,0,0,0,0,0,0,26,26,26,26,26,26,26,0,0,26,26,26,26,3,0,14,14,3,3,3,3,3,3,3,3,3,0,0,0,0,0,0,0,0,26,0,26,26,26,26};
//int seq6[] = {26,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,26,26,26,26,26,26,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,7,7,7,6,6,6,13,13,13,16,16,16,16,19,19,19,19,22,4,26,3,3,26,14,11,11,14,3,6,6,6,6,3,3,3,24,22,0,0,4,7,7,4,22,22,1,1,1,1,1,24,24,24,16,16,15,18,18,18,18,19,19,15,15,15,15,15,15,19,18,18,26,26,26,26,26,26,26,12,12,2,5,5,5,5,25,26,8,8,8,8,8,8,8,8,11,21,21,21,26,26,8,8,8,8,11,26,26,26,25,25,25,25,25,20,20,17,17,17,14,14,17,17,6,20,17,25,25,8,8,26,17,17,17,25,25,26,26,6,26,26,26,26,8,25,26,26,26,26,14,14,14,14,14,14,14,14,14,14,3,3,3,3,17,17,17,14,3,3,26,0,26,26,26,14,14,0,26,26,26,26,26,0,0,0,0,0,26,26,26,26,26,26,26,0,26,0,26,26,26,26,26,26,0,0,0,26,26,26,26,26,26};
//int seq7[] = {3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,7,7,10,10,10,7,10,10,13,13,16,16,16,19,19,19,24,19,19,19,26,10,7,10,7,10,7,26,14,14,6,6,6,7,4,1,1,1,1,1,1,1,1,1,1,22,19,19,16,16,16,19,19,19,19,19,19,19,19,19,18,18,15,5,5,8,8,8,8,5,2,2,2,2,5,5,5,25,25,25,25,25,25,25,25,25,25,25,8,8,8,8,8,11,11,11,11,14,14,26,26,11,11,11,5,8,26,26,11,25,25,25,5,5,26,26,25,25,25,26,14,14,25,17,17,3,17,17,17,17,3,17,17,6,6,6,6,6,20,17,17,20,14,17,17,17,17,17,14,14,14,17,17,17,3,3,3,3,3,26,26,0,26,0,0,26,26,26,26,3,0,14,26,26,26,26};
//int seq8[] = {26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,6,6,6,26,26,26,26,26,26,15,12,13,13,13,16,16,19,19,19,19,24,24,13,13,13,13,26,26,26,26,17,17,17,17,17,17,17,17,3,0,0,0,26,18,18,19,1,1,1,21,8,18,18,26,4,4,7,7,10,12,15,18,18,1,4,4,3,3,3,4,24,16,16,16,16,16,16,24,24,26,26,26,16,13,16,16,16,15,5,25,25,25,25,25,25,26,26,5,5,5,25,25,25,25,14,14,14,14,14,14,14,21,21,19,19,19,22,22,21,0,14,11,11,26,26,26,1,26,25,25,2,5,26,26,26};
//int seq9[] = {26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,7,7,7,7,7,7,7,4,4,4,26,26,15,15,16,16,16,16,19,19,19,19,19,19,19,16,16,16,16,16,26,26,5,8,8,8,26,25,25,26,7,6,6,6,6,6,6,6,26,26,4,7,7,4,4,4,4,4,24,1,1,1,1,22,24,24,16,19,24,24,24,24,24,24,1,1,0,21,21,21,19,19,19,19,18,18,18,18,18,18,18,18,15,15,15,15,15,15,15,15,15,18,8,8,5,5,5,2,2,23,25,8,8,5,25,25,25,25,25,17,14,11,25,25,25,25,20,17,17,17,17,17,17,17,20,20,20,20,17,17,17,17,14,17,17,17,17,17,17,17,17,17,17,17,17,17,14,14,14,17,17,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,26,26,26,26,26,26};
/*
int seq0[] = {17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,
              17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,
              17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,
              17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,
              17,17,17,17,17,17,17,7,6,7,7,4,7,17,17,10,10,10,10,10,10,
              10,10,12,13,13,15,4,7,3,3,3,3,11,17,17,3,3,3,3,11,11,
              11,11,11,14,14,6,6,3,6,17,3,4,3,3,3,4,4,4,4,4,4,
              4,4,4,4,4,4,1,1,1,1,1,1,1,1,1,1,13,13,13,13,12,
              17,17,17,9,5,5,5,5,5,5,5,9,9,9,9,9,5,5,5,2,2,
              2,16,16,16,5,5,16,2,2,17,14,14,14,14,16,16,16,16,14,14,16,
              16,14,11,14,14,11,11,14,14,14,14,14,14,11,11,11,11,3,11,11,11,
              3,3,3,3,14,14,3,3,3,3,11,0,14,11,3,3,3,3,3,3,3,
              3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,17,
              17,17,17,17,17,17,17,17,17,17};
*/

void init_hmm()
{
    /* Important! initialise rng  */
    ghmm_rng_init();



}


Hmm_Model* create_hmm_model(const int state_cnt, const int symbol_cnt)
{
    if (state_cnt < 0 || symbol_cnt < 0)
    {
        DBG("error! create_hmm_model. state or symbol count is negative\n");
        return NULL;
    }

    const int NUM_STATE = state_cnt;
    const int NUM_SYMBOL = symbol_cnt;

    Hmm_Model* hmm = (Hmm_Model*)calloc(1, sizeof(Hmm_Model));

    hmm->states = (ghmm_dstate *)calloc(NUM_STATE, sizeof(ghmm_dstate));


    // initialize state id
    hmm->silent_array = (int *)calloc(NUM_STATE, sizeof(int));

    // initialize state id
    hmm->id = (int *)malloc(NUM_STATE * sizeof(int));
    for (int state = 0; state < NUM_STATE; state++)
    {
        hmm->id[state] = state;
    }

    // initialize transition mat
    hmm->transition_mat = (double **)malloc(NUM_STATE * sizeof(double*));
    hmm->transition_mat_rev = (double **)malloc(NUM_STATE * sizeof(double*));
    for (int state_id =0; state_id < NUM_STATE; state_id++)
    {
        hmm->transition_mat[state_id] = (double *)malloc(NUM_STATE * sizeof(double));
        hmm->transition_mat_rev[state_id] = (double *)malloc(NUM_STATE * sizeof(double));

        for(int st = 0; st < NUM_STATE; st++)
        {
            hmm->transition_mat[state_id][st] = 0.5 / (double) NUM_STATE;
            hmm->transition_mat_rev[state_id][st] = 0.5 / (double) NUM_STATE;
            if (state_id == st){
                hmm->transition_mat[state_id][st] += 0.5;
                hmm->transition_mat_rev[state_id][st] += 0.5;
            }

        }
    }

    // initialize emission_mat - uniform
    hmm->emission_mat = (double **)malloc(NUM_STATE * sizeof(double*));
    for (int state_id = 0; state_id < NUM_STATE; state_id++)
    {
        hmm->emission_mat[state_id] = (double *)malloc(NUM_SYMBOL * sizeof(double));

        for(int symbol = 0; symbol < NUM_SYMBOL; symbol++)
        {
            hmm->emission_mat[state_id][symbol] = 1.0 / (double) symbol_cnt;
        }
    }


    /* initialise model */
    for (int state_id = 0; state_id < NUM_STATE; state_id++)
    {
        hmm->states[state_id].pi              = (state_id == 0) ? 1.0 : 0;
        hmm->states[state_id].b               = hmm->emission_mat[state_id];
        hmm->states[state_id].out_states      = NUM_STATE;
        hmm->states[state_id].out_a           = hmm->transition_mat[state_id];
        hmm->states[state_id].out_id          = hmm->id;
        hmm->states[state_id].in_states       = NUM_STATE;
        hmm->states[state_id].in_a            = hmm->transition_mat_rev[state_id];
        hmm->states[state_id].in_id           = hmm->id;
        hmm->states[state_id].fix             = 0;
    }


    /* setup model */
    hmm->model.model_type = 0;
    hmm->model.N = state_cnt;
    hmm->model.M = symbol_cnt;
    hmm->model.s = hmm->states;
    hmm->model.prior = -1;
    hmm->model.silent = hmm->silent_array;

    return hmm;

}


void free_hmm_model(Hmm_Model* hmm)
{


    for(int state_id = 0; state_id < hmm->model.N; state_id++){
        free(hmm->transition_mat[state_id]);
        free(hmm->transition_mat_rev[state_id]);
        free(hmm->emission_mat[state_id]);
    }

    free(hmm->transition_mat);
    free(hmm->transition_mat_rev);
    free(hmm->emission_mat);
    free(hmm->states);
    free(hmm->silent_array);
    free(hmm->id);

    free(hmm);

}

int test_model()
{
    const int NUM_STATES = 4;
    const int NUM_SYMBOLS = 27;

    int* seqs[10];
    int seqs_length[10];
//    seqs[0] = seq0;
    seqs[1] = seq1;
    seqs[2] = seq2;
    seqs[3] = seq3;
    seqs[4] = seq4;
    seqs[5] = seq5;
//    seqs[6] = seq6;
//    seqs[7] = seq7;
//    seqs[8] = seq8;
//    seqs[9] = seq9;

    int max_length = 0;
//    seqs_length[0] = sizeof(seq0) / sizeof(int);
    seqs_length[1] = sizeof(seq1) / sizeof(int);
    max_length = (seqs_length[1] > max_length) ? seqs_length[1] : max_length;
    seqs_length[2] = sizeof(seq2) / sizeof(int);
    max_length = (seqs_length[2] > max_length) ? seqs_length[2] : max_length;

    DBG("size of seq2 : %d\n", sizeof(seq2));
    seqs_length[3] = sizeof(seq3) / sizeof(int);
    max_length = (seqs_length[3] > max_length) ? seqs_length[3] : max_length;
    seqs_length[4] = sizeof(seq4) / sizeof(int);
    max_length = (seqs_length[4] > max_length) ? seqs_length[4] : max_length;
    seqs_length[5] = sizeof(seq5) / sizeof(int);
    max_length = (seqs_length[5] > max_length) ? seqs_length[5] : max_length;


//    seqs_length[6] = sizeof(seq6) / sizeof(int);
//    seqs_length[7] = sizeof(seq7) / sizeof(int);
//    seqs_length[8] = sizeof(seq8) / sizeof(int);
//    seqs_length[9] = sizeof(seq9) / sizeof(int);




    Hmm_Model* hmm_up = create_hmm_model(NUM_STATES, NUM_SYMBOLS);

    DBG("transition matrix:\n");
    ghmm_dmodel_A_print(stdout, &hmm_up->model, "", " ", "\n");
    DBG("observation symbol matrix:\n");
    ghmm_dmodel_B_print(stdout, &hmm_up->model, "", " ", "\n");

//    ghmm_dseq_print(my_output, stdout);

    ghmm_dseq *my_output = ghmm_dmodel_generate_sequences(&hmm_up->model, 1, max_length, 5, max_length);

    for (int i=0; i<5; i++)
    {
        int temp[max_length];
        memcpy(temp, seqs[i+1], sizeof(int)*seqs_length[i+1]);
        if( seqs_length[i+1] < max_length)
        {
            for (int j = seqs_length[i+1] ; j < max_length; j++)
                temp[j] = 26;
        }

        ghmm_dseq_copy(my_output->seq[i], temp, max_length);

    }

    hmm_evaluate(hmm_up, my_output);

    if(ghmm_dmodel_baum_welch(&hmm_up->model, my_output)) {
        DBG( "HMM error! \n");
    }
    DBG("HMM transition matrix:\n");
    ghmm_dmodel_A_print(stdout, &hmm_up->model, "", " ", "\n");
    DBG("HMM observation symbol matrix:\n");
    ghmm_dmodel_B_print(stdout, &hmm_up->model, "", " ", "\n");

    hmm_evaluate(hmm_up, my_output);

    ghmm_dseq_free(&my_output);
//    ghmm_dseq_print(my_output, stdout);



//    printf("after learning -> log_p_forward: %f\n", log_p_forward);

    save_model(hmm_up, "hmm_up");

    free_hmm_model(hmm_up);

    HMM_MODEL_MAX_LENGTH = max_length;

/*
    // read!
    Hmm_Model* read_model = read_model_from_file("hmm_up");

    ghmm_dseq *test_output = ghmm_dmodel_generate_sequences(&hmm_up->model, 1, 10, 1, 100);
    ghmm_dseq_print(test_output, stdout);
    int* new_seq = (int*)malloc(10 * sizeof(int));
    for(int i=0; i<10; i++) new_seq[i] = 0;
    ghmm_dseq_copy(test_output->seq[0], new_seq, 10);
    ghmm_dseq_print(test_output, stdout);

    if(ghmm_dmodel_baum_welch(&read_model->model, test_output)) {
        fprintf(stdout, "error! \n");
    }
    fprintf(stdout,"transition matrix:\n");
    ghmm_dmodel_A_print(stdout, &read_model->model, "", " ", "\n");
    fprintf(stdout,"observation symbol matrix:\n");
    ghmm_dmodel_B_print(stdout, &read_model->model, "", " ", "\n");

    free_hmm_model(read_model);
    ghmm_dseq_free(&test_output);
*/

    return 0;
}


double hmm_evaluate(Hmm_Model* hmm, ghmm_dseq* s)
{
    double **forward_alpha;
    double forward_scale[s->seq_len[0]];
    double log_p_forward;

    /* allocate matrix for forward algorithm */
    DBG("HMM applying forward algorithm to the sequence...");
    forward_alpha = ighmm_cmatrix_stat_alloc(s->seq_len[0], hmm->model.N);
    if (forward_alpha==NULL)
    {
        DBG("\nHMM could not alloc forward_alpha matrix\n");
        return 1.0;
    }

    // run ghmm_dmodel_forward
    if (ghmm_dmodel_forward(&hmm->model,
                            s->seq[0],
                            s->seq_len[0],
                            forward_alpha,
                            forward_scale,
                            &log_p_forward))
    {
        DBG(" HMM ghmm_dmodel_logp failed!");
        ighmm_cmatrix_stat_free(&forward_alpha);
        return 1;
    }

    DBG("HMM before learning -> log_p_forward: %f\n", log_p_forward);

    ighmm_cmatrix_stat_free(&forward_alpha);
    return log_p_forward;
}

void save_model(Hmm_Model* hmm, const char* file_name)
{
    FILE *fp;

    char *data_path = app_get_data_path();
    char path[128];

    // print whole model
    snprintf(path, sizeof(path), "%s%s", data_path, file_name);
    fp = fopen(path, "wr");
    ghmm_dmodel_print(fp, &hmm->model);
    fclose(fp);

    // print transition matrix
    snprintf(path, sizeof(path), "%s%s%s", data_path, file_name, "_tran");
    fp = fopen(path, "wr");
    for (int state_id = 0; state_id < hmm->model.N; state_id++)
    {
        for (int state = 0; state < hmm->model.N; state++)
        {
            fprintf(fp, "%f,", hmm->states[state_id].out_a[state]);
        }
        fseek(fp, -1, SEEK_CUR);
        fprintf(fp, "\r\n");
    }
    fclose(fp);

    // print emission matrix
    snprintf(path, sizeof(path), "%s%s%s", data_path, file_name, "_emis");
    fp = fopen(path, "wr");
    for (int state_id = 0; state_id < hmm->model.N; state_id++)
    {
        for (int symbol = 0; symbol < hmm->model.M; symbol++)
        {
            fprintf(fp, "%f,", hmm->emission_mat[state_id][symbol]);
        }
        fseek(fp, -1, SEEK_CUR);
        fprintf(fp, "\r\n");
    }
    fclose(fp);

    free(data_path);
}


Hmm_Model* read_model_from_file(const char* file_name)
{
    FILE *fp;
    char *data_path = app_get_data_path();
    char path[128];
    snprintf(path, sizeof(path), "%s%s", data_path, file_name);
    fp = fopen(path, "r");

    if (fp == NULL) {
        DBG("error! can not open the file '%s' in %s\n", file_name, __func__);
        return NULL;
    }

    ssize_t read;
    char *line = NULL;
    size_t len = 0;
    const char delim[] = "\t ,=;{}";


    // Get state cnt and symbol cnt
    int symbol_cnt = 0, state_cnt = 0;
    while((read = getline(&line, &len, fp)) != -1) {
        char *word, *wordPtr;
        word = strtok_r(line, delim, &wordPtr);

        if(strcmp(word, "N") == 0) {
            word = strtok_r(NULL, delim, &wordPtr);
            state_cnt = atoi(word);
        }

        else if(strcmp(word, "M") == 0) {
            word = strtok_r(NULL, delim, &wordPtr);
            symbol_cnt = atoi(word);
        }
    }
    fclose(fp);

    Hmm_Model* hmm = create_hmm_model(state_cnt, symbol_cnt);
    if(!hmm) {
        DBG("error! read_model_from_file");
        return NULL;
    }



    // read transition matrix
    snprintf(path, sizeof(path), "%s%s%s", data_path, file_name, "_tran");
    fp = fopen(path, "r");

    int  state_id = 0;
    while((read = getline(&line, &len, fp)) != -1) {
        char *word, *wordPtr;
        int  state = 0;
        word = strtok_r(line, delim, &wordPtr);
        hmm->model.s[state_id].in_a[state] = atof(word);
        hmm->model.s[state_id].out_a[state] = atof(word);
        for(state = 1; state < hmm->model.N; state++)
        {
            word = strtok_r(NULL, delim, &wordPtr);
            hmm->model.s[state_id].in_a[state] = atof(word);
            hmm->model.s[state_id].out_a[state] = atof(word);
        }

        state_id++;

    }
    fclose(fp);


    // read emission matrix
    state_id = 0;
    snprintf(path, sizeof(path), "%s%s%s", data_path, file_name, "_emis");
    fp = fopen(path, "r");
    while((read = getline(&line, &len, fp)) != -1) {
        char *word, *wordPtr;
        int symbol = 0;
        word = strtok_r(line, delim, &wordPtr);
        hmm->model.s[state_id].b[symbol] = atof(word);

        for(symbol = 1; symbol < hmm->model.M; symbol++)
        {
            word = strtok_r(NULL, delim, &wordPtr);
            hmm->model.s[state_id].b[symbol] = atof(word);
        }

        state_id++;
    }

    fclose(fp);

    if (line)
        free(line);

    free(data_path);

    ghmm_dmodel_print(stdout, &hmm->model);

    return hmm;

}

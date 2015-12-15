//
// Created by hobbang5 on 2015-12-15.
//
#include <stdlib.h>

#include <ghmm/matrix.h>
#include <ghmm/rng.h>
#include <ghmm/sequence.h>
#include <ghmm/model.h>
#include <ghmm/viterbi.h>
#include <ghmm/foba.h>

#include <ghmm/obsolete.h>
#include <ghmm/reestimate.h>

#include "hmm_model.h"

void init_hmm()
{
    /* Important! initialise rng  */
    ghmm_rng_init();
}


Hmm_Model* create_hmm_model(const int state_cnt, const int symbol_cnt)
{
    const int NUM_STATE = state_cnt;
    const int NUM_SYMBOL = symbol_cnt;

    Hmm_Model* hmm = (Hmm_Model*)calloc(1, sizeof(Hmm_Model));
    hmm->state_count = state_cnt;

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

    free(hmm->states);
    free(hmm->silent_array);
    free(hmm->id);

    for(int state_id = 0; state_id < hmm->state_count; state_id++){
        free(hmm->transition_mat[state_id]);
        free(hmm->transition_mat_rev[state_id]);
        free(hmm->emission_mat[state_id]);
    }

    free(hmm->transition_mat);
    free(hmm->transition_mat_rev);
    free(hmm->emission_mat);

}

int test_model()
{
    const int NUM_STATES = 4;
    const int NUM_SYMBOLS = 10;

    Hmm_Model* hmm_up = create_hmm_model(NUM_STATES, NUM_SYMBOLS);

    fprintf(stdout,"transition matrix:\n");
    ghmm_dmodel_A_print(stdout, &hmm_up->model, "", " ", "\n");
    fprintf(stdout,"observation symbol matrix:\n");
    ghmm_dmodel_B_print(stdout, &hmm_up->model, "", " ", "\n");

    ghmm_dseq *my_output = ghmm_dmodel_generate_sequences(&hmm_up->model, 0, 20, 10, 100);
    ghmm_dseq_print(my_output, stdout);

    double **forward_alpha;
    double forward_scale[my_output->seq_len[0]];
    double log_p_forward;

    /* allocate matrix for forward algorithm */
    fprintf(stdout,"applying forward algorithm to the sequence...");
    forward_alpha = ighmm_cmatrix_stat_alloc(my_output->seq_len[0], NUM_STATES);
    if (forward_alpha==NULL)
    {
        fprintf(stdout,"\n could not alloc forward_alpha matrix\n");
        return 1;
    }

    // run ghmm_dmodel_forward
    if (ghmm_dmodel_forward(&hmm_up->model,
                            my_output->seq[0],
                            my_output->seq_len[0],
                            forward_alpha,
                            forward_scale,
                            &log_p_forward))
    {
        fprintf(stdout,"ghmm_dmodel_logp failed!");
        ighmm_cmatrix_stat_free(&forward_alpha);
        return 1;
    }

    printf("before learning -> log_p_forward: %f\n", log_p_forward);




    if(ghmm_dmodel_baum_welch(&hmm_up->model, my_output)) {
        fprintf(stdout, "error! \n");
    }
    fprintf(stdout,"transition matrix:\n");
    ghmm_dmodel_A_print(stdout, &hmm_up->model, "", " ", "\n");
    fprintf(stdout,"observation symbol matrix:\n");
    ghmm_dmodel_B_print(stdout, &hmm_up->model, "", " ", "\n");



    if (ghmm_dmodel_forward(&hmm_up->model,
                            my_output->seq[0],
                            my_output->seq_len[0],
                            forward_alpha,
                            forward_scale,
                            &log_p_forward))
    {
        fprintf(stdout,"ghmm_dmodel_logp failed!");
        ighmm_cmatrix_stat_free(&forward_alpha);
        return 1;
    }

    printf("after learning -> log_p_forward: %f\n", log_p_forward);

    ighmm_cmatrix_stat_free(&forward_alpha);
    ghmm_dseq_free(&my_output);
    free(hmm_up);

    return 0;
}
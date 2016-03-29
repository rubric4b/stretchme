//
// Created by hobbang5 on 2016-03-22.
//

#ifndef __HMM_MODEL_H__
#define __HMM_MODEL_H__

#include <vector>

#include "xmm/models/hmm.h"
#include "glm/glm.hpp"

class Hmm_Model {

public:
    // constructor
    Hmm_Model();

    // destructor
    virtual ~Hmm_Model();

    // copy constructor
    Hmm_Model(const Hmm_Model &src);

    // load model from file which is parsed with json
//    virtual bool load_File() = 0;

    // save model to file which is parsed with json
//    virtual bool save_File() = 0;

    // get probability of model : interface function
    double get_Probability();
    virtual double get_Probability_child() = 0;

    // update observation : interface functiib
    double perform_Stretching(const glm::vec3 &curr_observation);
    virtual double perform_Stretching_child(const glm::vec3 &curr_observation) = 0;

    // reset model performing : interface function
    bool reset();
    virtual bool reset_child() = 0;

    // get flag end of performing
    bool is_PerformingDone();
    virtual bool is_PerformingDone_child() = 0;

    // initialize model, this method should be called from child class
    bool init_Hmm(unsigned int nbState, unsigned int tsDim, double threshold);

protected:
    // the number of transition states
    unsigned int m_nbState;

    // the number of dimensions of training sets
    unsigned int m_tsDim;

    // the probability which can be recognized as stretching motion
    double m_threshold;

private:
    // flag for init
    bool m_isInit;


public:
    // inline functions
    // getter and setter
    unsigned int get_NbState() { return m_nbState; }
    void set_NbState(unsigned int num) { m_nbState = num; }

    double get_Threshold() { return m_threshold; }
    void set_Threshold(double th) { m_threshold = th; }

    unsigned int get_TsDimension() { return m_tsDim; }
    void set_TsDimentsion(unsigned int dim) { m_tsDim = dim; }

    // get flag for initialize
    bool get_Init() { return m_isInit; }

};


#endif //__HMM_MODEL_H__

//
// Created by hobbang5 on 2016-03-23.
//

#ifndef __FILE_HANDLER_H__
#define __FILE_HANDLER_H__

#include <string>

#include "xmm/xmm.h"
#include "hmm_analyzer.h"

typedef enum {
    DATA_PATH,
    SHARED_DATA_PATH,
    RES_PATH,
    USER_PATH
}PathType;

bool write_hmm_to_file(const std::string &file_name, const xmm::HMM &model);

bool read_hmm_from_file(const std::string &file_name, xmm::HMM &model);

void record_training_set_from_file(Hmm_Analyzer *analyzer, const std::string &file_name, PathType type, int ts_phrase_index,
								   int ts_dim, xmm::TrainingSet &ts);

void set_observ_from_file(Hmm_Analyzer *analyzer, const std::string &file_name, PathType type);

#endif //__FILE_HANDLER_H__

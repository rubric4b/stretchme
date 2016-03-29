//
// Created by hobbang5 on 2016-03-23.
//

#ifndef __FILE_HANDLER_H__
#define __FILE_HANDLER_H__

#include <string>

#include "xmm/xmm.h"

bool write_hmm_to_file(const std::string &file_name, const xmm::HMM &model);

bool read_hmm_from_file(const std::string &file_name, xmm::HMM &model);

void record_training_set_from_file(const std::string &file_name, int ts_phrase_index, xmm::TrainingSet &ts);

#endif //__FILE_HANDLER_H__
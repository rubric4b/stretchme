//
// Created by hobbang5 on 2016-03-23.
//

#ifndef __FILE_HANDLER_H__
#define __FILE_HANDLER_H__

#include <string>

#include "xmm/xmm.h"

void record_training_set_from_file(xmm::TrainingSet& ts, int index, const std::string& file_name);

#endif //__FILE_HANDLER_H__

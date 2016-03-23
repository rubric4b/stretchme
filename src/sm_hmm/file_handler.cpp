//
// Created by hobbang5 on 2016-03-23.
//


#include <app.h>

#include "sm_hmm/file_handler.h"
#include "sm_hmm/hmm_analyzer_armup.h"
#include "sm_hmm/hmm_model_armup.h"
#include "logger.h"

static char* util_strtok(char* str, const char* delim, char** nextp)
{
    char* ret;

    if (str == NULL) {
        str = *nextp;
    }

    str += strspn(str, delim);

    if (*str == '\0') {
        return NULL;
    }

    ret = str;

    str += strcspn(str, delim);

    if (*str) {
        *str++ = '\0';
    }

    *nextp = str;

    return ret;
}



void record_training_set_from_file(xmm::TrainingSet& ts, int index, const std::string& file_name)
{
    //buffer
    std::ifstream in_file;

    // file path
    std::string file_path = app_get_resource_path() + file_name;

    // file open
    in_file.open(file_path.c_str(), std::ios::in | std::ios::binary);
    if(!in_file.is_open() || !in_file.good()) {
        std::string msg = "Failed to open file " + file_path;
        throw std::runtime_error(msg.c_str());
    }

    // record file to training set
    int line_cnt(0), ts_cnt(0);
    char line_buffer[512];
    std::vector<float> curr_observation(3);
    std::vector<float> observation(Hmm_ArmUp::ARM_UP_TS_DIMENSION);
    HA_ArmUp analyzer = HA_ArmUp();
    while (!in_file.getline(line_buffer, sizeof(line_buffer)).eof()) {
        char *word, *wordPtr;
        word = util_strtok(line_buffer, ",", &wordPtr); // time
        word = util_strtok(NULL, ",", &wordPtr); // accel - x
        curr_observation[0] = atof(word);
        word = util_strtok(NULL, ",", &wordPtr); // accel - y
        curr_observation[1] = atof(word);
        word = util_strtok(NULL, ",", &wordPtr); // accel - z
        curr_observation[2] = atof(word);

        if( analyzer.get_Observation(curr_observation, observation) ) {

            ts.recordPhrase(index, observation);
            ts_cnt++;
        }

        line_cnt++;
    }
    in_file.close();

    DBG("%s : %d lines are read", file_name.c_str(), line_cnt);
    DBG("%s : %d observation are recorded to training set", file_name.c_str(), ts_cnt);

}
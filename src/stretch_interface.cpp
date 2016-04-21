//
// Created by hobbang5 on 2016-03-28.
//

#include <fstream>
#include <sstream>

#include "stretch_interface.h"

#include "stretch_manager.h"
#include "sm_hmm/hmm_manager.h"
#include "sm_view.h"
#include "sm_popup.h"


#define sMgr stretch_Manager::Inst()
#define hMgr Hmm_Manager::Inst()
#define TRAINING_FILE_PATH "/opt/usr/media/"


sm_Sensor *accel = NULL;

void stretch_manager_release() {
    delete accel;
    sMgr.release();
}

void stretching_start(StretchConfig conf, Stretching_Result_Cb func, void* data) {
    if(accel) accel->stop();

    sMgr.start(conf, func, data);
}

void stretching_stop() {
    sMgr.stop();
}


void
auto_start_cb(sensor_h sensor, sensor_event_s *event, void *data) {

    static int mov_cnt(0);

    accel->m_prevData = accel->m_currData;
    accel->m_prevKData = accel->m_currKData;

    accel->m_timestamp = (unsigned int)(event->timestamp/1000 - accel->m_initTime);
    accel->m_currData = glm::vec3(event->values[0], event->values[1], event->values[2]);
    accel->m_kFilter.Step(accel->m_currData, accel->m_currKData);

    // initialize init time
    if(accel->m_initTime == 0) {
        accel->m_initTime = event->timestamp / 1000;

        return;
    }

    glm::vec3 diff_accel = accel->m_currKData - accel->m_prevKData;
    double diff_len = length(diff_accel);
    DBG("diff_accel : %f, %f, %f, len : %f\n", diff_accel.x, diff_accel.y, diff_accel.z, diff_len);

    if(diff_len > 1.0) {
        mov_cnt++;
//        prev_stamp = accel->m_timestamp;
//        accel->m_prevData = accel->m_currData;
    } else{
        mov_cnt = 0;
    }

    if(mov_cnt > 3) {
        // moving
        mov_cnt = 0;
        accel->stop();
        Start_Stretch_cb(data, NULL, NULL);
    }

}

void auto_start_stretch(void *data) {
    if(!accel) {
        accel = new sm_Sensor(SENSOR_ACCELEROMETER, auto_start_cb, data, 50);
    }

    accel->start();
}


void
data_gathering_cb(sensor_h sensor, sensor_event_s *event, void *data) {
    // out file stream
    static std::ofstream out_fstream;

    // file data path
    static std::string file_path;

    appdata_s *ad = (appdata_s *)data;
    Elm_Object_Item *nf_it;

    // initialize init time
    if(accel->m_initTime == 0) {
        std::ostringstream file_name;
        file_name << "training_data_" << ad->training_cnt << ".csv";
        file_path = TRAINING_FILE_PATH + file_name.str();

        out_fstream.open(file_path.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
        if(!out_fstream.is_open() || !out_fstream.good()) {
            std::string msg = "Failed to open file " + file_path;
            ERR("%s\n",msg.c_str());
        }else{
            std::string msg = "Success to open file " + file_path;
            DBG("%s\n",msg.c_str());
        }

        accel->m_initTime = event->timestamp / 1000;
    }

    accel->m_timestamp = (unsigned int)(event->timestamp/1000 - accel->m_initTime);

    std::ostringstream line;
    line << accel->m_timestamp << ","
        << event->values[0] << ","
        << event->values[1] << ","
        << event->values[2] << std::endl;

    DBG("%s",line.str().c_str());
    out_fstream << line.str();

    if(accel->m_timestamp > 5000) {
        DBG("data_gathering_cb end!\n");
        out_fstream.close();
        accel->stop();

        // disable app exit
        nf_it = elm_naviframe_top_item_get(ad->nf);
        elm_naviframe_item_pop_cb_set(nf_it, NULL, NULL);

        // goto first naviframe
        nf_it = elm_naviframe_bottom_item_get(ad->nf);
        elm_naviframe_item_pop_to(nf_it);

        popup_data_confirm_cb(data, NULL, NULL);
    }

}

void streching_date_gathering(void *data) {
    if(!accel) {
        accel = new sm_Sensor(SENSOR_ACCELEROMETER, data_gathering_cb, data, 10);
    }

    accel->start();
}

bool retraining_model(StretchType type) {
    return hMgr.retrain_Model(type);
}
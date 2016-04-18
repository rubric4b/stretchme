//
// Created by hobbang5 on 2016-03-28.
//
#include "sm_view.h"

#include "stretch_interface.h"

#include "stretch_manager.h"


#define sMgr stretch_Manager::Inst()


sm_Sensor *accel = NULL;

void stretch_manager_release() {
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

    // initialize init time
    if(accel->m_initTime == 0) {
        accel->m_initTime = event->timestamp / 1000;
        accel->m_prevData = glm::vec3(event->values[0], event->values[1], event->values[2]);
    }

    accel->m_timestamp = (unsigned int)(event->timestamp/1000 - accel->m_initTime);
    accel->m_currData = glm::vec3(event->values[0], event->values[1], event->values[2]);

    double diff_len = length(accel->m_currData - accel->m_prevData);

    if(diff_len > 0.3) {
        mov_cnt++;
        accel->m_prevData = accel->m_currData;
    }

    if(mov_cnt > 30) {
        // moving
        mov_cnt = 0;
        accel->stop();
        Start_Stretch_cb(data, NULL, NULL);
    }

}

void auto_start_stretch(void *data) {
    if(!accel) {
        accel = new sm_Sensor(SENSOR_ACCELEROMETER, auto_start_cb, data);
    }

    accel->start();

}
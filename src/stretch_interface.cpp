//
// Created by hobbang5 on 2016-03-28.
//

#include "stretch_interface.h"

#include "stretch_manager.h"

#define sMgr stretch_Manager::Inst()

void stretch_manager_release() {
    sMgr.release();
}

void stretching_start(StretchType type, StretchState state, Stretching_Result_Cb func, void* data) {
    sMgr.start(type, state, func, data);
}

void stretching_stop() {
    sMgr.stop();
}
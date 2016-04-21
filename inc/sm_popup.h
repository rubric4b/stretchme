//
// Created by hobbang5 on 2016-04-20.
//

#ifndef STRETCHME_SM_POPUP_H
#define STRETCHME_SM_POPUP_H

#include "main.h"
#include "logger.h"

#ifdef __cplusplus
extern "C" {
#endif

void popup_training_cb(void *data, Evas_Object *obj, void *event_info);
void popup_data_confirm_cb(void *data, Evas_Object *obj, void *event_info);
void popup_small_process_cb(void *data, Evas_Object *obj, void *event_info);
void popup_training_done_cb(void *data, Evas_Object *obj, void *event_info);

#ifdef __cplusplus
}
#endif


#endif //STRETCHME_SM_POPUP_H

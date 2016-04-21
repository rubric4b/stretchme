#ifndef __STRETCHME_VIEW_H__
#define __STRETCHME_VIEW_H__

#include "main.h"
#include "logger.h"

void
create_main_view(appdata_s *ad);

#ifdef __cplusplus
extern "C" {
#endif

void Start_Stretch_cb(void *data, Evas_Object *obj, void *event_info);
void Strecth_Guide_cb(void *data, Evas_Object *obj, void *event_info);
void Model_Retraining_cb(void *data, Evas_Object *obj, void *event_info);

#ifdef __cplusplus
}
#endif

#endif // __STRETCHME_VIEW_H__



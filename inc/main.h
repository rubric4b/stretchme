/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __STRETCHME_H__
#define __STRETCHME_H__

#include <app.h>
#include <Elementary.h>
#include <system_settings.h>
#include <dlog.h>
#include <efl_extension.h>

#include "logger.h"

#if !defined(PACKAGE)
#define PACKAGE "org.tizen.stretchme"
#endif

#define STRETCHING_WATCH_APP_ID "org.tizen.stretchtime"

#define EDJ_FILE "edje/stretch_ui_layout.edj"

#define ICON_DIR "/opt/usr/apps/org.tizen.stretchme/res/images"

typedef struct appdata {
	Evas_Object *nf;
	Evas_Object *label;
	Evas_Object *datetime;
	Eext_Circle_Surface *circle_surface;
	struct tm saved_time;

	Evas_Object *popup;
	Eina_Bool is_training; // or normal stretching
	unsigned short training_cnt;
	unsigned short stretch_sequence;

	char* training_prefix;

	// view
//	Ecore_Timer * fold_timer;
//	Eina_Bool is_stretch_success;
} appdata_s;

#ifdef __cplusplus
extern "C" {
#endif

void app_get_resource(const char *edj_file_in, char *edj_path_out, int edj_path_max);
void vibrate(int duration, int feedback);

#ifdef __cplusplus
}
#endif

#endif // __STRETCHME_H__


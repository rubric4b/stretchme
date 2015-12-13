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

#include <app.h>
#include <Elementary.h>
#include <system_settings.h>
#include <dlog.h>
#include <efl_extension.h>

#include "logger.h"

#if !defined(PACKAGE)
#define PACKAGE "org.example.stretchme"
#endif

#define EDJ_FILE "edje/stretch_ui_layout.edj"
#define GRP_MAIN "main"

#define FORMAT "%d/%b/%Y%H:%M"
#define ICON_DIR "/opt/usr/apps/org.example.stretchme/res/images"


// Callback functions -------------------------------------------------------------------------------------------------
static void Start_Stretch_cb(void *data, Evas_Object *obj, void *event_info);
static void Hold_Stretch_cb(void *data, Evas_Object *obj, void *event_info);
static void Fold_Stretch_cb(void *data, Evas_Object *obj, void *event_info);

static void Success_Strecth_cb(void *data, Evas_Object *obj, void *event_info);
static void Fail_Strecth_cb(void *data, Evas_Object *obj, void *event_info);
static void Result_cb(void *data, Evas_Object *obj, void *event_info);
static void Reward_cb(void *data, Evas_Object *obj, void *event_info);
static void Strecth_Guile_cb(void *data, Evas_Object *obj, void *event_info);

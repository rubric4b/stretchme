
#define  _GNU_SOURCE

#include <app.h>
#include <app_control.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <device/haptic.h>

#include "main.h"
#include "logger.h"
#include "sm_view.h"


#include <time.h>

void
app_get_resource(const char *edj_file_in, char *edj_path_out, int edj_path_max)
{
	char *res_path = app_get_resource_path();
	if (res_path) {
		snprintf(edj_path_out, edj_path_max, "%s%s", res_path, edj_file_in);
		free(res_path);
	}
}

void vibrate(int duration, int feedback)
{
	haptic_device_h haptic_handle;
	haptic_effect_h effect_handle;

	if(device_haptic_open(0, &haptic_handle) == DEVICE_ERROR_NONE) {

		LOGI("Connection to vibrator established");

		if(device_haptic_vibrate(haptic_handle, duration, feedback, &effect_handle) == DEVICE_ERROR_NONE) {
			LOGI("Device vibrates!");
		}
	}
}

static void
win_delete_request_cb(void *data, Evas_Object *obj, void *event_info)
{
	ui_app_exit();
}

static void
create_base_gui(appdata_s *ad)
{
	/* Window */
	Evas_Object *win, *conform;
	win = elm_win_util_standard_add(PACKAGE, PACKAGE);
	elm_win_autodel_set(win, EINA_TRUE);

	evas_object_smart_callback_add(win, "delete,request", win_delete_request_cb, NULL);

	/* Conformant */
	conform = elm_conformant_add(win);

	evas_object_size_hint_weight_set(conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(win, conform);
	evas_object_show(conform);

	/* Naviframe */
	ad->nf = elm_naviframe_add(conform);
	create_main_view(ad);
	elm_object_content_set(conform, ad->nf);
	eext_object_event_callback_add(ad->nf, EEXT_CALLBACK_BACK, eext_naviframe_back_cb, NULL);

	/* Add surface for circle object */
	ad->circle_surface = eext_circle_surface_naviframe_add(ad->nf);

	/* Show window after base gui is set up */
	evas_object_show(win);
}

static bool
app_create(void *data)
{
	/* Hook to take necessary actions before main event loop starts
		Initialize UI resources and application's data
		If this function returns true, the main loop of application starts
		If this function returns false, the application is terminated */
	appdata_s *ad = data;

	create_base_gui(ad);

	return true;
}

#define DATA_FILE_PATH "/opt/usr/media/stretching_data.txt"


static bool
_app_control_extra_data_cb(app_control_h app_control, const char *key, void *user_data)
{
	int ret;
	char *value;

	ret = app_control_get_extra_data(app_control, key, &value);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		dlog_print(DLOG_ERROR, LOG_TAG, "app_control_get_extra_data() failed. err = %d", ret);
	}
	dlog_print(DLOG_DEBUG, LOG_TAG, "[key] %s, [value] %s", key, value);

	if(strcmp(key, "timestamp") == 0)
	{
		dlog_print(DLOG_DEBUG, LOG_TAG, "[key] %s, [value] %s", key, value);
		// can not convert string timestamp to time_t
	}
	else if(strcmp(key, "timeformat") == 0)
	{
		char* ret;

		// make timestamp from time formatted string
		struct tm tm;
		ret = strptime(value, "%Y-%m-%d %H:%M:%S", &tm);
		time_t timestamp = mktime(&tm);

		dlog_print(DLOG_DEBUG, LOG_TAG, "[timeforamt] %s, [timestamp] %ld", value, timestamp);
		dlog_print(DLOG_DEBUG, LOG_TAG, "[%ld] : %d, %d, %d, %d : %d : %d\n", timestamp, tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

		// get difference between current and old
		// TODO: use real data
		time_t diff = difftime(timestamp, 1453861634);
		dlog_print(DLOG_DEBUG, LOG_TAG, "[diff time] %ld", diff);

		int d_day = diff / (60 * 60 * 24);
		diff -= d_day * 60 * 60 * 24;
		int d_hour = diff / (60 * 60);
		diff -= d_hour * 60 * 60;
		int d_min = diff / 60;
		diff -= d_min * 60;
		int d_sec = diff;

		dlog_print(DLOG_DEBUG, LOG_TAG, "[Diff] : %d, (%d : %d : %d)\n", d_day, d_hour, d_min, d_sec);
	}

	return true;
}

static void
app_control(app_control_h app_control, void *data)
{
	/* Handle the launch request. */

	int ret = app_control_foreach_extra_data(app_control, _app_control_extra_data_cb, 0);
	if (ret != APP_CONTROL_ERROR_NONE)
	   dlog_print(DLOG_ERROR, LOG_TAG, "app_control_foreach_extra_data() failed. err = %d", ret);
}

static void
app_pause(void *data)
{
	/* Take necessary actions when application becomes invisible. */
}

static void
app_resume(void *data)
{
	/* Take necessary actions when application becomes visible. */
}

static void
app_terminate(void *data)
{
	/* Release all resources. */
}

static void
ui_app_lang_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LANGUAGE_CHANGED*/
	char *locale = NULL;
	system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &locale);
	elm_language_set(locale);
	free(locale);
	return;
}

static void
ui_app_orient_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_DEVICE_ORIENTATION_CHANGED*/
	return;
}

static void
ui_app_region_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_REGION_FORMAT_CHANGED*/
}

static void
ui_app_low_battery(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_BATTERY*/
}

static void
ui_app_low_memory(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_MEMORY*/
}

int
main(int argc, char *argv[])
{

//	catch_test(argc, argv);
//	init_hmm();
//	test_model();

	appdata_s ad = {0,};
	int ret = 0;


	ui_app_lifecycle_callback_s event_callback = {0,};
	app_event_handler_h handlers[5] = {NULL, };

	event_callback.create = app_create;
	event_callback.terminate = app_terminate;
	event_callback.pause = app_pause;
	event_callback.resume = app_resume;
	event_callback.app_control = app_control;

	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY], APP_EVENT_LOW_BATTERY, ui_app_low_battery, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY], APP_EVENT_LOW_MEMORY, ui_app_low_memory, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_DEVICE_ORIENTATION_CHANGED], APP_EVENT_DEVICE_ORIENTATION_CHANGED, ui_app_orient_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED], APP_EVENT_LANGUAGE_CHANGED, ui_app_lang_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED], APP_EVENT_REGION_FORMAT_CHANGED, ui_app_region_changed, &ad);
	ui_app_remove_event_handler(handlers[APP_EVENT_LOW_MEMORY]);

	ret = ui_app_main(argc, argv, &event_callback, &ad);
	if (ret != APP_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "app_main() is failed. err = %d", ret);
	}

	return ret;
}



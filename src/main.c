#include "main.h"
#include "action_icon.h"
#include "sm_sensor.h"

#include "logger.h"

typedef struct appdata {
	Evas_Object *nf;
	Evas_Object *label;
	Evas_Object *datetime;
	Eext_Circle_Surface *circle_surface;
	struct tm saved_time;

	// sensor
	sensor_info* accel_sensor;
	sensor_info* gyro_sensor;

	Eina_Bool sensing :1;
} appdata_s;

#define FORMAT "%d/%b/%Y%H:%M"


static void
win_delete_request_cb(void *data, Evas_Object *obj, void *event_info)
{
	ui_app_exit();
}

static Eina_Bool
naviframe_pop_cb(void *data, Elm_Object_Item *it)
{
	ui_app_exit();
	return EINA_FALSE;
}

static void
label_text_set(Evas_Object *label, struct tm t)
{
	char buf[200] = {0};
	char text_buf[PATH_MAX];

	//Time gets from tm struct and set as text value
	strftime(buf, sizeof(buf), "%H:%M", &t);
	snprintf(text_buf, sizeof(text_buf), "<align=center><font_size=20>%s</font_size></align>", "Stretch up your arms");
	elm_object_text_set(label, text_buf);
}

static void
sensor_control(void *data, Eina_Bool start)
{
	appdata_s *ad = data;

	static Eina_Bool initialized = EINA_FALSE;

	if(start && ad->sensing == EINA_FALSE)
	{
		// first start
		if(initialized == EINA_FALSE)
		{
			// sensor
			ad->accel_sensor = sensor_init(SENSOR_ACCELEROMETER);

			if(ad->accel_sensor)
			{
				sensor_start(ad->accel_sensor);
			}

			ad->gyro_sensor = sensor_init(SENSOR_GYROSCOPE);

			if(ad->gyro_sensor)
			{
				sensor_start(ad->gyro_sensor);
			}
			initialized = EINA_TRUE;
		}
		else
		{
			// resume
			reset_measure();

			if(ad->accel_sensor)
			{
				sensor_listen_resume(ad->accel_sensor);
			}

			if(ad->gyro_sensor)
			{
				sensor_listen_resume(ad->gyro_sensor);
			}
		}

		ad->sensing = EINA_TRUE;
	}
	else if(start == EINA_FALSE && ad->sensing)
	{
		if(initialized == EINA_FALSE)
		{
			return;
		}
		else
		{
			// pause
			if(ad->accel_sensor)
			{
				sensor_listen_pause(ad->accel_sensor);
			}

			if(ad->gyro_sensor)
			{
				sensor_listen_pause(ad->gyro_sensor);
			}
		}

		ad->sensing = EINA_FALSE;
	}
}

static void
check_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = data;

	elm_datetime_value_get(ad->datetime, &ad->saved_time);

	label_text_set(ad->label, ad->saved_time);

	//View changed to main view.
	elm_naviframe_item_pop(ad->nf);

	//SENSOR CONTROL : stop
	sensor_control(data, EINA_FALSE);
}

static void
start_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = data;
	Evas_Object *button, *icon, *layout, *circle_datetime;
	Elm_Object_Item *nf_it = NULL;

	// TODO: make GUI for stretching

	layout = elm_layout_add(ad->nf);
	elm_layout_theme_set(layout, "layout", "circle", "datetime");

	button = elm_button_add(layout);
	elm_object_style_set(button, "bottom");

	//Image add for button icon.
	icon = elm_image_add(button);
	elm_image_file_set(icon, ICON_DIR "/check.png", NULL);
	elm_object_content_set(button, icon);
	evas_object_show(icon);

	elm_object_part_content_set(layout, "elm.swallow.btn", button);
	evas_object_smart_callback_add(button, "clicked", check_button_clicked_cb, ad);

	elm_object_part_text_set(layout, "elm.text", "Set Time");

	ad->datetime = elm_datetime_add(layout);

	//eext circle datetime add for circular feature of datetime.
	circle_datetime = eext_circle_object_datetime_add(ad->datetime, ad->circle_surface);
	//eext rotary event activated to circle datetime for gets rotary event.
	eext_rotary_object_event_activated_set(circle_datetime, EINA_TRUE);

	elm_datetime_format_set(ad->datetime, FORMAT);
	elm_datetime_value_set(ad->datetime, &ad->saved_time);

	elm_object_style_set(ad->datetime, "timepicker/circle");
	elm_object_part_content_set(layout, "elm.swallow.content", ad->datetime);

	nf_it = elm_naviframe_item_push(ad->nf, "Time picker", NULL, NULL, layout, NULL);
	elm_naviframe_item_title_enabled_set(nf_it, EINA_FALSE, EINA_FALSE);


	//SENSOR CONTROL : reset & start
	sensor_control(data, EINA_TRUE);

}

static void
create_main_view(appdata_s *ad)
{
	Elm_Object_Item *nf_it;
	Evas_Object *button, *layout;
	time_t local_time = time(NULL);
	struct tm *time_info = localtime(&local_time);

	ad->saved_time = *time_info;

	layout = elm_layout_add(ad->nf);
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_layout_theme_set(layout, "layout", "bottom_button", "default");
	evas_object_show(layout);

	ad->label = elm_label_add(layout);
	evas_object_size_hint_weight_set(ad->label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

#if 0
	label_text_set(ad->label, ad->saved_time);

	evas_object_show(ad->label);
	elm_object_part_content_set(layout, "elm.swallow.content", ad->label);
#else
	Evas_Object* actionIcon;
	//Image add for stretching action icon.
	actionIcon= elm_image_add(layout);
	elm_image_file_set(actionIcon, ICON_DIR "/Up_stretching.gif", NULL);
	evas_object_show(actionIcon);
	elm_object_part_content_set(layout, "elm.swallow.content", actionIcon);

	if (elm_image_animated_available_get(actionIcon)) {
		action_icon_set(actionIcon, true);
		action_icon_play_set(actionIcon, true, true, true);
	}
	else
	{
		DBG("Animation is NOT available\n");
	}
#endif

	button = elm_button_add(layout);
	elm_object_style_set(button, "bottom");
	elm_object_text_set(button, "Start!!");
	elm_object_part_content_set(layout, "elm.swallow.button", button);
	evas_object_smart_callback_add(button, "clicked", start_button_clicked_cb, ad);
	evas_object_show(button);

	nf_it = elm_naviframe_item_push(ad->nf, "Stretch UP", NULL, NULL, layout, NULL);
	elm_naviframe_item_pop_cb_set(nf_it, naviframe_pop_cb, NULL);
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

static void
app_control(app_control_h app_control, void *data)
{
	/* Handle the launch request. */
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

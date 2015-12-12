#include "main.h"
#include "action_icon.h"
#include "logger.h"

typedef struct appdata {
	Evas_Object *nf;
	Evas_Object *label;
	Evas_Object *datetime;
	Eext_Circle_Surface *circle_surface;
	struct tm saved_time;
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

// Callbacks here -----------------------------------------------------------------------------------------------------
static void
set_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = data;

	elm_datetime_value_get(ad->datetime, &ad->saved_time);

	label_text_set(ad->label, ad->saved_time);

	//View changed to main view.
	elm_naviframe_item_pop(ad->nf);
}

// Entrance the stretching - Now testing here for adding label
static void
Start_Stretch_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = data;
	Evas_Object *layout, *Unfolding_Animation, *button;

	Elm_Object_Item *nf_it = NULL;

	layout = elm_layout_add(ad->nf);
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	//elm_layout_theme_set(layout, "layout", "bottom_button", "default");
	elm_layout_theme_set(layout, "layout", "application", "default");
	evas_object_show(layout);

	// Add animation
	Unfolding_Animation = elm_image_add(layout);
	//elm_object_style_set(Unfolding_Animation, "center");
	elm_image_file_set(Unfolding_Animation, ICON_DIR "/Unfolding_twin.gif", NULL);
	evas_object_show(Unfolding_Animation);
	elm_object_part_content_set(layout, "elm.swallow.content", Unfolding_Animation);

	// Animation availability check
	if (elm_image_animated_available_get(Unfolding_Animation)) {
		action_icon_set(Unfolding_Animation, true);
		action_icon_play_set(Unfolding_Animation, true, true, true);
	}
	else
	{
		DBG("Animation is NOT available\n");
	}

	// Add label
	Evas_Object* lab;
	lab = elm_label_add(Unfolding_Animation);
	evas_object_size_hint_weight_set(lab, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_text_set(lab, "Text Input");
	elm_object_part_content_set(Unfolding_Animation, NULL, lab);
	evas_object_show(lab);

	// Add button
	button = elm_button_add(layout);
	elm_object_style_set(button, "bottom");
	elm_object_text_set(button, "Testing!");
	elm_object_part_content_set(layout, "elm.swallow.button", button);

	evas_object_smart_callback_add(Unfolding_Animation, "clicked", Hold_Stretch_cb, ad);
	evas_object_show(button);

	nf_it = elm_naviframe_item_push(ad->nf, "Unfolding", NULL, NULL, layout, NULL);
	elm_naviframe_item_title_enabled_set(nf_it, false, true);
	//elm_naviframe_item_pop_cb_set(nf_it, naviframe_pop_cb, NULL);
}

// Peak of the stretching - Holding posture
static void
Hold_Stretch_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = data;
	Evas_Object *layout, *Hold_Animation, *button;
	Elm_Object_Item *nf_it = NULL;

	layout = elm_layout_add(ad->nf);
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_layout_theme_set(layout, "layout", "bottom_button", "default");
	evas_object_show(layout);

	// Add animation
	Hold_Animation = elm_image_add(layout);
	elm_object_style_set(Hold_Animation, "center");
	elm_image_file_set(Hold_Animation, ICON_DIR "/Blink.gif", NULL);
	evas_object_show(Hold_Animation);
	elm_object_part_content_set(layout, "elm.swallow.content", Hold_Animation);

	// Animation availability check
	if (elm_image_animated_available_get(Hold_Animation)) {
		action_icon_set(Hold_Animation, true);
		action_icon_play_set(Hold_Animation, true, true, true);
	}
	else
	{
		DBG("Animation is NOT available\n");
	}

	// Add button
	button = elm_button_add(layout);
	elm_object_style_set(button, "bottom");

	elm_object_text_set(button, "Holding!");
	elm_object_part_content_set(layout, "elm.swallow.button", button);
	evas_object_smart_callback_add(button, "clicked", Fold_Stretch_cb, ad);
	evas_object_show(button);

	nf_it = elm_naviframe_item_push(ad->nf, "Holding", NULL, NULL, layout, NULL);
	elm_naviframe_item_title_enabled_set(nf_it, false, true);
	//elm_naviframe_item_pop_cb_set(nf_it, naviframe_pop_cb, NULL);
}

// Folding step of the stretching
static void
Fold_Stretch_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = data;
	Evas_Object *layout, *Fold_Animation, *button;
	Elm_Object_Item *nf_it = NULL;

	layout = elm_layout_add(ad->nf);
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_layout_theme_set(layout, "layout", "bottom_button", "default");
	evas_object_show(layout);

	// Add animation
	Fold_Animation = elm_image_add(layout);
	elm_object_style_set(Fold_Animation, "center");
	elm_image_file_set(Fold_Animation, ICON_DIR "/Folding_bg.gif", NULL);
	evas_object_show(Fold_Animation);
	elm_object_part_content_set(layout, "elm.swallow.content", Fold_Animation);

	// Animation availability check
	if (elm_image_animated_available_get(Fold_Animation)) {
		action_icon_set(Fold_Animation, true);
		action_icon_play_set(Fold_Animation, true, true, true);
	}
	else
	{
		DBG("Animation is NOT available\n");
	}

	// Add button
	button = elm_button_add(layout);
	elm_object_style_set(button, "bottom");

	elm_object_text_set(button, "Folding");
	elm_object_part_content_set(layout, "elm.swallow.button", button);
	evas_object_smart_callback_add(button, "clicked", Success_Strecth_cb, ad);
	evas_object_show(button);

	nf_it = elm_naviframe_item_push(ad->nf, "Holding", NULL, NULL, layout, NULL);
	elm_naviframe_item_title_enabled_set(nf_it, false, true);
	//elm_naviframe_item_pop_cb_set(nf_it, naviframe_pop_cb, NULL);
}

// Success
static void
Success_Strecth_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = data;
	Evas_Object *layout, *Success_image, *button;
	Elm_Object_Item *nf_it = NULL;

	layout = elm_layout_add(ad->nf);
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_layout_theme_set(layout, "layout", "bottom_button", "default");
	evas_object_show(layout);

	// Add animation
	Success_image = elm_image_add(layout);
	elm_object_style_set(Success_image, "center");
	elm_image_file_set(Success_image, ICON_DIR "/Success.png", NULL);
	evas_object_show(Success_image);
	elm_object_part_content_set(layout, "elm.swallow.content", Success_image);

	// Add button
	button = elm_button_add(layout);
	elm_object_style_set(button, "bottom");

	elm_object_text_set(button, "한 번 더 하기");
	elm_object_part_content_set(layout, "elm.swallow.button", button);
	evas_object_smart_callback_add(button, "clicked", Fail_Strecth_cb, ad);
	evas_object_show(button);

	nf_it = elm_naviframe_item_push(ad->nf, "한 번 더 하기", NULL, NULL, layout, NULL);
	elm_naviframe_item_title_enabled_set(nf_it, false, true);
	//elm_naviframe_item_pop_cb_set(nf_it, naviframe_pop_cb, NULL);
}

// Fail
static void
Fail_Strecth_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = data;
	Evas_Object *layout, *Fail_image, *button;
	Elm_Object_Item *nf_it = NULL;

	layout = elm_layout_add(ad->nf);
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_layout_theme_set(layout, "layout", "bottom_button", "default");
	evas_object_show(layout);

	// Add animation
	Fail_image = elm_image_add(layout);
	elm_object_style_set(Fail_image, "center");
	elm_image_file_set(Fail_image, ICON_DIR "/Fail.png", NULL);
	evas_object_show(Fail_image);
	elm_object_part_content_set(layout, "elm.swallow.content", Fail_image);

	// Add button
	button = elm_button_add(layout);
	elm_object_style_set(button, "bottom");

	elm_object_text_set(button, "다시 시도");
	elm_object_part_content_set(layout, "elm.swallow.button", button);
	evas_object_smart_callback_add(button, "clicked", Result_cb, ad);
	evas_object_show(button);

	nf_it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, layout, NULL);
	elm_naviframe_item_title_enabled_set(nf_it, false, true);
	//elm_naviframe_item_pop_cb_set(nf_it, naviframe_pop_cb, NULL);
}

// Result
static void
Result_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = data;
	Evas_Object *layout, *Result_image, *button;
	Elm_Object_Item *nf_it = NULL;

	layout = elm_layout_add(ad->nf);
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_layout_theme_set(layout, "layout", "bottom_button", "default");
	evas_object_show(layout);

	// Add animation
	Result_image = elm_image_add(layout);
	elm_object_style_set(Result_image, "center");
	elm_image_file_set(Result_image, ICON_DIR "/Result.png", NULL);
	evas_object_show(Result_image);
	elm_object_part_content_set(layout, "elm.swallow.content", Result_image);

	// Add button
	button = elm_button_add(layout);
	elm_object_style_set(button, "bottom");

	elm_object_text_set(button, "종료");
	elm_object_part_content_set(layout, "elm.swallow.button", button);
	evas_object_smart_callback_add(button, "clicked", Reward_cb, ad);
	evas_object_show(button);

	nf_it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, layout, NULL);
	elm_naviframe_item_title_enabled_set(nf_it, false, true);
	//elm_naviframe_item_pop_cb_set(nf_it, naviframe_pop_cb, NULL);
}

// Reward
static void
Reward_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = data;
	Evas_Object *layout, *Reward_image, *button;
	Elm_Object_Item *nf_it = NULL;

	layout = elm_layout_add(ad->nf);
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_layout_theme_set(layout, "layout", "bottom_button", "default");
	evas_object_show(layout);

	// Add animation
	Reward_image = elm_image_add(layout);
	elm_object_style_set(Reward_image, "center");
	elm_image_file_set(Reward_image, ICON_DIR "/Reward.png", NULL);
	evas_object_show(Reward_image);
	elm_object_part_content_set(layout, "elm.swallow.content", Reward_image);

	// Add button
	button = elm_button_add(layout);
	elm_object_style_set(button, "bottom");

	elm_object_text_set(button, "종료");
	elm_object_part_content_set(layout, "elm.swallow.button", button);
	evas_object_smart_callback_add(button, "clicked", set_clicked_cb, ad);
	evas_object_show(button);

	nf_it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, layout, NULL);
	elm_naviframe_item_title_enabled_set(nf_it, false, true);
	//elm_naviframe_item_pop_cb_set(nf_it, naviframe_pop_cb, NULL);
}

// Timer display - Not visited in current flow
static void
time_set_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = data;
	Evas_Object *button, *icon, *layout, *circle_datetime;
	Elm_Object_Item *nf_it = NULL;

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
	evas_object_smart_callback_add(button, "clicked", set_clicked_cb, ad);

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
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
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
	//evas_object_smart_callback_add(button, "clicked", time_set_button_clicked_cb, ad);	// For bypass Timer display
	evas_object_smart_callback_add(button, "clicked", Start_Stretch_cb, ad);
	evas_object_show(button);

	nf_it = elm_naviframe_item_push(ad->nf, "Setting time", NULL, NULL, layout, NULL);
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

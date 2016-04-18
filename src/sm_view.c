#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <glib-object.h>
#include <json-glib.h>
#include <device/power.h>

#include "action_icon.h"
#include "sm_view.h"
#include "sm_data.h"
#include "stretch_interface.h"

// Callback functions -------------------------------------------------------------------------------------------------
//static void Start_Stretch_cb(void *data, Evas_Object *obj, void *event_info);
static void Hold_Stretch_cb(void *data, Evas_Object *obj, void *event_info);
static void Fold_Stretch_cb(void *data, Evas_Object *obj, void *event_info);

static void Success_Strecth_cb(void *data, Evas_Object *obj, void *event_info);
static void Fail_Strecth_cb(void *data, Evas_Object *obj, void *event_info);
static void Reward_cb(void *data, Evas_Object *obj, void *event_info);
static void Strecth_Guide_cb(void *data, Evas_Object *obj, void *event_info);

static StretchState stretch_cur_state = STRETCH_STATE_NONE;

#define STRETCHING_DATA_FILEPATH "/opt/usr/share/stretching.json"

static void write_stretching_data_to_json_file(unsigned int current_time)
{
#if 1
JsonParser *jsonParser  =  NULL;
GError *error  =  NULL;
jsonParser = json_parser_new ();

#else
	JsonBuilder *builder = json_builder_new ();

	json_builder_begin_object (builder);

	json_builder_set_member_name (builder, "last_success");
	json_builder_add_int_value (builder, current_time);
/*
	json_builder_add_string_value (builder, "http://www.gnome.org/img/flash/two-thirty.png");

	json_builder_set_member_name (builder, "size");
	json_builder_begin_array (builder);
	json_builder_add_int_value (builder, 652);
	json_builder_add_int_value (builder, 242);
	json_builder_end_array (builder);
*/
	json_builder_end_object (builder);

	JsonGenerator *gen = json_generator_new ();
	JsonNode * root = json_builder_get_root (builder);
	json_generator_set_root (gen, root);
	gboolean ret = json_generator_to_file (gen, STRETCHING_DATA_FILEPATH);

	if(ret)
	{
		DBG("json file (%s) writing success\n");
	}
	else
	{
		DBG("json file (%s) writing failed\n");
	}

	json_node_free (root);
	g_object_unref (gen);
	g_object_unref (builder);
#endif
}

static void emit_current_time_to_watchapp(void *data, char* key)
{
	app_control_h app_control;

	app_control_create(&app_control);
	app_control_set_operation(app_control, APP_CONTROL_OPERATION_VIEW);
	app_control_set_app_id(app_control, STRETCHING_WATCH_APP_ID);

	struct timeval current;
	gettimeofday(&current, NULL);

	char timestring[20];
	snprintf(timestring, 20, "%ld", current.tv_sec);

	// TODO: need to store the last success time in db or file
	app_control_add_extra_data(app_control, key, timestring);

	if (app_control_send_launch_request(app_control, NULL, NULL) == APP_CONTROL_ERROR_NONE)
	{
	   DBG("Succeeded to launch a %s app.", STRETCHING_WATCH_APP_ID);
	}
	else
	{
	   DBG("Failed to launch a %s app.", STRETCHING_WATCH_APP_ID);
	}

	app_control_destroy(app_control);

	write_stretching_data_to_json_file(current.tv_sec);

}

static void Stretch_Result_cb(StretchConfig conf, StretchResult result, void *data)
{
	appdata_s *ad = data;

	DBG("Stretch_Result_cb:conf[mode,type,state]=%d,%d,%d, result(%d)\n", conf.mode, conf.type, conf.state, result);

	switch(conf.state)
	{
		case STRETCH_STATE_UNFOLD:
			if(result == STRETCH_SUCCESS)
			{
				// go to hold view
				Hold_Stretch_cb(data, NULL, NULL);
			}
			else if(result == STRETCH_FAIL)
			{
				// go to fail view
				Fail_Strecth_cb(data, NULL, NULL);
			}
		break;

		case STRETCH_STATE_HOLD :
			if(result == STRETCH_SUCCESS)
			{
				// store the result at app_data
				ad->is_stretch_success = EINA_TRUE;

				// go to fold view
				Fold_Stretch_cb(data, NULL, NULL);
			}
			else if(result == STRETCH_FAIL)
			{
				// store the result at app_data
				ad->is_stretch_success = EINA_FALSE;
				// go to fail view
				Fail_Strecth_cb(data, NULL, NULL);
			}
		break;

		default:
		{
			;
		}

}
}

static Eina_Bool
stop_sensor(void *data, Elm_Object_Item *it)
{
	stretching_stop();

	return true;
}

static Eina_Bool
naviframe_pop_cb(void *data, Elm_Object_Item *it)
{
	stretch_manager_release();
	device_power_release_lock(POWER_LOCK_DISPLAY);
	ui_app_exit();
	return EINA_FALSE;
}

/*
static void
naviframe_back_cb(void *data, Evas_Object *obj, void *event_info)
{
    struct appdata *ad = data;
    stretching_stop();
    elm_naviframe_item_pop(ad->nf);
}
*/

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

static Eina_Bool folding_timer_cb(void* data)
{
	struct appdata *ad = data;

	// go to success/fail view
	if(ad->is_stretch_success)
	{
		Success_Strecth_cb(data, NULL, NULL);
	}
	else
	{
		Fail_Strecth_cb(data, NULL, NULL);
	}

	if(ad->fold_timer)
	{
		ecore_timer_del(ad->fold_timer);
		ad->fold_timer = NULL;
	}

	return ECORE_CALLBACK_DONE;
}


static void
Strecth_Guide_cb(void *data, Evas_Object *obj, void *event_info);

// Entrance the stretching - Now testing here for adding label
void
Start_Stretch_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = data;
	Evas_Object *layout, *Unfolding_Animation;

	Elm_Object_Item *nf_it = NULL;

	/* Base Layout */
	char edj_path[PATH_MAX] = {0, };
	app_get_resource(EDJ_FILE, edj_path, (int)PATH_MAX);

	layout = elm_layout_add(ad->nf);
	elm_layout_file_set(layout, edj_path, "anim_img_and_center_text"); // custom theme

	elm_object_part_text_set(layout, "text", "Fold your hands<br>and stretch arms up high");

	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(layout);


	// Add animation
	Unfolding_Animation = elm_image_add(layout);
	//elm_object_style_set(Unfolding_Animation, "center");
	elm_image_file_set(Unfolding_Animation, ICON_DIR "/Unfolding_High.gif", NULL);
	evas_object_show(Unfolding_Animation);
	elm_object_part_content_set(layout, "elm.swallow.animation", Unfolding_Animation);

	// Animation availability check
	if (elm_image_animated_available_get(Unfolding_Animation)) {
		action_icon_set(Unfolding_Animation, true);
		action_icon_play_set(Unfolding_Animation, true, true, true);
	}
	else
	{
		DBG("Animation is NOT available\n");
	}

	// TODO: remove this callback when HMM works well
	evas_object_smart_callback_add(Unfolding_Animation, "clicked", Hold_Stretch_cb, ad);

	nf_it = elm_naviframe_item_push(ad->nf, "Unfolding", NULL, NULL, layout, NULL);
	elm_naviframe_item_title_enabled_set(nf_it, false, true);

	// exit app by "back"
	elm_naviframe_item_pop_cb_set(nf_it, naviframe_pop_cb, NULL);

	// stretching result checking
	const StretchConfig armup_unfold_conf = { STRETCH_MODE_EVAL, STRETCH_TYPE_ARM_UP, STRETCH_STATE_UNFOLD};
	stretching_start(armup_unfold_conf, Stretch_Result_cb, ad);
	stretch_cur_state = STRETCH_STATE_UNFOLD;

	 device_power_request_lock(POWER_LOCK_DISPLAY, 0);
}

static void Hold_Stretch_Anim_Finish_Cb(void *data, Evas_Object *obj)
{
	if(stretch_cur_state == STRETCH_STATE_HOLD)
		Fold_Stretch_cb(data, NULL, NULL);
}

// Peak of the stretching - Holding posture
static void
Hold_Stretch_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = data;

	vibrate(300, 99);

	DBG("%s(%d)\n", __FUNCTION__, __LINE__);

	stretching_stop();
	const StretchConfig armup_hold_conf = { STRETCH_MODE_EVAL, STRETCH_TYPE_ARM_UP, STRETCH_STATE_HOLD};
	stretching_start(armup_hold_conf, Stretch_Result_cb, ad);
	stretch_cur_state = STRETCH_STATE_HOLD;

	DBG("%s(%d)\n", __FUNCTION__, __LINE__);

	Evas_Object *layout, *Hold_Animation;
	Elm_Object_Item *nf_it = NULL;

	/* Base Layout */
	char edj_path[PATH_MAX] = {0, };
	app_get_resource(EDJ_FILE, edj_path, (int)PATH_MAX);

	layout = elm_layout_add(ad->nf);
	elm_layout_file_set(layout, edj_path, "anim_img_and_upper_text"); // custom theme
	evas_object_show(layout);

	elm_object_part_text_set(layout, "text", "Keep the posture");

	// Add animation
	Hold_Animation = elm_image_add(layout);
	elm_object_style_set(Hold_Animation, "center");
	elm_image_file_set(Hold_Animation, ICON_DIR "/Hold_One_Color.gif", NULL);
	evas_object_show(Hold_Animation);
	elm_object_part_content_set(layout, "elm.swallow.animation", Hold_Animation);

	// Animation availability check
	if (elm_image_animated_available_get(Hold_Animation)) {
		action_icon_set(Hold_Animation, true);
		action_icon_play_set(Hold_Animation, true, false, true);

		// when holding animation is finished, go to next
		action_icon_finish_callback_add(Hold_Animation, Hold_Stretch_Anim_Finish_Cb, data);
	}
	else
	{
		DBG("Animation is NOT available\n");
	}

	nf_it = elm_naviframe_item_push(ad->nf, "Holding", NULL, NULL, layout, NULL);
	elm_naviframe_item_title_enabled_set(nf_it, false, true);

	DBG("%s(%d)\n", __FUNCTION__, __LINE__);

	// exit app by "back"
	elm_naviframe_item_pop_cb_set(nf_it, naviframe_pop_cb, NULL);
}

// Folding step of the stretching
static void
Fold_Stretch_cb(void *data, Evas_Object *obj, void *event_info)
{
	vibrate(300, 99);

	struct appdata *ad = data;
	Evas_Object *layout, *Folding_Animation;

	Elm_Object_Item *nf_it = NULL;

	stretching_stop();
	stretch_cur_state = STRETCH_STATE_FOLD;

	// set success since user succeed to keep hold time
	ad->is_stretch_success = EINA_TRUE;
//	stretching_start(STRETCH_TYPE_ARM_UP, STRETCH_STATE_FOLD, Stretch_Result_cb, ad);

	/* Base Layout */
	char edj_path[PATH_MAX] = {0, };
	app_get_resource(EDJ_FILE, edj_path, (int)PATH_MAX);

	layout = elm_layout_add(ad->nf);
	elm_layout_file_set(layout, edj_path, "anim_img_and_center_text"); // custom theme

	elm_object_part_text_set(layout, "text", "Get back to the origin with deep breathing");

	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(layout);


	// Add animation
	Folding_Animation = elm_image_add(layout);
	//elm_object_style_set(Folding_Animation, "center");
	elm_image_file_set(Folding_Animation, ICON_DIR "/Folding_High.gif", NULL);
	evas_object_show(Folding_Animation);
	elm_object_part_content_set(layout, "elm.swallow.animation", Folding_Animation);

	// Animation availability check
	if (elm_image_animated_available_get(Folding_Animation)) {
		action_icon_set(Folding_Animation, true);
		action_icon_play_set(Folding_Animation, true, true, true);
	}
	else
	{
		DBG("Animation is NOT available\n");
	}

#if 0
	// randomly success for demo
	int type = random() % 10;

//	if(type < 5)
		evas_object_smart_callback_add(Folding_Animation, "clicked", Success_Strecth_cb, ad);
//	else
//		evas_object_smart_callback_add(Folding_Animation, "clicked", Fail_Strecth_cb, ad);
#else
	ad->fold_timer = ecore_timer_add(4.0f, folding_timer_cb, ad);
#endif

	nf_it = elm_naviframe_item_push(ad->nf, "Folding", NULL, NULL, layout, NULL);
	elm_naviframe_item_title_enabled_set(nf_it, false, true);

	// exit app by "back"
	elm_naviframe_item_pop_cb_set(nf_it, naviframe_pop_cb, NULL);
}

// Success
static void
Success_Strecth_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = data;
	Evas_Object *layout, *bg, *Success_image, *button;
	Elm_Object_Item *nf_it = NULL;

	stretching_stop();
	stretch_cur_state = STRETCH_STATE_NONE;

	char edj_path[PATH_MAX] = {0, };
	app_get_resource(EDJ_FILE, edj_path, (int)PATH_MAX);

	layout = elm_layout_add(ad->nf);
	elm_layout_file_set(layout, edj_path, "stretch_result"); // custom theme
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(layout);

	// Add bg
	bg = elm_image_add(layout);
	elm_object_style_set(bg, "center");

	srandom(time(NULL));
	unsigned int achieve = ((random() % 5) + 1 )*20;

	switch(achieve)
	{

		case 20:
			elm_image_file_set(bg, ICON_DIR "/Circle_White_15px_20p.png", NULL);
		break;
		case 40:
			elm_image_file_set(bg, ICON_DIR "/Circle_White_15px_40p.png", NULL);
		break;
		case 60:
			elm_image_file_set(bg, ICON_DIR "/Circle_White_15px_60p.png", NULL);
		break;
		case 80:
			elm_image_file_set(bg, ICON_DIR "/Circle_White_15px_80p.png", NULL);
		break;
		default:
	elm_image_file_set(bg, ICON_DIR "/Circle_White_15px.png", NULL);
		break;
	}

	evas_object_color_set(bg, 35, 202, 224, 255); // blue

	evas_object_show(bg);
	elm_object_part_content_set(layout, "elm.swallow.bg", bg);


#if 1
#if 1

#if 1

	// result image
	Success_image = elm_image_add(layout);
	elm_object_style_set(Success_image, "center");
	elm_image_file_set(Success_image, ICON_DIR "/Success_Picto.png", NULL);
	evas_object_show(Success_image);
	elm_object_part_content_set(layout, "elm.swallow.content", Success_image);

	// text
	elm_object_part_text_set(layout, "text", "Success!!");

#else

	// Add Scroller
	Evas_Object* circle_scroller, *scroller;

	scroller = elm_scroller_add(layout);
	evas_object_size_hint_min_set(scroller, 360, 360);
	evas_object_show(scroller);

	elm_object_part_content_set(layout, "elm.swallow.content", scroller);

	/* Create Circle Scroller */
	circle_scroller = eext_circle_object_scroller_add(scroller, ad->circle_surface);

	/* Set Scroller Policy */
	eext_circle_object_scroller_policy_set(circle_scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);

	/* Activate Rotary Event */
	eext_rotary_object_event_activated_set(circle_scroller, EINA_TRUE);

// USE 1 IMAGE

	// result image
	Success_image = elm_image_add(scroller);
	elm_object_style_set(Success_image, "center");
	elm_image_file_set(Success_image, ICON_DIR "/Success_Reward.png", NULL);
	elm_image_no_scale_set(Success_image, EINA_TRUE);
	evas_object_show(Success_image);

	elm_object_content_set(scroller, Success_image);

#endif

#else

// USE BOX

	Evas_Object* box, *icon, *present;

	box = elm_box_add(scroller);
	elm_box_horizontal_set(box, EINA_FALSE);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(box);

	icon = elm_image_add(box);
	elm_object_style_set(icon, "center");
	elm_image_file_set(icon, ICON_DIR "/Success_Picto.png", NULL);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
//	elm_image_no_scale_set(icon, EINA_TRUE);
	evas_object_show(icon);

	elm_box_pack_end(box, icon);

	present = elm_image_add(box);
	elm_object_style_set(present, "center");
	elm_image_file_set(present, ICON_DIR "/Success_long.png", NULL);
//	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
//	evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_image_no_scale_set(present, EINA_TRUE);
	evas_object_show(present);

	elm_box_pack_end(box, present);

	elm_object_content_set(scroller, box);

#endif

#else

// USE LAYOUT

	Evas_Object* scroll_layout;

	scroll_layout = elm_layout_add(scroller);
	elm_layout_file_set(scroll_layout, edj_path, "stretch_success_content"); // custom theme
//	evas_object_size_hint_weight_set(scroll_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
//	evas_object_size_hint_align_set(scroll_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_resize(scroll_layout, 360, 720);
	evas_object_show(scroll_layout);

	elm_object_content_set(scroller, scroll_layout);

#endif

	// Add button
	button = elm_button_add(layout);
	elm_object_style_set(button, "bottom");

	elm_object_text_set(button, "Once again?");
	elm_object_part_content_set(layout, "elm.swallow.button", button);
	evas_object_smart_callback_add(button, "clicked", Strecth_Guide_cb, ad);
	evas_object_show(button);

	nf_it = elm_naviframe_item_push(ad->nf, "again", NULL, NULL, layout, NULL);
	elm_naviframe_item_title_enabled_set(nf_it, false, true);

	// exit app by "back"
	elm_naviframe_item_pop_cb_set(nf_it, naviframe_pop_cb, NULL);

	vibrate(100, 99);
	usleep(1000*100); // 100 ms
	vibrate(100, 99);


	device_power_release_lock(POWER_LOCK_DISPLAY);

	// send the success time to stretchtime watch app
	emit_current_time_to_watchapp(ad, "last_success_time");
}

// Fail
static void
Fail_Strecth_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = data;
	Evas_Object *layout, *Fail_image, *button, *bg;
	Elm_Object_Item *nf_it = NULL;

	stretching_stop();
	stretch_cur_state = STRETCH_STATE_NONE;

	char edj_path[PATH_MAX] = {0, };
	app_get_resource(EDJ_FILE, edj_path, (int)PATH_MAX);

	layout = elm_layout_add(ad->nf);
	elm_layout_file_set(layout, edj_path, "stretch_result"); // custom theme
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(layout);

	// Add bg
	bg = elm_image_add(layout);
	elm_object_style_set(bg, "center");
	elm_image_file_set(bg, ICON_DIR "/Circle_White_15px.png", NULL);
	evas_object_color_set(bg, 252, 116, 75, 255); // red

	evas_object_show(bg);
	elm_object_part_content_set(layout, "elm.swallow.bg", bg);


	// Add icon
	Fail_image = elm_image_add(layout);
	elm_object_style_set(Fail_image, "center");
	elm_image_file_set(Fail_image, ICON_DIR "/Fail_Picto.png", NULL);
	evas_object_show(Fail_image);
	elm_object_part_content_set(layout, "elm.swallow.content", Fail_image);

	// text
	elm_object_part_text_set(layout, "text", "Fail to stretch...");

	// Add button
	button = elm_button_add(layout);
	elm_object_style_set(button, "bottom");

	elm_object_text_set(button, "Retry");
	elm_object_part_content_set(layout, "elm.swallow.button", button);
	evas_object_smart_callback_add(button, "clicked", Strecth_Guide_cb, ad);
	evas_object_show(button);

	nf_it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, layout, NULL);
	elm_naviframe_item_title_enabled_set(nf_it, false, true);

	// exit app by "back"
	elm_naviframe_item_pop_cb_set(nf_it, naviframe_pop_cb, NULL);

	vibrate(700, 99);

	device_power_release_lock(POWER_LOCK_DISPLAY);
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

	elm_object_text_set(button, "Exit");
	elm_object_part_content_set(layout, "elm.swallow.button", button);
	evas_object_smart_callback_add(button, "clicked", set_clicked_cb, ad);
	evas_object_show(button);

	nf_it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, layout, NULL);
	elm_naviframe_item_title_enabled_set(nf_it, false, true);

	// exit app by "back"
	elm_naviframe_item_pop_cb_set(nf_it, naviframe_pop_cb, NULL);
}

void
Strecth_Guide_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = data;
	Elm_Object_Item *nf_it;
	Evas_Object *button, *layout;
	time_t local_time = time(NULL);
	struct tm *time_info = localtime(&local_time);

	ad->saved_time = *time_info;

	/* Base Layout */
	char edj_path[PATH_MAX] = {0, };
	app_get_resource(EDJ_FILE, edj_path, (int)PATH_MAX);

	layout = elm_layout_add(ad->nf);
	elm_layout_file_set(layout, edj_path, "guide_with_btn"); // custom theme

	//layout = elm_layout_add(ad->nf);
	//evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	//evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	//elm_layout_theme_set(layout, "layout", "bottom_button", "default");
	evas_object_show(layout);

	//ad->label = elm_label_add(layout);
	//evas_object_size_hint_weight_set(ad->label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	Evas_Object* actionIcon;
	//Image add for stretching action icon.
	actionIcon= elm_image_add(layout);

	elm_image_file_set(actionIcon, ICON_DIR "/Up_stretching.gif", NULL);

	// resizeing
	//evas_object_size_hint_weight_set(actionIcon, 0.5, 0.5);
	//evas_object_size_hint_padding_set(actionIcon, 0,0,0,200);

	evas_object_show(actionIcon);
	elm_object_part_content_set(layout, "elm.swallow.animation", actionIcon);

	if (elm_image_animated_available_get(actionIcon)) {
		action_icon_set(actionIcon, true);
		action_icon_play_set(actionIcon, true, true, true);
	}
	else
	{
		DBG("Animation is NOT available\n");
	}

	// Scroller
	Evas_Object* circle_scroller, *scroller, *label;

	scroller = elm_scroller_add(layout);
	evas_object_show(scroller);

	elm_object_part_content_set(layout, "elm.swallow.text_scroller", scroller);

	/* Create Circle Scroller */
	circle_scroller = eext_circle_object_scroller_add(scroller, ad->circle_surface);

	/* Set Scroller Policy */
	eext_circle_object_scroller_policy_set(circle_scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);

	/* Activate Rotary Event */
	eext_rotary_object_event_activated_set(circle_scroller, EINA_TRUE);

	label = elm_label_add(scroller);
	elm_label_line_wrap_set(label, ELM_WRAP_MIXED);
	elm_object_text_set(label, "<align=center><font_size=38> <font color=#FF0000>Stretch up</font color> your folding arms.</font_size> <br> "
			"<font_size=30 color=#999999>Fold your hands and stretch arms up high. "
			"After then keep the stretching for some seconds. Finally release your arms by feedback.</font_size></align>");
	elm_object_content_set(scroller, label);
	evas_object_show(label);

	// Button
	button = elm_button_add(layout);
	elm_object_style_set(button, "bottom");
	elm_object_text_set(button, "Start"); // Start!
	elm_object_part_content_set(layout, "elm.swallow.button", button);
	evas_object_smart_callback_add(button, "clicked", Start_Stretch_cb, ad);
	evas_object_show(button);

	nf_it = elm_naviframe_item_push(ad->nf, "Stretching guide", NULL, NULL, layout, NULL);
	elm_naviframe_item_title_enabled_set(nf_it, false, true);

	// exit app by "back"
	elm_naviframe_item_pop_cb_set(nf_it, naviframe_pop_cb, NULL);

	// TODO: DO this when stretching is succeeded!
	store_last_time_to_current();


	auto_start_stretch(data);
}

void
create_main_view(appdata_s *ad)
{
	Evas_Object *layout, *bg, *button;
	Elm_Object_Item *nf_it = NULL;

#if 0
	// it has randomly 3 types for time that user had stretched before.
	srandom(time(NULL));
	int type = random() % 3;
#else
	time_t diff = get_elapsed_time_from_last();
	int type = get_awareness_level_from_data(diff);

	int d_day = diff / (60 * 60 * 24);
	diff -= d_day * 60 * 60 * 24;
	int d_hour = diff / (60 * 60);
	diff -= d_hour * 60 * 60;
	int d_min = diff / 60;

#endif

	int colors[4][4] = {
		{112, 198, 19, 255}, // green
		{239, 188, 69, 255}, // yellow
		{252, 116, 75, 255}, // red
		{252, 116, 75, 255} // red
	};

	vibrate(300, 99);

	time_t local_time = time(NULL);
	struct tm *time_info = localtime(&local_time);
	ad->saved_time = *time_info;

	/* Base Layout */
	char edj_path[PATH_MAX] = {0, };
	app_get_resource(EDJ_FILE, edj_path, (int)PATH_MAX);

	layout = elm_layout_add(ad->nf);
	elm_layout_file_set(layout, edj_path, "strech_main"); // custom theme

	// Text setting
	elm_object_part_text_set(layout, "text", "Do stretching<br>with your arms");

	char text2string[50];

	switch(type)
	{
	case 1:
		snprintf(text2string, sizeof(text2string), "%s : %d %s", "The last", d_min, (d_min > 1) ? "minutes ago" : "minute ago");
		break;
	case 2:
	case 3:
		snprintf(text2string, sizeof(text2string), "%s : %d %s", "The last", d_hour, (d_hour > 1) ? "hours ago" : "hour ago");
		break;
	case 4:
		snprintf(text2string, sizeof(text2string), "%s", "Try to release your body");
		break;
	default:
		snprintf(text2string, sizeof(text2string), "%s", "It is the time to stretch!");
		break;
	}

	elm_object_part_text_set(layout, "text2", text2string);

	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(layout);

	// Add background
	bg = elm_image_add(layout);
	elm_image_file_set(bg, ICON_DIR "/Circle_White_15px.png", NULL);
	evas_object_color_set(bg, colors[type-1][0], colors[type-1][1], colors[type-1][2], colors[type-1][3]);
	evas_object_show(bg);
	elm_object_part_content_set(layout, "elm.swallow.content", bg);

	// Add button
	button = elm_button_add(layout);
	elm_object_style_set(button, "bottom");

	elm_object_text_set(button, "Start");
	elm_object_part_content_set(layout, "elm.swallow.button", button);
	evas_object_show(button);

	// Set next display as callback function
	evas_object_smart_callback_add(button, "clicked", Strecth_Guide_cb, ad);

	// Display current page
	nf_it = elm_naviframe_item_push(ad->nf, "Enter the stretching", NULL, NULL, layout, NULL);
	elm_naviframe_item_title_enabled_set(nf_it, false, true);

	// exit app when the 1st depth is poped
	elm_naviframe_item_pop_cb_set(nf_it, naviframe_pop_cb, NULL);
}



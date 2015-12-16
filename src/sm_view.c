#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "action_icon.h"
#include "sm_view.h"
#include "stretch_manager.h"

// Callback functions -------------------------------------------------------------------------------------------------
static void Start_Stretch_cb(void *data, Evas_Object *obj, void *event_info);
static void Hold_Stretch_cb(void *data, Evas_Object *obj, void *event_info);
static void Fold_Stretch_cb(void *data, Evas_Object *obj, void *event_info);

static void Success_Strecth_cb(void *data, Evas_Object *obj, void *event_info);
static void Fail_Strecth_cb(void *data, Evas_Object *obj, void *event_info);
static void Result_cb(void *data, Evas_Object *obj, void *event_info);
static void Reward_cb(void *data, Evas_Object *obj, void *event_info);
static void Strecth_Guide_cb(void *data, Evas_Object *obj, void *event_info);

static void Stretch_Result_cb(StretchType type, StretchState state, StretchResult result, void *data)
{
	appdata_s *ad = data;

	DBG("type %d, state %d, result %d\n", type, state, result);

	switch(state)
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
static void
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

	elm_object_part_text_set(layout, "text", "두 손을 깍지 끼고<br>머리 위로 뻗으세요");

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
	stretching_start(STRETCH_ARM_UP, STRETCH_STATE_UNFOLD, Stretch_Result_cb, ad);

	elm_naviframe_item_pop_cb_set(nf_it, stop_sensor, ad);

}

static void Hold_Stretch_Anim_Finish_Cb(void *data, Evas_Object *obj)
{
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
	stretching_start(STRETCH_ARM_UP, STRETCH_STATE_HOLD, Stretch_Result_cb, ad);

	DBG("%s(%d)\n", __FUNCTION__, __LINE__);

	Evas_Object *layout, *Hold_Animation;
	Elm_Object_Item *nf_it = NULL;

	/* Base Layout */
	char edj_path[PATH_MAX] = {0, };
	app_get_resource(EDJ_FILE, edj_path, (int)PATH_MAX);

	layout = elm_layout_add(ad->nf);
	elm_layout_file_set(layout, edj_path, "anim_img_and_upper_text"); // custom theme
	evas_object_show(layout);

	elm_object_part_text_set(layout, "text", "현재 자세를 유지하세요");

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

	// set success since user succeed to keep hold time
	ad->is_stretch_success = EINA_TRUE;
//	stretching_start(STRETCH_ARM_UP, STRETCH_STATE_FOLD, Stretch_Result_cb, ad);

	/* Base Layout */
	char edj_path[PATH_MAX] = {0, };
	app_get_resource(EDJ_FILE, edj_path, (int)PATH_MAX);

	layout = elm_layout_add(ad->nf);
	elm_layout_file_set(layout, edj_path, "anim_img_and_center_text"); // custom theme

	elm_object_part_text_set(layout, "text", "천천히 심호흡하며<br>원래 자세로 돌아가세요");

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
	ad->fold_timer = ecore_timer_add(3.0f, folding_timer_cb, ad);
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
	evas_object_color_set(bg, 35, 202, 224, 255); // blue

	evas_object_show(bg);
	elm_object_part_content_set(layout, "elm.swallow.bg", bg);


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

#if 1
#if 1

// USE 1 IMAGE

	// result image
	Success_image = elm_image_add(scroller);
	elm_object_style_set(Success_image, "center");
	elm_image_file_set(Success_image, ICON_DIR "/Success_Reward.png", NULL);
	elm_image_no_scale_set(Success_image, EINA_TRUE);
	evas_object_show(Success_image);

	elm_object_content_set(scroller, Success_image);

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

	elm_object_text_set(button, "한 번 더 하기");
	elm_object_part_content_set(layout, "elm.swallow.button", button);
	evas_object_smart_callback_add(button, "clicked", Strecth_Guide_cb, ad);
	evas_object_show(button);

	nf_it = elm_naviframe_item_push(ad->nf, "한 번 더 하기", NULL, NULL, layout, NULL);
	elm_naviframe_item_title_enabled_set(nf_it, false, true);

	// exit app by "back"
	elm_naviframe_item_pop_cb_set(nf_it, naviframe_pop_cb, NULL);

	vibrate(100, 99);
	usleep(1000*100); // 100 ms
	vibrate(100, 99);
}

// Fail
static void
Fail_Strecth_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = data;
	Evas_Object *layout, *Fail_image, *button, *bg;
	Elm_Object_Item *nf_it = NULL;

	stretching_stop();

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

	// Add button
	button = elm_button_add(layout);
	elm_object_style_set(button, "bottom");

	elm_object_text_set(button, "다시 시도");
	elm_object_part_content_set(layout, "elm.swallow.button", button);
	evas_object_smart_callback_add(button, "clicked", Strecth_Guide_cb, ad);
	evas_object_show(button);

	nf_it = elm_naviframe_item_push(ad->nf, NULL, NULL, NULL, layout, NULL);
	elm_naviframe_item_title_enabled_set(nf_it, false, true);

	// exit app by "back"
	elm_naviframe_item_pop_cb_set(nf_it, naviframe_pop_cb, NULL);

	vibrate(700, 99);
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

	// exit app by "back"
	elm_naviframe_item_pop_cb_set(nf_it, naviframe_pop_cb, NULL);
}

static void
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
	elm_object_text_set(label, "<align=center><font_size=38>팔을 <font color=#FF0000>위로</font color> 뻗어서 스트레칭 해보세요.</font_size> <BR> "
			"<font_size=30 color=#999999>양손을 깍지 끼고 양 팔을 머리위로 쭉 뻗은 후 신호를 기다리세요. "
			"신호에 맞추어 기다렸다가 내리세요.</font_size></align>");
	elm_object_content_set(scroller, label);
	evas_object_show(label);

	// Button
	button = elm_button_add(layout);
	elm_object_style_set(button, "bottom");
	elm_object_text_set(button, "시작하기"); // Start!
	elm_object_part_content_set(layout, "elm.swallow.button", button);
	evas_object_smart_callback_add(button, "clicked", Start_Stretch_cb, ad);
	evas_object_show(button);

	nf_it = elm_naviframe_item_push(ad->nf, "Setting time", NULL, NULL, layout, NULL);
	elm_naviframe_item_title_enabled_set(nf_it, false, true);
}

void
create_main_view(appdata_s *ad)
{
	Evas_Object *layout, *bg, *button;
	Elm_Object_Item *nf_it = NULL;

	// it has randomly 3 types for time that user had stretched before.
	srandom(time(NULL));
	int type = random() % 3;

	int colors[3][4] = {
		{112, 198, 19, 255}, // green
		{239, 188, 69, 255}, // yellow
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
	elm_object_part_text_set(layout, "text", "팔을 뻗어서<br>스트레칭 해보세요");

	char text2string[50];
	snprintf(text2string, sizeof(text2string), "%s : %d%s", "마지막 스트레칭", (type+1), "시간 전");

	elm_object_part_text_set(layout, "text2", text2string);

	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(layout);

	// Add background
	bg = elm_image_add(layout);
	elm_image_file_set(bg, ICON_DIR "/Circle_White_15px.png", NULL);
	evas_object_color_set(bg, colors[type][0], colors[type][1], colors[type][2], colors[type][3]);
	evas_object_show(bg);
	elm_object_part_content_set(layout, "elm.swallow.content", bg);

	// Add button
	button = elm_button_add(layout);
	elm_object_style_set(button, "bottom");

	elm_object_text_set(button, "스트레칭 시작");
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



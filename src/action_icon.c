
#include "action_icon.h"
#include "logger.h"

static const char *anim_data_key = "_action_icon_anim";
static const char *cb_data_key = "_action_icon_callback";

/**
 * animation data
 */
typedef struct {
	int frame_count;
	int cur_frame;
	double frame_duration;

	Eina_Bool repeat;
	Eina_Bool play;

	Ecore_Timer *anim_timer;
	Evas_Object *img;
}Action_Icon_Animation;

/**
 * function callback data
 */
typedef struct {
   const char *event;
   Action_Icon_Cb func;
   void *func_data;
}Action_Icon_Callback;

static void _action_icon_data_deleted_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Action_Icon_Animation *animation = (Action_Icon_Animation *)evas_object_data_get(obj, anim_data_key);

	if (animation) {
		if(animation->anim_timer) {
			ecore_timer_del(animation->anim_timer);
			animation->anim_timer = NULL;
		}
	}

	Action_Icon_Callback *cb = evas_object_data_del(obj, cb_data_key);
	if (cb != NULL) {
		free(cb);
		cb = NULL;
	}

	Action_Icon_Animation *d = evas_object_data_del(obj, anim_data_key);
	if (d != NULL) {
		free(d);
		d= NULL;
	}

	evas_object_event_callback_del(obj, EVAS_CALLBACK_DEL, _action_icon_data_deleted_cb);
}

static Eina_Bool _action_icon_anim_cb(void *user_data)
{
	Evas_Object* obj = (Evas_Object*)user_data;

	if (strcmp("elm_image", evas_object_type_get(obj)))
			return ECORE_CALLBACK_CANCEL;

	Action_Icon_Animation *data = (Action_Icon_Animation *)evas_object_data_get(obj, anim_data_key);
	if (data == NULL) return ECORE_CALLBACK_CANCEL;

	if (!data->play) return ECORE_CALLBACK_CANCEL;

	data->cur_frame++;

	if (data->cur_frame > data->frame_count) {
		if (!data->repeat) {
			data->play = EINA_FALSE;

			// "finish" callback
			Action_Icon_Callback *cb = (Action_Icon_Callback *)evas_object_data_get(obj, cb_data_key);
			if(cb)
			{
				DBG("_action_icon_anim_cb callback\n");
				cb->func(cb->func_data, obj);
			}
			else
			{
				DBG("_action_icon_anim_cb NO callback\n");
			}

			return ECORE_CALLBACK_CANCEL;
		}

		data->cur_frame = data->cur_frame % data->frame_count;
	}

	evas_object_image_animated_frame_set(data->img, data->cur_frame);
	data->frame_duration = evas_object_image_animated_frame_duration_get(data->img, data->cur_frame, 0);

//	DBG("frame count : %d/%d, frame duration : %f ms\n", data->cur_frame, data->frame_count, data->frame_duration);

	if (data->frame_duration > 0.f) {
		ecore_timer_interval_set(data->anim_timer, data->frame_duration);
	}

	return ECORE_CALLBACK_RENEW;
}

void action_icon_play_set(Evas_Object *obj, Eina_Bool play, Eina_Bool repeat, Eina_Bool reset)
{
	if (strcmp("elm_image", evas_object_type_get(obj)))
			return;

	Action_Icon_Animation *data = (Action_Icon_Animation *)evas_object_data_get(obj, anim_data_key);
	if (data == NULL) return;

	data->repeat = repeat;

	if (data->play == play) return;

	if (play) {
		if (reset) {
			data->frame_count = evas_object_image_animated_frame_count_get(data->img);
			data->cur_frame = 1;
			data->frame_duration = evas_object_image_animated_frame_duration_get(data->img, data->cur_frame, 0);
			evas_object_image_animated_frame_set(data->img, data->cur_frame);
		}

		if (data->anim_timer) {
			ecore_timer_del(data->anim_timer);
			data->anim_timer = NULL;
		}

//		DBG("frame count : %d/%d, frame duration : %f ms\n", data->cur_frame, data->frame_count, data->frame_duration);

		data->anim_timer = ecore_timer_add(data->frame_duration, _action_icon_anim_cb, obj);

	} else {
		if (data->anim_timer) {
			ecore_timer_del(data->anim_timer);
			data->anim_timer = NULL;
		}
	}
	data->play = play;
}

void action_icon_set(Evas_Object *obj, Eina_Bool play)
{
	if (strcmp("elm_image", evas_object_type_get(obj)))
		return;

	Action_Icon_Animation *data = (Action_Icon_Animation *)evas_object_data_get(obj, anim_data_key);

	Evas_Object *img = elm_image_object_get(obj);
	if (!evas_object_image_animated_get(img)) return;

	if (play) {
		if (data == NULL) {
			data = malloc(sizeof(Action_Icon_Animation));
			memset(data, 0x00, sizeof(Action_Icon_Animation));
		}

		evas_object_event_callback_del(obj, EVAS_CALLBACK_DEL, _action_icon_data_deleted_cb);
		evas_object_event_callback_add(obj, EVAS_CALLBACK_DEL, _action_icon_data_deleted_cb, NULL);

		data->img = img;
		data->frame_count = evas_object_image_animated_frame_count_get(img);

		data->cur_frame = 1;
		data->frame_duration = evas_object_image_animated_frame_duration_get(img, data->cur_frame, 0);
		evas_object_image_animated_frame_set(img, data->cur_frame);
		evas_object_data_set(obj, anim_data_key, data);
	} else {
		if (data != NULL) {
			evas_object_event_callback_del(obj, EVAS_CALLBACK_DEL, _action_icon_data_deleted_cb);
			free(data);
		}
	}
}

void action_icon_finish_callback_add(Evas_Object *obj, Action_Icon_Cb func, const void* data)
{
	if (strcmp("elm_image", evas_object_type_get(obj)))
		return;

	Action_Icon_Callback *cb = (Action_Icon_Callback *)evas_object_data_get(obj, cb_data_key);

	if(cb == NULL)
	{
		cb = (Action_Icon_Callback *)malloc(sizeof(Action_Icon_Callback));
		if (!cb) return;
	}

	memset(cb, 0x00, sizeof(Action_Icon_Callback));
	cb->event = "finish";
	cb->func = func;
	cb->func_data = (void *)data;

	evas_object_data_set(obj, cb_data_key, cb);

	DBG("action_icon_finish_callback_add\n");
}

void action_icon_finish_callback_del(Evas_Object *obj, Action_Icon_Cb func)
{
	if (strcmp("elm_image", evas_object_type_get(obj)))
		return;

	Action_Icon_Callback *cb = (Action_Icon_Callback *)evas_object_data_get(obj, cb_data_key);
	if(cb && cb->func == func)
	{
		Action_Icon_Callback *tmp = evas_object_data_del(obj, cb_data_key);
		if (tmp != NULL) {
			free(tmp);
			tmp = NULL;
		}
	}
	else
	{
		DBG("action_icon_finish_callback_del, NO callback\n");
	}

	DBG("action_icon_finish_callback_del\n");
}




#include "action_icon.h"
#include "logger.h"

static const char *_data_key = "_action_icon_anim";

struct anim_data {
	int frame_count;
	int cur_frame;
	double frame_duration;

	Eina_Bool repeat;
	Eina_Bool play;

	Ecore_Timer *anim_timer;
	Evas_Object *img;
};

static void _action_icon_data_deleted_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	struct anim_data *anim_data = evas_object_data_get(obj, _data_key);

	if (anim_data && anim_data->anim_timer) {
		ecore_timer_del(anim_data->anim_timer);
		anim_data->anim_timer = NULL;
	}

	struct anim_data *d = evas_object_data_del(obj, _data_key);
	if (d != NULL) {
		free(d);
		d= NULL;
	}
	evas_object_event_callback_del(obj, EVAS_CALLBACK_DEL, _action_icon_data_deleted_cb);
}

static Eina_Bool _action_icon_anim_cb(void *user_data)
{
	struct anim_data *data = user_data;
	if (!data->play) return ECORE_CALLBACK_CANCEL;

	data->cur_frame++;

	if (data->cur_frame > data->frame_count) {
		if (!data->repeat) {
			data->play = EINA_FALSE;
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

	struct anim_data *data = evas_object_data_get(obj, _data_key);
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

		data->anim_timer = ecore_timer_add(data->frame_duration, _action_icon_anim_cb, data);

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

	struct anim_data *data = evas_object_data_get(obj, _data_key);

	Evas_Object *img = elm_image_object_get(obj);
	if (!evas_object_image_animated_get(img)) return;

	if (play) {
		if (data == NULL) {
			data = malloc(sizeof(struct anim_data));
			memset(data, 0x00, sizeof(struct anim_data));
		}

		evas_object_event_callback_del(obj, EVAS_CALLBACK_DEL, _action_icon_data_deleted_cb);
		evas_object_event_callback_add(obj, EVAS_CALLBACK_DEL, _action_icon_data_deleted_cb, NULL);

		data->img = img;
		data->frame_count = evas_object_image_animated_frame_count_get(img);

		data->cur_frame = 1;
		data->frame_duration = evas_object_image_animated_frame_duration_get(img, data->cur_frame, 0);
		evas_object_image_animated_frame_set(img, data->cur_frame);
		evas_object_data_set(obj, _data_key, data);
	} else {
		if (data != NULL) {
			evas_object_event_callback_del(obj, EVAS_CALLBACK_DEL, _action_icon_data_deleted_cb);
			free(data);
		}
	}
}

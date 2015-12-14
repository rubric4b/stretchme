#ifndef __ACTION_ICON_H__
#define __ACTION_ICON_H__

/**
 * referenced from
 * https://developer.tizen.org/community/code-snippet/native-code-snippet/play-animation-gif-once-elmentary-image.?langredirect=1
 */
#include <Elementary.h>

typedef void (*Action_Icon_Cb)(void *data, Evas_Object *obj);

void action_icon_play_set(Evas_Object *obj, Eina_Bool play, Eina_Bool repeat, Eina_Bool reset);
void action_icon_set(Evas_Object *obj, Eina_Bool play);

/**
 * This callback will be called only when the animation's repeat mode is false.
 */
void action_icon_finish_callback_add(Evas_Object *obj, Action_Icon_Cb func, const void* data);
void action_icon_finish_callback_del(Evas_Object *obj, Action_Icon_Cb func);

#endif // __ACTION_ICON_H__ //

#ifndef __ACTION_ICON_H__
#define __ACTION_ICON_H__

/**
 * referenced from 
 * https://developer.tizen.org/community/code-snippet/native-code-snippet/play-animation-gif-once-elmentary-image.?langredirect=1
 */
#include <Elementary.h>

void action_icon_play_set(Evas_Object *obj, Eina_Bool play, Eina_Bool repeat, Eina_Bool reset);
void action_icon_set(Evas_Object *obj, Eina_Bool play);

#endif // __ACTION_ICON_H__ //

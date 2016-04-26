//
// Created by hobbang5 on 2016-04-20.
//

#include "sm_view.h"
#include "sm_popup.h"

static void
popup_hide_cb(void *data, Evas_Object *obj, void *event_info)
{
    if(!data) return;
    elm_popup_dismiss(data);
}

static void
popup_hide_finished_cb(void *data, Evas_Object *obj, void *event_info)
{
    if(!data) return;
    evas_object_del(data);
}

static void response_training_yes_cb(void *data, Evas_Object *obj, void *event_info)
{
    appdata_s *ad = (appdata_s *) data;

    if(!data) return;
    elm_popup_dismiss(ad->popup);

    Stretch_Guide_cb(data, NULL, NULL);

}

void popup_training_cb(void *data, Evas_Object *obj, void *event_info)
{
    Evas_Object *btn;
    Evas_Object *icon;
    Evas_Object *layout;
    appdata_s *ad = (appdata_s *) data;
    ad->training_cnt = 1;

    ad->popup = elm_popup_add(ad->nf);
    elm_object_style_set(ad->popup, "circle");
    evas_object_size_hint_weight_set(ad->popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    eext_object_event_callback_add(ad->popup, EEXT_CALLBACK_BACK, popup_hide_cb, ad->popup);
    evas_object_smart_callback_add(ad->popup, "dismissed", popup_hide_finished_cb, ad->popup);

    layout = elm_layout_add(ad->popup);
    elm_layout_theme_set(layout, "layout", "popup", "content/circle/buttons2");
    elm_object_part_text_set(layout, "elm.text.title", "TRAINING MODE");

    elm_object_part_text_set(layout, "elm.text", "트레이닝 모드로 전환합니다.<br>"
            " <font color=#FF0000>2가지</font color> 동작을 각각 <font color=#FF0000>3번</font color> 반복합니다.<br>"
            "계속하시겠습니까?");
    elm_object_content_set(ad->popup, layout);

    btn = elm_button_add(ad->popup);
    elm_object_style_set(btn, "popup/circle/left");
    evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_object_part_content_set(ad->popup, "button1", btn);
    evas_object_smart_callback_add(btn, "clicked", popup_hide_cb, ad->popup);

    icon = elm_image_add(btn);
    elm_image_file_set(icon, ICON_DIR"/tw_ic_popup_btn_delete.png", NULL);
    evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_object_part_content_set(btn, "elm.swallow.content", icon);
    evas_object_show(icon);

    btn = elm_button_add(ad->popup);
    elm_object_style_set(btn, "popup/circle/right");
    evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_object_part_content_set(ad->popup, "button2", btn);
    evas_object_smart_callback_add(btn, "clicked", response_training_yes_cb, data);

    icon = elm_image_add(btn);
    elm_image_file_set(icon, ICON_DIR"/tw_ic_popup_btn_check.png", NULL);
    evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_object_part_content_set(btn, "elm.swallow.content", icon);
    evas_object_show(icon);

    evas_object_show(ad->popup);
}

static void response_data_no_cb(void *data, Evas_Object *obj, void *event_info)
{
    appdata_s *ad = (appdata_s *) data;

    if(!data) return;

    // close popup
    elm_popup_dismiss(ad->popup);

    // add new naviframe
    Stretch_Guide_cb(data, NULL, NULL);

}

static void response_data_yes_cb(void *data, Evas_Object *obj, void *event_info)
{
    appdata_s *ad = (appdata_s *) data;

    if(!data) return;

    ad->training_cnt++;
    if(ad->training_cnt > 3) {
        ad->stretch_sequence--;
        ad->training_cnt = 1;
    }

    // close popup
    elm_popup_dismiss(ad->popup);

    // add new naviframe
    Stretch_Guide_cb(data, NULL, NULL);

}

void popup_data_confirm_cb(void *data, Evas_Object *obj, void *event_info)
{
    Evas_Object *btn;
    Evas_Object *icon;
    Evas_Object *layout;
    Evas_Smart_Cb yes_cb;
    char buff[512];
    appdata_s *ad = (appdata_s *) data;

    ad->popup = elm_popup_add(ad->nf);
    elm_object_style_set(ad->popup, "circle");
    evas_object_size_hint_weight_set(ad->popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    eext_object_event_callback_add(ad->popup, EEXT_CALLBACK_BACK, popup_hide_cb, ad->popup);
    evas_object_smart_callback_add(ad->popup, "dismissed", popup_hide_finished_cb, ad->popup);

    layout = elm_layout_add(ad->popup);
    elm_layout_theme_set(layout, "layout", "popup", "content/circle/buttons2");
    elm_object_part_text_set(layout, "elm.text.title", "TRAINING MODE");

    if(ad->stretch_sequence == 0) {  //ARM_UP
        if(ad->training_cnt >= 3) {
            snprintf(buff, sizeof(buff),
                     "모든 동작을 <font color=#FF0000>완료</font color>하셨습니다.<br>"
                             "끝내시겠습니까?");
            yes_cb = popup_small_process_cb;
        } else {
            snprintf(buff, sizeof(buff),
                     "위로 팔뻗기 동작을 <font color=#FF0000>%d회</font color> 수행하셨습니다.<br>"
                             "한번 더 하시겠습니까?",
                     ad->training_cnt);
            yes_cb = response_data_yes_cb;
        }
    }else if(ad->stretch_sequence == 1) {  //FORWARD
        if(ad->training_cnt >= 3) {
            snprintf(buff, sizeof(buff),
                     "앞으로 팔뻗기 동작을 <font color=#FF0000>완료</font color>하셨습니다.<br>"
                             "다음 동작으로 진행하시겠습니까??");
            yes_cb = response_data_yes_cb;
        } else {
            snprintf(buff, sizeof(buff),
                     "앞으로 팔뻗기 동작을 <font color=#FF0000>%d회 </font color> 수행하셨습니다.<br>"
                             "한번 더 하시겠습니까?",
                     ad->training_cnt);
            yes_cb = response_data_yes_cb;
        }

    }

    elm_object_part_text_set(layout, "elm.text", buff);
    elm_object_content_set(ad->popup, layout);

    btn = elm_button_add(ad->popup);
    elm_object_style_set(btn, "popup/circle/left");
    evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_object_part_content_set(ad->popup, "button1", btn);
    evas_object_smart_callback_add(btn, "clicked", response_data_no_cb, data);

    icon = elm_image_add(btn);
    elm_image_file_set(icon, ICON_DIR"/tw_ic_popup_btn_delete.png", NULL);
    evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_object_part_content_set(btn, "elm.swallow.content", icon);
    evas_object_show(icon);

    btn = elm_button_add(ad->popup);
    elm_object_style_set(btn, "popup/circle/right");
    evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_object_part_content_set(ad->popup, "button2", btn);
    evas_object_smart_callback_add(btn, "clicked", yes_cb, data);

    icon = elm_image_add(btn);
    elm_image_file_set(icon, ICON_DIR"/tw_ic_popup_btn_check.png", NULL);
    evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_object_part_content_set(btn, "elm.swallow.content", icon);
    evas_object_show(icon);

    evas_object_show(ad->popup);
}

void
popup_small_process_cb(void *data, Evas_Object *obj, void *event_info)
{
    Evas_Object *layout, *progressbar;
    appdata_s *ad = (appdata_s *)data;

    elm_popup_dismiss(ad->popup);

    ad->popup = elm_popup_add(ad->nf);
    elm_object_style_set(ad->popup, "circle");
    eext_object_event_callback_add(ad->popup, EEXT_CALLBACK_BACK, popup_hide_cb, ad->popup);
    evas_object_smart_callback_add(ad->popup, "dismissed", popup_hide_finished_cb, ad->popup);

    layout = elm_layout_add(ad->popup);
    char edj_path[PATH_MAX] = {0, };
    app_get_resource(EDJ_FILE, edj_path, (int)PATH_MAX);
    elm_layout_file_set(layout, edj_path, "popup_progressbar");
    elm_object_content_set(ad->popup, layout);
    elm_object_part_text_set(layout,"elm.text",
            "동작 인식 모델<br>"
            "최적화 중입니다...");
    evas_object_show(layout);

    progressbar = elm_progressbar_add(layout);
    elm_object_style_set(progressbar, "process/popup/small");
    evas_object_size_hint_align_set(progressbar, EVAS_HINT_FILL, 0.5);
    evas_object_size_hint_weight_set(progressbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_progressbar_pulse(progressbar, EINA_TRUE);
    elm_object_part_content_set(layout, "elm.swallow.content", progressbar);
    evas_object_show(progressbar);

    evas_object_show(ad->popup);

    Model_Retraining_cb(data, NULL, NULL);
}

void popup_training_done_cb(void *data, Evas_Object *obj, void *event_info)
{
    Evas_Object *btn;
    Evas_Object *layout;
    appdata_s *ad = (appdata_s *) data;

    ad->popup = elm_popup_add(ad->nf);
    elm_object_style_set(ad->popup, "circle");
    evas_object_size_hint_weight_set(ad->popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    eext_object_event_callback_add(ad->popup, EEXT_CALLBACK_BACK, popup_hide_cb, ad->popup);
    evas_object_smart_callback_add(ad->popup, "dismissed", popup_hide_finished_cb, ad->popup);

    layout = elm_layout_add(ad->popup);
    elm_layout_theme_set(layout, "layout", "popup", "content/circle");

    elm_object_part_text_set(layout, "elm.text", "최적화가 완료되었습니다!");
    elm_object_content_set(ad->popup, layout);

    btn = elm_button_add(ad->popup);
    elm_object_style_set(btn, "bottom");
    elm_object_text_set(btn, "OK");
    evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_object_part_content_set(ad->popup, "button1", btn);
    evas_object_smart_callback_add(btn, "clicked", popup_hide_cb, ad->popup);

    evas_object_show(ad->popup);
}
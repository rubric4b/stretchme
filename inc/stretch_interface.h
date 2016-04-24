//
// Created by hobbang5 on 2016-03-28.
//

#ifndef STRETCHME_STRETCH_INTERFACE_H
#define STRETCHME_STRETCH_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    STRETCH_MODE_NONE,
    STRETCH_MODE_TRAIN,
    STRETCH_MODE_EVAL,
    STRETCH_MODE_DATA
}StretchMode;

typedef enum
{
    STRETCH_TYPE_NONE,
    STRETCH_TYPE_ARM_UP,
    STRETCH_TYPE_ARM_UP_AND_SWING, // NOT YET SUPPORTED
    STRETCH_TYPE_ARM_FORWARD, // NOT YET SUPPORTED
    STRETCH_TYPE_ARM_BACK, // NOT YET SUPPORTED
    STRETCH_TYPE_NUM
}StretchType;

typedef enum
{
    STRETCH_STATE_NONE,
    STRETCH_STATE_UNFOLD, // stretch the limbs
    STRETCH_STATE_HOLD, // stretching at the maximum position
    STRETCH_STATE_SWING_1, // swing left or up from holding position
    STRETCH_STATE_SWING_1_BACK, // swing back to holding position
    STRETCH_STATE_SWING_2, // swing right or down from holding position
    STRETCH_STATE_SWING_2_BACK, // swing back to holding position
    STRETCH_STATE_FOLD, // back to idle state
    STRETCH_STATE_CONFIRM // for training mode
}StretchState;

typedef struct _StretchConfig
{
    StretchMode mode;
    StretchType type;
    StretchState state;
}StretchConfig;

typedef enum
{
    STRETCH_SUCCESS,
    STRETCH_FAIL,
    STRETCH_CANCEL // canceled by another request
}StretchResult;

typedef void (*Stretching_Result_Cb)(StretchConfig conf, StretchResult result, void *data);

/**
 * Sensitivity 0.0 ~ 1.0
 * 0.0 means insensitive => HIGH probability for success
 * 0.5 is default value
 */
//void stretching_set_sensitivity(float sensitivity);

/**
 * Stretching manager is singleton
 * It can handle the only one stretching action
 * If you ask to start it again before the result callback is returned, then previous request will be canceled
 */
void stretching_start(StretchConfig conf, Stretching_Result_Cb func, void* data);
void stretching_stop();
void stretch_manager_release();

void auto_start_stretch(void *data);
void streching_data_gathering(void *data);
bool retraining_model(StretchType type);
void hmm_init();

/**
 * get the last matching rate (percentage)
 */
//float stretching_get_matching_rate();

#ifdef __cplusplus
}
#endif

#endif //STRETCHME_STRETCH_INTERFACE_H

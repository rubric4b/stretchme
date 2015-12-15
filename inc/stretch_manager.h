#ifndef __STRETCH_MANAGER_H__
#define __STRETCH_MANAGER_H__

#include <stdio.h>
#include <stdlib.h>

typedef enum
{
	STRETCH_ARM_UP,
	STRETCH_ARM_UP_AND_SWING, // NOT YET SUPPORTED
	STRETCH_ARM_FORWARD, // NOT YET SUPPORTED
	STRETCH_ARM_BACK, // NOT YET SUPPORTED
	STRETCH_TYPE_NUM
}StretchType;

typedef enum
{
	STRETCH_STATE_UNFOLD, // stretch the limbs
	STRETCH_STATE_HOLD, // stretching at the maximum position
	STRETCH_STATE_SWING_1, // swing left or up from holding position
	STRETCH_STATE_SWING_1_BACK, // swing back to holding position
	STRETCH_STATE_SWING_2, // swing right or down from holding position
	STRETCH_STATE_SWING_2_BACK, // swing back to holding position
	STRETCH_STATE_FOLD // back to idle state
}StretchState;

typedef enum
{
	STRETCH_SUCCESS,
	STRETCH_FAIL,
	STRETCH_CANCEL // canceled by another request
}StretchResult;

typedef void (*Stretching_Result_Cb)(StretchType type, StretchState state, StretchResult result, void *data);

/**
 * Sensitivity 0.0 ~ 1.0
 * 0.0 means insensitive => HIGH probability for success
 * 0.5 is default value
 */
void stretching_set_sensitivity(float sensitivity);

/**
 * Stretching manager is singleton
 * It can handle the only one stretching action
 * If you ask to start it again before the result callback is returned, then previous request will be canceled
 */
void stretching_start(StretchType type, StretchState state, Stretching_Result_Cb func, void* data);
void stretching_stop();

/**
 * get the last matching rate (percentage)
 */
float stretching_get_matching_rate();

#endif // __STRETCH_MANAGER_H__

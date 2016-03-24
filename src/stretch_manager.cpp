#include <string.h>
#include <sensor/sensor.h>
#include <sm_sensor.h>

#include "logger.h"
#include "stretch_manager.h"
#include "sm_hmm/hmm_manager.h"

#ifndef bool
typedef unsigned char bool;
#endif

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

using namespace std;
using namespace glm;

typedef struct
{
	StretchType type;
	StretchState state;

	float last_matching_rate;
	float sensitivity;

	// callback
	Stretching_Result_Cb func;
	void* func_data;

	bool is_progress;

	// sensor
	sensor_info* accel;

	// sm_hmm model
	Hmm_Manager * hmm_mgr;

}StretchManager;

static StretchManager* sMgr = NULL;

static void sensor_control(bool enable, bool reset)
{
	static bool isEnabled = false;
	if(reset)
	{
		reset_measure();
	}

	if(isEnabled && !enable)
	{
		sensor_listen_pause(sMgr->accel);
		isEnabled = enable;
	}
	else if(!isEnabled && enable)
	{
		sensor_listen_resume(sMgr->accel);
		isEnabled = enable;
	}
}

static void stretching_sensor_cb(void* data)
{
	// get sensor data
	sensor_data_info curr_si = get_current_sensor_data();
	StretchResult stretch_result = STRETCH_FAIL;
	Hmm_Manager * hMgr = sMgr->hmm_mgr;
	hMgr->set_CurrentType(sMgr->type);

	static bool callback_flag = false;

	switch(sMgr->state) {
		case STRETCH_STATE_UNFOLD : {
			hMgr->perform_Stretching(curr_si.kAcc);

			if(curr_si.timestamp > 3500 && hMgr->is_End() || curr_si.timestamp > 10000) {
				DBG("%4d log p = %5f\n",curr_si.timestamp, hMgr->get_Probability());
				if(-hMgr->get_Probability() < hMgr->get_Threshold()) {
					stretch_result = STRETCH_SUCCESS;
				}

				callback_flag = true;
			}

		}
			break;

		case STRETCH_STATE_HOLD : {

			if(curr_si.timestamp > 1000) {
				callback_flag = true;
				stretch_result = STRETCH_SUCCESS;
			}

		}
			break;

		default:
			break;

	}


	if(callback_flag) {
		callback_flag = false;

		hMgr->reset_Model_Performing(sMgr->type);
		sMgr->func(sMgr->type, sMgr->state, stretch_result, sMgr->func_data);

	}

}

static void stretch_manager_initialize()
{
	if(sMgr)
		return;

	sMgr = (StretchManager*)malloc(sizeof(StretchManager));
	memset(sMgr, 0x00, sizeof(StretchManager));

	sMgr->sensitivity = 0.5f;
	sMgr->last_matching_rate = 0.0f;

	// sensor initialize
	sMgr->accel = sensor_init(SENSOR_ACCELEROMETER);

	sensor_start(sMgr->accel);

	sensor_listen_pause(sMgr->accel);
	reset_measure();

	sMgr->hmm_mgr = new Hmm_Manager();

}

void stretch_manager_release() {
	if(sMgr) {
		if(sMgr->hmm_mgr) delete sMgr->hmm_mgr;

		if(sMgr->accel) {
			sensor_stop(sMgr->accel);
			sensor_release(sMgr->accel);
		}

		free(sMgr);
		sMgr = NULL;

	}
}

/**
 * Sensitivity 0.0 ~ 1.0
 * 0.0 means insensitive => HIGH probability for success
 * 0.5 is default value
 */
void stretching_set_sensitivity(float sensitivity)
{
	stretch_manager_initialize();

	sMgr->sensitivity = sensitivity;
}

/**
 * Stretching manager is singleton
 * It can handle the only one stretching action
 * If you ask to start it again before the result callback is returned, then previous request will be canceled
 */
void stretching_start(StretchType type, StretchState state, Stretching_Result_Cb func, void* data)
{
	stretch_manager_initialize();

	if(sMgr->is_progress && sMgr->func)
	{
		sMgr->func(sMgr->type, sMgr->state, STRETCH_CANCEL, sMgr->func_data);
	}

	sMgr->type = type;
	sMgr->state = state;
	sMgr->func = func;
	sMgr->func_data = data;
	sMgr->is_progress = true;

	// turn on the sensor
	sensor_control(true, true);

	// register callback to get sensor event data
	sensor_callback_register(stretching_sensor_cb, NULL);

	// TODO: HMM
	// get sequence

}

void stretching_stop()
{
	sensor_control(false, false);

	if(sMgr && sMgr->is_progress && sMgr->func)
	{
		sMgr->func(sMgr->type, sMgr->state, STRETCH_CANCEL, sMgr->func_data);
	}

	sMgr->is_progress = false;
}

/**
 * get the last matching rate (percentage)
 */
float stretching_get_matching_rate()
{
	if(sMgr)
	{
		return sMgr->last_matching_rate;
	}

	return 0.0f;
}


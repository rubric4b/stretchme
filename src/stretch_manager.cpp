#include <string.h>
#include <sensor/sensor.h>
#include <sm_sensor.h>
#include <logger.h>

#include "sequence.h"
#include "stretch_manager.h"
#include "sm_sensor.h"

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
	sensor_info* gyro;

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
		sensor_listen_pause(sMgr->gyro);
		isEnabled = enable;
	}
	else if(!isEnabled && enable)
	{
		sensor_listen_resume(sMgr->accel);
		sensor_listen_resume(sMgr->gyro);
		isEnabled = enable;
	}
}

static void stretching_sensor_cb(void* data)
{
	// get sensor data
	SensorIntegration si = get_current_sensor_data();
	Sequence seq;
	Sequence seq_pca;

	if (si.linearAcc.size() > 200)
	{
		vector<vec3>::iterator itr = si.linearAcc.end();

		itr--;
		float len = 0;
		DBG("Sequence last11111 %f %f %f num: %d\n", itr.base()->x,itr.base()->y,itr.base()->z, seq.GetRefNum(*itr));
		len = length(*itr--);
		DBG("Sequence last22222 %f %f %f num: %d\n", itr.base()->x,itr.base()->y,itr.base()->z, seq.GetRefNum(*itr));
		len += length(*itr--);
		DBG("Sequence last33333 %f %f %f num: %d\n", itr.base()->x,itr.base()->y,itr.base()->z, seq.GetRefNum(*itr));
		len += length(*itr--);

		DBG("Sequence tail length: %f\n", len);
		// not moving
		if (len < 2.0)
		{

			DBG("Sequence generate!\n");
			seq.CreateSymbols(si.linearAcc);
			seq.PrintSymbols();

			DBG("Pca Sequence generate!\n");
			seq_pca.CreateSymbols(si.pcaAcc);
			seq_pca.PrintSymbols();

			sMgr->func(sMgr->type, sMgr->state, STRETCH_SUCCESS, sMgr->func_data);
			// make false the progressing
			sMgr->is_progress = false;
			stretching_stop();
		}


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
	sMgr->gyro = sensor_init(SENSOR_GYROSCOPE);

	sensor_start(sMgr->accel);
	sensor_start(sMgr->gyro);

	sensor_listen_pause(sMgr->accel);
	sensor_listen_pause(sMgr->gyro);
	reset_measure();
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


#include "main.h"
#include "sm_sensor.h"
#include "stretch_manager.h"
#include "sm_hmm/hmm_manager.h"
#include "logger.h"

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

#if 0
// TODO: need to operate the stretching manager by timer instead of sensor based hmm
static Eina_Bool _stretching_timer_cb(void *data)
{
//	stretch_Manager* mgr = static_cast<stretch_Manager*>(data);

	return ECORE_CALLBACK_CANCEL;
}
#endif

void stretch_Manager::init() {
	DBG("stretch_Manager::init()\n");
	m_sensitivity = 0.5f;
	m_lastMatchingRate = 0.0f;

	// sensor initialize
	m_accel = sm_Sensor(SENSOR_ACCELEROMETER);
}

void stretch_Manager::release() {
	DBG("stretch_Manager::release()\n");
	m_accel.stop();
	m_accel.release();
}

void stretch_Manager::start(StretchConfig conf, Stretching_Result_Cb func, void *data) {
	appdata_s *ad = (appdata_s *)data;

	DBG("stretch_Manager::start() - conf[mode,type,state] = %d,%d,%d\n", conf.mode, conf.type, conf.state);
	if(m_isProgress && m_resultCbFunc) {
		m_isProgress = false;
		m_resultCbFunc(m_stConf, STRETCH_CANCEL, m_resultCbData);
	}

	m_exType = ad->ex_type;
	m_stConf = conf;
	m_resultCbFunc = func;
	m_resultCbData = data;
	m_isProgress = true;

	// turn on the sensor
	m_accel.register_Callback(sensorCb, this);
	m_accel.start();

#if 0
	if(m_exType == EXPERIMENT_1)
	{
		m_timer = ecore_timer_add(2.5f, _stretching_timer_cb, this);
	}
#endif

}

stretch_Manager::stretch_Manager() :
	m_stConf(),
	m_lastMatchingRate(0),
	m_sensitivity(0),
	m_resultCbFunc(NULL),
	m_isProgress(false),
	m_accel(SENSOR_ACCELEROMETER)
{
	const static StretchConfig init_conf = {STRETCH_MODE_NONE, STRETCH_TYPE_NONE, STRETCH_STATE_NONE};
	m_stConf = init_conf;
}

stretch_Manager::~stretch_Manager() {
	m_accel.release();
}

void stretch_Manager::stop() {
	m_accel.pause();

	if(m_isProgress && m_resultCbFunc) {
		//m_isProgress = false;
		m_resultCbFunc(m_stConf, STRETCH_CANCEL, m_resultCbData);
	}


}

void stretch_Manager::sensorCb(const sm_Sensor &sensor, void *data) {
	stretch_Manager* mgr = static_cast<stretch_Manager*>(data);
	mgr->eval(sensor);
}

#define hMgr Hmm_Manager::Inst()

void stretch_Manager::eval(const sm_Sensor &sensor) {
	// get sensor data
	StretchResult stretch_result = STRETCH_FAIL;
	bool callback_flag(false);
	static vec3 ob_hold(0);

	hMgr.set_CurrentType(m_stConf.type);

	switch (m_stConf.type) {
		case STRETCH_TYPE_ARM_UP : {
			switch(m_stConf.state) {
				case STRETCH_STATE_UNFOLD : {
					hMgr.perform_Stretching( sensor.m_currKData );

					if(hMgr.is_End() || sensor.m_timestamp > 30000) {
						callback_flag = true;
						double prob = hMgr.get_Probability();
						DBG("%4d log p = %5f\n",sensor.m_timestamp, prob);

						if(-prob < hMgr.get_Threshold() && prob != 0) {
							stretch_result = STRETCH_SUCCESS;
						}
//						stretch_result = STRETCH_SUCCESS; //TESTCODE!!
					}
				}

#if 0 //EXPERIMENT == 1
				if (sensor.m_timestamp > 2500) {
					callback_flag = true;
					stretch_result = STRETCH_SUCCESS;
				}
#endif

					break;

				case STRETCH_STATE_HOLD : {
					if(sensor.m_timestamp > 1000) {
						if (length(ob_hold) == 0) {
							ob_hold = sensor.m_currData;
						}
						vec3 cur_norm = normalize(sensor.m_currKData);
						vec3 pre_norm = normalize(ob_hold);

						double theta = acos(dot(cur_norm, pre_norm));

						if (theta > radians(10.0) && !isnan(theta)) {
							callback_flag = true;
							stretch_result = STRETCH_FAIL;
						}
					}

					if(sensor.m_timestamp > 5000) {
						callback_flag = true;
						stretch_result = STRETCH_SUCCESS;
					}
				}

					break;

				case STRETCH_STATE_FOLD : {
					/*if (sensor.m_timestamp > 1000) {
						hMgr.analyze_Observation(sensor.m_currKData);
					}*/

//					if (hMgr.get_End() || sensor.m_timestamp > 5000) {
					if (sensor.m_timestamp > 2000) {
						DBG("fold time stamp %d\n", sensor.m_timestamp);
						callback_flag = true;
						stretch_result = STRETCH_SUCCESS;
					}
				}
					break;

				default: //switch STATE
					break;

			}
		}
			break;

		case STRETCH_TYPE_ARM_FORWARD : {

			switch (m_stConf.state) {
				case STRETCH_STATE_UNFOLD : {
					hMgr.perform_Stretching(sensor.m_currKData);

					if (hMgr.is_End() || sensor.m_timestamp > 30000) {
						callback_flag = true;
						double prob = hMgr.get_Probability();
						DBG("%4d log p = %5f\n", sensor.m_timestamp, prob);

						if (-prob < hMgr.get_Threshold() && prob != 0) {
							stretch_result = STRETCH_SUCCESS;
						}
//						stretch_result = STRETCH_SUCCESS; //TESTCODE!!
					}

#if 0 //EXPERIMENT == 1
					if (sensor.m_timestamp > 2500) {
						callback_flag = true;
						stretch_result = STRETCH_SUCCESS;
					}
#endif
				}
					break;

				case STRETCH_STATE_HOLD : {
					if (sensor.m_timestamp > 1000) {
						if (length(ob_hold) == 0) {
							ob_hold = sensor.m_currData;
						}
						vec3 cur_norm = normalize(sensor.m_currKData);
						vec3 pre_norm = normalize(ob_hold);

						double theta = acos(dot(cur_norm, pre_norm));

						if (theta > radians(10.0) && !isnan(theta)) {
							callback_flag = true;
							stretch_result = STRETCH_FAIL;
						}
					}

					if (sensor.m_timestamp > 5000) {
						callback_flag = true;
						stretch_result = STRETCH_SUCCESS;
					}

				}
					break;

				case STRETCH_STATE_FOLD : {
					/*if (sensor.m_timestamp > 1000) {
						hMgr.analyze_Observation(sensor.m_currKData);
					}*/

//					if (hMgr.get_End() || sensor.m_timestamp > 5000) {
					if (sensor.m_timestamp > 2000) {
						DBG("fold time stamp %d\n", sensor.m_timestamp);
						callback_flag = true;
						stretch_result = STRETCH_SUCCESS;
					}
				}
					break;

				default: //switch STATE
					break;


			}
			break;

			default:  //switch TYPE

					break;

		}

	}

	if(callback_flag) {
		if(stretch_result == STRETCH_FAIL || m_stConf.state == STRETCH_STATE_FOLD) {
			m_accel.stop();
		}else{
			m_accel.pause();
		}

		m_accel.register_Callback(NULL, NULL);
		ob_hold = vec3(0);

		m_isProgress = false;
		hMgr.reset_Model_Performing();

		m_resultCbFunc(m_stConf, stretch_result, m_resultCbData);

	}
}



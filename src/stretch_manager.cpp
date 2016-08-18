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

#if 1
// TODO: need to operate the stretching manager by timer instead of sensor based hmm
static Eina_Bool _stretching_timer_cb(void *data)
{
	stretch_Manager* mgr = static_cast<stretch_Manager*>(data);
	mgr->timerCb();

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

#define EX1_UNFOLD_DURATION	2.5f
#define HOLD_DURATION	5.0f
#define FOLD_DURATION	2.0f

void stretch_Manager::start(StretchConfig conf, Stretching_Result_Cb func, void *data) {
	appdata_s *ad = (appdata_s *)data;

	DBG("exType[%d] conf[mode,type,state] = %d,%d,%d\n", ad->ex_type, conf.mode, conf.type, conf.state);
	if(m_isProgress && m_resultCbFunc) {
		m_isProgress = false;
		m_resultCbFunc(m_stConf, STRETCH_CANCEL, 0.0f, m_resultCbData, NULL, 0);
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
		if(m_timer)
		{
			ecore_timer_del(m_timer);
			m_timer = NULL;
		}

		if(conf.state == STRETCH_STATE_UNFOLD)
		{
		DBG("log reset");
			mProb = 0.0f;
			mResult = STRETCH_FAIL;

			m_timer = ecore_timer_add(UNFOLD_DURATION + 0.1f, _stretching_timer_cb, this);
		}
		else if(conf.state == STRETCH_STATE_HOLD)
		{
			m_timer = ecore_timer_add(HOLD_DURATION + 0.1f, _stretching_timer_cb, this);
		}
		else if(conf.state == STRETCH_STATE_FOLD)
		{
			m_timer = ecore_timer_add(FOLD_DURATION + 0.1f, _stretching_timer_cb, this);
		}
	}
#endif

}

void stretch_Manager::timerCb()
{
	m_resultCbFunc(m_stConf, mResult, mProb, m_resultCbData, NULL, 0);
}


stretch_Manager::stretch_Manager() :
	m_stConf(),
	m_lastMatchingRate(0),
	m_sensitivity(0),
	m_resultCbFunc(NULL),
	m_resultCbData(NULL),
	m_isProgress(false),
	m_accel(SENSOR_ACCELEROMETER),
	m_exType(EXPERIMENT_MAX),
	m_timer(NULL),
	mProb(0.0),
	mResult()
{
	const static StretchConfig init_conf = {STRETCH_MODE_NONE, STRETCH_TYPE_NONE, STRETCH_STATE_NONE};
	m_stConf = init_conf;
}

stretch_Manager::~stretch_Manager() {
	m_accel.release();
}

void stretch_Manager::stop() {
	DBG("stop config[%d,%d,%d]",m_stConf.mode, m_stConf.state, m_stConf.type);
	m_accel.stop();

	if(m_isProgress && m_resultCbFunc) {
		m_isProgress = false;
		m_resultCbFunc(m_stConf, STRETCH_CANCEL, 0.0f, m_resultCbData, NULL, 0);
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
	static double prob = 0.0f;
	static bool hold_fail_flag(false);

	hMgr.set_CurrentType(m_stConf.type);

	switch (m_stConf.type) {
		case STRETCH_TYPE_ARM_UP :
		case STRETCH_TYPE_ARM_FORWARD :
		{
			switch(m_stConf.state)
			{
				case STRETCH_STATE_UNFOLD : {
					hMgr.perform_Stretching( sensor.m_currKData );
					if((m_exType == EXPERIMENT_1) ?
					   (sensor.m_timestamp > EX1_UNFOLD_DURATION * 1000) :
					   (hMgr.is_End() || sensor.m_timestamp > 8000)) {
						callback_flag = true;
						prob = hMgr.get_Probability();
						DBG("%4d log p = %5f\n",sensor.m_timestamp, prob);

						if(prob > hMgr.get_Threshold() && prob != 0) {
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
							hold_fail_flag = true;
							DBG("log : hold fail");
						}
					}

					if((m_exType == EXPERIMENT_1) ?
					   (sensor.m_timestamp > HOLD_DURATION * 1000) :
					   (hold_fail_flag || sensor.m_timestamp > HOLD_DURATION * 1000)) {
						DBG("hold time stamp %d\n", sensor.m_timestamp);
						callback_flag = true;
						stretch_result = hold_fail_flag ? STRETCH_FAIL : STRETCH_SUCCESS;
					}
				}

					break;

				case STRETCH_STATE_FOLD : {
					/*if (sensor.m_timestamp > 1000) {
						hMgr.analyze_Observation(sensor.m_currKData);
					}*/

//					if (hMgr.get_End() || sensor.m_timestamp > 5000) {
					if (sensor.m_timestamp > (FOLD_DURATION * 1000)) {
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

#if 0
		case STRETCH_TYPE_ARM_FORWARD :
		{

			switch (m_stConf.state) {
				case STRETCH_STATE_UNFOLD : {
					hMgr.perform_Stretching(sensor.m_currKData);

					if (hMgr.is_End() || sensor.m_timestamp > (EX1_UNFOLD_DURATION * 1000)) {
						callback_flag = true;
						prob = hMgr.get_Probability();
						DBG("%4d log p = %5f(t : %f)\n", sensor.m_timestamp, prob, hMgr.get_Threshold());

						if (prob > hMgr.get_Threshold() && prob != 0) {
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
							DBG("log : hold fail");
						}
					}

					if (sensor.m_timestamp > (HOLD_DURATION * 1000)) {
						DBG("hold time stamp %d\n", sensor.m_timestamp);
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
					if (sensor.m_timestamp > (FOLD_DURATION * 1000)) {
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
#endif
			default:  //switch TYPE
				break;

	}

	if(callback_flag) {
		/*if(stretch_result == STRETCH_FAIL || m_stConf.state == STRETCH_STATE_FOLD) {
			m_accel.stop();
		}else{
			m_accel.pause();
		}*/
		m_accel.stop();

		m_accel.register_Callback(NULL, NULL);
		ob_hold = vec3(0);
		hold_fail_flag = false;

		m_isProgress = false;
		hMgr.reset_Model_Performing();

		m_resultCbFunc(m_stConf, stretch_result, prob, m_resultCbData,
					   hMgr.get_MotionData(), hMgr.get_MotionCount());

		/*if(m_exType == EXPERIMENT_1)
		{
			DBG("log : result (%d), p(%f)", stretch_result, prob);
			mResult = stretch_result;
			mProb = prob;
		}
		else
		{

		}*/
	}
}



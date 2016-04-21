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

void stretch_Manager::init() {
	DBG("stretch_Manager::init()\n");
	m_sensitivity = 0.5f;
	m_lastMatchingRate = 0.0f;

	// sensor initialize
	m_accel = sm_Sensor(SENSOR_ACCELEROMETER);
	m_accel.start();
	m_accel.pause();
}

void stretch_Manager::release() {
	DBG("stretch_Manager::release()\n");
	m_accel.stop();
	m_accel.release();
}

void stretch_Manager::start(StretchConfig conf, Stretching_Result_Cb func, void *data) {
	DBG("stretch_Manager::start() - conf[mode,type,state] = %d,%d,%d\n", conf.mode, conf.type, conf.state);
	if(m_isProgress && m_resultCbFunc) {
		m_isProgress = false;
		m_resultCbFunc(m_stConf, STRETCH_CANCEL, m_resultCbData);
	}

	m_stConf = conf;
	m_resultCbFunc = func;
	m_resultCbData = data;
	m_isProgress = true;

	// turn on the sensor
	m_accel.register_Callback(sensorCb, this);
	m_accel.start();


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

	hMgr.set_CurrentType(m_stConf.type);


	switch(m_stConf.state) {
		case STRETCH_STATE_UNFOLD : {
			hMgr.perform_Stretching( sensor.m_currKData );

			if(sensor.m_timestamp > 3500 && hMgr.is_End() || sensor.m_timestamp > 9000) {
				double prob = hMgr.get_Probability();
				DBG("%4d log p = %5f\n",sensor.m_timestamp, prob);
				if(-prob < hMgr.get_Threshold() && prob != 0) {
					stretch_result = STRETCH_SUCCESS;
				}

				callback_flag = true;
			}

		}
			break;

		case STRETCH_STATE_HOLD : {
			if(sensor.m_timestamp > 5000) {
				callback_flag = true;
				stretch_result = STRETCH_SUCCESS;
			}

		}
			break;

		default:
			break;

	}

	if(callback_flag) {
		m_isProgress = false;

		hMgr.reset_Model_Performing(m_stConf.type);
		m_resultCbFunc(m_stConf, stretch_result, m_resultCbData);

	}
}


















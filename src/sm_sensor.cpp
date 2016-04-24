#include "sm_sensor.h"
#include "logger.h"

sm_Sensor::sm_Sensor(sensor_type_e sensor_type,
					 sensor_event_cb event_cb_func,
					 void* event_cb_data,
					 unsigned int update_ms) :
	m_isStart(false),
	m_initTime(0),
	m_timestamp(0),
	m_prevData(0),
	m_prevKData(0),
	m_currData(0),
	m_currKData(0),
	m_snType(SENSOR_LAST),
	m_snHandle(NULL),
	m_snListener(NULL),
	m_snListenerCbFunc(event_cb_func),
	m_snListenerCbData(event_cb_data),
	m_on_snListenerCb(false),
	m_updateMs(update_ms),
	m_valueMin(0),
	m_valueMax(0),
	m_valueRange(0),
	m_snCbFunc(NULL),
	m_snCbData(NULL),
	m_kFilter(KalmanGearS2::ACCELEROMETER, 0.00001)
{
	if(!event_cb_data) m_snListenerCbData = this;
	init(sensor_type);
}

bool sm_Sensor::init(sensor_type_e type) {
	int error;
	bool supported;

	error = sensor_is_supported(type, &supported);
	if(error) {
		ERR("sensor type %d is not supported\n", type);
		return false;
	}

	// sensor handle
	m_snType = type;

	error = sensor_get_default_sensor(m_snType, &m_snHandle);
	if(error) {
		ERR("sensor_get_default_sensor failed\n");
		return false;
	}

	sensor_get_min_range(m_snHandle, &m_valueMin);
	sensor_get_max_range(m_snHandle, &m_valueMax);
	m_valueRange = m_valueMax - m_valueMin;

	// listener
	error = sensor_create_listener(m_snHandle, &m_snListener);
	if(error) {
		ERR("accel_listener creation failed\n");
		return false;
	}

	// listener callback, equal to resume();
	error = sensor_listener_set_event_cb(m_snListener, m_updateMs, m_snListenerCbFunc, m_snListenerCbData);
	if(error) {
		ERR("callback registration failed\n");
		return false;
	}
	m_on_snListenerCb = true;

	error = sensor_listener_set_option(m_snListener, SENSOR_OPTION_ON_IN_SCREEN_OFF);
	if(error) {
		ERR("sensor option failed\n");
		return false;
	}

	DBG("sucessed to initilize %d type sensor \n", type);
	return true;

}

bool sm_Sensor::start() {
	m_isStart = true;

	DBG("sm_Sensor::start()\n");
	reset();

	if(!m_on_snListenerCb) resume();

	int error = sensor_listener_start(m_snListener);
	if(error) {
		ERR("sensor start failed\n");
		return false;
	}

	return true;
}

bool sm_Sensor::stop() {
	m_isStart = false;

	DBG("sm_Sensor::stop()\n");
	int error = sensor_listener_unset_event_cb(m_snListener);
	if(error) {
		ERR("callback unregistration failed\n");
		return false;
	}
	m_on_snListenerCb = false;

	error = sensor_listener_stop(m_snListener);
	if(error) {
		ERR("sensor stop failed\n");
		return false;
	}


	return true;
}

bool sm_Sensor::release() {
	DBG("sm_Sensor::release()\n");
	if(m_snListener) {
		int error = sensor_destroy_listener(m_snListener);
		if(error) {
			ERR("sensor destory failed\n");
			return false;
		}
	}

	return true;
}

bool sm_Sensor::pause() {
	DBG("sm_Sensor::pause()\n");
	int error = sensor_listener_unset_event_cb(m_snListener);
	if(error) {
		ERR("callback unregistration failed\n");
		return false;
	}
	m_on_snListenerCb = false;

	return true;
}

bool sm_Sensor::resume() {
	DBG("sm_Sensor::resume()\n");
	int error = sensor_listener_set_event_cb(m_snListener, m_updateMs, m_snListenerCbFunc, m_snListenerCbData);
	if(error) {
		ERR("callback registration failed\n");
		return false;
	}
	m_on_snListenerCb = true;

	return true;
}

void sm_Sensor::reset() {
	m_initTime = 0;
	m_timestamp = 0;

	m_prevData = vec3(0);
	m_prevKData = vec3(0);
	m_currData = vec3(0);
	m_currKData = vec3(0);

	m_kFilter.Reset();
}

void sm_Sensor::register_Callback(Sensor_Cb sensor_cb_func, void *data) {
	m_snCbFunc = sensor_cb_func;
	m_snCbData = data;
}


void
sm_Sensor::listenerCb(sensor_h sensor, sensor_event_s *event, void *sensor_ptr)
{
	sm_Sensor* ss = static_cast<sm_Sensor*>(sensor_ptr);

	ss->tick(event);

	if(ss->m_snCbFunc) {
		ss->m_snCbFunc(*ss, ss->m_snCbData);
	}

}

sm_Sensor::~sm_Sensor() {
	release();
}

void sm_Sensor::tick(sensor_event_s *event) {
	m_prevData = m_currData;
	m_prevKData = m_currKData;

	// initialize init time
	if(m_initTime == 0) {
		m_initTime = event->timestamp / 1000;
	}

	m_timestamp = (unsigned int)(event->timestamp/1000 - m_initTime);

	switch (m_snType) {
		case SENSOR_ACCELEROMETER: {
			// Get sensor information
			m_currData = vec3(event->values[0], event->values[1], event->values[2]);
			m_kFilter.Step(m_currData, m_currKData);

/*
			DBG("n %.3f,%.3f,%.3f | k %.3f,%.3f,%.3f | t %d\n",
				m_currData.x, m_currData.y, m_currData.z,
				m_currKData.x, m_currKData.y, m_currKData.z,
			m_timestamp);
*/
		}
			break;

		default:
			break;
	}

}













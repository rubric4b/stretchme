#ifndef __SM_SENSOR_H__
#define __SM_SENSOR_H__

#include <sensor.h>

#include "glm/glm.hpp"
#include "kalman_manager.h"

#define SMSN_DEFAULT_UPDATE_MS 10

class sm_Sensor {
	typedef void (*Sensor_Cb)(const sm_Sensor &sensor, void *data);

public:
	sm_Sensor(sensor_type_e sensor_type,
			  sensor_event_cb event_cb_func = listenerCb,
			  unsigned int update_ms = SMSN_DEFAULT_UPDATE_MS);
	~sm_Sensor();

	bool init(sensor_type_e type);
	bool start();
	bool stop();
	bool release();
	bool pause();
	bool resume();
	void reset();

	void tick(sensor_event_s *event);

	void register_Callback(Sensor_Cb sensor_cb_func, void *data);

public:

	// abbreviate: sn = sensor
	unsigned long long m_initTime;
	unsigned int m_timestamp; // id

	glm::vec3 m_prevData;
	glm::vec3 m_prevKData;
	glm::vec3 m_currData;
	glm::vec3 m_currKData;

	sensor_type_e m_snType;
	sensor_h m_snHandle;
	sensor_listener_h m_snListener;
	sensor_event_cb m_snListenerCbFunc;
	bool m_on_snListenerCb;

	unsigned int m_updateMs;
	float m_valueMin;		/**< Minimal value */
	float m_valueMax;		/**< Maximal value */
	float m_valueRange;		/**< Values range */

	Sensor_Cb m_snCbFunc;
	void *m_snCbData;

	KalmanGearS2 m_kFilter;


private:
	//typedef void (*sensor_event_cb)(sensor_h sensor, sensor_event_s *event, void *data);
	static void listenerCb(sensor_h sensor, sensor_event_s *event, void *sensor_ptr);

};

#endif // __SM_SENSOR_H__ //

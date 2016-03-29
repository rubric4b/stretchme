#ifndef __STRETCH_MANAGER_H__
#define __STRETCH_MANAGER_H__

#include "stretch_interface.h"
#include "sm_sensor.h"

#include "singleton.hpp"

class stretch_Manager : public Singleton<stretch_Manager> {

public:
	stretch_Manager();
	virtual ~stretch_Manager();

	void start(StretchType type, StretchState state, Stretching_Result_Cb func, void* data);
	void stop();

	void init();
	void release();
	void eval(const sm_Sensor &sensor);

private:
	StretchType m_stType;
	StretchState m_stState;

	float m_lastMatchingRate;
	float m_sensitivity;

	// callback
	Stretching_Result_Cb m_resultCbFunc;
	void *m_resultCbData;

	bool m_isProgress;

	// sensor
	sm_Sensor m_accel;


private:
	//typedef void (*Sensor_Cb)(const sm_Sensor &sensor);
	static void sensorCb(const sm_Sensor &sensor, void *data);

};

#endif // __STRETCH_MANAGER_H__

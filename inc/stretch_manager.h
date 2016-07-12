#ifndef __STRETCH_MANAGER_H__
#define __STRETCH_MANAGER_H__

#include "main.h"
#include "stretch_interface.h"
#include "sm_sensor.h"

#include <Ecore.h>

#include "singleton.hpp"

class stretch_Manager : public Singleton<stretch_Manager> {

public:
	stretch_Manager();
	virtual ~stretch_Manager();

	void start(StretchConfig conf, Stretching_Result_Cb func, void* data);
	void stop();

	void init();
	void release();
	void eval(const sm_Sensor &sensor);

	void timerCb();

private:
	StretchConfig m_stConf;

	float m_lastMatchingRate;
	float m_sensitivity;

	// callback
	Stretching_Result_Cb m_resultCbFunc;
	void *m_resultCbData;

	bool m_isProgress;

	// sensor
	sm_Sensor m_accel;

	Experiment_Type m_exType;
	Ecore_Timer* m_timer; // timer for experiment 1. i.e., operating with timer instead of sensor based hmm

	// for ex1
	double mProb;
	StretchResult mResult;

private:
	//typedef void (*Sensor_Cb)(const sm_Sensor &sensor);
	static void sensorCb(const sm_Sensor &sensor, void *data);

};

#endif // __STRETCH_MANAGER_H__

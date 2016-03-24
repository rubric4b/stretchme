#include <vector>
#include <limits>
#include <math.h>

#include "sm_sensor.h"
#include "logger.h"
#include "pca/embedppca.h"
#include "kalman_manager.h"

#define UPDATE_RATE 10 // millisecond

using namespace glm;

sensor_data_info prev;
sensor_data_info current;

// sensor callback
static Sensor_Cb sensor_callback_func = NULL;
static void * sensor_callback_func_data = NULL;

static void reset_sensor_data_info(sensor_data_info& si)
{
	si.timestamp = 0;
	si.isAccUpdated = 0;
	si.acc = vec3(0);
	si.kAcc = vec3(0);

}

static unsigned long long initTime = 0;

static void
on_sensor_event(sensor_h sensor, sensor_event_s *event, void *user_data)
{
	static unsigned long long accCount = 0;

	static KalmanGearS2 accKalman(KalmanGearS2::ACCELEROMETER, 0.00001);

	// Select a specific sensor with a sensor handle
	// This example uses sensor type, assuming there is only 1 sensor for each type
	sensor_type_e type;
	sensor_get_type(sensor, &type);

	bool reset = false;

	// initialize all
	if(initTime == 0)
	{
		initTime = event->timestamp/1000;

		reset_sensor_data_info(prev);
		reset_sensor_data_info(current);

		accKalman.Reset();

		// skip the 1st data since it has accelerometer data only. We need correct pair of (accel & gyro)
		//	   return;
		reset = true;
	}

	unsigned int timeDiff = (unsigned int)(event->timestamp/1000 - initTime);

	switch (type)
	{
		case SENSOR_ACCELEROMETER:
		{
			// Use sensor information
			current.acc = vec3(event->values[0], event->values[1], event->values[2]);
			accKalman.Step(current.acc, current.kAcc);

			current.isAccUpdated = true;

		}
			break;

		default:
			break;
	}


	// the first time
	if(reset && current.isAccUpdated)
	{
		current.timestamp = timeDiff;
		current.isAccUpdated = false;

		prev = current;

		return;
	}


	if( current.isAccUpdated )
	{
		current.timestamp = timeDiff;
		float dt = (float)(current.timestamp - prev.timestamp)/1000;

//		DBG("ALLDATA\t%6d, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f\n", timeDiff
//				, current.acc.x, current.acc.y, current.acc.z, current.kAcc.x, current.kAcc.y, current.kAcc.z
//				, current.gyro.x, current.gyro.y, current.gyro.z,	current.kGyro.x, current.kGyro.y, current.kGyro.z
//				, linearAcc.x, linearAcc.y, linearAcc.z, kLinearAcc.x, kLinearAcc.y, kLinearAcc.z
//				, current.pos.x, current.pos.y, current.pos.z, current.kpos.x, current.kpos.y, current.kpos.z);

//		DBG("gyro \t%6d, %f, %f, %f\n", timeDiff, current.gyro.x, current.gyro.y, current.gyro.z);
//		DBG("kGyro \t%6d, %f, %f, %f\n", timeDiff, current.kGyro.x, current.kGyro.y, current.kGyro.z);
//		vec3 kAcc_diff = current.kAcc - prev.kAcc;
//		if(length(kAcc_diff) > 0.2)
//		{
//			kAcc_diff = normalize(kAcc_diff);
//			DBG("kAcc diff \t%6d, %f, %f, %f\n", timeDiff, kAcc_diff.x, kAcc_diff.y, kAcc_diff.z);
//		}
//		DBG("kAccel \t%6d, %f, %f, %f\n", timeDiff, current.kAcc.x, current.kAcc.y, current.kAcc.z);
// 		DBG("gyro length \t%6d, %f\n", timeDiff, length(current.gyro));
//		DBG("kaccel length between %f\n", length(current.kAcc - prev.kAcc));

		//1 pca
/*		#define PCA_DATA_NUM 10

		if(gLinearAcc.size() >= PCA_DATA_NUM)
		{
			Eigen::MatrixXd eigen = Eigen::MatrixXd::Zero(PCA_DATA_NUM, 3);
			Eigen::MatrixXd data = Eigen::MatrixXd::Zero(PCA_DATA_NUM, 3);
			Eigen::MatrixXd mean = Eigen::MatrixXd::Zero(1, 3);

			for(int i = 0; i < PCA_DATA_NUM; i++)
			{
				data(i, 0) = gLinearAcc[i].x;
				data(i, 1) = gLinearAcc[i].y;
				data(i, 2) = gLinearAcc[i].z;
			}

			GPCMEmbedPPCA(eigen, data, mean);

			// use only the best principle eigen
			vec3 pca;
			pca.x = eigen(0, 0);
			pca.y = eigen(0, 1);
			pca.z = eigen(0, 2);
			current.pcaAcc.push_back(pca);

			DBG("mean (%.2f, %.2f, %.2f), eigen (%.2f, %.2f, %.2f)\n",
				mean(0, 0), mean(0, 1), mean(0, 2),
				eigen(0, 0), eigen(0,1), eigen(0,2));

			gLinearAcc.clear();
		}*/

		if(sensor_callback_func)
		{
			sensor_callback_func(sensor_callback_func_data);
		}

		current.isAccUpdated = false;
		prev = current;
	}
}

sensor_info*
sensor_init(sensor_type_e sensor_type){
	int error;
	bool supported;

	error = sensor_is_supported(sensor_type, &supported);

	if(error)
	{
		ERR("sensor type %d is not supported\n", sensor_type);
		return NULL;
	}

	sensor_info* sensor = (sensor_info*)malloc(sizeof(sensor_info));
	memset(sensor, 0x00, sizeof(sensor_info));

	// sensor handle
	sensor->type = sensor_type;
	error = sensor_get_default_sensor(sensor->type, &sensor->handle);

	if(error)
	{
		ERR("sensor_get_default_sensor failed\n");
		free(sensor);
		return NULL;
	}

	sensor_get_min_range(sensor->handle, &sensor->value_min);
	sensor_get_max_range(sensor->handle, &sensor->value_max);
	sensor->value_range = sensor->value_max - sensor->value_min;

	// listener
	error = sensor_create_listener(sensor->handle, &sensor->listener);
	if(error)
	{
		ERR("accel_listener creation failed\n");
		free(sensor);
		return NULL;
	}

	// listener callback
	error = sensor_listener_set_event_cb(sensor->listener, UPDATE_RATE, on_sensor_event, NULL);
	if(error)
	{
		ERR("callback registeration failed\n");
		free(sensor);
		return NULL;
	}

	error = sensor_listener_set_option(sensor->listener, SENSOR_OPTION_ON_IN_SCREEN_OFF);
	if(error)
	{
		ERR("sensor option failed\n");
	}

	return sensor;
}

void sensor_start(sensor_info* sensor)
{
	int error;
	error = sensor_listener_start(sensor->listener);

	if(error)
	{
		ERR("sensor start failed\n");
		return;
	}
}

void sensor_listen_pause(sensor_info* sensor)
{
	int error;
	error = sensor_listener_unset_event_cb(sensor->listener);

	if(error)
	{
		ERR("callback unregistration failed\n");
	}
}

void sensor_listen_resume(sensor_info* sensor)
{
	int error;
	error = sensor_listener_set_event_cb(sensor->listener, UPDATE_RATE, on_sensor_event, sensor);

	if(error)
	{
		ERR("callback registration failed\n");
	}
}

void sensor_stop(sensor_info* sensor)
{
	int error;
	error = sensor_listener_unset_event_cb(sensor->listener);

	if(error)
	{
		ERR("callback unregistration failed\n");
	}

	error = sensor_listener_stop(sensor->listener);

	if(error)
	{
		ERR("sensor stop failed\n");
		return;
	}
}

void sensor_release(sensor_info* sensor)
{
	int error;
	error = sensor_destroy_listener(sensor->listener);

	if(error)
	{
		ERR("sensor destory failed\n");
		return;
	}

	free(sensor);
}

void reset_measure()
{
	initTime = 0;
}

sensor_data_info & get_current_sensor_data()
{
	return current;
}

sensor_data_info & get_prev_sensor_data()
{
	return prev;
}


void sensor_callback_register(Sensor_Cb func, void* data)
{
	sensor_callback_func = func;
	sensor_callback_func_data = data;
}

/*
vec3 get_pca_eigen()
{

	Eigen::MatrixXd eigen = Eigen::MatrixXd::Zero(current.linearAcc.size(), 3);
	Eigen::MatrixXd data = Eigen::MatrixXd::Zero(current.linearAcc.size(), 3);
	Eigen::MatrixXd mean = Eigen::MatrixXd::Zero(1, 3);

	for(int i = 0; i < current.linearAcc.size(); i++)
	{
		data(i, 0) = current.linearAcc[i].x;
		data(i, 1) = current.linearAcc[i].y;
		data(i, 2) = current.linearAcc[i].z;
	}

	GPCMEmbedPPCA(eigen, data, mean);

	// use only the best principle eigen
	vec3 pca;
	pca.x = eigen(0, 0);
	pca.y = eigen(0, 1);
	pca.z = eigen(0, 2);

	DBG("mean (%.2f, %.2f, %.2f), eigen (%.2f, %.2f, %.2f)\n",
		mean(0, 0), mean(0, 1), mean(0, 2),
		eigen(0, 0), eigen(0,1), eigen(0,2));

	return pca;

}
*/

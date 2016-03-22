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
	si.isAccUpdated = si.isGyroUpdated = 0;
	si.acc = vec3(0);
	si.gyro = vec3(0);

	si.kAcc = vec3(0);
	si.kGyro = vec3(0);
//	si.kpos = vec3(0);

//	si.qDeviceOrientation = quat(1.0, 0.0, 0.0, 0.0);
//	si.qKDeviceOrientation = quat(1.0, 0.0, 0.0, 0.0);

//	si.linearAcc.clear();
//	si.kLinearAcc.clear();

}

#define ROUND_COEFFICIENT 100

static float adjust_error_value(float v)
{
	if(fabs(v) < 0.1)
		return 0.f;

	int tmp = round(v * ROUND_COEFFICIENT);

	return (float)tmp / ROUND_COEFFICIENT;
}

static vec3 adjust_error_vector(vec3 v)
{
	return vec3(adjust_error_value(v.x),
				adjust_error_value(v.y),
				adjust_error_value(v.z));
}

static unsigned long long initTime = 0;

const vec3 INIT_GRAVITY_VECTOR(-0.33, 0.33, 9.87);
//const vec3 INIT_GRAVITY_VECTOR(0.0, 0.0, 9.9);

std::vector<vec3> gLinearAcc; // for PCA

double length_2(const vec3& in) {
	double len = length(in);
	return len * len;
}

/**
 * get quatanion between 2 vectors
 */
static glm::quat get_rotation_between(glm::vec3 u, glm::vec3 v)
{

	double k_cos_theta = dot(u, v);
	double k = sqrt(length_2(u) * length_2(v));

	if( k_cos_theta / k == -1)
	{
		// 180 degree rotation around any orthogonal vector;
		return glm::quat(0, glm::vec3(0));
	}

	return normalize(glm::quat(k_cos_theta + k, glm::cross(u, v)));

}

static glm::quat get_device_orientation(float dt, glm::vec3 gyro, glm::quat prevOrientation)
{
	// quaternion for gyroscope's angle
	glm::quat angleVelocity = glm::quat(0, gyro);
	angleVelocity = prevOrientation * angleVelocity;
	angleVelocity *= 0.5f;
	angleVelocity *= dt;

	glm::quat newOrientation = prevOrientation + angleVelocity;
	newOrientation = glm::normalize(newOrientation);

	return newOrientation;
}

static glm::vec3 get_linear_acceleration(glm::quat deviceOrientation, glm::vec3 acc)
{
	//1 Linear acceleration
	//3 : Subtract gravity vector from adjusted accelerometer vector (rotated by inverse of device orientation)
	// subtract gravity from adjusted accelerometer
	// current accel to world space

	glm::vec3 adjustedAcc = glm::mat3_cast(deviceOrientation) * acc;
//	vec3 linearAcc = adjust_error_vector(adjustedAcc - INIT_GRAVITY_VECTOR);
	glm::vec3 linearAcc = adjustedAcc - INIT_GRAVITY_VECTOR;

	return linearAcc;
}

static void
on_sensor_event(sensor_h sensor, sensor_event_s *event, void *user_data)
{
	static unsigned long long accCount = 0;
	static unsigned long long gyroCount = 0;

	static KalmanGearS2 accKalman(KalmanGearS2::ACCELEROMETER, 0.00001);
	static KalmanGearS2 gyroKalman(KalmanGearS2::GYROSCOPE, 0.00001);

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
		gyroKalman.Reset();

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
//			sangbin_kalman(current.acc, current.kAcc, reset);
			accKalman.Step(current.acc, current.kAcc);

			current.isAccUpdated = true;

/*
			double diff = abs(length(current.acc) - length(prev.acc)) / length(prev.acc);
			diff *= 100.0;

			if(diff < 2.0)
				accCount++;
			else
				accCount = 0;
*/
		}
			break;

		case SENSOR_GYROSCOPE :
		{
			// real? degree??
			double x_angle = event->values[0]; //* (pi<double>() / 180.0); // degree to radian
			double y_angle = event->values[1]; //* (pi<double>() / 180.0);
			double z_angle = event->values[2]; //* (pi<double>() / 180.0);

//			current.gyro = adjust_error_vector( vec3(x_angle, y_angle, z_angle));
			current.gyro = vec3(x_angle, y_angle, z_angle);
			gyroKalman.Step(current.gyro, current.kGyro);

			current.isGyroUpdated = true;

/*
			double curr_length = (length(current.gyro) == 0) ? 0.0001 : length(current.gyro);
			double prev_length = (length(prev.gyro) == 0) ? 0.0001 : length(prev.gyro);
			double diff = abs(curr_length - prev_length) / prev_length;
			diff *= 100.0;

			if(diff < 1.0)
				gyroCount++;
			else
				gyroCount = 0;
*/
		}
			break;

		default:
			break;
	}


	// the first time
	if(reset && current.isAccUpdated)
	{
		current.timestamp = timeDiff;
//		current.qDeviceOrientation = get_rotation_between(current.acc, INIT_GRAVITY_VECTOR);
//		current.qKDeviceOrientation = get_rotation_between(current.kAcc, INIT_GRAVITY_VECTOR);
//		current.pos = vec3(0, 0, 0);
//		current.kpos = vec3(0, 0, 0);
//		current.vel = vec3(0, 0, 0);
		current.isAccUpdated = false;
		current.isGyroUpdated = false;

		prev = current;

		return;
	}


	/*// Update rotation
	if (accCount > 20 && gyroCount > 20)
	{

		current.timestamp = timeDiff;
		current.qDeviceOrientation = get_rotation_between(current.acc, INIT_GRAVITY_VECTOR);

		accCount = 0;
		gyroCount = 0;

		current.isAccUpdated = false;
		current.isGyroUpdated = false;

		prev = current;

		return;
	}*/


	if( current.isAccUpdated && current.isGyroUpdated )
	{
		current.timestamp = timeDiff;
		float dt = (float)(current.timestamp - prev.timestamp)/1000;

/*
		//1 device orientation
		current.qDeviceOrientation = get_device_orientation(dt, current.gyro, prev.qDeviceOrientation);
		current.qKDeviceOrientation = get_device_orientation(dt, current.kGyro, prev.qKDeviceOrientation);

		//1 linear acceleration
		vec3 linearAcc = get_linear_acceleration(current.qDeviceOrientation, current.acc);
		vec3 kLinearAcc = get_linear_acceleration(current.qKDeviceOrientation, current.kAcc);
*/

//		gLinearAcc.push_back(linearAcc);
//		current.linearAcc.push_back(linearAcc);
//		current.kLinearAcc.push_back(kLinearAcc);

		//1 Position
		//3 : integrate linear acceleration
#if 0
		// using velocity

		float scalar = linearAcc.length();

		if(scalar > 1.0f)
			current.vel = prev.vel + linearAcc * dt;
		else
			current.vel = vec3(0, 0, 0);

//		DBG("VELOCITY\t( %6d )\t%.2f\t%.2f\t%.5f\t%.2f\n", timeDiff, scalar, current.vel.x(), current.vel.y(), current.vel.z());

		current.pos = prev.pos + 0.5f * current.vel * dt;
#else
/*
		// directly double integration
		current.pos = prev.pos + 0.5f * linearAcc * dt * dt;
		current.kpos = prev.kpos + 0.5f * kLinearAcc * dt * dt;
*/
#endif

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



#if 0
		//1 using kalman
		vec3 out_pos;
		vec3 out_vel;

		kalman(current.pos, linearAcc, out_pos, out_vel, false /* reset at the first time*/);
		DBG("POSITION_k \t( %6d )\t%.4f\t%.4f\t%.4f\t%.4f\t%.4f\t%.4f\n", timeDiff, current.pos.x, current.pos.y, current.pos.z, out_pos.x, out_pos.y, out_pos.z);

//		current.pos = out_pos;
//		current.vel = out_vel;
#endif
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
		current.isGyroUpdated = false;
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
	sensor_callback_func = func,
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

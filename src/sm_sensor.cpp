#include <vector>

#include <math.h>

#include "sm_sensor.h"
#include "logger.h"

#include <limits>

#include <sm_sensor.h>

#include "pca/embedppca.h"
#include "kalman_manager.h"
#include "sequence.h"

#define UPDATE_RATE 10 // millisecond

using namespace glm;

SensorIntegration prev;
SensorIntegration current;


static void ResetSensorIntegration(SensorIntegration& si)
{
	si.timestamp = 0;
	si.acc_updated = si.gyro_updated = 0;
	si.acc = vec3(0);
	si.gyro = vec3(0);
	si.pos = vec3(0);
	si.vel = vec3(0);

	si.qDeviceOrientation = quat(1.0, 0.0, 0.0, 0.0);

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

static unsigned long long init_time = 0;

const vec3 INIT_GRAVITY_VECTOR(-0.33, 0.33, 9.87);
//const vec3 INIT_GRAVITY_VECTOR(0.0, 0.0, 9.9);

std::vector<vec3> gLinearAcc;
std::vector<vec3> gPC;

double length_2(const vec3& in) {
	double len = length(in);
	return len * len;
}

quat get_rotation_between(vec3 u, vec3 v)
{

	double k_cos_theta = dot(u, v);
	double k = sqrt(length_2(u) * length_2(v));

	if( k_cos_theta / k == -1)
	{
		// 180 degree rotation around any orthogonal vector;
		return quat(0, vec3(0));
	}

	return normalize(quat(k_cos_theta + k, cross(u, v)));

}


static void
on_sensor_event(sensor_h sensor, sensor_event_s *event, void *user_data)
{
	sensor_info * si = static_cast<sensor_info *>(user_data);

	static unsigned long long acc_cnt = 0;
	static unsigned long long gyro_cnt = 0;

	// Select a specific sensor with a sensor handle
	// This example uses sensor type, assuming there is only 1 sensor for each type
	sensor_type_e type;
	sensor_get_type(sensor, &type);

	bool reset = false;

	// initialize all
	if(init_time == 0)
	{
		init_time = event->timestamp/1000;

		ResetSensorIntegration(prev);
		ResetSensorIntegration(current);

		// skip the 1st data since it has accelerometer data only. We need correct pair of (accel & gyro)
		//	   return;
		reset = true;
	}

   unsigned int time_diff = (unsigned int)(event->timestamp/1000 - init_time);

   switch (type)
   {
		case SENSOR_ACCELEROMETER:
		{
			// Use sensor information
			current.acc = vec3(event->values[0], event->values[1], event->values[2]);
			current.acc_updated = true;

			double diff = abs(length(current.acc) - length(prev.acc)) / length(prev.acc);
			diff *= 100.0;

			if(diff < 2.0)
				acc_cnt++;
			else
				acc_cnt = 0;


//			DBG("ACCELEROM\t( %6d )\t%.2f\t%.2f\t%.2f\n", time_diff, current.acc.x, current.acc.y, current.acc.z);
//			DBG("ACCELEROM\t length %f\n",length(current.acc));
//			DBG("ACCELEROM\t diff %f\n", diff);
		}
		break;

		case SENSOR_GYROSCOPE :
		{
			// real? degree??
			double x_angle = event->values[0] * (pi<double>() / 180.0); // degree to radian
			double y_angle = event->values[1] * (pi<double>() / 180.0);
			double z_angle = event->values[2] * (pi<double>() / 180.0);

			current.gyro = adjust_error_vector( vec3(x_angle, y_angle, z_angle));
			current.gyro_updated = true;

			double curr_length = (length(current.gyro) == 0) ? 0.0001 : length(current.gyro);
			double prev_length = (length(prev.gyro) == 0) ? 0.0001 : length(prev.gyro);
			double diff = abs(curr_length - prev_length) / prev_length;
			diff *= 100.0;

			if(diff < 1.0)
				gyro_cnt++;
			else
				gyro_cnt = 0;

//			DBG("GYROSCOPE\t( %6d )\t%f\t%f\t%f\n", time_diff, current.gyro.x, current.gyro.y, current.gyro.z);
//			DBG("GYROSCOPE\t diff %f\n", diff);
		}
		break;

		default:
		break;
   }


	// the first time
	if(reset && current.acc_updated)
	{
//		ResetSensorIntegration(current);

		current.timestamp = time_diff;
		current.qDeviceOrientation = get_rotation_between(current.acc, INIT_GRAVITY_VECTOR);
		current.pos = vec3(0, 0, 0);
		current.vel = vec3(0, 0, 0);
		current.acc_updated = false;
		current.gyro_updated = false;

		prev = current;

//		DBG("initial ORIENTATA\t( %6d )\t%.2f\t%.5f\t%.2f\t%.2f\n", time_diff, current.qDeviceOrientation.w(), current.qDeviceOrientation.x(), current.qDeviceOrientation.y(), current.qDeviceOrientation.z());


		return;
	}


	// Update rotation
	if (acc_cnt > 20 && gyro_cnt > 20)
	{

		current.timestamp = time_diff;
		current.qDeviceOrientation = get_rotation_between(current.acc, INIT_GRAVITY_VECTOR);

		acc_cnt = 0;
		gyro_cnt = 0;

//		DBG("updated\t( %6d )\t%.2f\t%.2f\t%.2f\n", time_diff, current.acc.x, current.acc.y, current.acc.z);
//		DBG("updated\t( %6d )\t%.5f\t%.5f\t%.5f\t%.5f\n", time_diff,
//			current.qDeviceOrientation.w, current.qDeviceOrientation.x, current.qDeviceOrientation.y, current.qDeviceOrientation.z);

		current.acc_updated = false;
		current.gyro_updated = false;

		prev = current;

		return;
	}


	if( current.acc_updated && current.gyro_updated )
	{
		current.timestamp = time_diff;
		float dt = (float)(current.timestamp - prev.timestamp)/1000;

		// quaternion for gyroscope's angle
		quat angleVelocity = quat(0, current.gyro);
		angleVelocity = prev.qDeviceOrientation * angleVelocity;
		angleVelocity *= 0.5f;
		angleVelocity *= dt;

		current.qDeviceOrientation = prev.qDeviceOrientation + angleVelocity;
		current.qDeviceOrientation = normalize(current.qDeviceOrientation);

//		DBG("prevORIENTATA\t( %6d )\t%.5f\t%.5f\t%.5f\t%.5f\n", time_diff,
//			prev.qDeviceOrientation.w, prev.qDeviceOrientation.x, prev.qDeviceOrientation.y, prev.qDeviceOrientation.z);
//		DBG("currORIENTATA\t( %6d )\t%.5f\t%.5f\t%.5f\t%.5f\n", time_diff,
//			current.qDeviceOrientation.w, current.qDeviceOrientation.x, current.qDeviceOrientation.y, current.qDeviceOrientation.z);

		//1 Linear acceleration
		//3 : Subtract gravity vector from adjusted accelerometer vector (rotated by inverse of device orientation)
		// subtract gravity from adjusted accelerometer
		// current accel to world space
		vec3 adjustedAcc = mat3_cast(current.qDeviceOrientation) * current.acc;

//		DBG("ADJUSTEDACC\t( %6d )\t%.2f\t%.2f\t%.2f\n",	time_diff, adjustedAcc.x, adjustedAcc.y, adjustedAcc.z);

		vec3 linearAcc = adjust_error_vector(adjustedAcc - INIT_GRAVITY_VECTOR);

		gLinearAcc.push_back(linearAcc);
		current.linearAcc.push_back(linearAcc);

		DBG("LINEARACC\t( %6d )\t%.2f\t%.5f\t%.2f\n", time_diff, linearAcc.x, linearAcc.y, linearAcc.z);


		//1 Position
		//3 : integrate linear acceleration
#if 0
		// using velocity

		float scalar = linearAcc.length();

		if(scalar > 1.0f)
			current.vel = prev.vel + linearAcc * dt;
		else
			current.vel = vec3(0, 0, 0);

//		DBG("VELOCITY\t( %6d )\t%.2f\t%.2f\t%.5f\t%.2f\n", time_diff, scalar, current.vel.x(), current.vel.y(), current.vel.z());

		current.pos = prev.pos + 0.5f * current.vel * dt;
#else
		// directly double integration
		current.pos = prev.pos + 0.5f * linearAcc * dt * dt;
#endif
//		DBG("POSITION_o \t( %6d )\t%.4f\t%.4f\t%.4f\n", time_diff, current.pos.x, current.pos.y, current.pos.z);

#if 1
		//1 using kalman
		vec3 out_pos;
		vec3 out_vel;

		kalman(current.pos, linearAcc, out_pos, out_vel, false /* reset at the first time*/);
		DBG("POSITION_k \t( %6d )\t%.4f\t%.4f\t%.4f\t%.4f\t%.4f\t%.4f\n", time_diff, current.pos.x, current.pos.y, current.pos.z, out_pos.x, out_pos.y, out_pos.z);

//		current.pos = out_pos;
//		current.vel = out_vel;
#endif
		//1 pca
		#define PCA_DATA_NUM 5

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
			gPC.push_back(pca);

			DBG("mean (%.2f, %.2f, %.2f), eigen (%.2f, %.2f, %.2f)\n",
				mean(0, 0), mean(0, 1), mean(0, 2),
				eigen(0, 0), eigen(0,1), eigen(0,2));


			gLinearAcc.clear();
		}

		current.acc_updated = false;
		current.gyro_updated = false;
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

void sensor_deinit(sensor_info* sensor)
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
	init_time = 0;
}


const SensorIntegration & get_current_sensor_data()
{
	return current;
}

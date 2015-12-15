#include <vector>

#include <math.h>

#include "sm_sensor.h"
#include "logger.h"

#include <limits>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "pca/embedppca.h"

#define UPDATE_RATE 20 // millisecond


using namespace glm;

const vec3 GRAVITY_VECTOR(0, 0, 9.8f);

typedef struct
{
	unsigned int timestamp; // id

	vec3 acc;
	bool acc_updated;
	vec3 gyro;
	bool gyro_updated;

	quat qAccelOrientation; // Quaternion between accelerometer vector and GRAVITY_VECTOR vector
	quat qDeviceOrientation; // Quaternion of device orientation

	vec3 vel;
	vec3 pos;

	quat orientation;

}SensorIntegration;

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

	si.qAccelOrientation = quat(1.0, 0.0, 0.0, 0.0);
	si.qDeviceOrientation = quat(1.0, 0.0, 0.0, 0.0);

	si.orientation = quat(1.0, 0.0, 0.0, 0.0);
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

std::vector<vec3> gLinearAcc;
std::vector<vec3> gPC;

static void
on_sensor_event(sensor_h sensor, sensor_event_s *event, void *user_data)
{
	// Select a specific sensor with a sensor handle
	// This example uses sensor type, assuming there is only 1 sensor for each type
	sensor_type_e type;
	sensor_get_type(sensor, &type);


	// initialize all
	if(init_time == 0)
	{
		init_time = event->timestamp/1000;

		ResetSensorIntegration(prev);
		ResetSensorIntegration(current);

		// skip the 1st data since it has accelerometer data only. We need correct pair of (accel & gyro)
		//	   return;
	}

   unsigned int time_diff = (unsigned int)(event->timestamp/1000 - init_time);

   switch (type)
   {
		case SENSOR_ACCELEROMETER:
		{
			// Use sensor information
			current.acc = adjust_error_vector(vec3(event->values[0],
												   event->values[1],
												   event->values[2]));
			current.acc_updated = true;

//			current.qAccelOrientation =
//					current.qAccelOrientation.FromTwoVectors(current.acc, GRAVITY_VECTOR);

//			DBG("ACCELEROM\t( %6d )\t%.2f\t%.2f\t%.2f\n", time_diff, current.acc.x, current.acc.y, current.acc.z);
		}
		break;

		case SENSOR_GYROSCOPE :
		{
			// real? degree??
			double x_angle = event->values[0] * (pi<double>() / 180.0); // degree to radian
			double y_angle = event->values[1] * (pi<double>() / 180.0);
			double z_angle = event->values[2] * (pi<double>() / 180.0);

			current.gyro = adjust_error_vector(vec3(x_angle, y_angle, z_angle));
			current.gyro_updated = true;

//			DBG("GYROSCOPE\t( %6d )\t%.2f\t%.2f\t%.2f\n", time_diff, current.gyro.x, current.gyro.y, current.gyro.z);
		}
		break;

		default:
		break;
   }

	// the first time
	if(time_diff < 2 && current.acc_updated)
	{
		current.timestamp = time_diff;
		vec3 axis = cross(GRAVITY_VECTOR, current.acc);
		vec3 gravity = GRAVITY_VECTOR;
		vec3 curacc = current.acc;
		float half_angle = dot(normalize(gravity), normalize(curacc));
		half_angle = acos(half_angle) / 2.0f;

		current.qDeviceOrientation =
				quat(cos(half_angle), vec3(sin(half_angle)) * axis);

//		current.qDeviceOrientation =
//				current.qDeviceOrientation.FromTwoVectors(GRAVITY_VECTOR, current.acc);
				// the first device orientation

		normalize(current.qDeviceOrientation);
		normalize(current.qAccelOrientation);

		current.pos = vec3(0, 0, 0);

		prev = current;

//		DBG("initial ORIENTATA\t( %6d )\t%.2f\t%.5f\t%.2f\t%.2f\n", time_diff, current.qDeviceOrientation.w(), current.qDeviceOrientation.x(), current.qDeviceOrientation.y(), current.qDeviceOrientation.z());

		ResetSensorIntegration(current);

		return;
	}

	if( current.acc_updated && current.gyro_updated )
	{
		current.timestamp = time_diff;
		float dt = (float)(current.timestamp - prev.timestamp)/1000;

//		vec3 deltaGyro = current.gyro * dt;

		// quaternion for gyroscope's angle
		quat angleVelocity = quat(0, current.gyro.x, current.gyro.y, current.gyro.z);
		angleVelocity = prev.orientation * angleVelocity;
		angleVelocity *= 0.5;
		angleVelocity *= dt;

		current.orientation = prev.orientation + angleVelocity;

		current.orientation = normalize(current.orientation);

//		DBG("FUCK QUAT! (%f, %f, %f, %f)\n", current.orientation.x, current.orientation.y, current.orientation.z, current.orientation.w );

		float lenggg =sqrt(current.orientation.x * current.orientation.x +
								   current.orientation.y * current.orientation.y +
								   current.orientation.z * current.orientation.z +
								   current.orientation.w * current.orientation.w
								   );
//		DBG("LENGTH! %f\n",lenggg);

//		DBG("aORIENTATA\t( %6d )\t%.5f\t%.5f\t%.5f\t%.5f\n", time_diff, prev.qDeviceOrientation.w(), prev.qDeviceOrientation.x(), prev.qDeviceOrientation.y(), prev.qDeviceOrientation.z());
//		DBG("cORIENTATA\t( %6d )\t%.5f\t%.5f\t%.5f\t%.5f\n", time_diff, current.qDeviceOrientation.w(), current.qDeviceOrientation.x(), current.qDeviceOrientation.y(), current.qDeviceOrientation.z());

		//1 Linear acceleration
		//3 : Subtract gravity vector from adjusted accelerometer vector (rotated by inverse of device orientation)
#if 1
		// subtract gravity from adjusted accelerometer
//		vec3 adjustedAcc = current.qDeviceOrientation.inverse() * current.acc;
		vec3 adjustedAcc = mat3_cast(current.orientation) * current.acc;
//		DBG("ADJUSTEDACC\t( %6d )\t%.2f\t%.2f\t%.2f\n",	time_diff, adjustedAcc.x, adjustedAcc.y, adjustedAcc.z);

		vec3 linearAcc = adjust_error_vector(adjustedAcc - GRAVITY_VECTOR);
#else
		// subtract accelerometer from adjusted gravity
		vec3 adjustGravity = current.qDeviceOrientation * GRAVITY_VECTOR;
		vec3 tmpAcc = current.acc - adjustGravity;
		vec3 linearAcc = current.qDeviceOrientation.inverse() * tmpAcc;
#endif

		vec3 acc = linearAcc;

		gLinearAcc.push_back(acc);

		DBG("LINEARACC\t( %6d )\t%.2f\t%.5f\t%.2f\n",
			time_diff, linearAcc.x, linearAcc.y, linearAcc.z);

		//1 Position
		//3 : integrate linear acceleration
#if 1
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
		current.pos = prev.pos + 0.5 * linearAcc * dt * dt;
#endif
//		DBG("POSITION \t( %6d )\t%.2f\t%.2f\t%.2f\n", time_diff, current.pos.x(), current.pos.y(), current.pos.z());


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


		prev = current;
		ResetSensorIntegration(current);
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
	error = sensor_listener_set_event_cb(sensor->listener, UPDATE_RATE, on_sensor_event, NULL);

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
}

void reset_measure()
{
	init_time = 0;
}

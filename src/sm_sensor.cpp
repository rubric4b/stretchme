#include <vector>

#include <math.h>

#include "sm_sensor.h"
#include "logger.h"

#include <limits>

#define EIGEN_DONT_VECTORIZE
#define EIGEN_DISABLE_UNALIGNED_ARRAY_ASSERT
#include "Eigen\Geometry"

#include "pca\embedppca.h"

#define UPDATE_RATE 20 // millisecond

using namespace Eigen;

Eigen::Vector3d GRAVITY_VECTOR(0, 0, 9.8f);

typedef struct
{
	unsigned int timestamp; // id

	Eigen::Vector3d acc;
	bool acc_updated;
	Eigen::Vector3d gyro;
	bool gyro_updated;

	Eigen::Quaterniond qAccelOrientation; // Quaternion between accelerometer vector and GRAVITY_VECTOR vector
	Eigen::Quaterniond qDeviceOrientation; // Quaternion of device orientation

	Eigen::Vector3d vel;
	Eigen::Vector3d pos;

}SensorIntegration;

SensorIntegration prev;
SensorIntegration current;

static Eigen::Quaterniond ScalarMult(Eigen::Quaterniond& qt, double Scalar)
{
	Eigen::Quaterniond output;
	output.x() = qt.x() *Scalar;
	output.y() = qt.y() *Scalar;
	output.z() = qt.z() *Scalar;
	output.w() = qt.w() *Scalar;
	return output;
}

/*
static Eigen::Quaterniond qtSum(Eigen::Quaterniond& a, Eigen::Quaterniond& b)
{
	Eigen::Quaterniond output;
	output.x() = a.x() + b.x();
	output.y() = a.y() + b.y();
	output.z() = a.z() + b.z();
	output.w() = a.w() + b.w();
	return output;
}
*/

static void ResetSensorIntegration(SensorIntegration& si)
{
	si.timestamp = 0;
	si.acc_updated = si.gyro_updated = 0;
	si.acc = Eigen::Vector3d(0, 0, 0);
	si.gyro = Eigen::Vector3d(0, 0, 0);
	si.pos = Eigen::Vector3d(0, 0, 0);
	si.vel = Eigen::Vector3d(0, 0, 0);

	si.qAccelOrientation = si.qAccelOrientation.Identity();
	si.qDeviceOrientation = si.qDeviceOrientation.Identity();
}

#define ROUND_COEFFICIENT 100

static float adjust_error_value(float v)
{
	if(fabs(v) < 0.1)
		return 0.f;

	int tmp = round(v * ROUND_COEFFICIENT);

	return (float)tmp / ROUND_COEFFICIENT;
}

static Eigen::Vector3d adjust_error_vector(Eigen::Vector3d v)
{
	return Eigen::Vector3d(adjust_error_value(v.x()), adjust_error_value(v.y()), adjust_error_value(v.z()));
}

static unsigned long long init_time = 0;

struct vec3
{
	float x;
	float y;
	float z;
};

std::vector<vec3> gLinearAcc;
std::vector<vec3> gPC;

static void
on_sensor_event(sensor_h sensor, sensor_event_s *event, void *user_data)
{
	// Select a specific sensor with a sensor handle
	// This example uses sensor type, assuming there is only 1 sensor for each type
	sensor_type_e type;
	sensor_get_type(sensor, &type);

#if 0 // USE_CF

	static Eigen::Vector3d gyroAngle = Eigen::Vector3d(0, 0, 0);
	static Eigen::Vector3d accAngle = Eigen::Vector3d(0, 0, 0);
	static Eigen::Vector3d CFAngle = Eigen::Vector3d(0, 0, 0);
#endif

	// initialize all
	if(init_time == 0)
	{
		init_time = event->timestamp/1000;

		ResetSensorIntegration(prev);
		ResetSensorIntegration(current);

#if 0 // USE_CF
		gyroAngle = Eigen::Vector3d(0, 0, 0);
		accAngle = Eigen::Vector3d(0, 0, 0);
		CFAngle = Eigen::Vector3d(0, 0, 0);
#endif
		// skip the 1st data since it has accelerometer data only. We need correct pair of (accel & gyro)
		//	   return;
	}

   unsigned int time_diff = (unsigned int)(event->timestamp/1000 - init_time);

   switch (type)
   {
		case SENSOR_ACCELEROMETER:
		{
			// Use sensor information
			current.acc = adjust_error_vector(Eigen::Vector3d(event->values[0], event->values[1], event->values[2]));
			current.acc_updated = true;
			current.qAccelOrientation = current.qAccelOrientation.FromTwoVectors(current.acc, GRAVITY_VECTOR);

			DBG("ACCELEROM\t( %6d )\t%.2f\t%.2f\t%.2f\n", time_diff, current.acc.x(), current.acc.y(), current.acc.z());
		}
		break;

		case SENSOR_GYROSCOPE :
		{
			double x_angle = event->values[0] * (M_PI / 180.0); // degree to radian
			double y_angle = event->values[1] * (M_PI / 180.0);
			double z_angle = event->values[2] * (M_PI / 180.0);

			current.gyro = adjust_error_vector(Eigen::Vector3d(x_angle, y_angle, z_angle));
			current.gyro_updated = true;

			DBG("GYROSCOPE\t( %6d )\t%.2f\t%.2f\t%.2f\n", time_diff, current.gyro.x(), current.gyro.y(), current.gyro.z());
		}
		break;

		default:
		break;
   }

	// the first time
	if(time_diff < 2 && current.acc_updated == true)
	{
		current.timestamp = time_diff;
		current.qDeviceOrientation = current.qDeviceOrientation.FromTwoVectors(GRAVITY_VECTOR, current.acc); // the first device orientation
		current.qDeviceOrientation.normalize();
		current.pos = Eigen::Vector3d(0, 0, 0);
		prev = current;

		DBG("initial ORIENTATA\t( %6d )\t%.2f\t%.5f\t%.2f\t%.2f\n", time_diff, current.qDeviceOrientation.w(), current.qDeviceOrientation.x(), current.qDeviceOrientation.y(), current.qDeviceOrientation.z());

		ResetSensorIntegration(current);

		return;
	}

	if( current.acc_updated && current.gyro_updated )
	{
		current.timestamp = time_diff;
		float dt = (float)(current.timestamp - prev.timestamp)/1000;

		Eigen::Vector3d deltaGyro = current.gyro * dt;

		// quaternion for gyroscope's angle
		Eigen::Quaterniond deltaOrientation = Eigen::Quaterniond(1, deltaGyro.x(), deltaGyro.y(), deltaGyro.z());
		deltaOrientation.normalize();

		//1 Device orientation
		//3 : integrate delta orientation by gyroscope sensor
#if 0
		Eigen::Quaterniond qDot = ScalarMult(prev.qDeviceOrientation, 0.5) * deltaOrientation;
//		current.qDeviceOrientation = qtSum(prev.qDeviceOrientation, ScalarMult(qDot, (float)dt));
		current.qDeviceOrientation = prev.qDeviceOrientation * ScalarMult(qDot, (float)dt);
#else
		current.qDeviceOrientation = prev.qDeviceOrientation * deltaOrientation;
#endif
		current.qDeviceOrientation.normalize();

#if 0 // USE_CF
		//1 Complementary filter test
		gyroAngle += deltaGyro * 180.f / M_PI; // radian to degree

		float accXangle = (float)(atan2(current.acc.y(), current.acc.z()) * 180 / M_PI);
		float accYangle = (float)(atan2(current.acc.z(), current.acc.x()) * 180 / M_PI);
		float accZangle = (float)(atan2(current.acc.x(), current.acc.y()) * 180 / M_PI);

		if(accXangle > 180)
			accXangle -= 360.f;
		if(accYangle > 180)
			accYangle -= 360.f;
		if(accZangle > 180)
			accZangle -= 360.f;

		CFAngle = Eigen::Vector3d(
			0.98 * (CFAngle.x() + deltaGyro.x()) + (1 - 0.98) * accXangle
			,0.98 * (CFAngle.y() + deltaGyro.y()) + (1 - 0.98) * accYangle
			,0.98 * (CFAngle.z() + deltaGyro.z()) + (1 - 0.98) * accZangle);

		DBG("orientation : gyro (%.2f, %.2f, %.2f), acc (%.2f, %.2f, %.2f), CF (%.2f, %.2f, %.2f)\n",
			gyroAngle.x(), gyroAngle.y(), gyroAngle.z(),
			accXangle, accYangle, accZangle,
			CFAngle.x(), CFAngle.y(), CFAngle.z());
#endif

		DBG("aORIENTATA\t( %6d )\t%.5f\t%.5f\t%.5f\t%.5f\n", time_diff, prev.qDeviceOrientation.w(), prev.qDeviceOrientation.x(), prev.qDeviceOrientation.y(), prev.qDeviceOrientation.z());
		DBG("cORIENTATA\t( %6d )\t%.5f\t%.5f\t%.5f\t%.5f\n", time_diff, current.qDeviceOrientation.w(), current.qDeviceOrientation.x(), current.qDeviceOrientation.y(), current.qDeviceOrientation.z());

		//1 Linear acceleration
		//3 : Subtract gravity vector from adjusted accelerometer vector (rotated by inverse of device orientation)
#if 1
		// subtract gravity from adjusted accelerometer
		Eigen::Vector3d adjustedAcc = current.qDeviceOrientation.inverse() * current.acc;
		Eigen::Vector3d linearAcc = adjust_error_vector(adjustedAcc - GRAVITY_VECTOR);
#else
		// subtract accelerometer from adjusted gravity
		Eigen::Vector3d adjustGravity = current.qDeviceOrientation * GRAVITY_VECTOR;
		Eigen::Vector3d tmpAcc = current.acc - adjustGravity;
		Eigen::Vector3d linearAcc = current.qDeviceOrientation.inverse() * tmpAcc;
#endif

		vec3 acc;
		acc.x = linearAcc.x();
		acc.y = linearAcc.y();
		acc.z = linearAcc.z();
		gLinearAcc.push_back(acc);

		DBG("LINEARACC\t( %6d )\t%.2f\t%.5f\t%.2f\n", time_diff, linearAcc.x(), linearAcc.y(), linearAcc.z());

		//1 Position
		//3 : integrate linear acceleration
#if 1
		// using velocity

		float scalar = sqrt(linearAcc.x() * linearAcc.x() + linearAcc.y() * linearAcc.y() + linearAcc.z() * linearAcc.z() );

		if(scalar > 1.0f)
			current.vel = prev.vel + linearAcc * dt;
		else
			current.vel = Eigen::Vector3d(0, 0, 0);

		DBG("VELOCITY\t( %6d )\t%.2f\t%.2f\t%.5f\t%.2f\n", time_diff, scalar, current.vel.x(), current.vel.y(), current.vel.z());

		current.pos = prev.pos + 0.5 * current.vel * dt;
#else
		// directly double integration
		current.pos = prev.pos + 0.5 * linearAcc * dt * dt;
#endif
		DBG("POSITION \t( %6d )\t%.2f\t%.2f\t%.2f\n", time_diff, current.pos.x(), current.pos.y(), current.pos.z());


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

			DBG("mean (%.2f, %.2f, %.2f), eigen (%.2f, %.2f, %.2f)\n", mean(0, 0), mean(0, 1), mean(0, 2), eigen(0, 0), eigen(0,1), eigen(0,2));

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


#if 1 //QUATERNION_TEST
	Eigen::Quaterniond test;
	test = test.FromTwoVectors(Eigen::Vector3d(2, 0, 0), Eigen::Vector3d(0, 0, 2));

	Eigen::Quaterniond testNorm = test.normalized();
	Eigen::Quaterniond testInv = testNorm.inverse();

	Eigen::Vector3d result = testInv * Eigen::Vector3d(2, 0, 0);

	DBG("Quaternion : %.3f, %.3f, %.3f, %.3f\n", test.x(), test.y(), test.z(), test.w());
	DBG("Quaternion normalized : %.3f, %.3f, %.3f, %.3f\n", testNorm.x(), testNorm.y(), testNorm.z(), testNorm.w());
	DBG("Quaternion inversed : %.3f, %.3f, %.3f, %.3f\n", testInv.x(), testInv.y(), testInv.z(), testInv.w());
	DBG("Result (2, 0, 0) : %.3f, %.3f, %.3f\n", result.x(), result.y(), result.z());

	result = result - Eigen::Vector3d(0, 0, 2);

	DBG("Result linear : %.3f, %.3f, %.3f\n", result.x(), result.y(), result.z());

test.setIdentity();
testNorm.setIdentity();
testInv.setIdentity();

test = test.FromTwoVectors(Eigen::Vector3d(0, 2, 0), Eigen::Vector3d(0, 0, 2));
testNorm = test.normalized();
testInv = testNorm.inverse();
result = testInv * Eigen::Vector3d(0, 2, 0);

	DBG("Quaternion : %.3f, %.3f, %.3f, %.3f\n", test.x(), test.y(), test.z(), test.w());
	DBG("Quaternion normalized : %.3f, %.3f, %.3f, %.3f\n", testNorm.x(), testNorm.y(), testNorm.z(), testNorm.w());
	DBG("Quaternion inversed : %.3f, %.3f, %.3f, %.3f\n", testInv.x(), testInv.y(), testInv.z(), testInv.w());
	DBG("Result (0, 2, 0) : %.3f, %.3f, %.3f\n", result.x(), result.y(), result.z());

result = result - Eigen::Vector3d(0, 0, 2);

DBG("Result linear : %.3f, %.3f, %.3f\n", result.x(), result.y(), result.z());


test.setIdentity();
testNorm.setIdentity();
testInv.setIdentity();

test = test.FromTwoVectors(Eigen::Vector3d(0, 0, 2), Eigen::Vector3d(0, 0, 2));
testNorm = test.normalized();
testInv = testNorm.inverse();
result = testInv * Eigen::Vector3d(0, 0, 2);

	DBG("Quaternion : %.3f, %.3f, %.3f, %.3f\n", test.x(), test.y(), test.z(), test.w());
	DBG("Quaternion normalized : %.3f, %.3f, %.3f, %.3f\n", testNorm.x(), testNorm.y(), testNorm.z(), testNorm.w());
	DBG("Quaternion inversed : %.3f, %.3f, %.3f, %.3f\n", testInv.x(), testInv.y(), testInv.z(), testInv.w());
	DBG("Result (0, 0, 2) : %.3f, %.3f, %.3f\n", result.x(), result.y(), result.z());

result = result - Eigen::Vector3d(0, 0, 2);

DBG("Result linear : %.3f, %.3f, %.3f\n", result.x(), result.y(), result.z());

#endif
}


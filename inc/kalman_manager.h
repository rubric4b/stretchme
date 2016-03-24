#ifndef __KALMAN_MANAGER_H__
#define __KALMAN_MANAGER_H__

#include <glm/glm.hpp>

using namespace glm;

// default prediction level
static const float DEFAULT_PREDICTION_LEVEL = 0.00001;

/**
 * SIMPLE kalman filter implementation for Samsung Gear S2 device
 *
 * It has initial parameters for Gear S2 device already
 * Application can use this for limited use case only (i.e., for Accelerometer and Gyroscope data)
 * Application can get different output according to control the prediction level
 */
class KalmanGearS2
{
public:
	enum Type
	{
		ACCELEROMETER,
		GYROSCOPE
	};

	/**
	 * kalman filter can be initialized with accelerometer or gyroscope type
	 * prediction_level can be customised. Greater value means the output of Step() follows the raw input data as well
	 */
	KalmanGearS2(Type type, float prediction_level = DEFAULT_PREDICTION_LEVEL);

	/**
	 * Go to next step
	 * @param in input vector, e.g., accelerometer of gyroscope
	 * @param out output vector which is prediced by kalman filter
	 */
	void Step(glm::vec3 in, glm::vec3& out);

	/**
	 * Reset the filter to initial state
	 */
	void Reset();

private:
	void Initialize();

private:
	Type mType;

	float mPredictionLevel;

	unsigned long mStep;

	glm::vec3 mPredictionMean;
	glm::mat3x3 mPredictionCov;
	glm::vec3 mPrevOutput;

	glm::mat3x3 mA;
	glm::mat3x3 mB;
	glm::mat3x3 mC;

	glm::mat3x3 mR; // prediction noise

};


#endif // __KALMAN_MANAGER_H__

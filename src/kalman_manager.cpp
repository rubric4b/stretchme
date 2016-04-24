
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <cmath>
#include <string.h>

#include "kalman_manager.h"
#include "logger.h"

using namespace std;

////////////////////////////////////////////////////////////

static const glm::mat3x3 EYE(
					1.0, 0.0, 0.0,
					0.0, 1.0, 0.0,
					0.0, 0.0, 1.0);

// covariance from accelerations during static condition
static const glm::mat3x3 COV_Q(
					0.0006644, 0.0000704, 0.0000698,
					0.0000704, 0.0009777, 0.0002738,
					0.0000698, 0.0002738, 0.0009198);

// covariance from accelerations during static condition
static const glm::mat3x3 COV_Q_gyro(
					1.0, 0.0, 0.0,
					0.0, 1.0, 0.0,
					0.0, 0.0, 1.0);

static const glm::vec3 CONTROL(0, 0, 0);

KalmanGearS2::KalmanGearS2(Type type, float prediction_level)
: mType(type)
, mPredictionLevel(prediction_level)
, m_isInit(false)
{
	Initialize();
}

void KalmanGearS2::Step(glm::vec3 in, glm::vec3& out)
{
	if(!m_isInit) {
		m_isInit = true;
		out = in;
		mPrevOutput = out;

		return;
	}

	// prediction
	mPredictionMean = mA * mPrevOutput + mB * CONTROL;
	mPredictionCov = mA * mPredictionCov * glm::transpose(mA) + mR;

	// correction
	glm::mat3x3 K = (mPredictionCov * glm::transpose(mC)) * glm::inverse(mC * mPredictionCov * glm::transpose(mC) + (mType == ACCELEROMETER ? COV_Q : COV_Q_gyro));

	out = mPredictionMean + K * (in - mC * mPredictionMean);
	mPredictionCov = (EYE - K * mC) * mPredictionCov;

	mPrevOutput = out;
}

void KalmanGearS2::Reset()
{
	mStep = 0;
	m_isInit = false;
	Initialize();
}

void KalmanGearS2::Initialize()
{
	mStep = 0;

	mA = EYE;
	mB = EYE;
	mC = EYE;

	mR = EYE * mPredictionLevel;

	mPredictionCov = EYE;
	mPrevOutput = glm::vec3(0, 0, 0);
}


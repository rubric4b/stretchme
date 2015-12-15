
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <cmath>
#include <string.h>

#include "kalman/plane.h"
#include "kalman_manager.h"

using namespace std;
using namespace Kalman;


// Simple uniform distribution of zero mean and unit variance
float uniform(void)
{
   return((((float)rand())/(RAND_MAX-1) - 0.5f)* 3.464101615138f);
}

// Simple approximation of normal dist. by the sum of uniform dist.
float normal()
{
  int n = 6;
  int i;
  float temp = 0.0;

  for(i = 0; i < n; i++)
    temp += uniform();
  temp /= sqrt((float)n);
  return temp;
}

struct FLOAT3CONT{
	float x;
	float y;
	float z;
};


#define N 4 // number of state
#define M 2 // number of measure

// covariant matrix
static const double _P0[] = {	10.0*10.0,		0.0,		0.0,		0.0,
								0.0,			10.0*10.0,	0.0,		0.0,
								0.0,			0.0,		10.0*10.0,	0.0,
								0.0,			0.0,		0.0,		10.0*10.0
							};

static const double _P1[] = {	10.0*10.0,		0.0,		0.0,		0.0,
								0.0,			10.0*10.0,	0.0,		0.0,
								0.0,			0.0,		10.0*10.0,	0.0,
								0.0,			0.0,		0.0,		10.0*10.0
							};

static const double _P2[] = {	10.0*10.0,		0.0,		0.0,		0.0,
								0.0,			10.0*10.0,	0.0,		0.0,
								0.0,			0.0,		10.0*10.0,	0.0,
								0.0,			0.0,		0.0,		10.0*10.0
							};

class KalmanFilter
{
public:
	KalmanFilter(unsigned int nState, unsigned int nMeasure)
	: n(nState)
	, m(nMeasure)
	{
		Vector tmpX(n);
		x = tmpX;
		x2 = tmpX;
		x3 = tmpX;

		Matrix tmpP0(N, N, _P0);
		P0 = tmpP0;
		Matrix tmpP1(N, N, _P1);
		P1 = tmpP1;
		Matrix tmpP2(N, N, _P2);
		P2 = tmpP2;

		Vector tmpZ(m);
		z = tmpZ;
		z2 = tmpZ;
		z3 = tmpZ;

		x(1) = 0;		// pos X;
		x(2) = 0;		// vel X;
		x(3) = 0;		// pos Y;
		x(4) = 0;		// vel Y;

		x2(1) = 0;		// pos Y;
		x2(2) = 0;		// vel Z;
		x2(3) = 0;		// pos Y;
		x2(4) = 0;		// vel Z;

		x3(1) = 0;		// pos Z;
		x3(2) = 0;		// vel Z;
		x3(3) = 0;		// pos X;
		x3(4) = 0;		// vel X;

		filter.init(x, P0);
		filter2.init(x2, P1);
		filter3.init(x3, P2);

	}

	void Reset()
	{
		x(1) = 0;		// pos X;
		x(2) = 0;		// vel X;
		x(3) = 0;		// pos Y;
		x(4) = 0;		// vel Y;

		x2(1) = 0;		// pos Y;
		x2(2) = 0;		// vel Z;
		x2(3) = 0;		// pos Y;
		x2(4) = 0;		// vel Z;

		x3(1) = 0;		// pos Z;
		x3(2) = 0;		// vel Z;
		x3(3) = 0;		// pos X;
		x3(4) = 0;		// vel X;

		Matrix tmpP0(N, N, _P0);
		P0 = tmpP0;
		Matrix tmpP1(N, N, _P1);
		P1 = tmpP1;
		Matrix tmpP2(N, N, _P2);
		P2 = tmpP2;

		filter.init(x, P0);
		filter2.init(x2, P1);
		filter3.init(x3, P2);
	}

public:
	const unsigned int n;
	const unsigned int m;

	// input
	Vector x;
	Vector x2;
	Vector x3;

	// covariant
	Matrix P0;
	Matrix P1;
	Matrix P2;

	// result
	Vector z;
	Vector z2;
	Vector z3;

	// kalman filter instances
	cPlaneEKF filter;
	cPlaneEKF filter2;
	cPlaneEKF filter3;

};

/**
 * "reset" does not work. WHY???
 */
void kalman(vec3 pos, vec3 linear_acc, vec3& out_pos, vec3& out_vel, bool reset)
{
	static KalmanFilter* kf = NULL;
	static unsigned long nStep = 0;

	if(reset && nStep > 0 && kf)
	{
		delete kf;
		kf = NULL;
		nStep = 0;
	}

	if(!kf)
	{
		kf = new KalmanFilter(N, M);
	}
/*
	if(reset && nStep > 1)
	{
		kf->Reset();
		nStep = 0;
	}
*/
	// filter
	kf->z(1) = pos.x;
	kf->z(2) = pos.y;

	Vector u(M);
	u(1) = linear_acc.x;
	u(2) = linear_acc.y;

	kf->filter.step(u, kf->z);

	kf->z2(1) = pos.y;
	kf->z2(2) = pos.z;

	Vector u2(M);
	u2(1) = linear_acc.y;
	u2(2) = linear_acc.z;

	kf->filter2.step(u2, kf->z2);

	kf->z3(1) = pos.z;
	kf->z3(2) = pos.x;

	Vector u3(M);
	u3(1) = linear_acc.z;
	u3(2) = linear_acc.x;

	kf->filter3.step(u3, kf->z3);

	out_pos.x = (kf->filter.getX())(1);
	out_pos.y = (kf->filter2.getX())(1);
	out_pos.z = (kf->filter3.getX())(1);

	out_vel.x = (kf->filter.getX())(2);
	out_vel.y = (kf->filter2.getX())(2);
	out_vel.z = (kf->filter3.getX())(2);

	nStep++;

	return;
}


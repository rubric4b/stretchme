// -------------- plane.cpp - Example of Extended Kalman filter ------------------------//
//
// This file is part of kfilter.
// kfilter is a C++ variable-dimension extended kalman filter library.
//
// Copyright (C) 2004        Vincent Zalzal, Sylvain Marleau
// Copyright (C) 2001, 2004  Richard Gourdeau
// Copyright (C) 2004        GRPR and DGE's Automation sector
//                           ?ole Polytechnique de Montr?l
//
// Code adapted from algorithms presented in :
//      Bierman, G. J. "Factorization Methods for Discrete Sequential
//      Estimation", Academic Press, 1977.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

// --------------------------- Example of Extended Kalman filter ------------------------//
/*
% A plane flights in a 2D space where the x axis is the distance traveled
% by the plane and y axis is its altitude.  This system can be represented
% by the fallowing equations:
% (This is just an example)
%
% xpp = F/m - bx/m * xp^2
% ypp = p/m * xp^2 - g
%
% where m is the plane's weight (1000 kg)
%       bx is the drag coefficient (0.35 N/m?s?
%       p is the lift force (3.92 N/m?s?
%       g is the gravitational acceleration (9.8 m/s?
%       F is the motor's thrust
%
% A station on the ground (at the origin) mesures the angle between the
% plane and the ground (x axis) and the distance between the plane and the station.
% These measures are based and the fallowing equations:
%
% theta = atan2(y,x)
% r = sqrt(x^2+y^2)
%
% The variance error matrix of the mesures is:
%
% R = [0.01^2  0
%      0       50^2]
%
% V = [1 0;
%      0 1];
%
% The variance error matrix of the plane's model is: WQW'
%
% Q = [0.01^2    0;
%      0         0.01^2];
%
% W = [0 0;
%      1 0;
%      0 0;
%      0 1];
%
*/


#include <cmath>
#include <iostream>
#include "kalman/plane.h"

using namespace std;

cPlaneEKF::cPlaneEKF() 
{
	setDim(4, 1, 2, 2, 2);

	Period = 0.02;
	//Gravity = 9.8;
	//Bfriction = 0.35;
	//Portance = 3.92;
	//Mass = 1000;
}

void cPlaneEKF::makeBaseA()
{
	A(1, 1) = 1.0;
	// A(1,2) = Period - Period*Period*Bfriction/Mass*x(2);
	A(1, 3) = 0.0;
	A(1, 4) = 0.0;

	A(2, 1) = 0.0;
	// A(2,2) = 1 - 2*Period*Bfriction/Mass*x(2);
	A(2, 3) = 0.0;
	A(2, 4) = 0.0;

	A(3, 1) = 0.0;
	// A(3,2) = Period*Period*Portance/Mass*x(2);
	A(3, 3) = 1.0;

	A(4, 1) = 0.0;
	// A(4,2) = 2*Period*Portance/Mass*x(2);
	A(4, 3) = 0.0;
	A(4, 4) = 1.0;
}

void cPlaneEKF::makeA()
{
	// A(1,1) = 1.0;
	A(1, 2) = Period;
	// A(1,3) = 0.0;
	// A(1,4) = 0.0;
	A(3, 4) = Period;
	// A(2,1) = 0.0;
	A(2, 2) = 1;
	// A(2,3) = 0.0;
	// A(2,4) = 0.0;

	// A(3,1) = 0.0;
	A(3, 2) = 0;
	// A(3,3) = 1.0;
	// A(3,4) = Period;

	// A(4,1) = 0.0;
	A(4, 2) = 0;
	// A(4,3) = 0.0;
	// A(4,4) = 1.0;
}

void cPlaneEKF::makeBaseW()
{
	// 6 by 3
	
	W(1, 1) = 0.0;	W(1, 2) = 0.0;	
	W(2, 1) = 1.0;	W(2, 2) = 0.0;	
	W(3, 1) = 0.0;	W(3, 2) = 0.0;	
	W(4, 1) = 0.0;	W(4, 2) = 1.0;	
}

#define NOISE_VAR 0.001f

void cPlaneEKF::makeBaseQ()
{
	// 3 by 3
	Q(1, 1) = NOISE_VAR*NOISE_VAR; 			Q(1, 2) = NOISE_VAR*NOISE_VAR;		
	Q(2, 1) = NOISE_VAR*NOISE_VAR;			Q(2, 2) = NOISE_VAR*NOISE_VAR;			
}

void cPlaneEKF::makeBaseH()
{
	// H(1,1) = -x(3)/(x(1)*x(1)+x(3)*x(3));
	H(1,2) = 0.0;
	// H(1,3) = x(1)/(x(1)*x(1)+x(3)*x(3));
	H(1,4) = 0.0;

	// H(2,1) = x(1)/sqrt(x(1)*x(1)+x(3)*x(3));
	H(2,2) = 0.0;
	// H(2,3) = x(3)/sqrt(x(1)*x(1)+x(3)*x(3));
	H(2,4) = 0.0;
}

void cPlaneEKF::makeH()
{
	// 3 by 6 

	//H(1,1) = -x(3)/(x(1)*x(1)+x(3)*x(3));
	//// H(1,2) = 0.0;
	//H(1,3) = x(1)/(x(1)*x(1)+x(3)*x(3));
	//// H(1,4) = 0.0;

	//H(2,1) = x(1)/sqrt(x(1)*x(1)+x(3)*x(3));
	//// H(2,2) = 0.0;
	//H(2,3) = x(3)/sqrt(x(1)*x(1)+x(3)*x(3));
	//// H(2,4) = 0.0;

	H(1, 1) = 1;
	// H(1,2) = 0.0;
	H(1, 3) = 0;
	// H(1,4) = 0.0;

	H(2, 1) = 0;
	// H(2,2) = 0.0;
	H(2, 3) = 1;
	// H(2,4) = 0.0;
}

void cPlaneEKF::makeBaseV()
{
	V(1,1) = 1.0;
	V(2,2) = 1.0;
}

void cPlaneEKF::makeBaseR()
{
	R(1,1) = 50*50;
	R(2,2) = 50*50;
}

void cPlaneEKF::makeProcess()
{
	Vector x_(x.size());

	x_(1) = x(1) + x(2)*Period + u(1)*Period*Period;			// x pos
	x_(2) = x(2) + u(1)*Period;									// x vel	
	x_(3) = x(3) + x(4)*Period + u(2)*Period*Period;			// y pos
	x_(4) = x(4) + u(2)*Period;									// y vel

	x.swap(x_);
}

void cPlaneEKF::makeMeasure()
{
	z(1) = x(1)*0.95; 
	z(2) = x(3)*0.95;
}


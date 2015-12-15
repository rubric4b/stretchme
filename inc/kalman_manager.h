#ifndef __KALMAN_MANAGER_H__
#define __KALMAN_MANAGER_H__

#include <glm/glm.hpp>

using namespace glm;

void kalman(vec3 pos, vec3 linear_acc, vec3& ad_pos, vec3& vel, bool reset);

#endif // __KALMAN_MANAGER_H__

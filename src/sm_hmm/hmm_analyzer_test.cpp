#include "sm_hmm/hmm_analyzer_test.h"

#include <algorithm>
#include "sm_hmm/hmm_model_test_armup.h"
#include "logger.h"
#include "glm/gtc/quaternion.hpp"


#define MOVING_THRESHOLD 1
#define MOVING_THRESHOLD_CNT 30
#define MOVING_WINDOW 15

const unsigned int HA_Test::INTERPOLATION_COUNT = 100;

using glm::vec3;
using glm::quat;
using std::vector;
using std::queue;

static const vec3 GRAVITY_DIR(0, 0, 1);

inline double length_2(const vec3 &in) {
	double len = length(in);
	return len * len;
}

// return the quaternion which rotates src to dst
inline quat get_rotation_between(const vec3 &src, const vec3 &dst) {

	double k_cos_theta = dot(src, dst);
	double k = sqrt(length_2(src) * length_2(dst));

	if (k_cos_theta / k == -1) {
		// 180 degree rotation around any orthogonal vector;
		return quat(0, vec3(0));
	}

	return normalize(quat(k_cos_theta + k, cross(src, dst)));

}

inline quat apply_rotation(const quat &lhs_rot, const vec3 &rhs_vec) {
	return lhs_rot * quat(0, rhs_vec) * glm::conjugate(lhs_rot);
}

HA_Test::HA_Test() :
		is_prev(false),
		m_isInitMove(false),
		m_isStay(false),
		m_moveCnt(0),
		m_stayCnt(0),
		m_windowQueue(),
		m_observations() {
	m_observations.reserve(800);
}

HA_Test::~HA_Test() {}

void HA_Test::reset() {
	DBG("HA_Test::reset()\n");
	is_prev = false;
	m_isInitMove = false;
	m_isStay = false;
	m_moveCnt = 0;
	m_stayCnt = 0;
	queue<vec3> empty;
	std::swap(m_windowQueue, empty);
	m_observations.clear();
}

bool HA_Test::get_Observation(const vec3 &curr_observation, vector<float> &observation) {

	return false;
}

bool HA_Test::is_End() {
//    DBG("is_End() = stay, init_move(%d, %d)\n", is_stay, is_init_move);
	return (m_isStay && m_isInitMove);
}

bool HA_Test::get_Observation(vector<float> &curr_observation, vector<float> &observation) {
	if (curr_observation.size() != 3) {
		ERR("current observation size is not the same with three!\n");
		return false;
	}

	vec3 curr(curr_observation[0], curr_observation[1], curr_observation[2]);

	return get_Observation(curr, observation);
}


/// return 움직이면 true, 안움직이면 false
bool HA_Test::analyze(const glm::vec3 &curr_observation) {

	m_windowQueue.push(curr_observation);

	if (m_windowQueue.size() > MOVING_WINDOW) {
		m_windowQueue.pop();
	}
	vec3 post = m_windowQueue.front();
	float diff = glm::abs(glm::length(post) - glm::length(curr_observation));

	quat rot = get_rotation_between(post, curr_observation);
	vec3 rot_euler = glm::eulerAngles(rot);
//	LOGI("length %f    %f", length(rot_euler), length(vec3(glm::radians(1.0))));



	return !m_isInitMove ? ((length(rot_euler) > length(vec3(glm::radians(10.0)))) && (diff > 0.1))
						 : (length(rot_euler) > length(vec3(glm::radians(3.0))));

}

bool HA_Test::set_Observation(const glm::vec3 &curr_observation) {

	if (is_End())
		return false;

	bool is_move = analyze(curr_observation);

	if (!m_isInitMove && is_move) {
		queue<vec3> moveWindow(m_windowQueue);
		for (; !moveWindow.empty(); moveWindow.pop()) {
			m_observations.push_back(moveWindow.front());
		}
		m_isInitMove = true;
	}

	if (m_isInitMove) {
		m_observations.push_back(curr_observation); //움직임이 모두 담김.

		if (is_move) {
			m_stayCnt = 0;
		} else {
			m_stayCnt++;
		}
	}

	if (m_stayCnt > MOVING_WINDOW) {
		for (int i = 0; i < MOVING_WINDOW; i++) {
			m_observations.pop_back();
		}
		LOGI("move cnt [ %d ]", m_observations.size());
		m_isStay = true;
	}

	return is_move;
}

inline vec3 lerp(const vec3 &lhs, const vec3 &rhs, float t) {
	return lhs * t + rhs * (1.f - t);
}

bool HA_Test::calculate_Observation(VecData &out_observ) {
	// interpolation step
	vector<vec3> observations;
	observations.reserve(INTERPOLATION_COUNT);
	float total_size = static_cast<float>(m_observations.size());
	float step_size = (total_size - 1.0f) / static_cast<float>(INTERPOLATION_COUNT - 1);
//	float min_len(9.8f), max_len(9.8f);
	m_observations.push_back(m_observations.back()); // add last value for n+1 index

	for (float idx = 0.f; idx < total_size; idx += step_size) {
		float t = idx - glm::floor(idx);
		unsigned int n = static_cast<unsigned int> (glm::floor(idx));
		vec3 v = lerp(m_observations.at(n), m_observations.at(n + 1), t);
		float v_len = glm::length(v);
//		min_len = glm::min(min_len, v_len);
//		max_len = glm::max(max_len, v_len);
		observations.push_back(v);
//		LOGI("n[%d], v [ %8.6f, %8.6f, %8.6f ]", n, v.x, v.y, v.z);
	}

	LOGI("interpolation result : size [ %d ], min [ %f ], max [ %f ]", observations.size(), 0, 0);

	vec3 base_v = observations.front(); //get the base observation vector
	quat rot_to_g = get_rotation_between(base_v, GRAVITY_DIR);
//	float mid = (min_len + max_len) / 2.f;
//	float val_len = max_len - mid;

	vector<vec3>::iterator iter = observations.begin();
	vec3 prev = *iter;
	for (; iter != observations.end(); prev = *iter++) {
		ob_Container container = {0,};
		quat v = apply_rotation(rot_to_g, *iter);
		v.w = 0;
		v = glm::normalize(v);
//		vec3 v = *iter;
//		vec3 v = glm::eulerAngles(get_rotation_between(*iter, GRAVITY_DIR));
		container.observ[DIR_X] = v.x;
		container.observ[DIR_Y] = v.y;
		container.observ[DIR_Z] = v.z;
		container.observ[DIFF_X] = iter->x - prev.x;
		container.observ[DIFF_Y] = iter->y - prev.y;
		container.observ[DIFF_Z] = iter->z - prev.z;

		container.observ[LENGTH] = length(*iter);
		out_observ.push_back(container);
/*
		LOGI("container vec [%f,%f,%f,%f]",
			 out_observ.back().observ[DIR_X],
			 out_observ.back().observ[DIR_Y],
			 out_observ.back().observ[DIR_Z],
			 out_observ.back().observ[LENGTH]
		);
*/
	}

//	VecData(out_observ.rbegin(), out_observ.rend()).swap(out_observ);


	LOGI("front [%f, %f, %f, %f], size [ %d ]",
		 out_observ.front().observ[DIR_X],
		 out_observ.front().observ[DIR_Y],
		 out_observ.front().observ[DIR_Z],
		 out_observ.front().observ[LENGTH], out_observ.size());

	LOGI("back [%f, %f, %f, %f], size [ %d ]",
		 out_observ.back().observ[DIR_X],
		 out_observ.back().observ[DIR_Y],
		 out_observ.back().observ[DIR_Z],
		 out_observ.back().observ[LENGTH], out_observ.size());


	return true;
}


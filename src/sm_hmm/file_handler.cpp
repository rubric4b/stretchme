//
// Created by hobbang5 on 2016-03-23.
//


#include <app.h>

#include "sm_hmm/file_handler.h"
#include "kalman_manager.h"
#include "logger.h"

static char *util_strtok(char *str, const char *delim, char **nextp) {
	char *ret;

	if (str == NULL) {
		str = *nextp;
	}

	str += strspn(str, delim);

	if (*str == '\0') {
		return NULL;
	}

	ret = str;

	str += strcspn(str, delim);

	if (*str) {
		*str++ = '\0';
	}

	*nextp = str;

	return ret;
}


bool write_hmm_to_file(const std::string &file_name, const xmm::HMM &model) {
	// out file stream
	std::ofstream out_fstream;

	// file data path
	std::string file_path = app_get_data_path() + file_name;

	out_fstream.open(file_path.c_str(), std::ios::out | std::ios::binary);
	if (!out_fstream.is_open() || !out_fstream.good()) {
		std::string msg = "Failed to open file " + file_path;
		ERR("%s\n", msg.c_str());
		return false;
	}

	JSONNode jsonfile = model.to_json();

	out_fstream << jsonfile.write_formatted();

	out_fstream.close();

	DBG("%s is saved", file_path.c_str());
	return true;

}


/**
 * @brief read method for python wrapping ('read' keyword forbidden, name has to be different)
 * @warning only defined if SWIGPYTHON is defined
 */
bool read_hmm_from_file(const std::string &file_name, xmm::HMM &model) {
	// in file stream
	std::ifstream in_file;

	// file path
	std::string file_path = app_get_data_path() + file_name;

	// file open
	in_file.open(file_path.c_str(), std::ios::in | std::ios::binary);
	if (!in_file.is_open() || !in_file.good()) {
		std::string msg = "Failed to open file " + file_path;
		ERR("%s\n", msg.c_str());
		return false;
	}

	std::string jsonstring;

	in_file.seekg(0, std::ios::end);
	jsonstring.reserve(in_file.tellg());
	in_file.seekg(0, std::ios::beg);

	jsonstring.assign((std::istreambuf_iterator<char>(in_file)),
					  std::istreambuf_iterator<char>());
	JSONNode jsonfile = libjson::parse(jsonstring);
	model.from_json(jsonfile);

	in_file.close();

	DBG("HMM is read from file %s\n", file_path.c_str());

	return true;
}


void
record_training_set_from_file(Hmm_Analyzer *analyzer,
							  const std::string &file_name,
							  PathType type,
							  int ts_phrase_index,
							  int ts_dim,
							  xmm::TrainingSet &ts) {
	// in file stream
	std::ifstream in_file;

	// file path
	std::string file_path;
	switch (type) {
		case DATA_PATH :
			file_path = app_get_data_path();
			break;
		case SHARED_DATA_PATH:
			file_path = app_get_shared_data_path();
			break;
		case RES_PATH :
			file_path = app_get_resource_path();
			break;
		case USER_PATH :
			file_path = "";
			break;
		default:
			break;
	}
	file_path += file_name;

	// file open
	in_file.open(file_path.c_str(), std::ios::in | std::ios::binary);
	if (!in_file.is_open() || !in_file.good()) {
		std::string msg = "Failed to open file " + file_path;
		ERR("%s\n", msg.c_str());
		throw std::runtime_error(msg.c_str());
	} else {
		std::string msg = "Success to open file " + file_path;
		DBG("%s\n", msg.c_str());
	}

	// record file to training set
	int line_cnt(0), ts_cnt(0);
	KalmanGearS2 m_kFilter(KalmanGearS2::ACCELEROMETER, 0.00001);

	char line_buffer[512];
	glm::vec3 curr(0);
	glm::vec3 curr_k_filtered(0);

	std::vector<float> observation(ts_dim);
	while (!in_file.getline(line_buffer, sizeof(line_buffer)).eof()) {
		char *word, *wordPtr;
		word = util_strtok(line_buffer, ",", &wordPtr); // time
		word = util_strtok(NULL, ",", &wordPtr); // accel - x
		curr.x = atof(word);
		word = util_strtok(NULL, ",", &wordPtr); // accel - y
		curr.y = atof(word);
		word = util_strtok(NULL, ",", &wordPtr); // accel - z
		curr.z = atof(word);
		m_kFilter.Step(curr, curr_k_filtered);

		if (analyzer->get_Observation(curr_k_filtered, observation)) {

			ts.recordPhrase(ts_phrase_index, observation);
			ts_cnt++;
		}

		line_cnt++;
	}
	in_file.close();
	analyzer->reset();

	DBG("%s : %d lines are read", file_name.c_str(), line_cnt);
	DBG("%s : %d observation are recorded to training set", file_name.c_str(), ts_cnt);

}


void
set_observ_from_file(Hmm_Analyzer *analyzer,
					 const std::string &file_name,
					 PathType type) {
	// in file stream
	std::ifstream in_file;

	// file path
	std::string file_path;
	switch (type) {
		case DATA_PATH :
			file_path = app_get_data_path();
			break;
		case SHARED_DATA_PATH:
			file_path = app_get_shared_data_path();
			break;
		case RES_PATH :
			file_path = app_get_resource_path();
			break;
		case USER_PATH :
			file_path = "";
			break;
		default:
			break;
	}
	file_path += file_name;

	// file open
	in_file.open(file_path.c_str(), std::ios::in | std::ios::binary);
	if (!in_file.is_open() || !in_file.good()) {
		std::string msg = "Failed to open file " + file_path;
		ERR("%s\n", msg.c_str());
		throw std::runtime_error(msg.c_str());
	} else {
		std::string msg = "Success to open file " + file_path;
		DBG("%s\n", msg.c_str());
	}

	// record file to training set
	int line_cnt(0), ts_cnt(0);
	KalmanGearS2 m_kFilter(KalmanGearS2::ACCELEROMETER, 0.00001);

	char line_buffer[512];
	glm::vec3 curr(0);
	glm::vec3 curr_k_filtered(0);

	while (!in_file.getline(line_buffer, sizeof(line_buffer)).eof()) {
		char *word, *wordPtr;
		word = util_strtok(line_buffer, ",", &wordPtr); // time
		word = util_strtok(NULL, ",", &wordPtr); // accel - x
		curr.x = atof(word);
		word = util_strtok(NULL, ",", &wordPtr); // accel - y
		curr.y = atof(word);
		word = util_strtok(NULL, ",", &wordPtr); // accel - z
		curr.z = atof(word);
		m_kFilter.Step(curr, curr_k_filtered);

		analyzer->set_Observation(curr_k_filtered);

		line_cnt++;
	}
	in_file.close();

	DBG("%s : %d lines are read", file_name.c_str(), line_cnt);

}
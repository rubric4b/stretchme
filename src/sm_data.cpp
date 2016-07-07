#include <fstream>
#include <sstream>

#include "main.h"
#include "sm_data.h"
#include "logger.h"

#define DATA_FILE_PATH "/opt/usr/media/stretching_data.txt"
#define EX_TYPE_LEN 1
#define TIME_LEN 19
#define TYPE_LEN 1
#define ACHIEVE_COUNT 1
#define ST_TYPE_LEN 1
#define RATE_INT_LEN 5
#define RATE_REAL_LEN 8
#define RATE_LEN (RATE_INT_LEN+1+RATE_REAL_LEN) // 12345.12345678
#define DATA_LINE_LENGTH (EX_TYPE_LEN + 1 + TIME_LEN + 1 + TYPE_LEN + 1 + ACHIEVE_COUNT + 1 + ST_TYPE_LEN + 1 + RATE_LEN + 1)

#define EXPERIMENT_TYPE1_FILE_PATH "/opt/usr/media/stretchme.ex1"
#define EXPERIMENT_TYPE2_FILE_PATH "/opt/usr/media/stretchme.ex2"
#define EXPERIMENT_TYPE3_FILE_PATH "/opt/usr/media/stretchme.ex3"

static char* util_strtok(char* str, const char* delim, char** nextp)
{
    char* ret;

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

/**
 * val : Float value
 * nInt : number of digit for integer part. if the val is less than zero, 1 slot (char) should be consumed by 'minus'
 * nReal : number of digit for real part.
 */
static std::string get_const_size_string_from_float(double val, int nInt, int nReal)
{
	std::ostringstream ret;

	int num_digit_of_int = 1;
	int int_of_val = abs((int)val);

	while(1)
	{
		int_of_val /= 10;
		if(int_of_val < 1)
			break;
		num_digit_of_int++;
	}

	if(val < 0)
	{
		// one slot should be consumed by '-'
		num_digit_of_int++;
	}

	for(int i = 0; i < nInt - num_digit_of_int; i++)
	{
		ret << " ";
	}

	ret << (int)val;

	ret << ".";

	double tmp = val - (int)val; // remove int part
	int tmp_int = 0;

	if(val < 0)
	{
		tmp = -tmp;
	}

	for(int i = 0; i < nReal; i++)
	{
		tmp *= 10; // shift left
		tmp_int = (int)(tmp);

		ret << tmp_int;

		tmp -= tmp_int; // remove int part
	}
	ret << "\0";

	DBG("#### RET_STR : [%s]", ret.str().c_str());

	return ret.str();
}

/**
 * store the time as the last in the file
 *
 * @param[in] timestamp time to store in file
 * @return true if file writing was succeeded
 */
bool store_last_time(time_t timestamp, LOG_TYPE type, int achieve_count, StretchType stt, double recog_rate)
{
	// open file
	std::ofstream out_file;
	out_file.open(DATA_FILE_PATH, std::ios::out | std::ofstream::app | std::ios::binary);

	if (out_file.is_open() && out_file.good())
	{
		char buf[DATA_LINE_LENGTH+1];

		struct tm* struct_time;
		struct_time = localtime(&timestamp);

		snprintf(buf, sizeof(buf), "%d,%04d-%02d-%02d %02d:%02d:%02d,%d,%d,%d,%s\n", get_experiment_type() + 1, struct_time->tm_year + 1900, struct_time->tm_mon + 1, struct_time->tm_mday, struct_time->tm_hour, struct_time->tm_min, struct_time->tm_sec,
		type, achieve_count, stt, get_const_size_string_from_float(recog_rate, RATE_INT_LEN, RATE_REAL_LEN).c_str() /* make 5.8f */);
		out_file << buf;

//		std::cout << "write : " << buf;

		out_file.close();
	}
	else
	{
        ERR("%s file open failed\n", DATA_FILE_PATH);
		return false;
	}

	return true;
}

/**
 * store the current time as the last in the file
 *
 * @return true if file writing was succeeded
 */
bool store_last_time_with_current(LOG_TYPE type, int achieve_count, StretchType stt, double recog_rate)
{
	time_t current_time;
	time(&current_time);

	return store_last_time(current_time, type, achieve_count, stt, recog_rate);
}

/**
 * get the last stretching time from stored file
 *
 * @param[out] timestamp the last stretching time for type
 * @param[in] type LOG_TYPE
 * @return true if success
 */
bool get_stored_last_time(time_t* timestamp, LOG_TYPE type)
{
	std::ifstream in_file;

	// file open
	in_file.open(DATA_FILE_PATH, std::ios::in | std::ios::binary);

	if (in_file.is_open() && in_file.good())
	{
		// go to end of the file
		in_file.seekg(0, in_file.end);

		// read line reversely
		while (in_file.tellg() > 1)
		{
			in_file.seekg(-DATA_LINE_LENGTH, std::ios::cur);

			std::string line;
			std::getline(in_file, line);
			int line_length = (int)line.length() +1;
			in_file.seekg(-line_length, std::ios::cur); // move to begin of the line

			char *word1, *word2, *wordPtr;
			char* pline = strdup(line.c_str()); // make char* from const char*

			// parsing time string
			word1 = util_strtok(pline+2, ",", &wordPtr); // time
			// parsing type string
			word2 = util_strtok(NULL, ",", &wordPtr); // type
			int st_type = atoi(word2);

			if(st_type == type)
			{
				// make timestamp from time formatted string
				struct tm tm;
				char* ret = strptime(word1, "%Y-%m-%d %H:%M:%S", &tm);

				DBG("last time for %d type : %s", type, word1);

				*timestamp = mktime(&tm);

				free(pline);
				in_file.close();

				return true;
			}
			else
			{
				DBG("No last %d type log");
			}

			free(pline);
		}

		in_file.close();
	}
	else
	{
        ERR("%s file open failed\n", DATA_FILE_PATH);
	}

	return false;

}

/**
 * get the number of stretching from stored file with type
 *
 * @param[in] type LOG_TYPE
 * @return true if success
 */
int get_counts_in_today(LOG_TYPE type)
{
	int ret_cnt(0);
	std::ifstream in_file;
	time_t succeed_time;
	struct tm today;

	time(&succeed_time); //get current time
	today = *localtime(&succeed_time); //convert time_t to tm struct

	// file open
	in_file.open(DATA_FILE_PATH, std::ios::in | std::ios::binary);

	if (in_file.is_open() && in_file.good())
	{
		// go to end of the file
		in_file.seekg(0, in_file.end);

		// read line reversely
		while (in_file.tellg() > 1)
		{
			in_file.seekg(-DATA_LINE_LENGTH, std::ios::cur);

			std::string line;
			std::getline(in_file, line);
			int line_length = (int)line.length() +1;
			in_file.seekg(-line_length, std::ios::cur);

			char *word1, *word2, *wordPtr;
			char* pline = strdup(line.c_str()); // make char* from const char*

			// parsing time string
			word1 = util_strtok(pline+2, ",", &wordPtr); // time
			// parsing type string
			word2 = util_strtok(NULL, ",", &wordPtr); // type
			int st_type = atoi(word2);

			if(st_type == type)
			{
				// make timestamp from time formatted string
				DBG("%s\n", word1);
				struct tm tm;
				char* ret = strptime(word1, "%Y-%m-%d %H:%M:%S", &tm);

				if(today.tm_mday != tm.tm_mday) // different day
					break;

				time_t log_time = mktime(&tm);
				if (difftime(succeed_time, log_time) >= 3500 || ret_cnt == 0 ) {
					succeed_time = log_time;

					// increase count
					ret_cnt++;
				}
			}

			free(pline);
		}

		in_file.close();
	}
	else
	{
		ERR("%s file open failed\n", DATA_FILE_PATH);
		return false;
	}

	return ret_cnt;
}

/**
 * get elapsed time from the last stretching time to current
 *
 * @return timestamp which contain difference between times
 */
double get_elapsed_time_from_last(LOG_TYPE type)
{
	time_t last;
	bool ret = get_stored_last_time(&last, type);

	if(ret)
	{
		// get current time
		time_t current_time;
		time(&current_time);

		return difftime(current_time, last);
	}

	return 0;
}

/**
 * get the level of awareness according to elapsed time from stored data
 *
 * @return level that 1 ~ 4, 1 means the slightness, 4 means the seriousness (over 1 day)
 */
int get_awareness_level()
{
	return get_awareness_level_from_data(get_elapsed_time_from_last(ST_SUCCESS));
}

/**
 * get the level of awareness according to elapsed time
 *
 * @param[in] diff time difference for elapsed time after the last stretching
 * @return level that 1 ~ 4, 1 means the slightness, 4 means the seriousness (over 1 day)
 */
int get_awareness_level_from_data(double diff)
{
	if(diff > 0) {
		if (diff > 60 * 60 * 24) {  // 1day
			return 4;
		} else if( diff > 60 * 60 * 3) {  // 3 hour
			return 3;
		} else if( diff > 60 * 30) { // 30 min
			return 2;
		} else {
			return 1;
		}
	}

	return 0;
}

Experiment_Type get_experiment_type()
{
	static Experiment_Type ext = EXPERIMENT_MAX;

	if(ext == EXPERIMENT_MAX)
	{
		std::ifstream ex_file;

		// file open
		ex_file.open(EXPERIMENT_TYPE1_FILE_PATH, std::ios::in | std::ios::binary);

		if (ex_file.is_open() && ex_file.good())
		{
			ext = EXPERIMENT_1;
			ex_file.close();
		}
		else
		{
			// file open
			ex_file.open(EXPERIMENT_TYPE2_FILE_PATH, std::ios::in | std::ios::binary);

			if (ex_file.is_open() && ex_file.good())
			{
				ext = EXPERIMENT_2;
				ex_file.close();
			}
			else
			{
#if 0
				// file open
				ex_file.open(EXPERIMENT_TYPE3_FILE_PATH, std::ios::in | std::ios::binary);

				if (ex_file.is_open() && ex_file.good())
				{
					ext = EXPERIMENT_3;
					ex_file.close();
				}
#else
				ext = EXPERIMENT_3; // in default
#endif
			}
		}
	}

	return ext;
}




#include <fstream>

#include "sm_data.h"
#include "logger.h"

#define DATA_FILE_PATH "/opt/usr/media/stretching_data.txt"
#define DATA_LINE_LENGTH 22

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
 * store the time as the last in the file
 *
 * @param[in] timestamp time to store in file
 * @return true if file writing was succeeded
 */
bool store_last_time(time_t timestamp, LOG_TYPE type)
{
	// open file
	std::ofstream out_file;
	out_file.open(DATA_FILE_PATH, std::ios::out | std::ofstream::app | std::ios::binary);

	if (out_file.is_open() && out_file.good())
	{
		char buf[25];

		struct tm* struct_time;
		struct_time = localtime(&timestamp);

		snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d,%d\n", struct_time->tm_year + 1900, struct_time->tm_mon + 1, struct_time->tm_mday, struct_time->tm_hour, struct_time->tm_min, struct_time->tm_sec, type);
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
bool store_last_time_with_current(LOG_TYPE type)
{
	time_t current_time;
	time(&current_time);

	return store_last_time(current_time, type);
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
			in_file.seekg(-line_length, std::ios::cur);

			char *word1, *word2, *wordPtr;
			char* pline = strdup(line.c_str()); // make char* from const char*

			// parsing time string
			word1 = util_strtok(pline, ",", &wordPtr); // time
			// parsing type string
			word2 = util_strtok(NULL, ",", &wordPtr); // type
			int st_type = atoi(word2);
			free(pline);

			if(st_type == type)
			{
				// make timestamp from time formatted string
				struct tm tm;
				char* ret = strptime(word1, "%Y-%m-%d %H:%M:%S", &tm);

				if(ret == NULL)
				{
					ERR("strptime is failed");
					return false;
				}

				*timestamp = mktime(&tm);

				break;
			}

		}

		in_file.close();
	}
	else
	{
        ERR("%s file open failed\n", DATA_FILE_PATH);
		return false;
	}

	return true;
}

/**
 * get elapsed time from the last stretching time to current
 *
 * @return timestamp which contain difference between times
 */
time_t get_elapsed_time_from_last(LOG_TYPE type)
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
int get_awareness_level_from_data(time_t diff)
{
	int level = 0;

	if(diff > 1)
	{
		int d_day = diff / (60 * 60 * 24);
		diff -= d_day * 60 * 60 * 24;
		int d_hour = diff / (60 * 60);
		diff -= d_hour * 60 * 60;
		int d_min = diff / 60;
		diff -= d_min * 60;
		int d_sec = diff;

		if(d_day >= 1)
			level = 4;
		else if(d_hour >= 3)
			level = 3;
		else if(d_hour > 1 && d_min >= 30)
			level = 2;
		else
			level = 1;
	}

	return level;
}



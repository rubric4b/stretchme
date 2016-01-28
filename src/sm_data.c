
#include "sm_data.h"
#include "logger.h"

#define DATA_FILE_PATH "/opt/usr/media/stretching_data.txt"

/**
 * store the time as the last in the file
 *
 * @param[in] timestamp time to store in file
 * @return true if file writing was succeeded
 */
bool store_last_time(time_t timestamp)
{
	// get the last success time from file
	FILE *fp = fopen(DATA_FILE_PATH, "w+");

	if(fp != NULL)
	{
		char timeformat[20];

		struct tm* struct_time;
		struct_time = localtime(&timestamp);

		snprintf(timeformat, sizeof(timeformat), "%04d-%02d-%02d %02d:%02d:%02d", struct_time->tm_year+1900, struct_time->tm_mon+1, struct_time->tm_mday, struct_time->tm_hour, struct_time->tm_min, struct_time->tm_sec);

		fwrite (timeformat, 1, sizeof(timeformat), fp);

		fclose(fp);

		DBG("%s(%d) file writing was succeeded with %s\n", __FUNCTION__, __LINE__, timeformat);
	}
	else
	{
		DBG("%s(%d) file writing was failed\n", __FUNCTION__, __LINE__);
		return false;
	}

	return true;
}

/**
 * store the current time as the last in the file
 *
 * @return true if file writing was succeeded
 */
bool store_last_time_to_current()
{
	time_t current_time;
	time(&current_time);

	return store_last_time(current_time);
}

/**
 * get the last stretching time from stored file
 *
 * @param[out] timestamp the last stretching file
 * @return true if success
 */
bool get_stored_last_time(time_t* timestamp)
{
	char timeformat[20];

	// get the last success time from file
	FILE *fp = fopen(DATA_FILE_PATH, "rw+");

	if(fp != NULL)
	{
		if(fgets(timeformat, sizeof(timeformat), fp) != NULL)
		{
			DBG("reading data : %s", timeformat);
		}
		else
		{
			return false;
		}

		fclose(fp);
	}
	else
	{
		return false;
	}

	// make timestamp from time formatted string
	struct tm tm;
	char* ret = strptime(timeformat, "%Y-%m-%d %H:%M:%S", &tm);
	if(ret == NULL)
	{
		dlog_print(DLOG_DEBUG, LOG_TAG, "strptime is failed");
		return false;
	}

	*timestamp = mktime(&tm);

	return true;
}

/**
 * get elapsed time from the last stretching time to current
 *
 * @return timestamp which contain difference between times
 */
time_t get_elapsed_time_from_last()
{
	time_t last;
	bool ret = get_stored_last_time(&last);

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
	return get_awareness_level_from_data(get_elapsed_time_from_last());
}

/**
 * get the level of awareness according to elapsed time
 *
 * @param[in] diff time difference for elapsed time after the last stretching
 * @return level that 1 ~ 4, 1 means the slightness, 4 means the seriousness (over 1 day)
 */
int get_awareness_level_from_data(time_t diff)
{
	int level = 1;

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
	}

	return level;
}


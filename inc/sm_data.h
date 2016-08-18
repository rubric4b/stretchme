#ifndef __SM_DATA_H__
#define __SM_DATA_H__

#define  _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "stretch_interface.h"

#ifndef bool
#define bool int
#endif

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif


#ifdef __cplusplus
extern "C" {
#endif

#define DAILY_GOAL_COUNT 5 /* over 5 */

typedef enum
{
	ST_TRIAL = 0,
	ST_SUCCESS,
	ST_FAIL
}LOG_TYPE;

/**
 * get the last stretching time from stored file
 *
 * @param[out] timestamp the last stretching file
 * @return true if success
 */
bool get_stored_last_time(time_t* timestamp, LOG_TYPE type);

/**
 * get elapsed time from the last stretching time to current
 *
 * @return timestamp which contain difference between times
 */
double get_elapsed_time_from_last(LOG_TYPE type);

/**
 * get the level of awareness according to elapsed time from stored data
 *
 * @return level that 1 ~ 4, 1 means the slightness, 4 means the seriousness (over 1 day)
 */
int get_awareness_level();

/**
 * get the level of awareness according to elapsed time
 *
 * @param[in] diff time difference for elapsed time after the last stretching
 * @return level that 1 ~ 4, 1 means the slightness, 4 means the seriousness (over 1 day)
 */
int get_awareness_level_from_data(double diff);

/**
 * store the time as the last in the file
 *
 * @param[in] timestamp time to store in file
 * @return true if file writing was succeeded
 */
bool store_last_time(time_t timestamp, LOG_TYPE type, int achieve_count, StretchType stt, double recog_rate,
					 const float *const data, int data_cnt);

/**
 * store the current time as the last in the file
 *
 * @return true if file writing was succeeded
 */
bool store_last_time_with_current(LOG_TYPE type, int achieve_count, StretchType stt, double recog_rate,
								  const float *const data, int data_cnt);


/**
 * get the number of stretching from stored file with type
 *
 * @param[in] type LOG_TYPE
 * @return counts
 */
int get_counts_in_today(LOG_TYPE type);

/*
 * check how many times to success for achievement of daily goal in a week
 * (from monday to sunday)
 */
int get_daily_goal_count_in_week();

/**
 * get experiment type
 * This function checks the experiment type via file config ONCE.
 *
 * @return Experiment_Type
 */
Experiment_Type get_experiment_type();


#ifdef __cplusplus
}
#endif


#endif // __SM_DATA_H__ //


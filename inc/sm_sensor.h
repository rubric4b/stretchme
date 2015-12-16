#ifndef __SM_SENSOR_H__
#define __SM_SENSOR_H__

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <stdlib.h>
#include <sensor.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	unsigned int timestamp; // id

	glm::vec3 acc;
	bool acc_updated;
	glm::vec3 gyro;
	bool gyro_updated;

	glm::quat qDeviceOrientation; // Quaternion of device orientation

	glm::vec3 vel;
	glm::vec3 pos;

	std::vector<glm::vec3> linearAcc;

}SensorIntegration;

typedef struct
{
	sensor_type_e type;
	sensor_h handle;
	sensor_listener_h listener;

	float value_min;		/**< Minimal value */
	float value_max;		/**< Maximal value */
	float value_range;		/**< Values range */

}sensor_info;

typedef void (*Sensor_Cb)(void *data);
sensor_info* sensor_init(sensor_type_e sensor_type);
void sensor_start(sensor_info* sensor);
void sensor_stop(sensor_info* sensor);
void sensor_deinit(sensor_info* sensor);
void sensor_listen_pause(sensor_info* sensor);
void sensor_listen_resume(sensor_info* sensor);
void reset_measure();
const SensorIntegration & get_current_sensor_data();

/**
 * register ONE callback function which called when the sensor event comes
 */
void sensor_callback_register(Sensor_Cb func, void* data);


#ifdef __cplusplus
}
#endif

#endif // __SM_SENSOR_H__ //

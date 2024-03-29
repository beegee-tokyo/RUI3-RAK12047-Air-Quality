/**
 * @file RAK1901_temp.cpp
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Initialize and read data from SHTC3 sensor
 * @version 0.1
 * @date 2022-04-11
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "main.h"
#include <rak1901.h> 

/** Sensor instance */
rak1901 shtc3;

/**
 * @brief Initialize the temperature and humidity sensor
 *
 * @return true if initialization is ok
 * @return false if sensor could not be found
 */
bool init_rak1901(void)
{
	if (!shtc3.init())
	{
		// MYLOG("T_H", "Could not initialize SHTC3");
		return false;
	}
	return true;
}

/**
 * @brief Read the temperature and humidity values
 *     Data is added to Cayenne LPP payload as channel
 *     LPP_CHANNEL_HUMID, LPP_CHANNEL_TEMP
 *
 */
void read_rak1901(void)
{
	// MYLOG("T_H", "Reading SHTC3");
	shtc3.update();

	// float temp_f = shtc3.temperature();
	// float humid_f = shtc3.humidity();

	// MYLOG("T_H", "T: %.2f H: %.2f", temp_f, humid_f);

	g_solution_data.addRelativeHumidity(LPP_CHANNEL_HUMID, shtc3.humidity());
	g_solution_data.addTemperature(LPP_CHANNEL_TEMP, shtc3.temperature());
}

/**
 * @brief Returns the latest values from the sensor
 *        or starts a new reading
 *
 * @param values array for temperature [0] and humidity [1]
 */
void get_rak1901_values(float *values)
{
	shtc3.update();
	values[0] = shtc3.temperature();
	values[1] = shtc3.humidity();
	return;
}

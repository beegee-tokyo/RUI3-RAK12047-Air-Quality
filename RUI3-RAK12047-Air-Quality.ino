/**
 * @file WisBlock-Sensor-Node.ino
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief RUI3 based code for easy testing of WisBlock I2C modules
 * @version 0.1
 * @date 2022-04-10
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "main.h"

/** Initialization results */
bool ret;

/** LoRaWAN packet */
WisCayenne g_solution_data(255);

/** Set the device name, max length is 10 characters */
char g_dev_name[64] = "RUI3 Air Quality";

/** Fport to be used to send data */
uint8_t g_fport = 2;

// Send frequency, default is off
uint32_t g_send_repeat_time = 0;

// Flag to enable confirmed messages
bool confirmed_msg_enabled = false;

/**
 * @brief Callback after packet was received
 *
 * @param data Structure with the received data
 */
void receiveCallback(SERVICE_LORA_RECEIVE_T *data)
{
	MYLOG("RX-CB", "RX, fP %d, DR %d, RSSI %d, SNR %d", data->Port, data->RxDatarate, data->Rssi, data->Snr);
#if MY_DEBUG > 0
	for (int i = 0; i < data->BufferSize; i++)
	{
		Serial.printf("%02X", data->Buffer[i]);
	}
	Serial.print("\r\n");
#endif
}

/**
 * @brief Callback after TX is finished
 *
 * @param status TX status
 */
void sendCallback(int32_t status)
{
	MYLOG("TX-CB", "TX %d", status);
	digitalWrite(LED_BLUE, LOW);
}

/**
 * @brief Callback after join request cycle
 *
 * @param status Join result
 */
void joinCallback(int32_t status)
{
	if (status != 0)
	{
		if (!(ret = api.lorawan.join()))
		{
			MYLOG("J-CB", "Fail! \r\n");
		}
	}
	else
	{
		// MYLOG("J-CB", "Joined\r\n");
		// We need at least DR3 for the packet size
		api.lorawan.dr.set(3);
		// MYLOG("J-CB", "DR3 %s", api.lorawan.dr.set(3) ? "OK" : "NOK");
		digitalWrite(LED_BLUE, LOW);
		if (g_send_repeat_time != 0)
		{
			// Start a unified C timer
			api.system.timer.start(RAK_TIMER_0, g_send_repeat_time, NULL);
		}
	}
}

/**
 * @brief Arduino setup, called once after reboot/power-up
 *
 */
void setup()
{
	// Setup the callbacks for joined and send finished
	api.lorawan.registerRecvCallback(receiveCallback);
	api.lorawan.registerSendCallback(sendCallback);
	api.lorawan.registerJoinCallback(joinCallback);

	pinMode(LED_GREEN, OUTPUT);
	digitalWrite(LED_GREEN, HIGH);
	pinMode(LED_BLUE, OUTPUT);
	digitalWrite(LED_BLUE, HIGH);

	pinMode(WB_IO2, OUTPUT);
	digitalWrite(WB_IO2, HIGH);

	// Use RAK_CUSTOM_MODE supresses AT command and default responses from RUI3
	// Serial.begin(115200, RAK_CUSTOM_MODE);
	// Use "normal" mode to have AT commands available
	Serial.begin(115200);

#ifdef _VARIANT_RAK4630_
	time_t serial_timeout = millis();
	// On nRF52840 the USB serial is not available immediately
	while (!Serial.available())
	{
		if ((millis() - serial_timeout) < 5000)
		{
			delay(100);
			digitalWrite(LED_GREEN, !digitalRead(LED_GREEN));
		}
		else
		{
			break;
		}
	}
#else
	// For RAK3172 just wait a little bit for the USB to be ready
	delay(5000);
#endif

	// Find WisBlock I2C modules
	find_modules();

	MYLOG("SET", "RAKwireless %s Node", g_dev_name);

	MYLOG("SET", "Setup your device with AT commands");

	digitalWrite(LED_GREEN, LOW);

	init_custom_at();
	get_at_setting(SEND_FREQ_OFFSET);

	// Create a unified timer
	api.system.timer.create(RAK_TIMER_0, sensor_handler, RAK_TIMER_PERIODIC);

	// Get the confirmed mode settings
	confirmed_msg_enabled = api.lorawan.cfm.get();
	MYLOG("SET", "Confirmed message is %s", api.lorawan.cfm.get() == 0 ? "off" : "on");
	// Show found modules
	announce_modules();
	if (!found_sensors[PM_ID].found_sensor && !found_sensors[VOC_ID].found_sensor)
	{
		// Switch off sensors
		digitalWrite(WB_IO2, LOW);
	}
}

/**
 * @brief sensor_handler is a timer function called every
 * g_send_repeat_time milliseconds. Default is 120000. Can be
 * changed with a custom AT command ATC+SENDFREQ
 *
 */
void sensor_handler(void *)
{
	if (!found_sensors[PM_ID].found_sensor && !found_sensors[VOC_ID].found_sensor)
	{
		// Switch on Sensors
		digitalWrite(WB_IO2, HIGH);
		delay(100);
	}
	// MYLOG("SENS", "Start");
	digitalWrite(LED_BLUE, HIGH);

	// Check if the node has joined the network
	if (!api.lorawan.njs.get())
	{
		// MYLOG("UPL", "Not joined, skip sending");
		return;
	}

	// Clear payload
	g_solution_data.reset();

	// Read sensor data
	get_sensor_values();

	// Add battery voltage
	g_solution_data.addVoltage(LPP_CHANNEL_BATT, api.system.bat.get());

	if (!found_sensors[PM_ID].found_sensor && !found_sensors[VOC_ID].found_sensor)
	{
		// Switch sensors off
		digitalWrite(WB_IO2, LOW);
	}
	MYLOG("UPL", "Packet size = %d", g_solution_data.getSize());
	// Send the packet
	if (api.lorawan.send(g_solution_data.getSize(), g_solution_data.getBuffer(), g_fport, confirmed_msg_enabled))
	{
		MYLOG("UPL", "Enqueued");
	}
	else
	{
		MYLOG("UPL", "Send fail");
	}
}

/**
 * @brief This example is complete timer
 * driven. The loop() does nothing than
 * sleep.
 *
 */
void loop()
{
	api.system.sleep.all();
}